#include <linux/in.h>
#include <net/ip.h>
#include <user/fs.h>
#include <asm/checksum.h>
#include "golp.h"

static u32 __udp_hash(struct net_device *dev, __be32 address, unsigned short port)
{
	return ipv4_portaddr_hash(dev_net(dev), address, port);
}

static struct golp_socket *__udp_socket_lookup(struct net_device *dev,
					       __be32 address,
					       unsigned short port)
{
	struct golp_priv *priv = netdev_priv(dev);
	struct golp_socket *sock;
	hash_for_each_possible(priv->udp_hash, sock, link,
			       __udp_hash(dev, address, port)) {
		if (sock->addr == address && sock->port == port)
			return sock;
	}
	return NULL;
}

static int open_udp_socket(__be32 address, unsigned short port)
{
	struct sockaddr_in sin = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = address,
		.sin_port = htons(port),
	};
	int sock, err;
	sock = host_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0)
		return sock;
	err = host_set_so_reuseport(sock, 1);
	if (err < 0)
		return err;
	err = host_bind(sock, &sin, sizeof(sin));
	if (err < 0)
		return err;
	return sock;
}

static int get_local_addr_for_route(__be32 remote, __be32 *local)
{
	static DEFINE_PER_CPU(int, sock_var) = -1;
	int *sock_ptr;
	int sock;
	struct sockaddr_in sin = {.sin_family = AF_INET, .sin_port = 1};
	int err;

	sock_ptr = &get_cpu_var(sock_var);
	if (*sock_ptr < 0) {
		*sock_ptr = host_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (*sock_ptr < 0)
			panic("couldn't create routing test socket: %pe", ERR_PTR(*sock_ptr));
	}
	sock = *sock_ptr;
	put_cpu_var(sock_var);

	sin.sin_addr.s_addr = remote;
	err = host_connect(sock, &sin, sizeof(sin));
	if (err < 0)
		return err;
	err = host_getname(sock, &sin, sizeof(sin), GETNAME_SOCK);
	if (err < 0)
		return err;
	*local = sin.sin_addr.s_addr;
	return 0;
}

int golp_udp4_ip_listen(struct net_device *dev, __be32 address, unsigned short port)
{
	struct golp_priv *priv = netdev_priv(dev);
	struct golp_socket *sock;

	spin_lock(&priv->sockets_lock);
	sock = __udp_socket_lookup(dev, address, port);
	if (sock) {
		golp_socket_hold(sock);
		goto out;
	}

	sock = golp_socket_alloc(dev, IPPROTO_UDP);
	if (sock == NULL) {
		printk("golp: failed to kmalloc for port %d\n", port);
		goto fail;
	}
	sock->fd = open_udp_socket(address, port);
	if (sock->fd < 0) {
		printk("golp: failed to reserve port %d: %pe\n", port, ERR_PTR(sock->fd));
		kfree(sock);
		goto fail;
	}
	sock->addr = address;
	sock->port = port;
	fd_listen(sock->fd, POLLIN, &sock->listener);
	hash_add(priv->udp_hash, &sock->link, __udp_hash(dev, address, port));
out:
	spin_unlock(&priv->sockets_lock);
	return 0;

fail:
	spin_unlock(&priv->sockets_lock);
	return 1;
}

void golp_udp4_ip_unlisten(struct net_device *dev, __be32 address, unsigned short port)
{
	struct golp_priv *priv = netdev_priv(dev);
	struct golp_socket *sock;
	
	spin_lock(&priv->sockets_lock);
	sock = __udp_socket_lookup(dev, address, port);
	BUG_ON(sock == NULL);
	/* TODO: this sucks */
	if (refcount_dec_and_test(&sock->refcount)) {
		hash_del(&sock->link);
		__golp_socket_destroy(sock);
	}
	spin_unlock(&priv->sockets_lock);
}

