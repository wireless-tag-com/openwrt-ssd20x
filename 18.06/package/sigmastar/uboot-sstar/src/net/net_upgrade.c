/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 *	SPDX-License-Identifier:	GPL-2.0
 */

/*
 * General Desription:
 *
 * The user interface supports commands for BOOTP, RARP, and TFTP.
 * Also, we support ARP internally. Depending on available data,
 * these interact as follows:
 *
 * BOOTP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *			- name of bootfile
 *	Next step:	ARP
 *
 * LINK_LOCAL:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *	Next step:	ARP
 *
 * RARP:
 *
 *	Prerequisites:	- own ethernet address
 *	We want:	- own IP address
 *			- TFTP server IP address
 *	Next step:	ARP
 *
 * ARP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *	We want:	- TFTP server ethernet address
 *	Next step:	TFTP
 *
 * DHCP:
 *
 *     Prerequisites:	- own ethernet address
 *     We want:		- IP, Netmask, ServerIP, Gateway IP
 *			- bootfilename, lease time
 *     Next step:	- TFTP
 *
 * TFTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- TFTP server IP address
 *			- TFTP server ethernet address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 *
 * NFS:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *			- name of bootfile (if unknown, we use a default name
 *			  derived from our own IP address)
 *	We want:	- load the boot file
 *	Next step:	none
 *
 * SNTP:
 *
 *	Prerequisites:	- own ethernet address
 *			- own IP address
 *	We want:	- network time
 *	Next step:	none
 */


#ifdef CONFIG_CMD_NETUPGRADE

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/compiler.h>
#include <linux/ctype.h>
#include <MsDebug.h>
#include <malloc.h>
#include <net.h>
#include "tftp.h"


ulong ts_1st = 0;
ulong ts_2nd = 0;
ulong delt_ts = 0;
char netupgrade_ing = 0;	//called in "net/bootp.c"
char delt_ts_changed = 0;

char netrty[32];
char macaddr_env[18];
int progresse_status = 0;
char TftpParameraReady = 0;
char netupgrade_force_state = 0;
static IPaddr_t AutoNetUpgradeToolIP;


#define NETUPGRADE_SCRIPTS_LENGTH  0x1000
#define NETUPGRADE_FILE            "net_upgrade_file"
#define NETUPGRADE_VERSION         "net_upgrade_version"

#define CMD_LINE_SIZE              128
#define MAX_LINE_SIZE              8000
#define SCRIPT_FILE_COMMENT        '#'
#define SCRIPT_FILE_END            '%'
#define IS_COMMENT(x)              (SCRIPT_FILE_COMMENT == (x))
#define IS_FILE_END(x)             (SCRIPT_FILE_END == (x))
#define IS_LINE_END(x)             ('\r' == (x)|| '\n' == (x))

#define NETUPGRADE_TIME_OUT 1000UL
#define NETUPGRADE_SERVICE_PORT_RECV 9000
#define NETUPGRADE_SERVICE_PORT_SEND 9001
#define MACADDR_WILDCARD "00:00:00:00:00:00"

struct netupgrade_udp_data {
	char        memaddr[4];   /* Load memory addr	  */
	char        devmac[6];    /* Device Mac addr	  */
	char        force;        /* devmac is useful     */
	char        version[16];  /* version              */
	char        status;       /* tftp upgrade status  */
	char        servIP[4];    /* tftp server IP addr  */
	char        flag;         /* flag, default: ":"   */
	char        name[64];     /* update file name     */
};

#define NETUPGRADE_UDP_DATA_SIZE (sizeof(struct netupgrade_udp_data))


char ip_data[128];
struct ip_udp_hdr *pIpData = NULL;      // point to ip_data
struct ip_udp_hdr *pIpUdpHdr = NULL;    // point to ip
struct netupgrade_udp_data *pUdpData = NULL;

char net_upgrade_cmd[6][64] = {
	"sf probe 0",
	"sf erase 0x0 0x1000000",
	"sf write 0x$(fileaddr) 0x0 0x$(filesize)",
	"setenv bootcmd bootm 0x14050000",
	"setenv cpu_part_start 0x14050000",
};


