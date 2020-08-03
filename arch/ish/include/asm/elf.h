#ifndef __ASM_ISH_ELF_H
#define __ASM_ISH_ELF_H

/* TODO: don't hard code ABIs like this? it sucks? seriously this entire file sucks */

#define ELF_PLATFORM "i686"

typedef u32 elf_greg_t;
/* #define ELF_NGREG (sizeof(struct user_regs_struct) / sizeof(elf_greg_t)) */
#define ELF_NGREG 16
typedef elf_greg_t elf_gregset_t[ELF_NGREG];
/* typedef struct user_i387_struct elf_fpregset_t; */
#define ELF_NFPREG 27
typedef elf_greg_t elf_fpregset_t[ELF_NFPREG];

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(x) \
	(((x)->e_machine == EM_386) || ((x)->e_machine == EM_486))

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS	ELFCLASS32
#define ELF_DATA	ELFDATA2LSB
#define ELF_ARCH	EM_386

#define ELF_EXEC_PAGESIZE 4096

#define ELF_ET_DYN_BASE		(TASK_SIZE / 3 * 2)
#define ELF_HWCAP		(0) // TODO

#define ARCH_HAS_SETUP_ADDITIONAL_PAGES
struct linux_binprm;
int arch_setup_additional_pages(struct linux_binprm *, int);

#endif
