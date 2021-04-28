#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/phy_hotplug.h>
#include <uapi/linux/if.h>

#ifdef	CONFIG_NET
static const char *phy_actions[] = {
	"linkdown", "linkup",
};

struct bh_event {
	char ifname[IFNAMSIZ];
	const char	*action;

	struct sk_buff		*skb;
	struct work_struct	work;
};

#define BH_SKB_SIZE	256
extern u64 uevent_next_seqnum(void);

static int bh_event_add_var(struct bh_event *event, int argv,
		const char *format, ...)
{
	static char buf[128];
	char *s;
	va_list args;
	int len;

	if (argv)
		return 0;

	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	if (len >= sizeof(buf)) {
		WARN_ON(1);
		return -ENOMEM;
	}

	s = skb_put(event->skb, len + 1);
	strcpy(s, buf);

	return 0;
}

static int phy_hotplug_fill_event(struct bh_event *event)
{
	int ret;

	ret = bh_event_add_var(event, 0, "HOME=%s", "/");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "PATH=%s",
					"/sbin:/bin:/usr/sbin:/usr/bin");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "SUBSYSTEM=%s", "phy");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "ACTION=%s", event->action);
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "IFNAME=%s", event->ifname);
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "SEQNUM=%llu", uevent_next_seqnum());

	return ret;
}

static void phy_hotplug_work(struct work_struct *work)
{
	struct bh_event *event = container_of(work, struct bh_event, work);
	int ret = 0;

	event->skb = alloc_skb(BH_SKB_SIZE, GFP_KERNEL);
	if (!event->skb)
		goto out_free_event;

	ret = bh_event_add_var(event, 0, "%s@", event->action);
	if (ret)
		goto out_free_skb;

	ret = phy_hotplug_fill_event(event);
	if (ret)
		goto out_free_skb;

	printk(KERN_NOTICE "phy: %s %s\n", event->ifname, event->action);

	NETLINK_CB(event->skb).dst_group = 1;
	broadcast_uevent(event->skb, 0, 1, GFP_KERNEL);

 out_free_skb:
	if (ret) {
		kfree_skb(event->skb);
	}
 out_free_event:
	kfree(event);
}

static int phy_hotplug_create_event(const char *ifname, const u32 status)
{
	struct bh_event *event;
	u32 port_status = status & 0x1;

	if (!ifname)
		return -1;

	event = kzalloc(sizeof(*event), GFP_ATOMIC);
	if (!event)
		return -ENOMEM;

	strncpy(event->ifname, ifname, IFNAMSIZ -1);
	event->action = phy_actions[port_status];

	INIT_WORK(&event->work, (void *)(void *)phy_hotplug_work);
	schedule_work(&event->work);

	return 0;
}


void phy_hotplug_event(const char *ifname, const u32 status)
{
	phy_hotplug_create_event(ifname, status);
}
#else
void phy_hotplug_event(const char *ifname, const u32 status)
{
	char *port_status;

	if (status) {
		port_status = "linkup";
	} else {
		port_status = "linkdown";
	}

	if (ifname)
		printk(KERN_NOTICE "phy: %s %s\n", ifname, port_status);
}
#endif /* CONFIG_NET */
EXPORT_SYMBOL(phy_hotplug_event);
