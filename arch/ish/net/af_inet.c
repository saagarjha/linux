#include <linux/init.h>
#include <linux/irqreturn.h>
#include <linux/net.h>
#include <linux/printk.h>
#include <linux/uio.h>
#include <net/sock.h>
#include <user/fs.h>

struct host_sock {
	struct sock sock;
	int fd;
};

static struct host_sock *host_sk(struct sock *sk)
{
	return (struct host_sock *) sk;
}

/* turn an iov_iter into an iovec array */

struct uiovec {
	struct user_iovec *vec;
	size_t len;
	size_t cap;
	struct page **pages;
	size_t nr_pages;
};
static void uiovec_grow(struct uiovec *uio)
{
	if (uio->len >= uio->cap) {
		if (uio->cap == 0)
			uio->cap = 8;
		else
			uio->cap *= 2;
		/* TODO check for failure */
		uio->vec = krealloc(uio->vec, uio->cap * sizeof(struct kvec), GFP_KERNEL);
	}
}
static int iterate_iov_to_uiovec(struct kvec *iov, void *array)
{
	struct uiovec *uio = array;
	uiovec_grow(uio);
	if (uio->vec == NULL)
		return -ENOMEM;
	uio->vec[uio->len].base = iov->iov_base;
	uio->vec[uio->len].len = iov->iov_len;
	uio->len++;
	return 0;
}
static int uiovec_from_iov_iter(struct iov_iter *iter, size_t n, struct uiovec *uio)
{
	int err;
	struct page **pages;
	size_t offset;
	ssize_t bytes;
	int npages;
	ssize_t i;

	uio->cap = 0;
	uio->len = 0;
	uio->vec = NULL;
	uio->nr_pages = 0;
	err = iov_iter_for_each_range(iter, n, iterate_iov_to_uiovec, uio);
	if (!(err == -EINVAL && uio->vec == NULL)) {
		return err;
	}

	/* returns a bunch of pages as pages, offset into the first page as start, and total byte length */
	bytes = iov_iter_get_pages_alloc(iter, &pages, n, &offset);
	if (bytes < 0)
		return bytes;
	if (bytes == 0)
		return -ENOMEM;

	npages = DIV_ROUND_UP(bytes + offset, PAGE_SIZE);
	for (i = 0; i < npages; i++) {
		size_t len;

		uiovec_grow(uio);
		len = min_t(size_t, PAGE_SIZE - offset, bytes);
		uio->vec[uio->len].base = kmap(pages[i]) + offset;
		uio->vec[uio->len].len = len;
		uio->len++;
		offset = 0;
		bytes -= len;
	}

	uio->pages = pages;
	uio->nr_pages = npages;
	return 0;
}
static void uiovec_free(struct uiovec *uio)
{
	size_t i;

	kfree(uio->vec);
	for (i = 0; i < uio->nr_pages; i++) {
		put_page(uio->pages[i]);
	}
}

/* util */

static int check_sockaddr(struct sock *sk, void *addr, size_t addr_len)
{
	DECLARE_SOCKADDR(struct sockaddr_in *, sin, addr);

	if (addr == NULL)
		return 0;

	switch (sk->sk_family) {
	case AF_INET:
		if (addr_len < sizeof(*sin))
			return -EINVAL;
		if (sin->sin_family != AF_INET)
			return -EAFNOSUPPORT;
		break;
	default:
		return -EAFNOSUPPORT;
	}
	return 0;
}

/* ping */

static int ping_init(struct socket *sock)
{
	struct host_sock *host = host_sk(sock->sk);
	int err;
	host->fd = host_socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
	if (host->fd < 0)
		return host->fd;
	err = fd_set_nonblock(host->fd);
	if (err < 0)
		goto err_close;
	err = fd_add_irq(host->fd, POLLIN|POLLOUT|EPOLLET, HOST_INET_IRQ, sock->sk);
	if (err < 0)
		goto err_close;
	return 0;

err_close:
	host_close(host->fd);
	return err;
}