static inline void NetCopyMacAddr(void *to, void *from)
{
	memcpy((void *)to, from, 6*sizeof(char));
}


void macaddr_to_string(unsigned long long *x, char *s)
{
	char *px = (char *)x;

	if ((NULL==x) || (NULL==s))
	{
		printf("the input point[*x or *s] is Null\n");
	}

	sprintf(s, "%02x:%02x:%02x:%02x:%02x:%02x",
		(int) ((*(px+0)) & 0xff),
		(int) ((*(px+1)) & 0xff),
		(int) ((*(px+2)) & 0xff),
		(int) ((*(px+3)) & 0xff),
		(int) ((*(px+4)) & 0xff),
		(int) ((*(px+5)) & 0xff)
	);
}


int macaddr_compare(char *x, char *s)
{
	size_t i = 0;
	int ret = -1;

	if ((NULL==x) || (NULL==s))
	{
		printf("the input point[*x or *s] is Null\n");
		return -1;
	}

	for (i=0; i<strlen((const char *)x);i++)
	{
		// make sure x is "0123456789abcdefABCDEF"
		if (!isxdigit(*(x+i))) continue;

		//x is Number
		if(isdigit(*(x+i)))
		{
			ret = (*(x+i)) - (*(s+i));
			if (0==ret) continue;
			else break;
		}

		//x is Char
		ret=(islower(*(x+i))?(*(x+i)):((*(x+i))+32)) - (islower(*(s+i))?(*(s+i)):((*(s+i))+32));
		if(0==ret) continue;
		else  break;
	}
	return ret;
}


static void UpgradeState_timeout(void)
{
	//puts("Timeout\n");
	net_set_state(NETLOOP_FAIL);
}


void UpgradeStatus_start(void)
{
	uchar * pkt = NULL;
	int dport = 9001;
	int sport = 10000 + (get_timer(0) % 4096);
	struct netupgrade_udp_data *s = NULL;

	NetSetTimeout(500UL, (thand_f *)UpgradeState_timeout);

	pkt = NetTxPacket + NetEthHdrSize() + IP_UDP_HDR_SIZE;
	memcpy(pkt, ip_data + IP_UDP_HDR_SIZE, NETUPGRADE_UDP_DATA_SIZE);
	s = (struct netupgrade_udp_data *)pkt;
	s->status = (char)progresse_status&0xff;

	memset(NetServerEther, 0, sizeof(NetServerEther));
	NetSendUDPPacket(NetServerEther, AutoNetUpgradeToolIP, dport, sport, NETUPGRADE_UDP_DATA_SIZE);
}


static void NetUpgradeTimeout(void)
{
	puts("NetUpgrade Timeout\n");

	ts_2nd = get_timer(0);
	delt_ts = get_timer(0)-ts_1st;
	net_set_state(NETLOOP_SUCCESS);
	printf("[NetUpgrade] ts_1st=0x%lx, ts_2nd=0x%lx, delt_ts=0x%lx\n",ts_1st,ts_2nd,delt_ts);
}


