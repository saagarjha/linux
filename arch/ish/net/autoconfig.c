#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/route.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/netlink.h>
#include <net/sock.h>
#include <net/route.h>

int __init golp_autoconfig(void)
{
	struct net *net = &init_net;
	struct net_device *dev;
	struct net_device *golp = NULL;
	int golp_index;
	int err;

	rtnl_lock();

	/* Bring up interfaces */
	for_each_netdev(net, dev) {
		if (strcmp(dev->name, "golp") == 0)
			golp = dev;
		else if (!(dev->flags & IFF_LOOPBACK))
			continue;
		if (dev_change_flags(dev, dev->flags | IFF_UP, NULL) < 0)
			pr_warn("autoconfig: failed to bring up %s", dev->name);
	}

	if (golp)
		golp_index = golp->ifindex;

	rtnl_unlock(); /* aaaaa */

	if (golp) {
		struct sk_buff *skb;
		struct ifaddrmsg *ifm;
		struct nlmsghdr *nlh;
		struct rtentry route = {};
		struct sockaddr_in *sin;

		/* devinet_ioctl */
		skb = alloc_skb(NLMSG_GOODSIZE, GFP_KERNEL);
		nlh = nlmsg_put(skb, 0, 0, RTM_NEWADDR, sizeof(*ifm), NLM_F_REQUEST);
		ifm = nlmsg_data(nlh);
		ifm->ifa_index = golp_index;
		ifm->ifa_family = AF_INET;
		ifm->ifa_prefixlen = 0;
		nla_put_in_addr(skb, IFA_LOCAL, 0x0100000a); /* 10.0.0.1 */
		nlmsg_end(skb, nlh);

		netlink_unicast(net->rtnl, skb, 0, 0);

		sin = (struct sockaddr_in *) &route.rt_gateway;
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = INADDR_ANY;
		sin = (struct sockaddr_in *) &route.rt_dst;
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = INADDR_ANY;
		sin = (struct sockaddr_in *) &route.rt_genmask;
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = INADDR_ANY;
		route.rt_flags = RTF_UP;
		route.rt_dev = "golp";
		err = ip_rt_ioctl(net, SIOCADDRT, &route);
		if (err < 0)
			pr_warn("failed to bring golp up: %d\n", err);
	}

	return 0;
}

late_initcall(golp_autoconfig);
