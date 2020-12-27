#include <linux/syscalls.h>
#include <asm/ldt.h>
#include <asm/uaccess.h>

int do_set_thread_area(struct task_struct *task, struct user_desc __user *u_info)
{
	struct user_desc info;
	
	if (copy_from_user(&info, u_info, sizeof(info)))
		return -EFAULT;

	/*
	 * On a real system, TLS works by creating a special segment pointing to
	 * the TLS buffer. Our shitty emulation of that is to ignore attempts to
	 * modify GS and add this address to any memory reference that uses GS.
	 */
	task_pt_regs(task)->tls = info.base_addr;

	if (info.entry_number == -1) {
		info.entry_number = 0xc;
		if (put_user(info.entry_number, &u_info->entry_number))
			return -EFAULT;
	}

	return 0;
}

SYSCALL_DEFINE1(set_thread_area, struct user_desc __user *, u_info)
{
	return do_set_thread_area(current, u_info);
}
