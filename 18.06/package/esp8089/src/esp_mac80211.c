/*
 * Copyright (c) 2011-2014 Espressif System.
 *
 *     MAC80211 support module
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/etherdevice.h>
#include <linux/workqueue.h>
#include <linux/nl80211.h>
#include <linux/ieee80211.h>
#include <linux/slab.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>
#include <linux/version.h>
#include <net/regulatory.h>
#include "esp_pub.h"
#include "esp_sip.h"
#include "esp_ctrl.h"
#include "esp_sif.h"
#include "esp_debug.h"
#include "esp_wl.h"
#include "esp_utils.h"

#define ESP_IEEE80211_DBG esp_dbg

#define GET_NEXT_SEQ(seq) (((seq) +1) & 0x0fff)

static u8 esp_mac_addr[ETH_ALEN * 2];
static u8 getaddr_index(u8 * addr, struct esp_pub *epub);

static
void
esp_op_tx(struct ieee80211_hw *hw, struct ieee80211_tx_control *control,
	  struct sk_buff *skb)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;

	ESP_IEEE80211_DBG(ESP_DBG_LOG, "%s enter\n", __func__);
	if (!mod_support_no_txampdu() &&
	    cfg80211_get_chandef_type(&epub->hw->conf.chandef) !=
	    NL80211_CHAN_NO_HT) {
		struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
		struct ieee80211_hdr *wh =
		    (struct ieee80211_hdr *) skb->data;
		if (ieee80211_is_data_qos(wh->frame_control)) {
			if (!(tx_info->flags & IEEE80211_TX_CTL_AMPDU)) {
				u8 tidno =
				    ieee80211_get_qos_ctl(wh)[0] &
				    IEEE80211_QOS_CTL_TID_MASK;
				struct esp_node *node =
				    esp_get_node_by_addr(epub, wh->addr1);
				{
					struct esp_tx_tid *tid =
					    &node->tid[tidno];
					//record ssn
					spin_lock_bh(&epub->tx_ampdu_lock);
					tid->ssn =
					    GET_NEXT_SEQ(le16_to_cpu
							 (wh->
							  seq_ctrl) >> 4);
					ESP_IEEE80211_DBG(ESP_DBG_TRACE,
							  "tidno:%u,ssn:%u\n",
							  tidno, tid->ssn);
					spin_unlock_bh(&epub->
						       tx_ampdu_lock);
				}
			} else {
				ESP_IEEE80211_DBG(ESP_DBG_TRACE,
						  "tx ampdu pkt, sn:%u, %u\n",
						  le16_to_cpu(wh->
							      seq_ctrl) >>
						  4, skb->len);
			}
		}
	}
#ifdef GEN_ERR_CHECKSUM
	esp_gen_err_checksum(skb);
#endif

	sip_tx_data_pkt_enqueue(epub, skb);
	if (epub)
		ieee80211_queue_work(hw, &epub->tx_work);
}

static int esp_op_start(struct ieee80211_hw *hw)
{
	struct esp_pub *epub;

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s\n", __func__);

	if (!hw) {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR, "%s no hw!\n", __func__);
		return -EINVAL;
	}

	epub = (struct esp_pub *) hw->priv;

	if (!epub) {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR, "%s no epub!\n",
				  __func__);
		return EINVAL;
	}
	/*add rfkill poll function */

	atomic_set(&epub->wl.off, 0);
	wiphy_rfkill_start_polling(hw->wiphy);
	return 0;
}

static void esp_op_stop(struct ieee80211_hw *hw)
{
	struct esp_pub *epub;

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s\n", __func__);

	if (!hw) {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR, "%s no hw!\n", __func__);
		return;
	}

	epub = (struct esp_pub *) hw->priv;

	if (!epub) {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR, "%s no epub!\n",
				  __func__);
		return;
	}

	atomic_set(&epub->wl.off, 1);

#ifdef HOST_RESET_BUG
	mdelay(200);
#endif

	if (epub->wl.scan_req) {
		hw_scan_done(epub, true);
		epub->wl.scan_req = NULL;
		//msleep(2);
	}
}

#ifdef CONFIG_PM
static int esp_op_suspend(struct ieee80211_hw *hw,
			  struct cfg80211_wowlan *wowlan)
{
	esp_dbg(ESP_DBG_OP, "%s\n", __func__);

	return 0;
}

static int esp_op_resume(struct ieee80211_hw *hw)
{
	esp_dbg(ESP_DBG_OP, "%s\n", __func__);

	return 0;
}
#endif				//CONFIG_PM

static int esp_op_add_interface(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;
	struct sip_cmd_setvif svif;

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s enter: type %d, addr %pM\n",
			  __func__, vif->type, vif->addr);

	memset(&svif, 0, sizeof(struct sip_cmd_setvif));
	memcpy(svif.mac, vif->addr, ETH_ALEN);
	evif->index = svif.index = getaddr_index(vif->addr, epub);
	evif->epub = epub;
	epub->vif = vif;
	svif.set = 1;
	if ((1 << svif.index) & epub->vif_slot) {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR,
				  "%s interface %d already used\n",
				  __func__, svif.index);
		return -EOPNOTSUPP;
	}
	epub->vif_slot |= 1 << svif.index;

	if (svif.index == ESP_PUB_MAX_VIF) {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR,
				  "%s only support MAX %d interface\n",
				  __func__, ESP_PUB_MAX_VIF);
		return -EOPNOTSUPP;
	}

	switch (vif->type) {
	case NL80211_IFTYPE_STATION:
		//if (svif.index == 1)
		//      vif->type = NL80211_IFTYPE_UNSPECIFIED;
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s STA \n", __func__);
		svif.op_mode = 0;
		svif.is_p2p = 0;
		break;
	case NL80211_IFTYPE_AP:
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s AP \n", __func__);
		svif.op_mode = 1;
		svif.is_p2p = 0;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s P2P_CLIENT \n", __func__);
		svif.op_mode = 0;
		svif.is_p2p = 1;
		break;
	case NL80211_IFTYPE_P2P_GO:
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s P2P_GO \n", __func__);
		svif.op_mode = 1;
		svif.is_p2p = 1;
		break;
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_WDS:
	case NL80211_IFTYPE_MONITOR:
	default:
		ESP_IEEE80211_DBG(ESP_DBG_ERROR,
				  "%s does NOT support type %d\n",
				  __func__, vif->type);
		return -EOPNOTSUPP;
	}

	sip_cmd(epub, SIP_CMD_SETVIF, (u8 *) & svif,
		sizeof(struct sip_cmd_setvif));
	return 0;
}

