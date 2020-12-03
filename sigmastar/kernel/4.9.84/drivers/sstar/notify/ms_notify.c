/*
* ms_notify.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>

#define MAX_PAYLOAD     256

static struct sock *_st_notify_sock;

static void _notify_reply(struct sk_buff * __skb)
{
	struct nlmsghdr *nlh;
	struct sk_buff *skb;

	unsigned char msg[MAX_PAYLOAD] = {0};

	//int res;

	do
	{
		skb = skb_get(__skb);

		if (skb->len < NLMSG_SPACE(0))
		{
			break;
		}

		nlh = nlmsg_hdr(skb);

	#if 0
        printk("recv skb from user space uid: %d pid: %d seq: %d\n", NETLINK_CREDS(skb)->uid, NETLINK_CREDS(skb)->pid, nlh->nlmsg_seq);
    #endif

		memcpy(msg, NLMSG_DATA(nlh), sizeof(msg));

		kfree_skb(skb);

		if (NULL == (skb = alloc_skb(NLMSG_SPACE(MAX_PAYLOAD), GFP_ATOMIC)))
		{
            break;
        }

		if (NULL == (nlh = nlmsg_put(skb, 0, 0, 0, MAX_PAYLOAD, 0)))
        {
            break;
        }

		nlh->nlmsg_flags = 0;
		memcpy(NLMSG_DATA(nlh), msg, MAX_PAYLOAD);
		NETLINK_CB(skb).portid = 0;
		NETLINK_CB(skb).dst_group = 1;
		netlink_broadcast(_st_notify_sock, skb, 0, 1, GFP_ATOMIC);
		//kfree_skb(skb);
		return;
	} while (0);

//nlmsg_failure:			/* Used by NLMSG_PUT */
	if (skb)
	{
		kfree_skb(skb);
	}

}

static int __init _notify_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.groups = 0,
		.input = _notify_reply,
	};

	if (NULL == (_st_notify_sock = netlink_kernel_create(&init_net, NETLINK_USERSOCK, &cfg)))
	{
		return -1;
	}

	printk("mstar notify driver install successfully\n");
	return 0;
}

static void __exit _notify_exit(void)
{

	netlink_kernel_release(_st_notify_sock);

	printk("mstar notify driver remove successfully\n");
}

module_init(_notify_init);
module_exit(_notify_exit);

MODULE_DESCRIPTION("notify reply server module");
MODULE_AUTHOR("SSTAR");
MODULE_LICENSE("GPL");