static void NetUpgradeHandler(uchar *pkt, unsigned dest, IPaddr_t sip, unsigned src, unsigned len)
{
	char macaddr[8];
	char macaddr_udp[18];

	if (0==TftpParameraReady)
	{
		memset(ip_data, 0, sizeof(ip_data));
		memcpy(ip_data, pIpUdpHdr, sizeof(ip_data));
	}

	if ((0==TftpParameraReady) && (':'==pUdpData->flag) && \
		(('y'==pUdpData->force) || ('n'==pUdpData->force))&& \
		(NETUPGRADE_SERVICE_PORT_RECV==ntohs(pIpData->udp_dst)))
	{
		memset(macaddr, '\0', sizeof(macaddr));
		NetCopyMacAddr(macaddr, pUdpData->devmac);
		macaddr_to_string((unsigned long long *)macaddr, macaddr_udp);

		if((!macaddr_compare(macaddr_udp, macaddr_env)) || \
		   (!macaddr_compare(macaddr_udp, MACADDR_WILDCARD)))
		{
			TftpParameraReady = 1;
			net_set_state(NETLOOP_SUCCESS);

			if ('y'==pUdpData->force)
			{
				netupgrade_force_state = 0x01;
			}
		}

		if ((0==TftpParameraReady) && (0==delt_ts_changed))
		{
			NetSetTimeout(0UL, (thand_f *)NULL);
			NetSetTimeout(3000UL, (thand_f *)NetUpgradeHandler);
			delt_ts_changed = 1;
			printf("set delt_ts_changed = 1\n");
		}
	}

	if (0==TftpParameraReady)
	{
		ts_2nd = get_timer(0);
		delt_ts = ts_2nd-ts_1st;
		debug_cond(DEBUG_DEV_PKT,"ts_1st=0x%lx, ts_2nd=0x%lx\n",ts_1st,ts_2nd);
		if ((NETUPGRADE_TIME_OUT*(delt_ts_changed?3:1)) < delt_ts)
		{
			net_set_state(NETLOOP_SUCCESS);
			printf("[NetUpgrade]delt_ts=0x%lx\n",delt_ts);
		}
	}
}


void NetUpgrade_start(void)
{
	char *pMacAddr = NULL;

	NetSetTimeout(NETUPGRADE_TIME_OUT, (thand_f *)NetUpgradeTimeout);
	net_set_udp_handler(NetUpgradeHandler);

	memset(macaddr_env, '\0', sizeof(macaddr_env));
	pMacAddr = getenv("ethaddr");
	if (NULL != pMacAddr)
	{
		memcpy(macaddr_env, pMacAddr, strlen(pMacAddr));
		printf("[NetUpgrade] Get Device MacAddr(%c%c:%c%c:%c%c:%c%c:%c%c:%c%c)\n",\
				macaddr_env[0], macaddr_env[1], macaddr_env[3], macaddr_env[4],\
				macaddr_env[6], macaddr_env[7], macaddr_env[9], macaddr_env[10],\
				macaddr_env[12],macaddr_env[13],macaddr_env[15],macaddr_env[16]);
	}
	else
	{
		net_set_state(NETLOOP_FAIL);
		printf("[NetUpgrade] ==== Get Mac addr from ENV fail ====\n");
	}
}


//if get the key's valule from ENV is different with "val" and do save operation
int net_upgrade_setenv(char* key, char* val)
{
	char * pstr = NULL;

	if (NULL == key)
	{
		printf("[NetUpgrade] input 'ker' is NULL\n");
		return -1;
	}

	if (NULL != (pstr = getenv(key)))
	{
		printf("[NetUpgrade] get %s=%s\n", key, pstr);
	}

	if ((NULL==pstr) || strcmp(val, pstr))
	{
		printf("[NetUpgrade] set \"%s=%s\" to env!\n",key, val);
		if (setenv(key, val))
		{
			printf("[NetUpgrade] set \"%s=%s\" err, and do again!\n",key, val);
			setenv(key, val);
		}

		if (CMD_RET_SUCCESS != run_command("saveenv", 0))
		{
			printf("[NetUpgrade] do \"saveenv\" err, and do again!\n");
			run_command("saveenv", 0);
		}
	}

	return 0;
}


int net_upgrade_dhcp(void)
{
	char * pstr = NULL;

	netupgrade_ing = 1;
	
	memset(netrty, 0, sizeof(netrty));
	if (NULL != (pstr = getenv("netretry")))
	{
		strncpy(netrty, pstr, ((sizeof(netrty)<strlen(pstr))?sizeof(netrty):strlen(pstr)));
	}

	net_upgrade_setenv("netretry", "2");

	if (CMD_RET_SUCCESS != run_command("dhcp", 0))
	{
		printf("[NetUpgrade] DHCP failed, and try to use the \"ipaddr\" which stored in env!\n");
		printf("[NetUpgrade] Get ipaddr \"%s\"\n",getenv("ipaddr"));
	}

	//write "netretry" back after upgrade have done
	//net_upgrade_setenv("netretry", netrty);
		
	netupgrade_ing = 0;
	
	return 0;
}


