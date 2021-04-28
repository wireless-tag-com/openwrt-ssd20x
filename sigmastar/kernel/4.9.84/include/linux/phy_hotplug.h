#ifndef __PHY_HOTPLUG_H
#define __PHY_HOTPLUG_H

#include <linux/types.h>

void phy_hotplug_event(const char *ifname, const u32 status);

#endif