static int esp_op_change_interface(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   enum nl80211_iftype new_type, bool p2p)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;
	struct sip_cmd_setvif svif;
	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s enter,change to if:%d \n",
			  __func__, new_type);

	if (new_type == NL80211_IFTYPE_AP) {
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter,change to AP \n",
				  __func__);
	}

	if (vif->type != new_type) {
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s type from %d to %d\n",
				  __func__, vif->type, new_type);
	}

	memset(&svif, 0, sizeof(struct sip_cmd_setvif));
	memcpy(svif.mac, vif->addr, ETH_ALEN);
	svif.index = evif->index;
	svif.set = 2;

	switch (new_type) {
	case NL80211_IFTYPE_STATION:
		svif.op_mode = 0;
		svif.is_p2p = p2p;
		break;
	case NL80211_IFTYPE_AP:
		svif.op_mode = 1;
		svif.is_p2p = p2p;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		svif.op_mode = 0;
		svif.is_p2p = 1;
		break;
	case NL80211_IFTYPE_P2P_GO:
		svif.op_mode = 1;
		svif.is_p2p = 1;
		break;
	case NL80211_IFTYPE_UNSPECIFIED:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_WDS:
	case NL80211_IFTYPE_MONITOR:
	default:
		ESP_IEEE80211_DBG(ESP_DBG_ERROR,
				  "%s does NOT support type %d\n",
				  __func__, vif->type);
		return -EOPNOTSUPP;
	}
	sip_cmd(epub, SIP_CMD_SETVIF, (u8 *) & svif,
		sizeof(struct sip_cmd_setvif));
	return 0;
}

static void esp_op_remove_interface(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;
	struct sip_cmd_setvif svif;

	ESP_IEEE80211_DBG(ESP_DBG_OP,
			  "%s enter, vif addr %pM, beacon enable %x\n",
			  __func__, vif->addr,
			  vif->bss_conf.enable_beacon);

	memset(&svif, 0, sizeof(struct sip_cmd_setvif));
	svif.index = evif->index;
	epub->vif_slot &= ~(1 << svif.index);

	if (evif->ap_up) {
		evif->beacon_interval = 0;
		del_timer_sync(&evif->beacon_timer);
		evif->ap_up = false;
	}
	epub->vif = NULL;
	evif->epub = NULL;

	sip_cmd(epub, SIP_CMD_SETVIF, (u8 *) & svif,
		sizeof(struct sip_cmd_setvif));

	/* clean up tx/rx queue */

}

#define BEACON_TIM_SAVE_MAX 20
u8 beacon_tim_saved[BEACON_TIM_SAVE_MAX];
int beacon_tim_count;
static void beacon_tim_init(void)
{
	memset(beacon_tim_saved, 0, BEACON_TIM_SAVE_MAX);
	beacon_tim_count = 0;
}

static u8 beacon_tim_save(u8 this_tim)
{
	u8 all_tim = 0;
	int i;
	beacon_tim_saved[beacon_tim_count] = this_tim;
	if (++beacon_tim_count >= BEACON_TIM_SAVE_MAX)
		beacon_tim_count = 0;
	for (i = 0; i < BEACON_TIM_SAVE_MAX; i++)
		all_tim |= beacon_tim_saved[i];
	return all_tim;
}

static bool beacon_tim_alter(struct sk_buff *beacon)
{
	u8 *p, *tim_end;
	u8 tim_count;
	int len;
	int remain_len;
	struct ieee80211_mgmt *mgmt;

	if (beacon == NULL)
		return false;

	mgmt = (struct ieee80211_mgmt *) ((u8 *) beacon->data);

	remain_len =
	    beacon->len - ((u8 *) mgmt->u.beacon.variable - (u8 *) mgmt +
			   12);
	p = mgmt->u.beacon.variable;

	while (remain_len > 0) {
		len = *(++p);
		if (*p == WLAN_EID_TIM) {	// tim field
			tim_end = p + len;
			tim_count = *(++p);
			p += 2;
			//multicast
			if (tim_count == 0)
				*p |= 0x1;
			if ((*p & 0xfe) == 0 && tim_end >= p + 1) {	// we only support 8 sta in this case
				p++;
				*p = beacon_tim_save(*p);
			}
			return tim_count == 0;
		}
		p += (len + 1);
		remain_len -= (2 + len);
	}

	return false;
}

unsigned long init_jiffies;
unsigned long cycle_beacon_count;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static void drv_handle_beacon(struct timer_list *tl)
#else
static void drv_handle_beacon(unsigned long data)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
	unsigned long data = tl->flags;
#endif
	struct ieee80211_vif *vif = (struct ieee80211_vif *) data;
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;
	struct sk_buff *beacon;
	struct sk_buff *skb;
	static int dbgcnt = 0;
	bool tim_reach = false;

	if (evif->epub == NULL)
		return;

	mdelay(2400 * (cycle_beacon_count % 25) % 10000 / 1000);

	beacon = ieee80211_beacon_get(evif->epub->hw, vif);

	tim_reach = beacon_tim_alter(beacon);

	if (beacon && !(dbgcnt++ % 600)) {
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, " beacon length:%d,fc:0x%x\n",
				  beacon->len,
				  ((struct ieee80211_mgmt *) (beacon->
							      data))->
				  frame_control);

	}

	if (beacon)
		sip_tx_data_pkt_enqueue(evif->epub, beacon);

	if (cycle_beacon_count++ == 100) {
		init_jiffies = jiffies;
		cycle_beacon_count -= 100;
	}
	mod_timer(&evif->beacon_timer,
		  init_jiffies +
		  msecs_to_jiffies(cycle_beacon_count *
				   vif->bss_conf.beacon_int * 1024 /
				   1000));
	//FIXME:the packets must be sent at home channel
	//send buffer mcast frames
	if (tim_reach) {
		skb = ieee80211_get_buffered_bc(evif->epub->hw, vif);
		while (skb) {
			sip_tx_data_pkt_enqueue(evif->epub, skb);
			skb =
			    ieee80211_get_buffered_bc(evif->epub->hw, vif);
		}
	}
}

static void init_beacon_timer(struct ieee80211_vif *vif)
{
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;

	ESP_IEEE80211_DBG(ESP_DBG_OP, " %s enter: beacon interval %x\n",
			  __func__, evif->beacon_interval);

	beacon_tim_init();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
	timer_setup(&evif->beacon_timer, drv_handle_beacon, (unsigned long)vif); //TBD, not init here...
#else
	init_timer(&evif->beacon_timer);	//TBD, not init here...
	evif->beacon_timer.data = (unsigned long) vif;
	evif->beacon_timer.function = drv_handle_beacon;
#endif
	cycle_beacon_count = 1;
	init_jiffies = jiffies;
	evif->beacon_timer.expires =
	    init_jiffies +
	    msecs_to_jiffies(cycle_beacon_count *
			     vif->bss_conf.beacon_int * 1024 / 1000);
	add_timer(&evif->beacon_timer);
}

static int esp_op_config(struct ieee80211_hw *hw, u32 changed)
{
	//struct ieee80211_conf *conf = &hw->conf;

	struct esp_pub *epub = (struct esp_pub *) hw->priv;

	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter 0x%08x\n", __func__,
			  changed);

	if (changed &
	    (IEEE80211_CONF_CHANGE_CHANNEL | IEEE80211_CONF_CHANGE_IDLE)) {
		sip_send_config(epub, &hw->conf);
	}

	return 0;
}