void golp_udp4_ip_listen_newaddr(struct net_device *dev, __be32 old_addr, __be32 new_addr, unsigned short port)
{
	struct golp_priv *priv = netdev_priv(dev);
	struct golp_socket *sock;
	
	spin_lock(&priv->sockets_lock);
	sock = __udp_socket_lookup(dev, old_addr, port);
	BUG_ON(sock == NULL);
	hash_del(&sock->link);
	sock->addr = new_addr;
	hash_add(priv->udp_hash, &sock->link, __udp_hash(dev, sock->addr, port));
	spin_unlock(&priv->sockets_lock);
}

void golp_udp4_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct golp_priv *priv = netdev_priv(dev);
	struct iphdr *ip = ip_hdr(skb);
	struct udphdr *udp = udp_hdr(skb);
	struct golp_socket *sock;
	struct sockaddr_in addr;
	struct user_iovec data;
	int err;

	if (ip_transport_len(skb) < sizeof(*udp))
		return golp_drop_tx(dev, skb, "udp packet truncated before header");
	if (ip_transport_len(skb) < ntohs(udp->len))
		return golp_drop_tx(dev, skb, "udp packet truncated after header");

	spin_lock(&priv->sockets_lock);
	sock = __udp_socket_lookup(dev, ip->saddr, ntohs(udp->source));
	if (!sock)
		sock = __udp_socket_lookup(dev, INADDR_ANY, ntohs(udp->source));
	if (sock)
		golp_socket_hold(sock);
	spin_unlock(&priv->sockets_lock);
	if (sock == NULL)
		return golp_drop_tx(dev, skb, "no socket available to send from %pI4:%d", &ip->saddr, ntohs(udp->source));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip->daddr;
	addr.sin_port = udp->dest;
	data.base = udp + 1;
	data.len = ntohs(udp->len);
	err = host_sendmsg(sock->fd, &data, 1, &addr, sizeof(addr), 0);
	golp_socket_put(sock);
	if (err < 0)
		return golp_drop_tx(dev, skb, "udp sendmsg failed: %pe", ERR_PTR(err));
	skb->dev->stats.tx_packets++;
	skb->dev->stats.tx_bytes += data.len;
}

void golp_udp4_rx(struct golp_socket *sock, struct net_device *dev)
{
	struct user_iovec iov = {
		.base = __golp_irq_buf,
		.len = sizeof(__golp_irq_buf),
	};
	for (;;) {
		struct sk_buff *skb;
		struct sockaddr_storage addr;
		DECLARE_SOCKADDR(struct sockaddr_in *, sin, &addr);
		int addr_len;
		int rcv_flags;
		ssize_t rcv_len;
		struct iphdr *ip;
		struct udphdr *udp;

		rcv_len = host_recvmsg(sock->fd, &iov, 1, &addr, &addr_len, &rcv_flags, MSG_DONTWAIT);
		if (rcv_len == -EAGAIN)
			break;
		if (rcv_len < 0) {
			golp_drop_rx(dev, "udp read failed: %pe", ERR_PTR(rcv_len));
			continue;
		}

		skb = netdev_alloc_skb(dev, rcv_len);
		if (skb == NULL) {
			golp_drop_rx(dev, "no memory for skb");
			continue;
		}
		udp = skb_push(skb, sizeof(*udp));
		udp->len = htons(sizeof(*udp) + rcv_len);
		udp->dest = htons(sock->port);
		udp->source = sin->sin_port;
		ip = skb_push(skb, sizeof(*ip));
		ip->protocol = IPPROTO_UDP;
		ip->saddr = sin->sin_addr.s_addr;
		ip->daddr = sock->addr;
		if (ip->daddr == INADDR_ANY) {
			int err = get_local_addr_for_route(ip->saddr, &ip->daddr);
			if (err < 0)
				panic("failed to route %pI4: %pe", &ip->saddr, ERR_PTR(err));
		}
		memcpy(skb_put(skb, rcv_len), __golp_irq_buf, rcv_len);
		golp_rx(skb);
	}
}