static char *get_netupdate_script_next_line(char **line_buf_ptr)
{
    char *line_buf;
    char *next_line;
    int i;

    line_buf = (*line_buf_ptr);

    // strip '\r', '\n' and comment
    while (1)
    {
        // strip '\r' & '\n'
        if (IS_LINE_END(line_buf[0]))
        {
            line_buf++;
        }
        // strip comment
        else if (IS_COMMENT(line_buf[0]))
        {
            for (i = 0; !IS_LINE_END(line_buf[0]) && i <= MAX_LINE_SIZE; i++)
            {
                line_buf++;
            }

            if (i > MAX_LINE_SIZE)
            {
                line_buf[0] = SCRIPT_FILE_END;

                printf ("Error: the max size of one line is %d!!!\n", MAX_LINE_SIZE); // <-@@@

                break;
            }
        }
        else
        {
            break;
        }
    }

    // get next line
    if (IS_FILE_END(line_buf[0]))
    {
        next_line = NULL;
    }
    else
    {
        next_line = line_buf;

        for (i = 0; !IS_LINE_END(line_buf[0]) && i <= MAX_LINE_SIZE; i++)
        {
            line_buf++;
        }

        if (i > MAX_LINE_SIZE)
        {
            next_line = NULL;

            printf ("Error: the max size of one line is %d!!!\n", MAX_LINE_SIZE); // <-@@@
        }
        else
        {
            line_buf[0] = '\0';
            *line_buf_ptr = line_buf + 1;
        }
    }

    printf("[andely][%s:%d]next_line=%s\n",__func__,__LINE__,next_line);
    return next_line;
}


int do_netupgrade_by_script(char * pfile)
{
    char cmdbuf[64];
    char * pscript_buf = NULL;
    char * pnext_line = NULL;
    char * pscript_back = NULL;

    if (NULL == pfile)
    {
        printf("[NetUpgrade] get UDP net_upgrade_script name is NULL\n");
        return -1;
    }

    printf("[NetUpgrade] get UDP net_upgrade_script name '%s'\n", pfile);

    pscript_buf = (char *)malloc(NETUPGRADE_SCRIPTS_LENGTH);
    if(NULL == pscript_buf)
    {
        UBOOT_ERROR("malloc fail \n");
        return -1;
    }

    // backup the pointer addr, because get_netupdate_script_next_line() will change it
    pscript_back = pscript_buf;

    memset(cmdbuf, 0, sizeof(cmdbuf));
    snprintf(cmdbuf, sizeof(cmdbuf), "tftp 0x%x mscript/%s", (unsigned int)pscript_buf, pfile);
    printf("[NetUpgrade] cmd: '%s'\n", cmdbuf);
    memset(pscript_buf, 0, NETUPGRADE_SCRIPTS_LENGTH);
    if (CMD_RET_SUCCESS != run_command(cmdbuf, 0))
    {
        printf("[NetUpgrade] get netupgrade script file fail!\n");
        free(pscript_buf);
        return -1;
    }

    while (NULL != (pnext_line = get_netupdate_script_next_line(&pscript_buf)))
    {
        UBOOT_INFO("\n>>> %s \n", pnext_line);

        if(run_command(pnext_line, 0) < 0)
        {
            UBOOT_ERROR("Error> cmd: %s Fail !!\n", pnext_line);
        }
    }

    free(pscript_back);

    return 0;
}