static void esp_op_bss_info_changed(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif,
				    struct ieee80211_bss_conf *info,
				    u32 changed)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;

	// ieee80211_bss_conf(include/net/mac80211.h) is included in ieee80211_sub_if_data(net/mac80211/ieee80211_i.h) , does bssid=ieee80211_if_ap's ssid ?
	// in 2.6.27, ieee80211_sub_if_data has ieee80211_bss_conf while in 2.6.32 ieee80211_sub_if_data don't have ieee80211_bss_conf
	// in 2.6.27, ieee80211_bss_conf->enable_beacon don't exist, does it mean it support beacon always?
	// ESP_IEEE80211_DBG(ESP_DBG_OP, " %s enter: vif addr %pM, changed %x, assoc %x, bssid %pM\n", __func__, vif->addr, changed, info->assoc, info->bssid);
	// sdata->u.sta.bssid

	ESP_IEEE80211_DBG(ESP_DBG_OP,
			  " %s enter: changed %x, assoc %x, bssid %pM\n",
			  __func__, changed, info->assoc, info->bssid);

	if (vif->type == NL80211_IFTYPE_STATION) {
		if ((changed & BSS_CHANGED_BSSID) ||
		    ((changed & BSS_CHANGED_ASSOC) && (info->assoc))) {
			ESP_IEEE80211_DBG(ESP_DBG_TRACE,
					  " %s STA change bssid or assoc\n",
					  __func__);
			evif->beacon_interval = info->aid;
			memcpy(epub->wl.bssid, (u8 *) info->bssid,
			       ETH_ALEN);
			sip_send_bss_info_update(epub, evif,
						 (u8 *) info->bssid,
						 info->assoc);
		} else if ((changed & BSS_CHANGED_ASSOC) && (!info->assoc)) {
			ESP_IEEE80211_DBG(ESP_DBG_TRACE,
					  " %s STA change disassoc\n",
					  __func__);
			evif->beacon_interval = 0;
			memset(epub->wl.bssid, 0, ETH_ALEN);
			sip_send_bss_info_update(epub, evif,
						 (u8 *) info->bssid,
						 info->assoc);
		} else {
			ESP_IEEE80211_DBG(ESP_DBG_TRACE,
					  "%s wrong mode of STA mode\n",
					  __func__);
		}
	} else if (vif->type == NL80211_IFTYPE_AP) {
		if ((changed & BSS_CHANGED_BEACON_ENABLED) ||
		    (changed & BSS_CHANGED_BEACON_INT)) {
			ESP_IEEE80211_DBG(ESP_DBG_TRACE,
					  " %s AP change enable %d, interval is %d, bssid %pM\n",
					  __func__, info->enable_beacon,
					  info->beacon_int, info->bssid);
			if (info->enable_beacon && evif->ap_up != true) {
				evif->beacon_interval = info->beacon_int;
				init_beacon_timer(vif);
				sip_send_bss_info_update(epub, evif,
							 (u8 *) info->
							 bssid, 2);
				evif->ap_up = true;
			} else if (!info->enable_beacon && evif->ap_up &&
				!(hw->conf.flags & IEEE80211_CONF_OFFCHANNEL)
			    ) {
				ESP_IEEE80211_DBG(ESP_DBG_TRACE,
						  " %s AP disable beacon, interval is %d\n",
						  __func__,
						  info->beacon_int);
				evif->beacon_interval = 0;
				del_timer_sync(&evif->beacon_timer);
				sip_send_bss_info_update(epub, evif,
							 (u8 *) info->
							 bssid, 2);
				evif->ap_up = false;
			}
		}
	} else {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR,
				  "%s op mode unspecified\n", __func__);
	}
}


static u64 esp_op_prepare_multicast(struct ieee80211_hw *hw,
				    struct netdev_hw_addr_list *mc_list)
{
	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);

	return 0;
}

static void esp_op_configure_filter(struct ieee80211_hw *hw,
				    unsigned int changed_flags,
				    unsigned int *total_flags,
				    u64 multicast)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;

	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);

	epub->rx_filter = 0;

	if (*total_flags & FIF_ALLMULTI)
		epub->rx_filter |= FIF_ALLMULTI;

	*total_flags = epub->rx_filter;
}

static int esp_op_set_key(struct ieee80211_hw *hw, enum set_key_cmd cmd,
			  struct ieee80211_vif *vif,
			  struct ieee80211_sta *sta,
			  struct ieee80211_key_conf *key)
{
	u8 i;
	int ret;
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;
	u8 ifidx = evif->index;
	u8 *peer_addr, isvalid;

	ESP_IEEE80211_DBG(ESP_DBG_OP,
			  "%s enter, flags = %x keyindx = %x cmd = %x mac = %pM cipher = %x\n",
			  __func__, key->flags, key->keyidx, cmd,
			  vif->addr, key->cipher);

	key->flags = key->flags | IEEE80211_KEY_FLAG_GENERATE_IV;

	if (sta) {
		if (memcmp(sta->addr, epub->wl.bssid, ETH_ALEN))
			peer_addr = sta->addr;
		else
			peer_addr = epub->wl.bssid;
	} else {
		peer_addr = epub->wl.bssid;
	}
	isvalid = (cmd == SET_KEY) ? 1 : 0;

	if ((key->flags & IEEE80211_KEY_FLAG_PAIRWISE)
	    || (key->cipher == WLAN_CIPHER_SUITE_WEP40
		|| key->cipher == WLAN_CIPHER_SUITE_WEP104)) {
		if (isvalid) {
			for (i = 0; i < 19; i++) {
				if (epub->hi_map[i].flag == 0) {
					epub->hi_map[i].flag = 1;
					key->hw_key_idx = i + 6;
					memcpy(epub->hi_map[i].mac,
					       peer_addr, ETH_ALEN);
					break;
				}
			}
		} else {
			u8 index = key->hw_key_idx - 6;
			epub->hi_map[index].flag = 0;
			memset(epub->hi_map[index].mac, 0, ETH_ALEN);
		}
	} else {
		if (isvalid) {
			for (i = 0; i < 2; i++)
				if (epub->low_map[ifidx][i].flag == 0) {
					epub->low_map[ifidx][i].flag = 1;
					key->hw_key_idx =
					    i + ifidx * 2 + 2;
					memcpy(epub->low_map[ifidx][i].mac,
					       peer_addr, ETH_ALEN);
					break;
				}
		} else {
			u8 index = key->hw_key_idx - 2 - ifidx * 2;
			epub->low_map[ifidx][index].flag = 0;
			memset(epub->low_map[ifidx][index].mac, 0,
			       ETH_ALEN);
		}
		//key->hw_key_idx = key->keyidx + ifidx * 2 + 1;
	}

	if (key->hw_key_idx >= 6) {
		/*send sub_scan task to target */
		//epub->wl.ptk = (cmd==SET_KEY) ? key : NULL;
		if (isvalid)
			atomic_inc(&epub->wl.ptk_cnt);
		else
			atomic_dec(&epub->wl.ptk_cnt);
		if (key->cipher == WLAN_CIPHER_SUITE_WEP40
		    || key->cipher == WLAN_CIPHER_SUITE_WEP104) {
			if (isvalid)
				atomic_inc(&epub->wl.gtk_cnt);
			else
				atomic_dec(&epub->wl.gtk_cnt);
		}
	} else {
		/*send sub_scan task to target */
		if (isvalid)
			atomic_inc(&epub->wl.gtk_cnt);
		else
			atomic_dec(&epub->wl.gtk_cnt);

		if ((key->cipher == WLAN_CIPHER_SUITE_WEP40
		     || key->cipher == WLAN_CIPHER_SUITE_WEP104)) {
			if (isvalid)
				atomic_inc(&epub->wl.ptk_cnt);
			else
				atomic_dec(&epub->wl.ptk_cnt);
			//epub->wl.ptk = (cmd==SET_KEY) ? key : NULL;
		}
	}