static int ping_release(struct socket *sock)
{
	struct host_sock *host = host_sk(sock->sk);
	host_close(host->fd); /* hopefully this unregisters the fd with the epoll */
	return 0;
}
static int ping_sendmsg(struct socket *sock, struct msghdr *m, size_t total_len)
{
	struct host_sock *host = host_sk(sock->sk);
	struct uiovec uio;
	int err;

	err = check_sockaddr(sock->sk, m->msg_name, m->msg_namelen);
	if (err < 0)
		return err;
	if (m->msg_control || m->msg_controllen) {
		printk("oh no a control message\n");
		return -EINVAL; // TODO
	}
	err = uiovec_from_iov_iter(&m->msg_iter, total_len, &uio);
	if (err < 0)
		return err;
	err = host_sendmsg(host->fd, uio.vec, uio.len, m->msg_name, m->msg_namelen, m->msg_flags);
	uiovec_free(&uio);
	return err;
}

static int ping_recvmsg(struct sock *sk, struct msghdr *m, size_t total_len, int noblock, int flags, int *addr_len)
{
	struct host_sock *host = host_sk(sk);
	struct uiovec uio;
	int err;
	long timeo;
	DEFINE_WAIT(wait);

	flags &= ~MSG_CMSG_COMPAT; /* doesn't matter for inet messages */

	err = uiovec_from_iov_iter(&m->msg_iter, total_len, &uio);
	if (err < 0)
		return err;
	timeo = sock_rcvtimeo(sk, noblock);
	for (;;) {
		BUG_ON(!rcu_read_lock_held());
		prepare_to_wait(sk_sleep(sk), &wait, TASK_INTERRUPTIBLE);
		err = host_recvmsg(host->fd, uio.vec, uio.len, m->msg_name, &m->msg_namelen, &m->msg_flags, flags);
		if (err != -EAGAIN)
			break;
		if (!timeo) {
			err = -ETIMEDOUT;
			break;
		}
		if (signal_pending(current)) {
			err = -ERESTARTSYS;
			break;
		}
		timeo = schedule_timeout(timeo);
	}
	finish_wait(sk_sleep(sk), &wait);
	uiovec_free(&uio);
	return err;
}

static __poll_t ping_poll(struct file *filp, struct socket *sock, struct poll_table_struct *wait)
{
	struct host_sock *host = host_sk(sock->sk);
	int f;

	sock_poll_wait(filp, sock, wait);
	f = fd_poll(host->fd);
	printk("ping poll %x\n", f);
	return f;
}

static struct proto_ops ping_ops = {
	.family = PF_INET,
	.owner = THIS_MODULE,
	.release = ping_release,
	.sendmsg = ping_sendmsg,
	.recvmsg = sock_common_recvmsg,
	.poll = ping_poll,
};
static struct proto ping_proto = {
	.name = "PING",
	.owner = THIS_MODULE,
	.obj_size = sizeof(struct host_sock),
	.recvmsg = ping_recvmsg,
};

static irqreturn_t host_inet_irq(int irq, void *dev_id)
{
	struct sock *sk = current_irq_data();
	rcu_read_lock();
	BUG_ON(!rcu_read_lock_held());
	wake_up_interruptible_all(sk_sleep(sk));
	rcu_read_unlock();
	return IRQ_HANDLED;
}

static int host_inet_create(struct net *net, struct socket *sock, int protocol, int kern)
{
	struct proto *proto = NULL;
	struct sock *sk;
	int err;

	sock->state = SS_UNCONNECTED;
	if ((sock->type == SOCK_DGRAM || sock->type == SOCK_RAW) && protocol == IPPROTO_ICMP) {
		sock->ops = &ping_ops;
		proto = &ping_proto;
	} else {
		printk("fuknope %d %d\n", sock->type, protocol);
		return -EINVAL;
	}

	sk = sk_alloc(net, PF_INET, GFP_KERNEL, proto, kern);
	if (!sk)
		return -ENOBUFS;
	sock_init_data(sock, sk);

	err = ping_init(sock);
	if (err < 0) {
		sk_common_release(sk);
		return err;
	}

	return 0;
}

static struct net_proto_family host_inet_family = {
	.family = AF_INET,
	.create = host_inet_create,
};

static int __init host_inet_init(void)
{
	int err = request_irq(HOST_INET_IRQ, host_inet_irq, 0, "host-inet", NULL);
	if (err < 0) {
		pr_err("host-inet: failed request_irq: %d\n", err);
		return err;
	}
	err = sock_register(&host_inet_family);
	if (err < 0) {
		pr_err("host-inet: failed sock_register: %d\n", err);
		return err;
	}
	return 0;
}
fs_initcall(host_inet_init);