int do_net_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    size_t i = 0;
    char cmdbuf[64];
    char memaddr_udp[16];
    char * pfile_udp = NULL;
    char * pfile_env = NULL;
    char   version_udp[17];
    char * pversion_env = NULL;
    char * file_suffix = NULL;
    char   net_upgrade_state = 0;

    ts_1st = get_timer(0);
    printf("[NetUpgrade] ts_1st=0x%lx\n",ts_1st);

    // initial the global parameter
    delt_ts = 0;
    delt_ts_changed = 0;
    TftpParameraReady = 0;
    net_upgrade_state = 0;
    netupgrade_force_state = 0;
    pIpData = (struct ip_udp_hdr *)ip_data;
    pUdpData = (struct netupgrade_udp_data *)(ip_data + IP_UDP_HDR_SIZE);

    if (NetLoop(NETUPGRADE) < 0)
    {
        printf("[NetUpgrade] ==== NetLoop(NETUPGRADE) return fail ====!\n");
        return -1;
    }


    if(1 == TftpParameraReady)
    {
        net_upgrade_dhcp();

        printf("[NetUpgrade] Get UDP data from port[%d]:\n", ntohs(pIpData->udp_dst));
        memset(cmdbuf, '\0', sizeof(cmdbuf));
        sprintf(cmdbuf, "md.b 0x%p 0x%x", ip_data, 5*16);
        run_command (cmdbuf, 0);
        printf("\n");

        //get download loadaddr from UDP data
        memset(memaddr_udp, '\0', sizeof(memaddr_udp));
        sprintf(memaddr_udp, "0x%02x%02x%02x%02x",pUdpData->memaddr[3],
                pUdpData->memaddr[2],pUdpData->memaddr[1],pUdpData->memaddr[0]);
        load_addr = simple_strtoul(memaddr_udp, NULL, 16);
        printf("[NetUpgrade] Get load_addr=0x%lx\n",load_addr);

        //get download file version from UDP data
        memset(version_udp, '\0', sizeof(version_udp));
        memcpy(version_udp, pUdpData->version, sizeof(version_udp)-1);
        printf("[NetUpgrade] Get UDP net_upgrade_file version '%s'\n",version_udp);

        //get Auto NetUpgrade Tool IP
        NetCopyIP(&AutoNetUpgradeToolIP, &pIpData->ip_src);
        printf("[NetUpgrade] Get PC Tool IP(%pI4)\n",&AutoNetUpgradeToolIP);

        memset(BootFile, '\0', sizeof(BootFile));
        NetCopyIP(&NetServerIP, pUdpData->servIP);
        ip_to_string(NetServerIP, (char *)BootFile);
        net_upgrade_setenv("serverip", BootFile);
        printf("[NetUpgrade] Get tftpserver IP(%pI4)\n",&NetServerIP);
        strcat(BootFile, &pUdpData->flag);
        printf("[NetUpgrade] Get MacAddr from UDP package(%pM)\n",pUdpData->devmac);

        for (i=0; i<strlen(BootFile); i++)
        {
            if ((0x0a==BootFile[i])||(0x0d==BootFile[i]))
                BootFile[i] = '\0';
        }

        //get netupgrade file name from UDP data
        pfile_udp = strchr(BootFile, ':');
        if (pfile_udp == NULL)
        {
            pfile_udp = BootFile;
        }
        else
        {
            pfile_udp = pfile_udp + 1;
        }


        // check netupgrade status
        if ((0x00 != netupgrade_force_state) || \
            (NULL == (pversion_env=getenv(NETUPGRADE_VERSION))))
        {
            net_upgrade_state = 1;
        }
        else
        {
            printf("[NetUpgrade] Get ENV net_upgrade_file version '%s'\n",pversion_env);

            if (NULL == (pfile_env=getenv(NETUPGRADE_FILE)))
            {
                net_upgrade_state = 1;
            }
            else
            {
                printf("[NetUpgrade] get ENV net_upgrade_file name '%s'\n",pfile_env);

                if ((0x00 != netupgrade_force_state) || \
                    (0 != strncmp(pfile_udp, pfile_env, 64)) || \
                    (0 != strncmp(version_udp, pversion_env, 16)))
                {
                    net_upgrade_state = 1;
                }
                else
                {
                    printf("[NetUpgrade] The NetUpgrade file&version are the same!\n");
                }
            }
        }
    }


    if (netupgrade_force_state)
    {
        printf("[NetUpgrade] == Force NetUpgrade ==\n");
    }


    //get netupgrade file name suffix
    file_suffix = strchr(pfile_udp, '.');

    // check the netupgrade file is bin or script
    // if the suffix is not ".bin" or ".BIN", we do netupgrade as script file
    if ((0!=strncmp(file_suffix, ".bin", 4)) && (0!=strncmp(file_suffix, ".BIN", 4)))
    {
        // 1st: do netupdate by script file
	    if ((net_upgrade_state) || (netupgrade_force_state))
	    {
		    // backup the netupgrade file name
		    // because do_netupgrade_by_script() will change it
            memset(cmdbuf, '\0', sizeof(cmdbuf));
		    snprintf(cmdbuf, sizeof(cmdbuf), "%s", pfile_udp);

            if (-1 == do_netupgrade_by_script(pfile_udp))
            {
                printf("[NetUpgrade] do netupdate by script fail!\n");
                return -1;
            }

            printf("[NetUpgrade] do netupdate by script done!\n");

            setenv(NETUPGRADE_VERSION, (const char*)version_udp);
            setenv(NETUPGRADE_FILE, (const char*)cmdbuf);
            run_command("saveenv", 0);
            run_command("reset", 0);
        }

        return 0;
    }


    // 2nd: do netupdate by one.bin
    if ((net_upgrade_state) || (netupgrade_force_state))
    {
        progresse_status = 0;
        NetLoop(UPGRADESTATE);

        memset(cmdbuf, 0, sizeof(cmdbuf));
        snprintf(cmdbuf, sizeof(cmdbuf), "tftp 0x%x %s", (unsigned int)load_addr, pfile_udp);
        if (CMD_RET_SUCCESS != run_command(cmdbuf, 0))
        {
            printf("[NetUpgrade] ==== do TFTPGET fail ====! \n");
            net_set_state(NETLOOP_FAIL);
            return -1;
        }
        else
        {
            net_set_state(NETLOOP_SUCCESS);

            for (i=0; i<6; i++)
            {
                switch (i) {
                    case 0:
                        break;

                    case 1:
                        progresse_status = 30;
                        NetLoop(UPGRADESTATE);
                        break;

                    case 2:
                        progresse_status = 70;
                        NetLoop(UPGRADESTATE);
                        break;

                    case 3:
                        progresse_status = 90;
                        NetLoop(UPGRADESTATE);
                        break;

                    case 4:
                    case 5:
                    default:
                        break;
                }
                printf("[NetUpgrade] run cmd '%s' \n",net_upgrade_cmd[i]);
                run_command((const char*)net_upgrade_cmd[i], 0);
            }

            setenv(NETUPGRADE_VERSION, (const char*)version_udp);
            setenv(NETUPGRADE_FILE, (const char*)pfile_udp);

            setenv("netretry", (const char*)netrty);

            sprintf(cmdbuf,"serverip %pI4",&NetServerIP);
            run_command(cmdbuf, 0);

            sprintf(cmdbuf,"ipaddr %pI4",&NetOurIP);
            run_command(cmdbuf, 0);

            sprintf(cmdbuf,"macaddr %c%c %c%c %c%c %c%c %c%c %c%c",\
                    macaddr_env[0], macaddr_env[1], macaddr_env[3], macaddr_env[4],\
                    macaddr_env[6], macaddr_env[7], macaddr_env[9], macaddr_env[10],\
                    macaddr_env[12],macaddr_env[13],macaddr_env[15],macaddr_env[16]);
            run_command(cmdbuf, 0);

            run_command("saveenv", 0);

            progresse_status = 100;
            NetLoop(UPGRADESTATE);

            run_command("reset", 0);

            return 0;
        }

        return 0;
    }
    return 0;
}


U_BOOT_CMD(
     net_upgrade ,    CONFIG_SYS_MAXARGS,    1,     do_net_upgrade,
     "do net update from the specified file that is in tftpserver\n",
     "    -  \n"
);

#endif //#ifdef CONFIG_CMD_NETUPGRADE