	ret = sip_send_setkey(epub, ifidx, peer_addr, key, isvalid);

	if ((key->cipher == WLAN_CIPHER_SUITE_TKIP
	     || key->cipher == WLAN_CIPHER_SUITE_TKIP)) {
		if (ret == 0)
			atomic_set(&epub->wl.tkip_key_set, 1);
	}

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s exit\n", __func__);
	return ret;
}

static void esp_op_update_tkip_key(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   struct ieee80211_key_conf *conf,
				   struct ieee80211_sta *sta,
				   u32 iv32, u16 * phase1key)
{
	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);

}

void hw_scan_done(struct esp_pub *epub, bool aborted)
{
	cancel_delayed_work_sync(&epub->scan_timeout_work);

	ESSERT(epub->wl.scan_req != NULL);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
	{
		struct cfg80211_scan_info info = {
			.aborted = aborted,
		};

		ieee80211_scan_completed(epub->hw, &info);
	}
#else
	ieee80211_scan_completed(epub->hw, aborted);
#endif
	if (test_and_clear_bit(ESP_WL_FLAG_STOP_TXQ, &epub->wl.flags)) {
		sip_trigger_txq_process(epub->sip);
	}
}

static void hw_scan_timeout_report(struct work_struct *work)
{
	struct esp_pub *epub =
	    container_of(work, struct esp_pub, scan_timeout_work.work);
	bool aborted;

	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "eagle hw scan done\n");

	if (test_and_clear_bit(ESP_WL_FLAG_STOP_TXQ, &epub->wl.flags)) {
		sip_trigger_txq_process(epub->sip);
	}
	/*check if normally complete or aborted like timeout/hw error */
	aborted = (epub->wl.scan_req) ? true : false;

	if (aborted == true) {
		epub->wl.scan_req = NULL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
	{
		struct cfg80211_scan_info info = {
			.aborted = aborted,
		};

		ieee80211_scan_completed(epub->hw, &info);
	}
#else
	ieee80211_scan_completed(epub->hw, aborted);
#endif
}

static int esp_op_set_rts_threshold(struct ieee80211_hw *hw, u32 value)
{
	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);

	return 0;
}

static int esp_node_attach(struct ieee80211_hw *hw, u8 ifidx,
			   struct ieee80211_sta *sta)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_node *node;
	u8 tidno;
	struct esp_tx_tid *tid;
	int i;

	spin_lock_bh(&epub->tx_ampdu_lock);

	if (hweight32(epub->enodes_maps[ifidx]) < ESP_PUB_MAX_STA
	    && (i = ffz(epub->enodes_map)) < ESP_PUB_MAX_STA + 1) {
		epub->enodes_map |= (1 << i);
		epub->enodes_maps[ifidx] |= (1 << i);
		node = (struct esp_node *) sta->drv_priv;
		epub->enodes[i] = node;
		node->sta = sta;
		node->ifidx = ifidx;
		node->index = i;

		for (tidno = 0, tid = &node->tid[tidno];
		     tidno < WME_NUM_TID; tidno++) {
			tid->ssn = 0;
			tid->cnt = 0;
			tid->state = ESP_TID_STATE_INIT;
		}


	} else {
		i = -1;
	}

	spin_unlock_bh(&epub->tx_ampdu_lock);
	return i;
}

static int esp_node_detach(struct ieee80211_hw *hw, u8 ifidx,
			   struct ieee80211_sta *sta)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	u32 map;
	int i;
	struct esp_node *node = NULL;

	spin_lock_bh(&epub->tx_ampdu_lock);
	map = epub->enodes_maps[ifidx];
	while (map != 0) {
		i = ffs(map) - 1;
		if (epub->enodes[i]->sta == sta) {
			epub->enodes[i]->sta = NULL;
			node = epub->enodes[i];
			epub->enodes[i] = NULL;
			epub->enodes_map &= ~(1 << i);
			epub->enodes_maps[ifidx] &= ~(1 << i);

			spin_unlock_bh(&epub->tx_ampdu_lock);
			return i;
		}
		map &= ~(1 << i);
	}

	spin_unlock_bh(&epub->tx_ampdu_lock);
	return -1;
}

struct esp_node *esp_get_node_by_addr(struct esp_pub *epub,
				      const u8 * addr)
{
	int i;
	u32 map;
	struct esp_node *node = NULL;
	if (addr == NULL)
		return NULL;
	spin_lock_bh(&epub->tx_ampdu_lock);
	map = epub->enodes_map;
	while (map != 0) {
		i = ffs(map) - 1;
		if (i < 0) {
			spin_unlock_bh(&epub->tx_ampdu_lock);
			return NULL;
		}
		map &= ~(1 << i);
		if (memcmp(epub->enodes[i]->sta->addr, addr, ETH_ALEN) ==
		    0) {
			node = epub->enodes[i];
			break;
		}
	}

	spin_unlock_bh(&epub->tx_ampdu_lock);
	return node;
}

struct esp_node *esp_get_node_by_index(struct esp_pub *epub, u8 index)
{
	u32 map;
	struct esp_node *node = NULL;

	if (epub == NULL)
		return NULL;

	spin_lock_bh(&epub->tx_ampdu_lock);
	map = epub->enodes_map;
	if (map & BIT(index)) {
		node = epub->enodes[index];
	} else {
		spin_unlock_bh(&epub->tx_ampdu_lock);
		return NULL;
	}

	spin_unlock_bh(&epub->tx_ampdu_lock);
	return node;
}

int esp_get_empty_rxampdu(struct esp_pub *epub, const u8 * addr, u8 tid)
{
	int index = -1;
	if (addr == NULL)
		return index;
	spin_lock_bh(&epub->rx_ampdu_lock);
	if ((index = ffz(epub->rxampdu_map)) < ESP_PUB_MAX_RXAMPDU) {
		epub->rxampdu_map |= BIT(index);
		epub->rxampdu_node[index] =
		    esp_get_node_by_addr(epub, addr);
		epub->rxampdu_tid[index] = tid;
	} else {
		index = -1;
	}
	spin_unlock_bh(&epub->rx_ampdu_lock);
	return index;
}

int esp_get_exist_rxampdu(struct esp_pub *epub, const u8 * addr, u8 tid)
{
	u8 map;
	int index = -1;
	int i;
	if (addr == NULL)
		return index;
	spin_lock_bh(&epub->rx_ampdu_lock);
	map = epub->rxampdu_map;
	while (map != 0) {
		i = ffs(map) - 1;
		if (i < 0) {
			spin_unlock_bh(&epub->rx_ampdu_lock);
			return index;
		}
		map &= ~BIT(i);
		if (epub->rxampdu_tid[i] == tid &&
		    memcmp(epub->rxampdu_node[i]->sta->addr, addr,
			   ETH_ALEN) == 0) {
			index = i;
			break;
		}
	}

	epub->rxampdu_map &= ~BIT(index);
	spin_unlock_bh(&epub->rx_ampdu_lock);
	return index;

}

