/*
* mdrv_hwnat_mcast.h- Sigmastar
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
/*
    Module Name:
    util.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2012-10-19      Initial version
*/

#ifndef _MCAST_TBL_WANTED
#define _MCAST_TBL_WANTED

//#define MCAST_DEBUG
#ifdef MCAST_DEBUG
#define MCAST_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define MCAST_PRINT(fmt, args...) { }
#endif



/*
 * EXPORT FUNCTION
 */
int MDrv_HWNAT_Mcast_Insert_Entry(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_px_en, uint8_t mc_px_qos_en, uint8_t mc_qos_qid);
int MDrv_HWNAT_Mcast_Update_Qid(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_qos_qid);
int MDrv_HWNAT_Mcast_Delete_Entry(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_px_en, uint8_t mc_px_qos_en, uint8_t mc_qos_qid);
void MDrv_HWNAT_Mcast_Dump(void);
void MDrv_HWNAT_Mcast_Delete_All(void);



#endif