static int esp_op_sta_add(struct ieee80211_hw *hw,
			  struct ieee80211_vif *vif,
			  struct ieee80211_sta *sta)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;
	int index;
	ESP_IEEE80211_DBG(ESP_DBG_OP,
			  "%s enter, vif addr %pM, sta addr %pM\n",
			  __func__, vif->addr, sta->addr);
	index = esp_node_attach(hw, evif->index, sta);

	if (index < 0)
		return -1;
	sip_send_set_sta(epub, evif->index, 1, sta, vif, (u8) index);
	return 0;
}

static int esp_op_sta_remove(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif,
			     struct ieee80211_sta *sta)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_vif *evif = (struct esp_vif *) vif->drv_priv;
	int index;

	ESP_IEEE80211_DBG(ESP_DBG_OP,
			  "%s enter, vif addr %pM, sta addr %pM\n",
			  __func__, vif->addr, sta->addr);

	//remove a connect in target
	index = esp_node_detach(hw, evif->index, sta);
	sip_send_set_sta(epub, evif->index, 0, sta, vif, (u8) index);

	return 0;
}


static void esp_op_sta_notify(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif,
			      enum sta_notify_cmd cmd,
			      struct ieee80211_sta *sta)
{

	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);

	switch (cmd) {
	case STA_NOTIFY_SLEEP:
		break;

	case STA_NOTIFY_AWAKE:
		break;

	default:
		break;
	}
}


static int esp_op_conf_tx(struct ieee80211_hw *hw,
			  struct ieee80211_vif *vif,
			  u16 queue,
			  const struct ieee80211_tx_queue_params *params)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);
	return sip_send_wmm_params(epub, queue, params);
}

static u64 esp_op_get_tsf(struct ieee80211_hw *hw,
			  struct ieee80211_vif *vif)
{
	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);

	return 0;
}

static void esp_op_set_tsf(struct ieee80211_hw *hw,
			   struct ieee80211_vif *vif, u64 tsf)
{
	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);
}

static void esp_op_reset_tsf(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif)
{
	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);

}

static void esp_op_rfkill_poll(struct ieee80211_hw *hw)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;

	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter \n", __func__);

	wiphy_rfkill_set_hw_state(hw->wiphy,
				  test_bit(ESP_WL_FLAG_RFKILL,
					   &epub->wl.
					   flags) ? true : false);
}

#ifdef HW_SCAN
static int esp_op_hw_scan(struct ieee80211_hw *hw,
			  struct ieee80211_vif *vif,
			  struct cfg80211_scan_request *req)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	int i, ret;
	bool scan_often = true;

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s\n", __func__);

	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "scan, %d\n", req->n_ssids);
	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "scan, len 1:%d,ssid 1:%s\n",
			  req->ssids->ssid_len,
			  req->ssids->ssid_len ==
			  0 ? "" : (char *) req->ssids->ssid);
	if (req->n_ssids > 1)
		ESP_IEEE80211_DBG(ESP_DBG_TRACE,
				  "scan, len 2:%d,ssid 2:%s\n",
				  (req->ssids + 1)->ssid_len,
				  (req->ssids + 1)->ssid_len ==
				  0 ? "" : (char *) (req->ssids +
						     1)->ssid);

	/*scan_request is keep allocate untill scan_done,record it
	   to split request into multi sdio_cmd */
	if (atomic_read(&epub->wl.off)) {
		esp_dbg(ESP_DBG_ERROR, "%s scan but wl off \n", __func__);
		return -EPERM;
	}

	if (req->n_ssids > 1) {
		struct cfg80211_ssid *ssid2 = req->ssids + 1;
		if ((req->ssids->ssid_len > 0 && ssid2->ssid_len > 0)
		    || req->n_ssids > 2) {
			ESP_IEEE80211_DBG(ESP_DBG_ERROR,
					  "scan ssid num: %d, ssid1:%s, ssid2:%s,not support\n",
					  req->n_ssids,
					  req->ssids->ssid_len ==
					  0 ? "" : (char *) req->ssids->
					  ssid,
					  ssid2->ssid_len ==
					  0 ? "" : (char *) ssid2->ssid);
			return -EINVAL;
		}
	}

	epub->wl.scan_req = req;

	for (i = 0; i < req->n_channels; i++)
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, "eagle hw_scan freq %d\n",
				  req->channels[i]->center_freq);
#if 0
	for (i = 0; i < req->n_ssids; i++) {
		if (req->ssids->ssid_len > 0) {
			req->ssids->ssid[req->ssids->ssid_len] = '\0';
			ESP_IEEE80211_DBG(ESP_DBG_TRACE,
					  "scan_ssid %d:%s\n", i,
					  req->ssids->ssid);
		}
	}
#endif

	/*in connect state, suspend tx data */
	if (epub->sip->support_bgscan &&
	    test_bit(ESP_WL_FLAG_CONNECT, &epub->wl.flags) &&
	    req->n_channels > 0) {

		scan_often = epub->scan_permit_valid
		    && time_before(jiffies, epub->scan_permit);
		epub->scan_permit_valid = true;

		if (!scan_often) {
/*                        epub->scan_permit = jiffies + msecs_to_jiffies(900);
                        set_bit(ESP_WL_FLAG_STOP_TXQ, &epub->wl.flags);
                        if (atomic_read(&epub->txq_stopped) == false) {
                                atomic_set(&epub->txq_stopped, true);
                                ieee80211_stop_queues(hw);
                        }
*/
		} else {
			ESP_IEEE80211_DBG(ESP_DBG_LOG, "scan too often\n");
			return -EACCES;
		}
	} else {
		scan_often = false;
	}

	/*send sub_scan task to target */
	ret = sip_send_scan(epub);

	if (ret) {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR,
				  "fail to send scan_cmd\n");
		return ret;
	} else {
		if (!scan_often) {
			epub->scan_permit =
			    jiffies + msecs_to_jiffies(900);
			set_bit(ESP_WL_FLAG_STOP_TXQ, &epub->wl.flags);
			if (atomic_read(&epub->txq_stopped) == false) {
				atomic_set(&epub->txq_stopped, true);
				ieee80211_stop_queues(hw);
			}
			/*force scan complete in case target fail to report in time */
			ieee80211_queue_delayed_work(hw,
						     &epub->
						     scan_timeout_work,
						     req->n_channels * HZ /
						     4);
		}
	}

	return 0;
}

static int esp_op_remain_on_channel(struct ieee80211_hw *hw,
				    struct ieee80211_channel *chan,
				    enum nl80211_channel_type channel_type,
				    int duration)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;

	ESP_IEEE80211_DBG(ESP_DBG_OP,
			  "%s enter, center_freq = %d duration = %d\n",
			  __func__, chan->center_freq, duration);
	sip_send_roc(epub, chan->center_freq, duration);
	return 0;
}

static int esp_op_cancel_remain_on_channel(struct ieee80211_hw *hw)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s enter \n", __func__);
	epub->roc_flags = 0;	// to disable roc state
	sip_send_roc(epub, 0, 0);
	return 0;
}
#endif

void esp_rocdone_process(struct ieee80211_hw *hw,
			 struct sip_evt_roc *report)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s enter, state = %d is_ok = %d\n",
			  __func__, report->state, report->is_ok);

	//roc process begin 
	if ((report->state == 1) && (report->is_ok == 1)) {
		epub->roc_flags = 1;	//flags in roc state, to fix channel, not change
		ieee80211_ready_on_channel(hw);
	} else if ((report->state == 0) && (report->is_ok == 1))	//roc process timeout
	{
		epub->roc_flags = 0;	// to disable roc state
		ieee80211_remain_on_channel_expired(hw);
	}
}

static int esp_op_set_bitrate_mask(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   const struct cfg80211_bitrate_mask
				   *mask)
{
	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s enter \n", __func__);
	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s vif->macaddr[%pM], mask[%d]\n",
			  __func__, vif->addr, mask->control[0].legacy);

	return 0;
}

void esp_op_flush(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		  u32 queues, bool drop)
{

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s enter \n", __func__);
	do {

		struct esp_pub *epub = (struct esp_pub *) hw->priv;
		unsigned long time = jiffies + msecs_to_jiffies(15);
		while (atomic_read(&epub->sip->tx_data_pkt_queued)) {
			if (!time_before(jiffies, time)) {
				break;
			}
			if (sif_get_ate_config() == 0) {
				ieee80211_queue_work(epub->hw,
						     &epub->tx_work);
			} else {
				queue_work(epub->esp_wkq, &epub->tx_work);
			}
			//sip_txq_process(epub);
		}
		mdelay(10);

	} while (0);
}

static int esp_op_ampdu_action(struct ieee80211_hw *hw,
			       struct ieee80211_vif *vif,
			       struct ieee80211_ampdu_params *params)
{
	int ret = -EOPNOTSUPP;
	enum ieee80211_ampdu_mlme_action action = params->action;
	struct ieee80211_sta *sta = params->sta;
	u16 tid = params->tid;
	u16 *ssn = &params->ssn;
	u8 buf_size = params->buf_size;
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct esp_node *node = (struct esp_node *) sta->drv_priv;
	struct esp_tx_tid *tid_info = &node->tid[tid];

	ESP_IEEE80211_DBG(ESP_DBG_OP, "%s enter \n", __func__);
	switch (action) {
	case IEEE80211_AMPDU_TX_START:
		if (mod_support_no_txampdu() ||
		    cfg80211_get_chandef_type(&epub->hw->conf.chandef) ==
		    NL80211_CHAN_NO_HT || !sta->ht_cap.ht_supported)
			return ret;

		//if (vif->p2p || vif->type != NL80211_IFTYPE_STATION)
		//      return ret;

		ESP_IEEE80211_DBG(ESP_DBG_TRACE,
				  "%s TX START, addr:%pM,tid:%u,state:%d\n",
				  __func__, sta->addr, tid,
				  tid_info->state);
		spin_lock_bh(&epub->tx_ampdu_lock);
		ESSERT(tid_info->state == ESP_TID_STATE_TRIGGER);
		*ssn = tid_info->ssn;
		tid_info->state = ESP_TID_STATE_PROGRESS;

		ieee80211_start_tx_ba_cb_irqsafe(vif, sta->addr, tid);
		spin_unlock_bh(&epub->tx_ampdu_lock);
		ret = 0;
		break;
	case IEEE80211_AMPDU_TX_STOP_CONT:
		ESP_IEEE80211_DBG(ESP_DBG_TRACE,
				  "%s TX STOP, addr:%pM,tid:%u,state:%d\n",
				  __func__, sta->addr, tid,
				  tid_info->state);
		spin_lock_bh(&epub->tx_ampdu_lock);
		if (tid_info->state == ESP_TID_STATE_WAIT_STOP)
			tid_info->state = ESP_TID_STATE_STOP;
		else
			tid_info->state = ESP_TID_STATE_INIT;
		ieee80211_stop_tx_ba_cb_irqsafe(vif, sta->addr, tid);
		spin_unlock_bh(&epub->tx_ampdu_lock);
		ret =
		    sip_send_ampdu_action(epub, SIP_AMPDU_TX_STOP,
					  sta->addr, tid, node->ifidx, 0);
		break;
	case IEEE80211_AMPDU_TX_STOP_FLUSH:
	case IEEE80211_AMPDU_TX_STOP_FLUSH_CONT:
		if (tid_info->state == ESP_TID_STATE_WAIT_STOP)
			tid_info->state = ESP_TID_STATE_STOP;
		else
			tid_info->state = ESP_TID_STATE_INIT;
		ret =
		    sip_send_ampdu_action(epub, SIP_AMPDU_TX_STOP,
					  sta->addr, tid, node->ifidx, 0);
		break;
	case IEEE80211_AMPDU_TX_OPERATIONAL:
		ESP_IEEE80211_DBG(ESP_DBG_TRACE,
				  "%s TX OPERATION, addr:%pM,tid:%u,state:%d\n",
				  __func__, sta->addr, tid,
				  tid_info->state);
		spin_lock_bh(&epub->tx_ampdu_lock);

		if (tid_info->state != ESP_TID_STATE_PROGRESS) {
			if (tid_info->state == ESP_TID_STATE_INIT) {
				printk(KERN_ERR "%s WIFI RESET, IGNORE\n",
				       __func__);
				spin_unlock_bh(&epub->tx_ampdu_lock);
				return -ENETRESET;
			} else {
				ESSERT(0);
			}
		}

		tid_info->state = ESP_TID_STATE_OPERATIONAL;
		spin_unlock_bh(&epub->tx_ampdu_lock);
		ret =
		    sip_send_ampdu_action(epub, SIP_AMPDU_TX_OPERATIONAL,
					  sta->addr, tid, node->ifidx,
					  buf_size);
		break;
	case IEEE80211_AMPDU_RX_START:
		if (mod_support_no_rxampdu() ||
		    cfg80211_get_chandef_type(&epub->hw->conf.chandef) ==
		    NL80211_CHAN_NO_HT || !sta->ht_cap.ht_supported)
			return ret;

		if ((vif->p2p && false)
		    || (vif->type != NL80211_IFTYPE_STATION && false)
		    )
			return ret;
		ESP_IEEE80211_DBG(ESP_DBG_TRACE,
				  "%s RX START %pM tid %u %u\n", __func__,
				  sta->addr, tid, *ssn);
		ret =
		    sip_send_ampdu_action(epub, SIP_AMPDU_RX_START,
					  sta->addr, tid, *ssn, 64);
		break;
	case IEEE80211_AMPDU_RX_STOP:
		ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s RX STOP %pM tid %u\n",
				  __func__, sta->addr, tid);
		ret =
		    sip_send_ampdu_action(epub, SIP_AMPDU_RX_STOP,
					  sta->addr, tid, 0, 0);
		break;
	default:
		break;
	}
	return ret;
}

static void esp_tx_work(struct work_struct *work)
{
	struct esp_pub *epub = container_of(work, struct esp_pub, tx_work);

	mutex_lock(&epub->tx_mtx);
	sip_txq_process(epub);
	mutex_unlock(&epub->tx_mtx);
}


int esp_start_ap(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	struct esp_pub *epub = (struct esp_pub *) hw->priv;
	struct sip_cmd_setvif svif;

	memset(&svif, 0, sizeof(struct sip_cmd_setvif));
	svif.set = 3;
	sip_cmd(epub, SIP_CMD_SETVIF, (u8 *) & svif,
		sizeof(struct sip_cmd_setvif));

    return 0;
}

static const struct ieee80211_ops esp_mac80211_ops = {
	.tx = esp_op_tx,
	.start = esp_op_start,
	.stop = esp_op_stop,
#ifdef CONFIG_PM
	.suspend = esp_op_suspend,
	.resume = esp_op_resume,
#endif
	.add_interface = esp_op_add_interface,
	.remove_interface = esp_op_remove_interface,
	.config = esp_op_config,

	.bss_info_changed = esp_op_bss_info_changed,
	.prepare_multicast = esp_op_prepare_multicast,
	.configure_filter = esp_op_configure_filter,
	.set_key = esp_op_set_key,
	.update_tkip_key = esp_op_update_tkip_key,
	//.sched_scan_start = esp_op_sched_scan_start,
	//.sched_scan_stop = esp_op_sched_scan_stop,
	.set_rts_threshold = esp_op_set_rts_threshold,
	.sta_notify = esp_op_sta_notify,
	.conf_tx = esp_op_conf_tx,
	.change_interface = esp_op_change_interface,
	.get_tsf = esp_op_get_tsf,
	.set_tsf = esp_op_set_tsf,
	.reset_tsf = esp_op_reset_tsf,
	.rfkill_poll = esp_op_rfkill_poll,
#ifdef HW_SCAN
	.hw_scan = esp_op_hw_scan,
	.remain_on_channel = esp_op_remain_on_channel,
	.cancel_remain_on_channel = esp_op_cancel_remain_on_channel,
#endif
	.ampdu_action = esp_op_ampdu_action,
	//.get_survey = esp_op_get_survey,
	.sta_add = esp_op_sta_add,
	.sta_remove = esp_op_sta_remove,
#ifdef CONFIG_NL80211_TESTMODE
	//CFG80211_TESTMODE_CMD(esp_op_tm_cmd)
#endif
	.set_bitrate_mask = esp_op_set_bitrate_mask,
	.flush = esp_op_flush,
    .start_ap = esp_start_ap,
};

struct esp_pub *esp_pub_alloc_mac80211(struct device *dev)
{
	struct ieee80211_hw *hw;
	struct esp_pub *epub;
	int ret = 0;

	hw = ieee80211_alloc_hw(sizeof(struct esp_pub), &esp_mac80211_ops);

	if (hw == NULL) {
		esp_dbg(ESP_DBG_ERROR, "ieee80211 can't alloc hw!\n");
		ret = -ENOMEM;
		return ERR_PTR(ret);
	}
	hw->wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;

	epub = hw->priv;
	memset(epub, 0, sizeof(*epub));
	epub->hw = hw;
	SET_IEEE80211_DEV(hw, dev);
	epub->dev = dev;

	skb_queue_head_init(&epub->txq);
	skb_queue_head_init(&epub->txdoneq);
	skb_queue_head_init(&epub->rxq);

	spin_lock_init(&epub->tx_ampdu_lock);
	spin_lock_init(&epub->rx_ampdu_lock);
	spin_lock_init(&epub->tx_lock);
	mutex_init(&epub->tx_mtx);
	spin_lock_init(&epub->rx_lock);

	INIT_WORK(&epub->tx_work, esp_tx_work);

	//epub->esp_wkq = create_freezable_workqueue("esp_wkq"); 
	epub->esp_wkq = create_singlethread_workqueue("esp_wkq");

	if (epub->esp_wkq == NULL) {
		ret = -ENOMEM;
		return ERR_PTR(ret);
	}
	epub->scan_permit_valid = false;
	INIT_DELAYED_WORK(&epub->scan_timeout_work,
			  hw_scan_timeout_report);

	return epub;
}


int esp_pub_dealloc_mac80211(struct esp_pub *epub)
{
	set_bit(ESP_WL_FLAG_RFKILL, &epub->wl.flags);

	destroy_workqueue(epub->esp_wkq);
	mutex_destroy(&epub->tx_mtx);

#ifdef ESP_NO_MAC80211
	free_netdev(epub->net_dev);
	wiphy_free(epub->wdev->wiphy);
	kfree(epub->wdev);
#else
	if (epub->hw) {
		ieee80211_free_hw(epub->hw);
	}
#endif

	return 0;
}

#if 0
static int esp_reg_notifier(struct wiphy *wiphy,
			    struct regulatory_request *request)
{
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	int i;

	ESP_IEEE80211_DBG(ESP_DBG_TRACE, "%s enter %d\n", __func__,
			  request->initiator);

	//TBD
}
#endif

/* 2G band channels */
static struct ieee80211_channel esp_channels_2ghz[] = {
	{.hw_value = 1,.center_freq = 2412,.max_power = 25},
	{.hw_value = 2,.center_freq = 2417,.max_power = 25},
	{.hw_value = 3,.center_freq = 2422,.max_power = 25},
	{.hw_value = 4,.center_freq = 2427,.max_power = 25},
	{.hw_value = 5,.center_freq = 2432,.max_power = 25},
	{.hw_value = 6,.center_freq = 2437,.max_power = 25},
	{.hw_value = 7,.center_freq = 2442,.max_power = 25},
	{.hw_value = 8,.center_freq = 2447,.max_power = 25},
	{.hw_value = 9,.center_freq = 2452,.max_power = 25},
	{.hw_value = 10,.center_freq = 2457,.max_power = 25},
	{.hw_value = 11,.center_freq = 2462,.max_power = 25},
	{.hw_value = 12,.center_freq = 2467,.max_power = 25},
	{.hw_value = 13,.center_freq = 2472,.max_power = 25},
	//{ .hw_value = 14, .center_freq = 2484, .max_power = 25 },
};

/* 11G rate */
static struct ieee80211_rate esp_rates_2ghz[] = {
	{
	 .bitrate = 10,
	 .hw_value = CONF_HW_BIT_RATE_1MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_1MBPS,
	 },
	{
	 .bitrate = 20,
	 .hw_value = CONF_HW_BIT_RATE_2MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_2MBPS,
	 .flags = IEEE80211_RATE_SHORT_PREAMBLE},
	{
	 .bitrate = 55,
	 .hw_value = CONF_HW_BIT_RATE_5_5MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_5_5MBPS,
	 .flags = IEEE80211_RATE_SHORT_PREAMBLE},
	{
	 .bitrate = 110,
	 .hw_value = CONF_HW_BIT_RATE_11MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_11MBPS,
	 .flags = IEEE80211_RATE_SHORT_PREAMBLE},
	{
	 .bitrate = 60,
	 .hw_value = CONF_HW_BIT_RATE_6MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_6MBPS,
	 },
	{
	 .bitrate = 90,
	 .hw_value = CONF_HW_BIT_RATE_9MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_9MBPS,
	 },
	{
	 .bitrate = 120,
	 .hw_value = CONF_HW_BIT_RATE_12MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_12MBPS,
	 },
	{
	 .bitrate = 180,
	 .hw_value = CONF_HW_BIT_RATE_18MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_18MBPS,
	 },
	{
	 .bitrate = 240,
	 .hw_value = CONF_HW_BIT_RATE_24MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_24MBPS,
	 },
	{
	 .bitrate = 360,
	 .hw_value = CONF_HW_BIT_RATE_36MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_36MBPS,
	 },
	{
	 .bitrate = 480,
	 .hw_value = CONF_HW_BIT_RATE_48MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_48MBPS,
	 },
	{
	 .bitrate = 540,
	 .hw_value = CONF_HW_BIT_RATE_54MBPS,
	 .hw_value_short = CONF_HW_BIT_RATE_54MBPS,
	 },
};

static void esp_pub_init_mac80211(struct esp_pub *epub)
{
	struct ieee80211_hw *hw = epub->hw;

	static const u32 cipher_suites[] = {
		WLAN_CIPHER_SUITE_WEP40,
		WLAN_CIPHER_SUITE_WEP104,
		WLAN_CIPHER_SUITE_TKIP,
		WLAN_CIPHER_SUITE_CCMP,
	};

	hw->max_listen_interval = 10;

	ieee80211_hw_set(hw, SIGNAL_DBM);
	ieee80211_hw_set(hw, HAS_RATE_CONTROL);
	ieee80211_hw_set(hw, SUPPORTS_PS);
	ieee80211_hw_set(hw, AMPDU_AGGREGATION);
	ieee80211_hw_set(hw, HOST_BROADCAST_PS_BUFFERING);
	//IEEE80211_HW_PS_NULLFUNC_STACK |   
	//IEEE80211_HW_CONNECTION_MONITOR |
	//IEEE80211_HW_BEACON_FILTER |
	//IEEE80211_HW_AMPDU_AGGREGATION |
	//IEEE80211_HW_REPORTS_TX_ACK_STATUS;
	hw->max_rx_aggregation_subframes = 0x40;
	hw->max_tx_aggregation_subframes = 0x40;

	hw->wiphy->cipher_suites = cipher_suites;
	hw->wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites);
	hw->wiphy->max_scan_ie_len =
	    epub->sip->tx_blksz - sizeof(struct sip_hdr) -
	    sizeof(struct sip_cmd_scan);

	/* ONLY station for now, support P2P soon... */
	hw->wiphy->interface_modes =
	    BIT(NL80211_IFTYPE_P2P_GO) |
	    BIT(NL80211_IFTYPE_P2P_CLIENT) |
	    BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP);

	hw->wiphy->software_iftypes |= BIT(NL80211_IFTYPE_AP);
	hw->wiphy->max_scan_ssids = 2;
	//hw->wiphy->max_sched_scan_ssids = 16;
	//hw->wiphy->max_match_sets = 16;

	hw->wiphy->max_remain_on_channel_duration = 5000;

	atomic_set(&epub->wl.off, 1);

	epub->wl.sbands[NL80211_BAND_2GHZ].band = NL80211_BAND_2GHZ;
	epub->wl.sbands[NL80211_BAND_2GHZ].channels = esp_channels_2ghz;
	epub->wl.sbands[NL80211_BAND_2GHZ].bitrates = esp_rates_2ghz;
	epub->wl.sbands[NL80211_BAND_2GHZ].n_channels =
	    ARRAY_SIZE(esp_channels_2ghz);
	epub->wl.sbands[NL80211_BAND_2GHZ].n_bitrates =
	    ARRAY_SIZE(esp_rates_2ghz);
	/*add to support 11n */
	epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.ht_supported = true;
	epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.cap = 0x116C;	//IEEE80211_HT_CAP_RX_STBC; //IEEE80211_HT_CAP_SGI_20;
	epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.ampdu_factor =
	    IEEE80211_HT_MAX_AMPDU_16K;
	epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.ampdu_density =
	    IEEE80211_HT_MPDU_DENSITY_NONE;
	memset(&epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.mcs, 0,
	       sizeof(epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.mcs));
	epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.mcs.rx_mask[0] = 0xff;
	//epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.mcs.rx_highest = 7;
	//epub->wl.sbands[NL80211_BAND_2GHZ].ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;

	/* BAND_5GHZ TBD */

	hw->wiphy->bands[NL80211_BAND_2GHZ] =
	    &epub->wl.sbands[NL80211_BAND_2GHZ];
	/* BAND_5GHZ TBD */

	/*no fragment */
	hw->wiphy->frag_threshold = IEEE80211_MAX_FRAG_THRESHOLD;

	/* handle AC queue in f/w */
	hw->queues = 4;
	hw->max_rates = 4;
	//hw->wiphy->reg_notifier = esp_reg_notify;

	hw->vif_data_size = sizeof(struct esp_vif);
	hw->sta_data_size = sizeof(struct esp_node);

	//hw->max_rx_aggregation_subframes = 8;
}

extern int ieee80211_hw_if_add(struct ieee80211_hw *hw, const char *name,
                    unsigned char name_assign_type,
                    struct wireless_dev **new_wdev, enum nl80211_iftype type,
                    struct vif_params *params);

int esp_register_mac80211(struct esp_pub *epub)
{
	int ret = 0;
	u8 *wlan_addr;
	u8 *p2p_addr;
	int idx;

	esp_pub_init_mac80211(epub);

	epub->hw->wiphy->addresses = (struct mac_address *) esp_mac_addr;
	memcpy(&epub->hw->wiphy->addresses[0], epub->mac_addr, ETH_ALEN);
	memcpy(&epub->hw->wiphy->addresses[1], epub->mac_addr, ETH_ALEN);
	wlan_addr = (u8 *) & epub->hw->wiphy->addresses[0];
	p2p_addr = (u8 *) & epub->hw->wiphy->addresses[1];

	for (idx = 0; idx < 64; idx++) {
		p2p_addr[0] = wlan_addr[0] | 0x02;
		p2p_addr[0] ^= idx << 2;
		if (strncmp(p2p_addr, wlan_addr, 6) != 0)
			break;
	}

	epub->hw->wiphy->n_addresses = 2;

	ret = ieee80211_register_hw(epub->hw);

	if (ret < 0) {
		ESP_IEEE80211_DBG(ESP_DBG_ERROR,
				  "unable to register mac80211 hw: %d\n",
				  ret);
		return ret;
	} else {
#if 0
		rtnl_lock();
		if (epub->hw->wiphy->interface_modes & (BIT(NL80211_IFTYPE_AP))) {
			ret = ieee80211_hw_if_add(epub->hw, "wlan%d", NULL, NET_NAME_ENUM, NL80211_IFTYPE_STATION, NULL);
			if (ret)
				wiphy_warn(epub->hw->wiphy, "Failed to add default virtual iface\n");
		}

		rtnl_unlock();
#endif
	}

	set_bit(ESP_WL_FLAG_HW_REGISTERED, &epub->wl.flags);

	return ret;
}

static u8 getaddr_index(u8 * addr, struct esp_pub *epub)
{
	int i;
	for (i = 0; i < ESP_PUB_MAX_VIF; i++)
		if (memcmp
		    (addr, (u8 *) & epub->hw->wiphy->addresses[i],
		     ETH_ALEN) == 0)
			return i;
	return ESP_PUB_MAX_VIF;
}
