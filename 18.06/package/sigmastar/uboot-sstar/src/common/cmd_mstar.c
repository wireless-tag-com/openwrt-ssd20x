#include <common.h>
#include <command.h>
#include <malloc.h>
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"


#include <ubi_uboot.h>
#include <cmd_osd.h>
#include <../drivers/mstar/aesdma/drvAESDMA.h>

// Global Function
#define BUF_SIZE 4096
#define KERNEL_HEAD_SIZE 0x40
#define USBUPGRDE_SCRIPT_BUF_SIZE 16*1024

// Define (CUSTOMIZATION)
#define ENV_BOOTDELAY               "bootdelay"
#define ENV_IPADDR                  "ipaddr"
#define ENV_SERVERIP                "serverip"
#define ENV_PARTNO                  "partno"
#define ENV_USB_UPGRADEIMAGE               "UsbUpgradeImage"
#define ENV_SD_UPGRADEIMAGE               "SdUpgradeImage"
#define ENV_EMMC_UPGRADEIMAGE               "EmmcUpgradeImage"
#define ENV_USBUPGRADEPORT               "UpgradePort"

#define DEFAULT_BOOTDELAY           "1"                 // boot delay
#define DEFAULT_IPADDR              "192.168.0.28"      // target (debug) board IP
#define DEFAULT_SERVERIP            "192.168.0.26"      // tftp server IP
#define DEFAULT_SCRIPT_FILE_NAME    "auto_update.txt"   // script file name

#define SCRIPT_FILE_COMMENT         '#'                 // symbol for comment
#define SCRIPT_FILE_END             '%'                 // symbol for file end

#define ARG_NUM_BOOTDELAY           4
#define ARG_NUM_IPADDR              2
#define ARG_NUM_SERVERIP            3
#define ARG_NUM_SCRIPT_FILE         1

// Macro
#define MAX_LINE_SIZE       8000
#define IS_COMMENT(x)       (SCRIPT_FILE_COMMENT == (x))
#define IS_FILE_END(x)      (SCRIPT_FILE_END == (x))
#define IS_LINE_END(x)      ('\r' == (x)|| '\n' == (x))

#define IS_ARG_AVAILABLE_BOOTDELAY(x)   ((x) > ARG_NUM_BOOTDELAY)
#define IS_ARG_AVAILABLE_IPADDR(x)      ((x) > ARG_NUM_IPADDR)
#define IS_ARG_AVAILABLE_SERVERIP(x)    ((x) > ARG_NUM_SERVERIP)
#define IS_ARG_AVAILABLE_SCRIPT_FILE(x) ((x) > ARG_NUM_SCRIPT_FILE)

#define ARG_BOOTDELAY(x)                (x)[ARG_NUM_BOOTDELAY]
#define ARG_IPADDR(x)                   (x)[ARG_NUM_IPADDR]
#define ARG_SERVERIP(x)                 (x)[ARG_NUM_SERVERIP]
#define ARG_SCRIPT_FILE(x)              (x)[ARG_NUM_SCRIPT_FILE]

#define GET_REG8_ADDR(x, y)  	      (x+(y)*2-((y)&1))
#define GET_REG16_ADDR(x, y)  	      (x+(y)*4)
#define RIU_BASE_ADDR           0x1F000000
#define REG_ADDR_BASE_PM_GPIO         GET_REG8_ADDR( RIU_BASE_ADDR, 0x0F00 )

//typedef struct
//{
//    char name[16];
//    unsigned int start;
//    unsigned int length;
//    unsigned int aligned;
//    unsigned int flag;
//} SCFG_MEMP_entry;


// Local Function


//static char scfg_token_buf[256];
//static char _tmp_buf[256];
//static int  scfg_fpos=0;
//static int  scfg_fsize=0;
#if defined(CONFIG_MS_SDMMC)
static int  init_sdcard_flag=0;
#endif

#if defined(CONFIG_MS_USB)
static int  init_usb_flag=0;
#endif

extern U16 _getPanelWidth(void);
extern U16 _getPanelHeight(void);


//
//static int get_scfg_token(char* MemoryFile)
//{
//
//    char c;
//    int idx = 0;
//
//    memset(scfg_token_buf,0,sizeof(scfg_token_buf));
//
//    scfg_token_buf[0] = '\0';
//
//    while(scfg_fpos<scfg_fsize)
//    {
//        while(1)
//        {
//            c = MemoryFile[scfg_fpos++];
//            if(scfg_fpos>=scfg_fsize)
//            {
//                scfg_token_buf[idx] = '\0';
//                return -1;
//            }
//            if((c != ' ') && (c != '\n')&& (c != '\t'))
//            {
//                if(c == '\r')
//                {
//                    c = MemoryFile[scfg_fpos++];
//                    c = MemoryFile[scfg_fpos++];
//                    break;
//                }
//                else
//                {
//                    break;
//                }
//            }
//        }
//        idx = 0;
//        if(idx>=sizeof(scfg_token_buf))
//    {
//        return -1;
//    }
//        scfg_token_buf[idx++] = c;
//        c =MemoryFile[scfg_fpos++];
//        while(1)
//        {
//            if(scfg_fpos>=scfg_fsize)
//            {
//                scfg_token_buf[idx] = '\0';
//                return -1;
//            }
//            if((c == ' ') || (c == '\n')||(c == '\t'))
//            {
//                break;
//            }
//            if(c == '\r')
//            {
//                c = MemoryFile[scfg_fpos++];
//                if(c == '\n')
//                {
//                    break;
//                }
//            }
//            if(idx>=sizeof(scfg_token_buf))
//            {
//                return -1;
//            }
//            scfg_token_buf[idx++] = c;
//            c = MemoryFile[scfg_fpos++];
//        }
//        scfg_token_buf[idx] = '\0';
//        return 0;
//    }
//
//    return -1;
//}
//
//typedef struct
//{
//    char name[16];
//    unsigned int miu_num;
//    unsigned int reserv0[3];     //1st MEM_INFO line
//    unsigned int miu_interval[4];
//    unsigned int dram_size[4];   //2nd MEM_INFO line
//    unsigned int reserv1[8];     //3rd MEM_INFO line
//    unsigned int reserv2[8];     //4th MEM_INFO line
//
//}MS_MEMCFG_HEAD;
//
//void parseSCA_MMAP(char *memory_map_file, char *out, int fsize)
//{
//
//    unsigned int value = 0;
//    unsigned int index = 4; //the size of MS_MEMCFG_HEAD is 4 lines
//    unsigned int length = 0;
//    //int memory_type_total;
//    int enable_miu_1=0;
//
//    SCFG_MEMP_entry *pscfg_memp=((SCFG_MEMP_entry *)(void *)(out));
//    MS_MEMCFG_HEAD *pscfg_memp_head=((MS_MEMCFG_HEAD *)(void *)(out));
//
//    scfg_fpos=0;
//    scfg_fsize=fsize;
//
//
//
//    //reset whole MEMCFG to zero
//    memset((void *)(out),0,sizeof(SCFG_MEMP_entry)*SCFG_MEMP_COUNT);
//
//    memset(pscfg_memp_head,0,sizeof(MS_MEMCFG_HEAD));
//    memcpy(pscfg_memp_head->name,"MST_MEMCFG",sizeof("MST_MEMCFG"));
//
//
//    printf("\n");
//    int res;
//    while(1)
//    {
//
//        if((res=get_scfg_token(memory_map_file))==-1)
//        {
//            printf("EOF or stream error, parse finished!!\n");
//            goto PARSE_DONE;
//        }
//        if(strcmp(scfg_token_buf, "//##MMAP_TBL_BEGIN") == 0)
//        {
//            get_scfg_token(memory_map_file);
//            //memory_type_total=simple_strtoul(scfg_token_buf, NULL, 0);
//            while(1)
//            {
//                memset(pscfg_memp[index].name,0,sizeof(pscfg_memp[index].name));
//                if(index>=(SCFG_MEMP_COUNT-1))
//                {
//                    printf("reach SCFG_MEMP_COUNT=%d!\n",SCFG_MEMP_COUNT);
//                    goto PARSE_DONE;
//                }
//
//                memcpy(_tmp_buf,scfg_token_buf,sizeof(_tmp_buf));
//                get_scfg_token(memory_map_file);
//
//                if(strcmp(scfg_token_buf, "//@") == 0)
//                {
//
//                    length = strlen(_tmp_buf);
//                    *(_tmp_buf+(length-1)) = '\0';
//
//                    memset(pscfg_memp[index].name,0,sizeof(pscfg_memp[index].name));
//                    memcpy(pscfg_memp[index].name,_tmp_buf,length>(sizeof(pscfg_memp[index].name)-1) ? (sizeof(pscfg_memp[index].name)-1) : length );
//
//                    get_scfg_token(memory_map_file);
//                    value=simple_strtoul(scfg_token_buf, NULL, 0);
//                    pscfg_memp[index].start = value;
//
//                    get_scfg_token(memory_map_file);
//                    value=simple_strtoul(scfg_token_buf, NULL, 0);
//                    pscfg_memp[index].length = value;
//                    get_scfg_token(memory_map_file);
//                    value=simple_strtoul(scfg_token_buf, NULL, 0);
//                    pscfg_memp[index].aligned = value;
//
//                    printf("%s, 0x%08x, 0x%08x\n",pscfg_memp[index].name,pscfg_memp[index].start,pscfg_memp[index].length);
//                    index++;
//                  }
//                  else if(strcmp(scfg_token_buf, "MST_MMAP_ENTRY_COUNT,") == 0)
//                  {
//                      printf("parse finished!!\n");
//                      memset(pscfg_memp[index].name,0,sizeof(pscfg_memp[index].name));
//                      goto PARSE_DONE;
//                  }
//
//            }
//        }
//        else if(strcmp(scfg_token_buf, "#define") == 0)
//        {
//            get_scfg_token(memory_map_file);
//            if(strcmp(scfg_token_buf, "MIU_INTERVAL") == 0 || strcmp(scfg_token_buf, "MIU_INTERVAL0") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,16);
//                pscfg_memp_head->miu_interval[0]=value;
//            }
//            if(strcmp(scfg_token_buf, "MIU_INTERVAL1") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,16);
//                pscfg_memp_head->miu_interval[1]=value;
//            }
//            if(strcmp(scfg_token_buf, "MIU_INTERVAL2") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,16);
//                pscfg_memp_head->miu_interval[2]=value;
//            }
//            if(strcmp(scfg_token_buf, "MIU_INTERVAL3") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,16);
//                pscfg_memp_head->miu_interval[3]=value;
//            }
//            else if(strcmp(scfg_token_buf, "MIU_DRAM_LEN0") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,16);
//                pscfg_memp_head->dram_size[0]=value;
//            }
//            else if(strcmp(scfg_token_buf, "MIU_DRAM_LEN1") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,16);
//                pscfg_memp_head->dram_size[1] = value;
//            }
//            else if(strcmp(scfg_token_buf, "MIU_DRAM_LEN2") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,16);
//                pscfg_memp_head->dram_size[2] = value;
//            }
//            else if(strcmp(scfg_token_buf, "MIU_DRAM_LEN3") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,16);
//                pscfg_memp_head->dram_size[3] = value;
//            }
//            else if(strcmp(scfg_token_buf, "ENABLE_MIU_1") == 0)
//            {
//                get_scfg_token(memory_map_file);
//                value=simple_strtoul(scfg_token_buf,NULL,0);
//                if(value>0)
//                {
//                    enable_miu_1=1;
//                }
//            }
//        }
//    }
//
//PARSE_DONE:
//    {
//        int i=0;
//        pscfg_memp_head->miu_num=0;
//        for(i=0;i<(sizeof(pscfg_memp_head->miu_interval)/sizeof(pscfg_memp_head->miu_interval[0]));i++)
//        {
//            if(pscfg_memp_head->miu_interval[i]>0)
//            {
//                pscfg_memp_head->miu_num+=1;
//            }
//        }
//
//        if(enable_miu_1 && pscfg_memp_head->miu_num==1)
//        {
//            pscfg_memp_head->miu_num=2;
//            pscfg_memp_head->miu_interval[1]=pscfg_memp_head->miu_interval[0];
//        }
//    }
//    printf("[MEMCFG] miu_num: %d\n         miu_interval: 0x%08X 0x%08X 0x%08X 0x%08X\n         dram_size: 0x%08X 0x%08X 0x%08X 0x%08X\n",pscfg_memp_head->miu_num, pscfg_memp_head->miu_interval[0], pscfg_memp_head->miu_interval[1], pscfg_memp_head->miu_interval[2], pscfg_memp_head->miu_interval[3], pscfg_memp_head->dram_size[0], pscfg_memp_head->dram_size[1], pscfg_memp_head->dram_size[2], pscfg_memp_head->dram_size[3]);
//    printf("SCA_MMAP is parsed done at 0x%X\n",(unsigned int)out);
//
//
//    return;
//
//}


#if defined(CONFIG_MS_SDMMC) || defined(CONFIG_MS_USB) || defined(CONFIG_CMD_NET) || defined(CONFIG_MS_EMMC)
static char *get_script_next_line(char **line_buf_ptr)
{
    char *line_buf;
    char *next_line;
    int i = 0;

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

    return next_line;
}
#endif


#if defined(CONFIG_MS_SDMMC)

//load script from SD casd
int do_mstar (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char* buffer=NULL;
    buffer=(char *)malloc(BUF_SIZE);
    if(buffer==NULL)
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    // setenv (prelude)
    if (IS_ARG_AVAILABLE_BOOTDELAY(argc) || getenv(ENV_BOOTDELAY) == NULL)
    {
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "setenv %s %s", ENV_BOOTDELAY, IS_ARG_AVAILABLE_BOOTDELAY(argc) ? ARG_BOOTDELAY(argv) : DEFAULT_BOOTDELAY);
        run_command(buffer, 0); // run_command("setenv "ENV_BOOTDELAY" "DEFAULT_BOOTDELAY, 0);
    }
    if (IS_ARG_AVAILABLE_IPADDR(argc) || getenv(ENV_IPADDR) == NULL)
    {
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "setenv %s %s", ENV_IPADDR, IS_ARG_AVAILABLE_IPADDR(argc) ? ARG_IPADDR(argv) : DEFAULT_IPADDR);
        run_command(buffer, 0); // run_command("setenv "ENV_IPADDR" "DEFAULT_IPADDR, 0);
    }
    if (IS_ARG_AVAILABLE_SERVERIP(argc) || getenv(ENV_SERVERIP) == NULL)
    {
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "setenv %s %s", ENV_SERVERIP, IS_ARG_AVAILABLE_SERVERIP(argc) ? ARG_SERVERIP(argv) : DEFAULT_SERVERIP);
        run_command(buffer, 0); // run_command("setenv "ENV_SERVERIP" "DEFAULT_SERVERIP, 0);
    }

    // load & run script
    {
        char *script_buf;
        char *next_line;
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "tftp %X %s", (U32)buffer, IS_ARG_AVAILABLE_SCRIPT_FILE(argc) ? ARG_SCRIPT_FILE(argv) : DEFAULT_SCRIPT_FILE_NAME);
        run_command(buffer, 0); // run_command("tftp 80400000 "DEFAULT_SCRIPT_FILE_NAME, 0);
        script_buf = buffer;
        while ((next_line = get_script_next_line(&script_buf)) != NULL)
        {
            printf ("\n>> %s \n", next_line);
            run_command(next_line, 0);
        }
    }
    free(buffer);

    return 0;
}
U_BOOT_CMD(
    mstar,  CONFIG_SYS_MAXARGS,    1,    do_mstar,
    "script via TFTP",
    ""
);

extern int fat_register_device (block_dev_desc_t * dev_desc, int part_no);

//load script from SD casd
int do_dstar (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char* buffer=NULL;
    buffer=(char *)malloc(BUF_SIZE);
    if((buffer==NULL))

    {
        printf("no memory for command string!!\n");
        return -1;
    }
//    else//debug for memory leak
//    {
//        printf("    [MALLOC]@0x%X\n",buffer);
//    }


    // setenv (prelude)
    if (IS_ARG_AVAILABLE_BOOTDELAY(argc) || getenv(ENV_BOOTDELAY) == NULL)
    {
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "setenv %s %s", ENV_BOOTDELAY, IS_ARG_AVAILABLE_BOOTDELAY(argc) ? ARG_BOOTDELAY(argv) : DEFAULT_BOOTDELAY);
        run_command(buffer, 0); // run_command("setenv "ENV_BOOTDELAY" "DEFAULT_BOOTDELAY, 0);
    }

#ifdef CONFIG_MS_SHOW_LOGO
    if (!IS_ARG_AVAILABLE_SCRIPT_FILE(argc))
    {
        memset((void * )GOP_DISP_ADDR, 0x00, _getPanelWidth()*_getPanelHeight()*4);
        DrawProgressBar((_getPanelWidth() - PROGRESS_BAR_WIDTH) / 2,((_getPanelHeight() - PROGRESS_BAR_HEIGHT) / 5) - 5, 0, 0);

    }
#endif

    // load & run script
    {
        char *script_buf;
        char *next_line;

        if(init_sdcard_flag==0)
        {
            run_command("mmc rescan 0",0);
        }
        init_sdcard_flag=1;
//#if defined (ENABLE_USB_LAN_MODULE)
//    #if (ENABLE_MSTAR_TITANIA_BD_MST090F_C01A)        //should refine it later
//        run_command("usb start 1",0);
//    #else
//        run_command("usb start",0);
//    #endif
//#else
//        run_command("estart", 0);
//#endif
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "fatload mmc 0 %X %s", (U32)buffer, IS_ARG_AVAILABLE_SCRIPT_FILE(argc) ? ARG_SCRIPT_FILE(argv) : DEFAULT_SCRIPT_FILE_NAME);
        run_command(buffer, 0); // run_command("tftp 80400000 "DEFAULT_SCRIPT_FILE_NAME, 0);
        script_buf = buffer;
        U32 u32Percentage = 0; U32 u32Pos = 9; U32 u32TextColor = COLOR_GREEN;
        while ((next_line = get_script_next_line(&script_buf)) != NULL)
        {
            printf ("\n>> %s \n", next_line);


            if (strcmp(next_line,"dstar scripts/[[CIS") == 0)
            {
                u32Percentage = 5;
                UPDPrintLineSize("PROGRAMING CIS...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }
            else if (strcmp(next_line,"dstar scripts/set_partition") == 0)
            {
                u32Percentage = 10;
                UPDPrintLineSize("PROGRAMING set_partition...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }
            else if (strcmp(next_line,"dstar scripts/[[misc") == 0)
            {
                u32Percentage = 15;
                UPDPrintLineSize("PROGRAMING misc...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"dstar scripts/[[logo") == 0)
            {
                u32Percentage = 25;
                UPDPrintLineSize("PROGRAMING logo...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }
            else if (strcmp(next_line,"dstar scripts/[[recovery") == 0)
            {
                u32Percentage = 35;
                UPDPrintLineSize("PROGRAMING recovery...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"dstar scripts/[[boot") == 0)
            {
                u32Percentage = 45;
                UPDPrintLineSize("PROGRAMING boot...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"dstar scripts/[[system") == 0)
            {
                u32Percentage = 60;
                UPDPrintLineSize("PROGRAMING system...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }
            else if (strcmp(next_line,"dstar scripts/[[data") == 0)
            {
                u32Percentage = 75;
                UPDPrintLineSize("PROGRAMING data...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"dstar scripts/[[cache") == 0)
            {
                u32Percentage = 85;
                UPDPrintLineSize("PROGRAMING cache...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"dstar scripts/[[custcon") == 0)
            {
                u32Percentage = 90;
                UPDPrintLineSize("PROGRAMING custcon...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"dstar scripts/[[pqbin") == 0)
            {
                u32Percentage = 95;
                UPDPrintLineSize("PROGRAMING pqbin...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"dstar scripts/[[config") == 0)
            {
                u32Percentage = 100;
                UPDPrintLineSize("PROGRAMING config...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }

            run_command(next_line, 0);

#ifdef CONFIG_MS_SHOW_LOGO
            if (u32Percentage != 0)
            {
                DrawProgressBar((_getPanelWidth() - PROGRESS_BAR_WIDTH) / 2,((_getPanelHeight() - PROGRESS_BAR_HEIGHT) / 5) - 5,u32Percentage, 0);
                //sprintf(buf_percent, "%d", u32Percentage);
                //UPDPrintLineSize(buf_percent, COLOR_WHITE, 2, POS_CENTER, 6);
            }
#endif
            if (u32Percentage == 100)
            {
                UPDPrintLineSize("PROGRAM Done", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }

        }
    }
    free(buffer);

    return 0;
}
U_BOOT_CMD(
    dstar,  CONFIG_SYS_MAXARGS,    1,    do_dstar,
    "script via SD/MMC",
    ""
);

int do_sdstar     (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char* buffer=NULL;
    char* UpgradeImage=NULL;
    char *script_buf;
    char *next_line;
    char tmp[64];
    int ret = -1;
#if defined(CONFIG_SSTAR_UPGRADE_UI)
    int total_cmd = 0;
    int cur_cmd = 0;
    char* buffer_caculate=NULL;
#endif
    buffer=(char *)malloc(USBUPGRDE_SCRIPT_BUF_SIZE);
    if((buffer==NULL))
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    memset(buffer, 0 , USBUPGRDE_SCRIPT_BUF_SIZE);
    UpgradeImage = getenv(ENV_SD_UPGRADEIMAGE);
    if(UpgradeImage == NULL)
    {
        printf("UpgradeImage env is null,use default SigmastarUpgradeSD.bin\n");
        UpgradeImage = "SigmastarUpgradeSD.bin";
        run_command("setenv SdUpgradeImage SigmastarUpgradeSD.bin", 0);
        run_command("saveenv",0);

    }
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"fatload mmc 0 %X %s 0x%x 0x0", (U32)buffer, UpgradeImage, USBUPGRDE_SCRIPT_BUF_SIZE);
    run_command(tmp, 0);    
#if defined(CONFIG_SSTAR_UPGRADE_UI)
    buffer_caculate = (char *)malloc(USBUPGRDE_SCRIPT_BUF_SIZE);
    if((buffer_caculate == NULL))
    {
        printf("no memory for command string!!\n");
        return -1;
    }
    memcpy(buffer_caculate, buffer, USBUPGRDE_SCRIPT_BUF_SIZE);
    script_buf = buffer_caculate;
    while (get_script_next_line(&script_buf) != NULL)
    {
        total_cmd++;
    }
    free(buffer_caculate);
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"dcache on");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"bootlogo 1 0 0 0 0");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp)); 
    snprintf(tmp, sizeof(tmp) - 1,"bootframebuffer bar 0");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"dcache off");
    run_command(tmp, 0);
#endif
    script_buf = buffer;
    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
        run_command(next_line, 0);
#if defined(CONFIG_SSTAR_UPGRADE_UI)
        cur_cmd++;
        memset(tmp,0,sizeof(tmp));
        snprintf(tmp, sizeof(tmp) - 1,"dcache on");
        run_command(tmp, 0);
        memset(tmp,0,sizeof(tmp));
        snprintf(tmp, sizeof(tmp) - 1,"bootframebuffer bar %d", cur_cmd * 100 / total_cmd);
        run_command(tmp, 0);
        memset(tmp,0,sizeof(tmp));
        snprintf(tmp, sizeof(tmp) - 1,"dcache off");
        run_command(tmp, 0);
#endif
    }
#if defined(CONFIG_SSTAR_UPGRADE_UI)
    memset(tmp,0,sizeof(tmp));    
    snprintf(tmp, sizeof(tmp) - 1,"dcache on");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp));    
    snprintf(tmp, sizeof(tmp) - 1, "bootframebuffer bar 100");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp));    
    snprintf(tmp, sizeof(tmp) - 1,"dcache off");
    run_command(tmp, 0);
#endif

    free(buffer);
    return ret;

}

U_BOOT_CMD(
    sdstar,  CONFIG_SYS_MAXARGS,    1,    do_sdstar,
    "script via sd package",
    ""
);


#endif

#if defined(CONFIG_MS_USB)
int do_usbstar     (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char* buffer=NULL;
    char* UpgradeImage=NULL;
    char* UpgradePort=NULL;
    char *script_buf;
    char *next_line;
    char tmp[64];
    int ret = -1;
#if defined(CONFIG_SSTAR_UPGRADE_UI)
    int total_cmd = 0;
    int cur_cmd = 0;
    char* buffer_caculate=NULL;
#endif

    buffer=(char *)malloc(USBUPGRDE_SCRIPT_BUF_SIZE);
    if((buffer==NULL))
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    memset(buffer, 0 , USBUPGRDE_SCRIPT_BUF_SIZE);
    UpgradeImage = getenv(ENV_USB_UPGRADEIMAGE);
    if(UpgradeImage == NULL)
    {
        printf("UpgradeImage env is null,use default SigmastarUpgrade.bin\n");
        UpgradeImage = "SigmastarUpgrade.bin";
        run_command("setenv UpgradeImage SigmastarUpgrade.bin", 0);
        run_command("saveenv",0);

    }
    //check usb init
    if(init_usb_flag==0)
    {
        
        UpgradePort = getenv(ENV_USBUPGRADEPORT);
        if(UpgradePort == NULL)
        {
            run_command("usb start 0", 0);
        }
        else
        {
            snprintf(tmp, sizeof(tmp) - 1,"usb start %s", UpgradePort);
            run_command(tmp, 0);
        }

        init_usb_flag=1;
    }
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"fatload usb 0 %X %s 0x4000 0x0", (U32)buffer, UpgradeImage);
    run_command(tmp, 0);
#if defined(CONFIG_SSTAR_UPGRADE_UI)
    buffer_caculate = (char *)malloc(USBUPGRDE_SCRIPT_BUF_SIZE);
    if((buffer_caculate == NULL))
    {
        printf("no memory for command string!!\n");
        return -1;
    }
    memcpy(buffer_caculate, buffer, USBUPGRDE_SCRIPT_BUF_SIZE);
    script_buf = buffer_caculate;
    while (get_script_next_line(&script_buf) != NULL)
    {
        total_cmd++;
    }
    free(buffer_caculate);
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"dcache on");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"bootlogo 1 0 0 0 0");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp)); 
    snprintf(tmp, sizeof(tmp) - 1,"bootframebuffer bar 0");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"dcache off");
    run_command(tmp, 0);
#endif
    script_buf = buffer;
    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
#ifndef CONFIG_SSTAR_UPGRADE_UI
        run_command(next_line, 0);
#else
        if (strcmp(next_line,"reset") != 0)
        {
            run_command(next_line, 0);
        }
        else
        {
            printf("because UPGRADE UI ON,skip reset command line,wait UI show OK,then reset \n");
        }
        cur_cmd++;
        memset(tmp,0,sizeof(tmp));
        snprintf(tmp, sizeof(tmp) - 1,"dcache on");
        run_command(tmp, 0);
        memset(tmp,0,sizeof(tmp));
        snprintf(tmp, sizeof(tmp) - 1,"bootframebuffer bar %d", cur_cmd * 100 / total_cmd);
        run_command(tmp, 0);
        memset(tmp,0,sizeof(tmp));
        snprintf(tmp, sizeof(tmp) - 1,"dcache off");
        run_command(tmp, 0);
#endif
    }
#if defined(CONFIG_SSTAR_UPGRADE_UI)
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"dcache on");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"bootframebuffer bar 100");
    run_command(tmp, 0);
    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"dcache off");
    run_command(tmp, 0);
    udelay(4*1000*1000);
    run_command("reset", 0);
#endif
    free(buffer);
    init_usb_flag=0;
    return ret;
}

int do_ustar (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char* buffer=NULL;
    int ret = -1;

    buffer=(char *)malloc(BUF_SIZE);
    if((buffer==NULL))

    {
        printf("no memory for command string!!\n");
        return -1;
    }
//    else//debug for memory leak
//    {
//        printf("    [MALLOC]@0x%X\n",buffer);
//    }

    // setenv (prelude)
    if (IS_ARG_AVAILABLE_BOOTDELAY(argc) || getenv(ENV_BOOTDELAY) == NULL)
    {
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "setenv %s %s", ENV_BOOTDELAY, IS_ARG_AVAILABLE_BOOTDELAY(argc) ? ARG_BOOTDELAY(argv) : DEFAULT_BOOTDELAY);
        run_command(buffer, 0); // run_command("setenv "ENV_BOOTDELAY" "DEFAULT_BOOTDELAY, 0);
    }


    if (!IS_ARG_AVAILABLE_SCRIPT_FILE(argc))
    {
#ifdef CONFIG_MS_SHOW_LOGO
        memset((void * )GOP_DISP_ADDR, 0x00, _getPanelWidth()*_getPanelHeight()*4);
        DrawProgressBar((_getPanelWidth() - PROGRESS_BAR_WIDTH) / 2,((_getPanelHeight() - PROGRESS_BAR_HEIGHT) / 5) - 5, 0, 0);
#endif
    }


    // load & run script
    {
        char *script_buf;
        char *next_line;

        if(init_usb_flag==0)
        {
            #if defined(CONFIG_ARCH_CEDRIC)
            run_command("usb start 2",0);
            #elif  defined(CONFIG_ARCH_CHICAGO)
            run_command("usb start",0);
            #endif
        }
        init_usb_flag=1;
//#if defined (ENABLE_USB_LAN_MODULE)
//    #if (ENABLE_MSTAR_TITANIA_BD_MST090F_C01A)        //should refine it later
//        run_command("usb start 1",0);
//    #else
//        run_command("usb start",0);
//    #endif
//#else
//        run_command("estart", 0);
//#endif
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "fatload usb 0 %X %s", (U32)buffer, IS_ARG_AVAILABLE_SCRIPT_FILE(argc) ? ARG_SCRIPT_FILE(argv) : DEFAULT_SCRIPT_FILE_NAME);
        run_command(buffer, 0); // run_command("tftp 80400000 "DEFAULT_SCRIPT_FILE_NAME, 0);
        script_buf = buffer;
        U32 u32Percentage = 0; U32 u32Pos = 9; U32 u32TextColor = COLOR_GREEN;
        while ((next_line = get_script_next_line(&script_buf)) != NULL)
        {
            printf ("\n>> %s \n", next_line);
            ret = 0;//at least one cmd

            if (strcmp(next_line,"ustar scripts/[[CIS") == 0)
            {
                u32Percentage = 5;
                UPDPrintLineSize("PROGRAMING CIS...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }
            else if (strcmp(next_line,"ustar scripts/set_partition") == 0)
            {
                u32Percentage = 10;
                UPDPrintLineSize("PROGRAMING set_partition...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }
            else if (strcmp(next_line,"ustar scripts/[[misc") == 0)
            {
                u32Percentage = 15;
                UPDPrintLineSize("PROGRAMING misc...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"ustar scripts/[[logo") == 0)
            {
                u32Percentage = 25;
                UPDPrintLineSize("PROGRAMING logo...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }
            else if (strcmp(next_line,"ustar scripts/[[recovery") == 0)
            {
                u32Percentage = 35;
                UPDPrintLineSize("PROGRAMING recovery...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"ustar scripts/[[boot") == 0)
            {
                u32Percentage = 45;
                UPDPrintLineSize("PROGRAMING boot...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"ustar scripts/[[system") == 0)
            {
                u32Percentage = 60;
                UPDPrintLineSize("PROGRAMING system...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }
            else if (strcmp(next_line,"ustar scripts/[[data") == 0)
            {
                u32Percentage = 75;
                UPDPrintLineSize("PROGRAMING data...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"ustar scripts/[[cache") == 0)
            {
                u32Percentage = 85;
                UPDPrintLineSize("PROGRAMING cache...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"ustar scripts/[[custcon") == 0)
            {
                u32Percentage = 90;
                UPDPrintLineSize("PROGRAMING custcon...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"ustar scripts/[[pqbin") == 0)
            {
                u32Percentage = 95;
                UPDPrintLineSize("PROGRAMING pqbin...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;

            }
            else if (strcmp(next_line,"ustar scripts/[[config") == 0)
            {
                u32Percentage = 100;
                UPDPrintLineSize("PROGRAMING config...", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }

            run_command(next_line, 0);


            if (u32Percentage != 0)
            {
#ifdef CONFIG_MS_SHOW_LOGO
                DrawProgressBar((_getPanelWidth() - PROGRESS_BAR_WIDTH) / 2,((_getPanelHeight() - PROGRESS_BAR_HEIGHT) / 5) - 5,u32Percentage, 0);
                //sprintf(buf_percent, "%d", u32Percentage);
                //UPDPrintLineSize(buf_percent, COLOR_WHITE, 2, POS_CENTER, 6);
#endif
            }

            if (u32Percentage == 100)
            {
                UPDPrintLineSize("PROGRAM Done", u32TextColor, 2, POS_CENTER, u32Pos);
                u32Pos+=1;
            }

        }
    }
    free(buffer);

    return ret;
}
U_BOOT_CMD(
    ustar,  CONFIG_SYS_MAXARGS,    1,    do_ustar,
    "script via USB",
    ""
);
U_BOOT_CMD(
    usbstar,  CONFIG_SYS_MAXARGS,    1,    do_usbstar,
    "script via USB package",
    ""
);

#endif

#ifdef CONFIG_CMD_NET
int do_estar (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char* buffer=NULL;
    int ret = -1;

    buffer=(char *)malloc(BUF_SIZE);
    if((buffer==NULL))

    {
        printf("no memory for command string!!\n");
        return -1;
    }
//    else//debug for memory leak
//    {
//        printf("    [MALLOC]@0x%X\n",buffer);
//    }


    // setenv (prelude)
    if (IS_ARG_AVAILABLE_BOOTDELAY(argc) || getenv(ENV_BOOTDELAY) == NULL)
    {
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "setenv %s %s", ENV_BOOTDELAY, IS_ARG_AVAILABLE_BOOTDELAY(argc) ? ARG_BOOTDELAY(argv) : DEFAULT_BOOTDELAY);
        run_command(buffer, 0); // run_command("setenv "ENV_BOOTDELAY" "DEFAULT_BOOTDELAY, 0);
    }


    if (!IS_ARG_AVAILABLE_SCRIPT_FILE(argc))
    {
#ifdef CONFIG_MS_SHOW_LOGO
        memset((void * )GOP_DISP_ADDR, 0x00, _getPanelWidth()*_getPanelHeight()*4);
        DrawProgressBar((_getPanelWidth() - PROGRESS_BAR_WIDTH) / 2,((_getPanelHeight() - PROGRESS_BAR_HEIGHT) / 5) - 5, 0, 0);
#endif
    }


    // load & run script
    {
        char *script_buf;
        char *next_line;

//        if(init_sdcard_flag==0)
//        {
//            #if defined(CONFIG_ARCH_CEDRIC)
//            run_command("usb start 2",0);
//            #elif  defined(CONFIG_ARCH_CHICAGO)
//            run_command("usb start",0);
//            #endif
//        }
//        init_sdcard_flag=1;
//#if defined (ENABLE_USB_LAN_MODULE)
//    #if (ENABLE_MSTAR_TITANIA_BD_MST090F_C01A)        //should refine it later
//        run_command("usb start 1",0);
//    #else
//        run_command("usb start",0);
//    #endif
//#else
//        run_command("estart", 0);
//#endif
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "tftp %X %s", (U32)buffer, IS_ARG_AVAILABLE_SCRIPT_FILE(argc) ? ARG_SCRIPT_FILE(argv) : DEFAULT_SCRIPT_FILE_NAME);
        run_command(buffer, 0); // run_command("tftp 80400000 "DEFAULT_SCRIPT_FILE_NAME, 0);
        script_buf = buffer;
//        U32 u32Percentage = 0; U32 u32Pos = 9; U32 u32TextColor = COLOR_GREEN;
        while ((next_line = get_script_next_line(&script_buf)) != NULL)
        {
            printf ("\n>> %s \n", next_line);
            ret = 0;//at least one cmd

            // if any cmd failed, stop execute the script
            if(run_command(next_line, 0))
            {
                ret = -1;
                break;
            }
        }
    }
    free(buffer);

    return ret;
}
U_BOOT_CMD(
    estar,  CONFIG_SYS_MAXARGS,    1,    do_estar,
    "script via network",
    ""
);

#endif


//int do_scfgenv (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
//{
//    char buf[0x40];
//
//
//    printf("updating SCFG environment variable...\n");
//    memset(buf,0,sizeof(buf));
//    sprintf(buf, "0x%X",SCFG_MEMP_START);
//    printf("memcfg: %s\r\n",buf);
//    setenv("memcfg", buf);
//
//    memset(buf,0,sizeof(buf));
//    sprintf(buf, "0x%X@0x%X",SCFG_PNLP_SIZE,SCFG_PNLP_START);
//    printf("pnlcfg: %s\r\n",buf);
//    setenv("pnlcfg", buf);
//
//    memset(buf,0,sizeof(buf));
//    sprintf(buf, "0x%X",SCFG_START);
//    setenv("SCFG_START", buf);
//
//    memset(buf,0,sizeof(buf));
//    sprintf(buf, "0x%X",SCFG_LEN);
//    setenv("SCFG_LEN", buf);
//
//    return 0;
//
//}


//int do_parseSCA_MMAP (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
//{
//    char *in=NULL;
//    char *out=NULL;
//
//    int stream_size=0x100000;
//
//    if(argc<3)
//    {
//        printf("invalid parameter!!\n");
//        return -1;
//    }
//
//    in=(char *)((void *)simple_strtoul(argv[1], NULL, 0));
//    out=(char *)((void *)simple_strtoul(argv[2], NULL, 0));
//
//    if(argc>3)
//    {
//        stream_size=simple_strtoul(argv[3], NULL, 0);
//    }
//
//    parseSCA_MMAP(in, out, stream_size);
//
//    return 0;
//
//}
//
//U_BOOT_CMD(
//    scfgenv,  CONFIG_SYS_MAXARGS,    1,    do_scfgenv,
//    "scfgenv",
//    ""
//);
//
//U_BOOT_CMD(
//        parseSCA_MMAP,  CONFIG_SYS_MAXARGS,    1,    do_parseSCA_MMAP,
//    "parseSCA_MMAP",
//    ""
//);

#if defined(CONFIG_CMD_SPINAND_CIS)
#include "../drivers/mstar/spinand/inc/common/spinand.h"
extern void nand_init(void);
extern int MDrv_SPINAND_WriteCIS_for_ROM(SPINAND_FLASH_INFO_TAG_t *pSpiNandInfoTagOut, int nCopies);
extern int MDrv_SPINAND_GetMtdParts(char *buf);
extern int MDrv_SPINAND_SearchCIS_in_DRAM(U8 *pu8_CISAddr, U8 *pu8_PNIAddr, SPINAND_FLASH_INFO_TAG_t *pSpiNandInfoTagOut);
extern int MDrv_SPINAND_ReadCISBlk(U8* pu8_DataBuf);
extern U8 MDrv_SPINAND_WB_DumpBBM(U8 *u8Data);
extern U8 MDrv_SPINAND_WB_BBM(U32 LBA, U32 PBA);

int writeSpinandCIS(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#define UNDEFINED_PBA ((U32)-1)
    U32 u32_CISAddr = 0;
    U32 u32_partInfo = 0;
    int SPINAND_SUCCESS = 0;
    U32 u32_ret = 0;
    U32 u32BL0Pba = UNDEFINED_PBA;
    U32 u32BL1Pba = UNDEFINED_PBA;
    U32 u32UbootPba = UNDEFINED_PBA;
    int nCopies = CIS_DEFAULT_BACKUP;
    SPINAND_FLASH_INFO_TAG_t stSpiNandInfoTagOut;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    U8 *auInfo = (U8*)&stSpiNandInfoTagOut.tSpiNandInfo;
    pSpiNandDrv->pu8_statusbuf = kmalloc(16, GFP_KERNEL);

    /*Read Device ID*/
    spi_nand_debug("Write CIS");
    spi_nand_debug("Board_nand_init");

    if(MDrv_SPINAND_Init(&(pSpiNandDrv->tSpinandInfo)) != TRUE)
    {
        spi_nand_debug("Init fail");
    }

    if(argc > 1)
        u32_CISAddr = simple_strtoul(argv[1], NULL, 0);
    if(argc > 2)
        u32_partInfo = simple_strtoul(argv[2], NULL, 0);
    if(argc > 3)
        u32BL0Pba = simple_strtoul(argv[3], NULL, 0);
    if(argc > 4)
        u32BL1Pba = simple_strtoul(argv[4], NULL, 0);
    if(argc > 5)
        u32UbootPba = simple_strtoul(argv[5], NULL, 0);
    if(argc > 6)
        nCopies = simple_strtoul(argv[6], NULL, 0);

    if(MDrv_SPINAND_SearchCIS_in_DRAM((u8 *)u32_CISAddr, (u8 *)u32_partInfo, &stSpiNandInfoTagOut) != SPINAND_SUCCESS)
    {
        spi_nand_err("SearchCIS_in_DRAM fail");
        return -1;
    }

    if(u32BL0Pba != UNDEFINED_PBA)
        /*stSpiNandInfoTagOut.tSpiNandInfo.u8_BL0PBA*/auInfo[0x1f] = (U8)u32BL0Pba;
    if(u32BL1Pba != UNDEFINED_PBA)
        /*stSpiNandInfoTagOut.tSpiNandInfo.u8_BL1PBA*/auInfo[0x20] = (U8)u32BL1Pba;
    if(u32UbootPba != UNDEFINED_PBA)
        /*stSpiNandInfoTagOut.tSpiNandInfo.u8_UBOOTPBA*/ auInfo[0x1e] = (U8)u32UbootPba;
    if(nCopies < 1 || nCopies > 5)
    {
        spi_nand_err("WriteCIS number of copies must be 1~5.");
        u32_ret = -1;
        return u32_ret;
    }

    pSpiNandDrv->pu8_pagebuf = kmalloc(pSpiNandDrv->tSpinandInfo.u16_PageByteCnt, GFP_KERNEL);
    pSpiNandDrv->pu8_sparebuf = kmalloc(pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt, GFP_KERNEL);

    if(MDrv_SPINAND_WriteCIS_for_ROM(&stSpiNandInfoTagOut, nCopies) != ERR_SPINAND_SUCCESS)
    {
        spi_nand_err("WriteCIS_for_ROM fail");
        u32_ret = -1;
        return u32_ret;
    }

    if( u32_ret==0 )
    {
        char *buf;
        buf = (char*) malloc (0x200 * sizeof(char));

        if(!buf)
        {
            spi_nand_err("Malloc fail for mtd string buffer\n");
            return -1;
        }

        nand_init(); // re init so nand0 can be found
        spi_nand_msg("Updating spinand mtdparts...");

        if(MDrv_SPINAND_GetMtdParts(buf) != SPINAND_SUCCESS)
        {
            spi_nand_err("GetMtdParts fail");
            return -1;
        }
        spi_nand_msg("%s",buf);
        setenv("mtdparts", buf);
        free(buf);
    }
    spi_nand_msg("Write CIS success!!\n");
    return 0;
}

U_BOOT_CMD(
    writecis,  CONFIG_SYS_MAXARGS,    1,    writeSpinandCIS,
    "Search CIS in dram then write to spinand.",
    "0xSNI_ADDR 0xPNI_ADDR [BL0_PBA [BL1_PBA [UBOOT_PBA [COPIES]]]]"
);


int do_readcisblk(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    void *addr;

	if(argc < 2)
		return -1;
	addr = (void *)simple_strtoul(argv[1], NULL, 16);

	MDrv_SPINAND_ReadCISBlk(addr);

    return 0;
}

U_BOOT_CMD(
    readcis,  CONFIG_SYS_MAXARGS,    1,    do_readcisblk,
    "Read cis block content",
    ""
);

int do_readBBM(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    void *addr;
    int i = 0;
    int bbmSize = 0x40;
       int status = 0;
    if(argc < 2)
        return -1;
    addr = (void *)simple_strtoul(argv[1], NULL, bbmSize);
    MDrv_SPINAND_WB_DumpBBM(addr);

    for(i=0; i < bbmSize; i+=2)
        printf("addr: %x\r\n", *((U16*)(addr+i)));
    
    /*Check bbm lut is full ?*/
    MDrv_SPINAND_ReadStatusRegister((U8*)&status, SPI_NAND_REG_STAT);
    printf("0xc0 status %x\r\n", status);

                              if(status & LUT_FULL)
                                printf("[WB bbm] LUT FULL, can't swap  !!!\r\n");
    
    return 0;
}

U_BOOT_CMD(
    readbbm,  CONFIG_SYS_MAXARGS,    1,    do_readBBM,
    "Read BBM table",
    ""
);

int do_swapBlock(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    U32 LBA;
    U32 PBA;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();

    if(argc < 3)
        return -1;
    LBA = simple_strtoul(argv[1], NULL, 0);
    PBA = simple_strtoul(argv[2], NULL, 0);

    if((LBA >= pSpiNandDrv->tSpinandInfo.u16_BlkCnt) || (PBA >= pSpiNandDrv->tSpinandInfo.u16_BlkCnt))
    {
        printf("LBA or PBA over block size 0x%x\r\n", pSpiNandDrv->tSpinandInfo.u16_BlkCnt);
        return -1;
    }

    MDrv_SPINAND_WB_BBM(LBA, PBA);
    return 0;
}

U_BOOT_CMD(
    do_bbm,  CONFIG_SYS_MAXARGS,    1,    do_swapBlock,
    "Swap block (winbond only), #do_bbm LBA PBA",
    ""
);

int do_ECC(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    U8 bInternalECC;
    U8 u8Status;

    if(argc < 2)
        return -1;

    bInternalECC = simple_strtoul(argv[1], NULL, 0);

    HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
    u8Status &= 0xEF;
    u8Status |= (bInternalECC<<4);
    HAL_SPINAND_WriteStatusRegister(u8Status, SPI_NAND_REG_FEAT);
    u8Status = 0x77;
    HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
    printf("after ECC engine: 0x%x \r\n", u8Status);
    return 0;
}

U_BOOT_CMD(
    do_ECC,  CONFIG_SYS_MAXARGS,    1,    do_ECC,
    "Enable or disable internal ecc. #do_ECC 0 (disable), do_ECC (enable)",
    ""
);

#endif


#if defined(CONFIG_CMD_CIS)
extern U32 drvNAND_WriteCIS_for_ROM_2(U8 *pu8_CISData);
extern U8* drvNAND_SearchNandInfo(U8 *pu8_NandInfoArray, U32 u32_ArraySize);
extern U32 drvNAND_Init(void);
extern void nand_init(void);
extern void drvNAND_GetMtdParts(char *buf);
extern int drvNAND_ReadCISBlk(U8* pu8_DataBuf);

#define MAX_NAND_INFO_ARRAY_SIZE 0x100000 //1MB

int do_cis(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    U32 nand_info=0;
    U32 part_info=0;
    U8 *cis_info=NULL;
    U8 *pu8nand_info=NULL;
    int result=0;

    printf("do_cis\r\n");

    drvNAND_Init();

    nand_info=simple_strtoul(argv[1], NULL, 0);
    part_info=simple_strtoul(argv[2], NULL, 0);

    pu8nand_info=drvNAND_SearchNandInfo((U8 *)nand_info,MAX_NAND_INFO_ARRAY_SIZE);


    //while(1);

    if(pu8nand_info==NULL)
    {
        printf("can not find correct NANDINFO, CIS failed...\n");
        return -1;
    }
    printf("NANDINFO found...\n");

    cis_info=(U8 *)malloc(1024);
    if(cis_info==NULL)
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    memcpy(cis_info,pu8nand_info,512);
    memcpy((cis_info+512),(U8 *)part_info,512);

    //if(drvNAND_WriteCIS(cis_info)!=0)
    if(drvNAND_WriteCIS_for_ROM_2(cis_info)!=0)
    {
        printf ("CIS failed!!\n");
        result=-1;
    }

    free(cis_info);

    if(result==0)
    {
        char *buf;
        buf = (char*) malloc (0x200 * sizeof(char));
        if(!buf)
        {
            printf("Malloc fail for mtd string buffer\n");
            return -1;
        }

        nand_init(); // re init so nand0 can be found
        printf("updating mtdparts...\n");
        drvNAND_GetMtdParts(buf);
        printf("%s\r\n",buf);
        setenv("mtdparts", buf);

        free(buf);
        printf("CIS success!!\n");
    }
    return result;

}

U_BOOT_CMD(
    cis,  CONFIG_SYS_MAXARGS,    1,    do_cis,
    "write CIS",
    ""
);

int do_readcisblk(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    void *addr;

	if(argc < 2)
		return -1;
	addr = (void *)simple_strtoul(argv[1], NULL, 16);

	drvNAND_ReadCISBlk(addr);

    return 0;
}

U_BOOT_CMD(
    readcis,  CONFIG_SYS_MAXARGS,    1,    do_readcisblk,
    "Read cis block content",
    ""
);

int do_checkbackup(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    U8 u8MagicData[] = {0x6B, 0x62, 0x61, 0x6B};
    /*Check bakup flag*/
    if(memcmp((const void *) (MS_BASE_REG_IMI_PA + 0x15000), (const void *) u8MagicData, sizeof(u8MagicData)) != 0)
    {
        /*disable bak*/
        setenv("kparts", "KERNEL");
        setenv("kbackup", "RECOVERY");
        puts("KERNEL: FROM NORMAL \n");
    }
    else
    {
        /*enable bak*/
        setenv("kparts", "RECOVERY");
        setenv("kbackup", "KERNEL");
        puts("KERNEL: FROM BACKUP \n");
    }
    return 0;
}

U_BOOT_CMD(
    checkBackup,  CONFIG_SYS_MAXARGS,    1,    do_checkbackup,
    "Check kernel backup flag in IMI",
    ""
);
#endif


#if defined(CONFIG_MS_SDMMC)
extern long do_fat_read_fstart (const char *filename, void *buffer, unsigned long maxsize, unsigned long fstart);
extern long do_fat_fsize (const char *filename, unsigned long *fsize);
int do_fat_fsread (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    long size;
    unsigned long offset;
    unsigned long count;
    unsigned long fstart;
    char buf [12];
    block_dev_desc_t *dev_desc=NULL;
    int dev=0;
    int part=1;
    char *ep;
    char *pfst;

    if (argc < 5) {
        printf( "usage: fatread <interface> <dev[:part]> "
            "<addr> <filename> [bytes] \n");
        return 1;
    }

    pfst=getenv("FSTART");

    if(pfst==NULL)
    {
        fstart=0;
        sprintf(buf, "%lX", fstart);
        setenv("FSTART",buf);
    }
    else
    {
        fstart = (int)simple_strtoul(pfst, NULL, 16);
    }




    dev = (int)simple_strtoul(argv[2], &ep, 16);
    dev_desc = get_dev(argv[1],dev);
    if (dev_desc == NULL) {
        puts("\n** Invalid boot device **\n");
        return 1;
    }
    if (*ep) {
        if (*ep != ':') {
            puts("\n** Invalid boot device, use `dev[:part]' **\n");
            return 1;
        }
        part = (int)simple_strtoul(++ep, NULL, 16);
    }
    if (fat_register_device(dev_desc,part)!=0) {
        printf("\n** Unable to use %s %d:%d for fatload **\n",
            argv[1], dev, part);
        return 1;
    }

    offset = simple_strtoul(argv[3], NULL, 16);

    if (argc == 6)
        count = simple_strtoul(argv[5], NULL, 16);
    else
        count = 0;

    size = do_fat_read_fstart(argv[4], (unsigned char *)offset, count, fstart);

    if(size==-1) {
        printf("\n** Unable to read \"%s\" from %s %d:%d **\n",
            argv[4], argv[1], dev, part);
        return 1;
    }

    printf("\n%ld bytes read\n", size);
    fstart+=size;
    sprintf(buf, "%lX", fstart);
    setenv("FSTART", buf);

    return 0;
}


U_BOOT_CMD(
    fatread,  CONFIG_SYS_MAXARGS,    1,    do_fat_fsread,
    "FAT fatread with FSTART",
    ""
);

#endif

#if defined(CONFIG_MS_NAND)
extern int ubi_mwrite(char *volume, void *buf, size_t size, int flag);

#if defined(CONFIG_MS_SDMMC)
#define SEGMENT_SIZE 0x800000
int do_ubi_mwrite_from_mmc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    long size;
    unsigned long mem_offset;
    unsigned long total_count=0;
    unsigned long fstart=0;
    unsigned long file_size=0;
    unsigned long left_size=0;
    block_dev_desc_t *dev_desc=NULL;
    int dev=0;
    int part=1;
    char *ep;

    if (argc < 6) {
        printf( "usage: ubimmc <interface> <dev[:part]> "
            "<addr> <filename> <volume name> \n");
        return 1;
    }

    total_count=0;
    fstart=0;

    dev = (int)simple_strtoul(argv[2], &ep, 16);
    dev_desc = get_dev(argv[1],dev);
    if (dev_desc == NULL) {
        puts("\n** Invalid boot device **\n");
        return 1;
    }
    if (*ep) {
        if (*ep != ':') {
            puts("\n** Invalid boot device, use `dev[:part]' **\n");
            return 1;
        }
        part = (int)simple_strtoul(++ep, NULL, 16);
    }
    if (fat_register_device(dev_desc,part)!=0) {
        printf("\n** Unable to use %s %d:%d for fatload **\n",
            argv[1], dev, part);
        return 1;
    }

    mem_offset = simple_strtoul(argv[3], NULL, 16);

    if(do_fat_fsize(argv[4],&file_size)!=0)
    {
        printf("unable to get file size!!\n");
        return 1;
    }


    left_size=file_size;

    // initial UBI write
    if(ubi_mwrite(argv[5],(unsigned char *)mem_offset,file_size,0)!=0)
    {
        return 1;
    }

    while(1)
    {

        unsigned long ubi_mwrite_success_flag=0;
        size = do_fat_read_fstart(argv[4], (unsigned char *)mem_offset, SEGMENT_SIZE, fstart);
        if(size==-1)
        {
                printf("\n** Unable to read \"%s\" from %s %d:%d **\n", argv[4], argv[1], dev, part);
                return 1;
        }



        if(left_size<=SEGMENT_SIZE) //last mwrite
        {
            ubi_mwrite_success_flag=left_size;
        }

        if(ubi_mwrite(argv[5],(unsigned char *)mem_offset,size,1)!=ubi_mwrite_success_flag)
        {
            printf("UBI update failed!!\n");
            return 1;
        }

        total_count+=size;
        left_size-=size;

        printf("   %ld bytes done\n", total_count);

        if(size<SEGMENT_SIZE)
        {
            break;
        }

        fstart+=size;

    }


    // finish UBI write
    if(ubi_mwrite(argv[5],(unsigned char *)mem_offset,file_size,2)!=0)
    {
        return 1;
    }

    printf("ubimmc: total %ld bytes UBI write successfully\n",total_count);


    return 0;
}

U_BOOT_CMD(
    ubimmc,  CONFIG_SYS_MAXARGS,    1,    do_ubi_mwrite_from_mmc,
    "ubimmc",
    ""
);
#endif

#endif

#ifdef CONFIG_MS_EMMC

#include <mmc.h>
#include "../drivers/mstar/emmc/inc/api/drv_eMMC.h"

#define EMMC_BLK_SZ         512
#define EMMC_RW_SHIFT       9

extern int curr_device;
extern int Write_EMMC_CIS(U8 *ptCISData);
extern struct mmc *find_mmc_device(int dev_num);
extern int mmc_init(struct mmc *mmc);
extern int get_mmc_num(void);
extern U32 eMMC_GetDevInfo(eMMC_INFO_t *peMMCInfo_t);
extern U32 eMMC_Init(void);
extern int create_new_NVRAM_partition(block_dev_desc_t *dev_desc, disk_partition_t *info);
int remove_NVRAM_partition(block_dev_desc_t *dev_desc, disk_partition_t *info);
extern void print_part_emmc (block_dev_desc_t *dev_desc);

extern int get_NVRAM_max_part_count(void);
unsigned char tmp_buf[EMMC_BLK_SZ];
ulong used_blk;

int do_emmc_cis(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    U32 part_info=0;
    U8 *cis_info=NULL;
    int result=0;

    printf("do_emmc_cis\r\n");


    part_info=simple_strtoul(argv[1], NULL, 0);

    //while(1);

    eMMC_Init();
    used_blk=0;
    cis_info=(U8 *)malloc(1024);
    if(cis_info==NULL)
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    eMMC_GetDevInfo((eMMC_INFO_t *)cis_info);
    memcpy((cis_info+512),(U8 *)part_info,512);

    //if(drvNAND_WriteCIS(cis_info)!=0)
    if(Write_EMMC_CIS((U8 *)cis_info)!=0)
    {
        printf ("CIS failed!!\n");
        result=-1;
    }

    free(cis_info);


    return result;

}

U_BOOT_CMD(
    emmc_cis,  CONFIG_SYS_MAXARGS,    1,    do_emmc_cis,
    "write EMMC_CIS",
    ""
);


static u32 do_mmc_empty_check(const void *buf, u32 len, u32 empty_flag)
{
    int i;

    if ((!len) || (len & 511))
        return -1;

    for (i = (len >> 2) - 1; i >= 0; i--)
        if (((const uint32_t *)buf)[i] != empty_flag)
            break;

    /* The resulting length must be aligned to the minimum flash I/O size */
    len = ALIGN((i + 1) << 2, 512);
    return len;
}

static u32 do_mmc_write_emptyskip(struct mmc *mmc, s32 start_blk, u32 cnt_blk,
                                            const void *buf, u32 empty_skip)
{
    u32 n = 0;

    if (1 == empty_skip) // 1 indicates skipping empty area, 0 means writing all the data
    {
        u32 nn, empty_flag, rcnt, wcnt, cur_cnt = cnt_blk;
        int boffset = start_blk;
        int doffset = (int)buf;

        if(mmc->ext_csd[181] != 0)
        {
            empty_flag = 0;
        }
        else
        {
            empty_flag = 0xffffffff;
        }

        while(cur_cnt > 0)
        {
            if (cur_cnt >= 0x800)
                wcnt = 0x800;
            else
                wcnt = cur_cnt;

            rcnt = do_mmc_empty_check((void *)doffset, (wcnt << 9), empty_flag);
            if (-1 == rcnt)
            {
                printf("The block num(0x%x) is wrong!", wcnt);
                return 0;
            }
            rcnt >>= 9;
            if (rcnt == 0)
            {
                boffset += wcnt;
                doffset += wcnt << 9;
                cur_cnt -= wcnt;
                n += wcnt;

                continue;
            }

            nn = mmc->block_dev.block_write(0, boffset, rcnt, (void *)doffset);
            if (nn == rcnt)
            {
                n += wcnt;
            }
            else
            {
                n += nn;
                printf("Only 0x%x blk written to blk 0x%x\n, need 0x%x", nn, boffset, rcnt);

                return n;
            }

            boffset += wcnt;
            doffset += wcnt << 9;
            cur_cnt -= wcnt;
        }
    }
    else
    {
        n = mmc->block_dev.block_write(0, start_blk, cnt_blk, buf);
    }

    return n;
}
#if 0
extern int lzop_decompress_part(const unsigned char *src, size_t src_len,
        unsigned char *dst, size_t *dst_len, size_t *src_alignlen, int part);

int do_unlzo (struct mmc *mmc, int argc, char * const argv[])
{
    int ret=0, cnt, cnt_total=0;
    unsigned char *AddrSrc=NULL, *AddrDst=NULL;
    size_t LengthSrc=0,  LengthDst=0;
    size_t LenTail=0, LenSpl=0, LenSrcAlign=0;
    disk_partition_t dpt;
    struct mmc *emmc;
    block_dev_desc_t *mmc_dev;
    s32 blk = -1, partnum = 0, n;
    u32 empty_skip = 0;
    int emmc_dev_index =0;

    AddrSrc = (unsigned char *) simple_strtol(argv[2], NULL, 16);
    LengthSrc = (size_t) simple_strtol(argv[3], NULL, 16);

    emmc_dev_index = CONFIG_MS_EMMC_DEV_INDEX;

    emmc = find_mmc_device(emmc_dev_index);

    mmc_dev =&emmc->block_dev;// mmc_get_dev(curr_device);
    if ((mmc_dev == NULL) ||
            (mmc_dev->type == DEV_TYPE_UNKNOWN)) {
        printf("no mmc device found!\n");
        return 1;
    }

    //Get the partition offset from partition name
    for(;;)
    {
        if(get_partition_info_emmc(mmc_dev, partnum, &dpt))
            break;
        if(!strcmp(argv[4], (const char *)dpt.name))
        {
            blk = dpt.start;
            break;
        }
        partnum++;
    }

    if(blk < 0)
    {
        printf("ERR:Please check the partition name!\n");
        return 1;
    }

    AddrDst = (unsigned char *) CONFIG_UNLZO_DST_ADDR;

    printf ("    Uncompressing ... \n");

    ret = lzop_decompress_part ((const unsigned char *)AddrSrc, LengthSrc,
                (unsigned char *)AddrDst, &LengthDst, &LenSrcAlign, 0);

    if (ret)
    {
        printf("LZO: uncompress, out-of-mem or overwrite error %d\n", ret);
        return 1;
    }

    if (argc == 6)
    {
        empty_skip = simple_strtoul(argv[5], NULL, 16);
    }

    /* We assume that the decompressed file is aligned to mmc block size
        when complete decompressing */
    cnt = LengthDst >> EMMC_RW_SHIFT;

    //n = mmc->block_dev.block_write(curr_device, blk, cnt, AddrDst);
    n = do_mmc_write_emptyskip(mmc, blk, cnt, AddrDst, empty_skip);
    if(n == cnt)
        cnt_total += cnt;
    else
    {
        printf("%d blocks written error at %d\n", cnt, blk);
        return 1;
    }

    /* If the decompressed file is not aligned to mmc block size, we should
        split the not aligned tail and write it in the next loop */
    LenTail = LengthDst & (EMMC_BLK_SZ - 1);

    if(LenTail)
    {
        memcpy((unsigned char *) CONFIG_UNLZO_DST_ADDR,
                    (const unsigned char *) (AddrDst + LengthDst - LenTail), LenTail);
        AddrDst = (unsigned char *) (CONFIG_UNLZO_DST_ADDR + LenTail);
    }else
        AddrDst = (unsigned char *) CONFIG_UNLZO_DST_ADDR;

    if(LenSrcAlign == LengthSrc)
        goto done;

    //Move the source address to the right offset
    AddrSrc += LenSrcAlign;

    printf("    Continue uncompressing and writing emmc...\n");

    for(;;)
    {
        LengthDst = 0;
        ret = lzop_decompress_part ((const unsigned char *)AddrSrc, LengthSrc,
                (unsigned char *)AddrDst, &LengthDst, &LenSrcAlign, 1);
        if (ret)
        {
            printf("LZO: uncompress, out-of-mem or overwrite error %d\n", ret);
            return 1;
        }

        LenSpl = LenTail + LengthDst;
        cnt = LenSpl >> EMMC_RW_SHIFT;

        if(cnt)
        {
            //n = mmc->block_dev.block_write(curr_device, (blk+cnt_total), cnt, (const unsigned char *)CONFIG_UNLZO_DST_ADDR);
            n = do_mmc_write_emptyskip(mmc, (blk+cnt_total), cnt, (const unsigned char *)CONFIG_UNLZO_DST_ADDR, empty_skip);
            if(n == cnt)
                cnt_total += cnt;
            else{
                printf("%d blocks written error at %d\n", cnt, (blk+cnt_total));
                return 1;
            }
        }

        LenTail = LenSpl & (EMMC_BLK_SZ - 1);
        if(LenTail)
        {
            memcpy((unsigned char *) CONFIG_UNLZO_DST_ADDR,
                        (const unsigned char *) (AddrDst + LengthDst - LenTail), LenTail);
            AddrDst = (unsigned char *) (CONFIG_UNLZO_DST_ADDR + LenTail);
        }else
            AddrDst = (unsigned char *) CONFIG_UNLZO_DST_ADDR;

        if(LenSrcAlign == LengthSrc)
            break;

        AddrSrc += LenSrcAlign;
    }

done:

    if(LenTail)
    {
        if(1 != mmc->block_dev.block_write(0, (blk + cnt_total),
                    1, (const unsigned char *)CONFIG_UNLZO_DST_ADDR))
        {
            printf("%d blocks written error at %d\n", cnt, blk);
            return 1;
        }
        cnt_total++;
    }
    printf("    Depressed OK! Write to %s partition OK!\nTotal write size: 0x%0x\n",
            argv[4], cnt_total << EMMC_RW_SHIFT);

    return 0;
}
#endif

extern int delete_NVRAM_all_partition(block_dev_desc_t *dev_desc);
extern U32 eMMC_IPVerify_Main(void);

int do_emmc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int emmc_dev_index =0;
    struct mmc *emmc;
    block_dev_desc_t *emmc_dev;
    disk_partition_t dpt;

    if (argc < 2)
        return CMD_RET_USAGE;


    emmc_dev_index = CONFIG_MS_EMMC_DEV_INDEX;

//    if (curr_device < 0) {
//        if (get_mmc_num() > 0)
//            curr_device = 0;
//        else {
//            puts("No MMC device available\n");
//            return 1;
//        }
//    }

    emmc = find_mmc_device(emmc_dev_index);


    if (!emmc) {
        printf("no mmc device at slot %x\n", curr_device);
        return 1;
    }

    if(!(emmc->has_init))
    {
        printf("Do mmc init first!\n");
        mmc_init(emmc);
        emmc->has_init = 1;
    }


    emmc_dev= &emmc->block_dev;

    if ((emmc_dev == NULL) ||
            (emmc_dev->type == DEV_TYPE_UNKNOWN)) {
        printf("no mmc device found!\n");
        return 1;
    }

    if(PART_TYPE_EMMC!=emmc_dev->part_type)
    {
        printf("not EMMC base partition!(part_type:%d)\n", emmc_dev->part_type);
        //TBD:
        if(PART_TYPE_UNKNOWN!=emmc_dev->part_type) {
            return 1;
        }
    }

	if(strncmp(argv[1], "ipverify", 8) == 0)
	{
        eMMC_IPVerify_Main();
		while(1);
	}

    if(strncmp(argv[1], "create", 6) == 0)
    {

        strcpy((char *)&dpt.name, argv[2]);
        if(simple_strtoull(argv[3], NULL, 16)==0x01)
        {
           dpt.size=emmc_dev->lba-used_blk-0xD000;
        }
        else
        {
            dpt.size = ALIGN(simple_strtoull(argv[3], NULL, 16), 512) / 512;
            used_blk+=dpt.size;
        }
        //printf("1111 emmc_dev->lba=%lx\n", emmc_dev->lba);
        //printf("1111 dpt.size=%lx\n", dpt.size);
        if(argc > 4)
            dpt.start = ALIGN(simple_strtoull(argv[4], NULL, 16), 512) / 512;
        else
            dpt.start = 0;

        if (create_new_NVRAM_partition(emmc_dev, &dpt) == 0){
            printf("Add new NVRAM Partition %s success!\n", dpt.name);
            return 0;
        }
        return 1;
    }
    else if(strncmp(argv[1], "remove", 6) == 0)
    {

        strcpy((char *)&dpt.name, (const char *)argv[2]);

        if (remove_NVRAM_partition(emmc_dev, &dpt) == 0){
            printf("Remove partition %s success!\n", dpt.name);
            return 0;
        }
        return 1;
    }
    else if(strncmp(argv[1], "rmgpt", 5) == 0)
    {
        if (delete_NVRAM_all_partition(emmc_dev) == 0){
            printf("delete all partition success!\n");
            return 0;
        }
        return 1;
    }
    else if(strcmp(argv[1], "unlzo") == 0)
    {
        int ret=0;

        if (argc < 5)
        {
            printf ("Usage:\n%s\n", cmdtp->usage);
            return 1;
           }
		#if 0
        ret = do_unlzo(emmc, argc, argv);
		#else
		printf("unsupport unlzo\n");
		return 1;
		#endif

        return ret;
    }
    else if(strncmp(argv[1], "slc", 3) == 0)
    {
        unsigned long long size;
        int reliable_write, ret;

        if (argc < 4) {
            printf("Usage:\n%s\n", cmdtp->usage);
            return 0;
        }

        size = simple_strtoul(argv[2], NULL, 16);
        reliable_write = simple_strtoul(argv[3], NULL, 16);
        if ((reliable_write != 0) && (reliable_write != 1))
        {
            printf("Reliable write enable can only be set to be 0 or 1!!!\n");
            return 0;
        }
        if ((size == 0) && (reliable_write == 0))
        {
            printf("Both of slc size and reliable write configuration are zero, please input proper values!!!\n");
            return 0;
        }

        ret = mmc_slc_mode(emmc, size, reliable_write);

        return ret;

    }
    else if(strncmp(argv[1], "part", 5) == 0)
    {
        print_part_emmc (emmc_dev);

        return 0;

    }else if (strncmp(argv[1], "read", 4) == 0) {
        void *addr = (void *)simple_strtoul(argv[2], NULL, 16);
        u32 n, n2, cnt, size, tail = 0, partnum = 1;
        s32 blk = -1;
        char* cmdtail = strchr(argv[1], '.');
        char* cmdlasttail = strrchr(argv[1], '.');

        size = simple_strtoul(argv[4], NULL, 16);
        cnt = ALIGN(size, 512) / 512;

        if((cmdtail)&&(!strncmp(cmdtail, ".p", 2))){
            for(partnum=0;partnum<get_NVRAM_max_part_count();partnum++)
            {

                int res=get_partition_info_emmc(emmc_dev, partnum, &dpt);

                if(res>0)continue;
                else if(res<0) break;

                if(!strcmp(argv[3], (const char *)dpt.name)){
                    blk = dpt.start;
                    if(!strncmp(cmdlasttail, ".continue", 9))
                    {
                        blk += simple_strtoul(argv[4], NULL, 16);
                        size = simple_strtoul(argv[5], NULL, 16);
                        cnt = ALIGN(size, 512) / 512;
                    }
                    break;
                }

            }
        }
        else if ((cmdtail)&&(!strncmp(cmdtail, ".cpart", 6)))
        {
			//argv[3] = type, argv[4] = startblk in partition, argv[5] = blkcnt
            blk  = simple_strtoul(argv[4], NULL, 16);
            size = simple_strtoul(argv[5], NULL, 16);
            cnt  = ALIGN(size, 512) / 512;
        }
        else if ((cmdtail)&&(!strncmp(cmdtail, ".boot", 5)))
        {
            addr = (void *)simple_strtoul(argv[3], NULL, 16);
            blk  = simple_strtoul(argv[4], NULL, 16);
            size = simple_strtoul(argv[5], NULL, 16);
            cnt  = ALIGN(size, 512) / 512;
        }
        else {
            blk = simple_strtoul(argv[3], NULL, 16);
        }

        if(blk < 0){
            printf("ERR:Please check the blk# or partiton name!\n");
            return 1;
        }

        /* unaligned size is allowed */
        if ((cnt << 9) > size)
        {
            cnt--;
            tail = size - (cnt << 9);
        }

        printf("\nMMC read: dev # %d, block # %d, count %d ... ",
                emmc_dev_index, blk, cnt);

#if defined(MMC_SPEED_TEST) && MMC_SPEED_TEST
        FCIE_HWTimer_Start();
#endif

#if 0
        n = mmc->block_dev.block_read(curr_device, blk, cnt, addr);
#else
        if ((cmdtail)&&(!strncmp(cmdtail, ".boot", 5)))
        {
            if (strncmp(argv[2], "1", 1) == 0)
                n = eMMC_ReadBootPart(addr, cnt << 9, blk, 1);
            else if (strncmp(argv[2], "2", 1) == 0)
                n = eMMC_ReadBootPart(addr, cnt << 9, blk, 2);
            else
            {
                printf("mmc access boot partition parameter not found!\n");
                return 1;
            }
            n = (n == 0) ? cnt : -1;

            if (tail)
            {
                if (strncmp(argv[2], "1", 1) == 0)
                    n2 = eMMC_ReadBootPart(tmp_buf, 512, (blk + cnt), 1);
                else if (strncmp(argv[2], "2", 1) == 0)
                    n2 = eMMC_ReadBootPart(tmp_buf, 512, (blk + cnt), 2);
                else
                {
                    printf("mmc access boot partition parameter not found!\n");
                    return 1;
                }

                n2 = (n2 == 0) ? 1 : -1;
                memcpy(((unsigned char *)addr + (cnt << 9)), tmp_buf, tail);
                n += n2;
                cnt++;
            }
        }
		else if((cmdtail)&&(!strncmp(cmdtail, ".cpart", 6)))
		{
			u16 u16_PartType = 0;
			if (strncmp(argv[3], "uboot", 5) == 0)
			{
				u16_PartType = eMMC_PART_EBOOT;
			}
			else if(strncmp(argv[3], "emmcenv", 7) == 0)
			{
				u16_PartType = eMMC_PART_ENV;
			}
			if(u16_PartType)
			{
				n = eMMC_ReadPartition(u16_PartType, addr, blk, cnt, 0);
				n = (n == 0) ? cnt : -1;
			}
			else
			{
				printf("Invalid Partition name for PNI partition\n");
				return -1;
			}
		}
        else
        {
            if (cnt > 0)
            {
                n = emmc->block_dev.block_read(0, blk, cnt, addr);
            }
            else if (cnt == 0)
            {
                n = 0;
            }

            if (tail)
            {
                n2 = emmc->block_dev.block_read(0, (blk + cnt), 1, tmp_buf);
                n2 = (n2 == 1) ? 1 : -1;
                memcpy(((unsigned char *)addr + (cnt << 9)), tmp_buf, tail);
                n += n2;
                cnt++;
            }
        }
#endif

        /* flush cache after read */
        flush_cache((ulong)addr, (cnt*512)); /* FIXME */

        printf("%d blocks read: %s\n",
                n, (n==cnt) ? "OK" : "ERROR");
        return (n == cnt) ? 0 : 1;
    }
    else if (strncmp(argv[1], "write", 5) == 0)
    {
        //if(argc < 5)
        //    return cmd_usage(cmdtp);
        void *addr = (void *)simple_strtoul(argv[2], NULL, 16);
        u32 n, cnt, partnum = 0, empty_skip = 0, cont = 0;
        char* cmdtail = strchr(argv[1], '.');
        char* cmdlasttail = strrchr(argv[1], '.');
        s32 blk = -1;


        cnt = ALIGN(simple_strtoul(argv[4], NULL, 16), 512) / 512;


        if((cmdtail)&&(!strncmp(cmdtail, ".p", 2))){
            for(partnum=0;partnum<get_NVRAM_max_part_count();partnum++)
            {
                int res=get_partition_info_emmc(emmc_dev, partnum, &dpt);

                if(res>0)continue;
                else if(res<0) break;

                if(!strcmp(argv[3], (const char *)dpt.name)){
                    blk = dpt.start;
                    if(!strncmp(cmdlasttail, ".continue", 9))
                    {
                        blk += simple_strtoul(argv[4], NULL, 16);
                        cnt = ALIGN(simple_strtoul(argv[5], NULL, 16), 512) / 512;
                        cont = 1;
                    }
                    break;
                }

            }
        }
        else if ((cmdtail)&&(!strncmp(cmdtail, ".cpart", 6)))
        {
			//argv[3] = type, argv[4] = startblk in partition, argv[5] = blkcnt
            blk = simple_strtoul(argv[4], NULL, 16);
            cnt = ALIGN(simple_strtoul(argv[5], NULL, 16), 512) / 512;
        }
        else if ((cmdtail)&&(!strncmp(cmdtail, ".boot", 5)))
        {
            addr = (void *)simple_strtoul(argv[3], NULL, 16);
            blk = simple_strtoul(argv[4], NULL, 16);
            cnt = ALIGN(simple_strtoul(argv[5], NULL, 16), 512) / 512;
        }
        else
            blk = simple_strtoul(argv[3], NULL, 16);

        if(partnum==get_NVRAM_max_part_count())
        {
            printf("ERR:Can not found partiton with name %s!\n",argv[3]);
            return 1;
        }

        if(blk < 0){
            printf("ERR:Please check the blk# or partiton name!\n");
            return 1;
        }

        printf("\nMMC write: dev # %d, block # %d, count %d ... ",
                emmc_dev_index, blk, cnt);

#if defined(MMC_SPEED_TEST) && MMC_SPEED_TEST
        FCIE_HWTimer_Start();
#endif

#if 0
        n = mmc->block_dev.block_write(curr_device, blk, cnt, addr);
#else
        if ((cmdtail)&&(!strncmp(cmdtail, ".boot", 5)))
        {
            if (strncmp(argv[2], "1", 1) == 0)
                n = eMMC_WriteBootPart(addr, cnt << 9, blk, 1);
            else if (strncmp(argv[2], "2", 1) == 0)
                n = eMMC_WriteBootPart(addr, cnt << 9, blk, 2);
            else
            {
                printf("mmc access boot partition parameter not found!\n");
                return 1;
            }

            n = (n == 0) ? cnt : -1;
        }
		else if((cmdtail)&&(!strncmp(cmdtail, ".cpart", 6)))
		{
			u16 u16_PartType = 0;
			if (strncmp(argv[3], "uboot", 5) == 0)
			{
				u16_PartType = eMMC_PART_EBOOT;
			}
			else if(strncmp(argv[3], "emmcenv", 7) == 0)
			{
				u16_PartType = eMMC_PART_ENV;
			}
			if(u16_PartType)
			{
				n = eMMC_WritePartition(u16_PartType, addr, blk, cnt, 0);
				n = (n == 0) ? cnt : -1;
			}
			else
			{
				printf("Invalid Partition name for PNI partition\n");
				return -1;
			}
		}
        else
        {
            if ((argc == 6) && (cont == 0))
            {
                empty_skip = simple_strtoul(argv[5], NULL, 16);
            }
            if ((argc == 7) && (cont == 1))
            {
                empty_skip = simple_strtoul(argv[6], NULL, 16);
            }

            n = do_mmc_write_emptyskip(emmc, blk, cnt, addr, empty_skip);
        }
#endif


        printf("%d blocks written: %s\n",
        n, (n == cnt) ? "OK" : "ERROR");
        return (n == cnt) ? 0 : 1;
    }
    else if (strncmp(argv[1], "erase", 5) == 0)
    {
        u32 boot_partition = 0;     //default user area partition
        u32 n, cnt = 0, partnum =0;
        char* cmdtail = strchr(argv[1], '.');
        u64 erase_size = 0;
        s32 start = -1;

        if(argc==4) {
            start = simple_strtoul(argv[2], NULL, 16);
            erase_size =  simple_strtoul(argv[3], NULL, 16);  //Bytes
            if((erase_size<=0) || (erase_size&0x1FF))
            {
                printf("invalied erase size, must aligned to 512 bytes\n");
                return 1;
            }

            cnt = erase_size >> 9;  // /unit 512B
        }

        if((cmdtail)&&(!strncmp(cmdtail, ".p", 2)))
        {
            if(argc!=3) //not specify partition name
                return cmd_usage(cmdtp);



            for(partnum=0;partnum<get_NVRAM_max_part_count();partnum++)
            {
                int res=get_partition_info_emmc(emmc_dev, partnum, &dpt);

                if(res>0)continue;
                else if(res<0) break;

                if(!strcmp(argv[2], (const char *)dpt.name)){
                    start = dpt.start;
                    cnt = dpt.size; //block number
                    break;
                }
            }

            if(cnt==0) {
                printf("ERR:invalid parameter, please check partiton name!\n");
                return 1;
            }
        }
//        else if((cmdtail)&&(!strncmp(cmdtail, ".boot", 5))){ //erase boot partition
//            boot_partition = 1;
//
//            start = 0;
//            cnt = g_eMMCDrv.u32_BOOT_SEC_COUNT;
//
//            if(cnt==0)
//            {
//                printf("ERR:emmc no boot partition size !\n");
//                return 1;
//            }
//        }
        else if(argc==2) {             //erase all blocks in user area partiiton
            start = 0;
            cnt = emmc->block_dev.lba;
        }

        if(start < 0)
        {
            printf("ERR:invalid parameter, please check the blk# or partiton name!\n");
            return 1;
        }

        if(cnt <= 0)
        {
            printf("ERR:invalid parameter, Please check size!\n");
            return 1;
        }

//        if(((!boot_partition)&&(cnt > emmc->block_dev.lba)) ||((boot_partition)&&(cnt>g_eMMCDrv.u32_BOOT_SEC_COUNT)))
//            printf("ERR:invalid parameter, please check the size#!\n");

        printf("\nMMC erase: dev # %d, %s part, block # %d, count %d ... \n",
                emmc_dev_index, boot_partition ? "boot" : "user area", start, cnt);

        if(!boot_partition)
            n = emmc->block_dev.block_erase(0, start, cnt);
//        else {
//
//            if (strncmp(argv[2], "1", 1) == 0)
//                n = eMMC_EraseBootPart(start, start + cnt - 1, 1);
//            else if (strncmp(argv[2], "2", 1) == 0)
//                n = eMMC_EraseBootPart(start, start + cnt - 1, 2);
//            else
//            {
//                printf("mmc access boot partition parameter not found!\n");
//                return 1;
//            }
//
//            n = (n == 0) ? cnt : -1;
//        }

        printf("%d blocks erase: %s\n",
                n, (n == cnt) ? "OK" : "ERROR");
        return (n == cnt) ? 0 : 1;
    }
    else
    {
        return CMD_RET_USAGE;
    }

    return 1;

}


U_BOOT_CMD(
    emmc,  CONFIG_SYS_MAXARGS,    1,    do_emmc,
    "EMMC function on NVRAM base partition",
    "emmc create [name] [size] - create mmc partition [name]\n"
    "emmc remove [name] - remove mmc partition [name]\n"
    "emmc rmgpt - clean all mmc partition table\n"
    "emmc part - list partitions \n"
    "emmc slc size relwr - set slc in the front of user area,  0xffffffff means max slc size\n"
    "emmc unlzo Src_Address Src_Length Partition_Name [empty_skip:0-disable,1-enable]- decompress lzo file and write to mmc partition \n"
    "emmc read.p addr partition_name size\n"
    "emmc read.p.continue addr partition_name offset size\n"
    "emmc write.p addr partition_name size [empty_skip:0-disable,1-enable]\n"
    "emmc write.p.continue addr partition_name offset size [empty_skip:0-disable,1-enable]\n"
    "emmc erase.p partition_name\n"
);


int do_emmcstar     (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char* buffer=NULL;
    char* UpgradeImage=NULL;
    char *script_buf;
    char *next_line;
    char tmp[64];
    int ret = -1;

    buffer=(char *)malloc(USBUPGRDE_SCRIPT_BUF_SIZE);
    if((buffer==NULL))
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    memset(buffer, 0 , USBUPGRDE_SCRIPT_BUF_SIZE);
    UpgradeImage = getenv(ENV_EMMC_UPGRADEIMAGE);
    if(UpgradeImage == NULL)
    {
        printf("UpgradeImage env is null,use default SigmastarUpgradeEMMC.bin\n");
        UpgradeImage = "SigmastarUpgradeEMMC.bin";
        run_command("setenv EmmcUpgradeImage SigmastarUpgradeEMMC.bin", 0);
        run_command("saveenv",0);

    }

    memset(tmp,0,sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1,"fatload mmc 1 %X %s 0x4000 0x0", (U32)buffer, UpgradeImage);
    run_command(tmp, 0);
    script_buf = buffer;
    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
        run_command(next_line, 0);
    }
    
    free(buffer);
    return ret;
    
}

U_BOOT_CMD(
    emmcstar,  CONFIG_SYS_MAXARGS,    1,    do_emmcstar,
    "script via emmc package",
    ""
);

#endif
/*
extern image_header_t *image_get_kernel(ulong img_addr, int verify);
int do_verifyimg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    image_header_t    *hdr;
    ulong        img_addr;


    // find out kernel image address
    if (argc < 2) {
        img_addr = load_addr;
        debug("*  kernel: default image load address = 0x%08lx\n",load_addr);
    } else {
        img_addr = simple_strtoul(argv[1], NULL, 16);
        debug("*  kernel: cmdline image address = 0x%08lx\n", img_addr);
    }

    printf("## Booting kernel from Legacy Image at %08lx ...\n", img_addr);

    hdr = image_get_kernel(img_addr, 1);

    if (!hdr){
        setenv("chkerr", "1");
    }

    return 1;
}


U_BOOT_CMD(
    verifyimg,    2,    1,    do_verifyimg,
    "verify uImage image from memory",
    "addr \n    - verify image stored in memory\n"
);

*/


#if defined(CONFIG_MS_PARTITION)
#include "drivers/mstar/partition/part_mxp.h"
#define MAX_RECORD_PRINT_COUNT 32
extern int mxp_save_table_from_mem(u32 mem_address);
static void print_mxp_record(int index,mxp_record* rc)
{
    printf("[mxp_record]: %d\n",index);
    printf("     name: %s\n",rc->name);
    printf("     type: 0x%02X\n",rc->type);
    printf("   format: 0x%02X\n",rc->format);
    printf("   backup: %s\n",( ((char)0xFF)==((char)rc->backup[0]) )?(""):((char *)rc->backup));
    printf("    start: 0x%08X\n",(unsigned int)rc->start);
    printf("     size: 0x%08X\n",(unsigned int)rc->size);
    printf("   status: 0x%02X\n",rc->status);
    printf("\n");

}
int do_mxp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(CONFIG_MS_ISP_FLASH)
extern int mxp_init_nor_flash(void);
    int ret=0;
    if((ret=mxp_init_nor_flash())<0)
    {
        return ret;
    }
#endif

    if(strncmp(argv[1], "t.list", 6) == 0)
    {
        if(argc>2)
        {
            int i=0;
            int mem_addr = simple_strtoul(argv[2], NULL, 16);
            mxp_record* recs=(mxp_record*)((void *)mem_addr);
            for(i=0;i<MAX_RECORD_PRINT_COUNT;i++)
            {

                if(MXP_PART_TYPE_TAG==recs[i].type)
                {
                    break;

                }
                print_mxp_record(i,&recs[i]);

            }
            printf("Available MXP record count:%d\n",i);
        }
        else
        {
            int count=mxp_get_total_record_count();
            int i=0;
            printf("Total MXP record count:%d\n",count);
            for(i=0;i<count;i++)
            {
                mxp_record rec;
                mxp_get_record_by_index(i,&rec);
                print_mxp_record(i,&rec);
            }
        }
    }
    else if(strncmp(argv[1], "t.update", 8) == 0)
    {
        u32 mem_addr = (u32)simple_strtoul(argv[2], NULL, 16);
        mxp_save_table_from_mem(mem_addr);
        //
    }
    else if(strncmp(argv[1], "t.load", 6) == 0)
    {
        mxp_load_table();
    }
    else if(strncmp(argv[1], "t.init", 6) == 0)
    {
        mxp_init_table();
    }
    else if(strncmp(argv[1], "r.del", 5) == 0)
    {
        int idx=mxp_get_record_index(argv[2]);
        if(idx>=0)
        {
            mxp_delete_record_by_index(idx);
        }
        else
        {
            printf("can not found mxp record: %s\n",argv[2]);
        }
    }
    else if(strncmp(argv[1], "r.set", 5) == 0)
    {

        if(argc<3)
        {
            printf("missing parameters\n");
            return CMD_RET_USAGE;
        }
        else if(3==argc)
        {
            mxp_record rec;
            u32 mem_addr = (u32)simple_strtoul(argv[2], NULL, 16);
            memcpy(&rec,(void *)mem_addr,sizeof(mxp_record));
            mxp_set_record(&rec);
        }
        else
        {
            int idx=mxp_get_record_index(argv[2]);
            if(idx>=0)
            {

                mxp_record rec;
                mxp_get_record_by_index(idx,&rec);

                if(strncmp(argv[3], "crc32", 5) == 0)
                {
                    rec.crc32=(u32)simple_strtoul(argv[4], NULL, 16);
                    mxp_set_record(&rec);
                }
                else if(strncmp(argv[3], "status", 5) == 0)
                {
                    rec.status=(u32)simple_strtoul(argv[4], NULL, 16);
                    mxp_set_record(&rec);
                }
                else if(strncmp(argv[3], "backup", 6) == 0)
                {
                    memcpy( rec.name,argv[4],(strlen(argv[4])>15)?15:strlen(argv[4]) );
                    rec.name[15]=0;
                    mxp_set_record(&rec);
                }
                else
                {
                    printf("unsupported mxp record setting property: %s\n",argv[3]);
                    ret = -1;
                }

            }
            else
            {
                printf("can not found mxp record: %s\n",argv[2]);
                ret = -1;
            }
        }
    }
    else if(strncmp(argv[1], "r.info", 6) == 0)
    {

        if(argc<3)
        {
            printf("missing parameters\n");
            return CMD_RET_USAGE;
        }
        else
        {
            int idx=mxp_get_record_index(argv[2]);
            if(idx>=0)
            {

                mxp_record rec;
                setenv_hex("sf_part_start", 0);
                setenv_hex("sf_part_size", 0);
                setenv_hex("cpu_part_start", 0);

                if(0==mxp_get_record_by_index(idx,&rec))
                {
                    print_mxp_record(0,&rec);
                    setenv_hex("sf_part_start", rec.start);
                    setenv_hex("sf_part_size", rec.size);
                    setenv_hex("cpu_part_start", rec.start+MS_SPI_ADDR);

                    if(strncmp(argv[2], "KERNEL", 6)==0)
                    {
                        setenv_hex("sf_kernel_start", rec.start);
                        setenv_hex("sf_kernel_size", rec.size);
                    }
                }
                else
                {
                    printf("failed to get MXP record with name: %s\n",argv[2]);
                    ret = -1;
                }


            }
            else
            {
                printf("can not found mxp record: %s\n",argv[2]);
                ret = -1;
            }
        }
    }
    else
    {
            return CMD_RET_USAGE;
    }

    return ret;
}
U_BOOT_CMD(
    mxp,  CONFIG_SYS_MAXARGS,    1,    do_mxp,
    "MXP function for Mstar MXP partition",
    "mxp t.list [memory] - list table records, if [memory] then list from [memory]\n"
    "mxp t.load - load table from storage\n"
    "mxp t.init - clean the table in storage with default empty records\n"
    "mxp t.update memory - update the table in storage from memory\n"
    "mxp r.del name - remove MXP record with name\n"
    "mxp r.set index crc32,status,backup value - set the MXP record property using index with value\n"
    "mxp r.set memory - set a MXP record using name from memory, if name is not exsited in table yet, new record will be created\n"
    "mxp r.info name - show the info of the record using name, the $sf_part_start and the $sf_part_size will be set if success\n"
//    "emmc remove [name] - remove mmc partition [name]\n"
//    "emmc rmgpt - clean all mmc partition table\n"
//    "emmc part - list partitions \n"
//    "emmc slc size relwr - set slc in the front of user area,  0xffffffff means max slc size\n"
//    "emmc unlzo Src_Address Src_Length Partition_Name [empty_skip:0-disable,1-enable]- decompress lzo file and write to mmc partition \n"
//    "emmc read.p addr partition_name size\n"
//    "emmc read.p.continue addr partition_name offset size\n"
//    "emmc write.p addr partition_name size [empty_skip:0-disable,1-enable]\n"
//    "emmc write.p.continue addr partition_name offset size [empty_skip:0-disable,1-enable]\n"
//    "emmc erase.p partition_name\n"
);

#endif

#if defined(CONFIG_MS_AESDMA)

void image_set_encrypt_flag(U8 *hdr)
{
    hdr[0x3F] = 'E';
}


int do_aesdma(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    aesdmaConfig config={0};
    u32 image_start=0;
    int ret=0;

    if(argc<5)
    {
        printf("missing parameters\n");
        return CMD_RET_USAGE;
    }

    if(strncmp(argv[1], "dec", 3) == 0)
    {
        config.bDecrypt=1;
    }
    else if(strncmp(argv[1], "enc", 3) == 0)
    {
        config.bDecrypt=0;
    }
    else
    {
        return CMD_RET_USAGE;
    }

    image_start = (u32)simple_strtoul(argv[2], NULL, 16);

    printf("image_addr = 0x%08X\n", image_start);

    if(!image_check_magic((void*)image_start))
    {
        printf("image header check failed, can't get data size\n");
        return CMD_RET_USAGE;
    }
    else
    {
        config.u32Size = image_get_data_size((void*)image_start);
        config.u32SrcAddr = config.u32DstAddr = image_start + image_get_header_size();
        printf("image header check ok, data size=0x%08X\n", config.u32Size);
        printf("data start addr=0x%08X\n", config.u32SrcAddr);
    }


    if(strncmp(argv[3], "ECB", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_ECB;
    }
    else if(strncmp(argv[3], "CTR", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_CTR;
    }
    else if(strncmp(argv[3], "CBC", 3) == 0)
    {
        config.eChainMode = E_AESDMA_CHAINMODE_CBC;
    }
    else
    {
        printf("use default chainmode - CBC\n");
        config.eChainMode = E_AESDMA_CHAINMODE_CBC;
    }

    if(strncmp(argv[4], "CIPHER", 6) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_CIPHER;
        config.pu16Key = (U16*)(KEY_CUST_LOAD_ADDRESS+image_get_header_size());
    }
    else if(strncmp(argv[4], "EFUSE", 5) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_EFUSE;
    }
    else if(strncmp(argv[4], "HW", 2) == 0)
    {
        config.eKeyType = E_AESDMA_KEY_HW;
    }
    else
    {
        printf("use default keytype - EFUSE\n");
        config.eKeyType = E_AESDMA_KEY_EFUSE;
    }

    MDrv_AESDMA_Run(&config);

    if(config.bDecrypt)
    {
        printf("Decrypt done!\n");
    }
    else
    {
        printf("Encrypt done!\n");
        image_set_encrypt_flag((U8*)image_start);
    }

    if(6 == argc)
    {
        char* buffer=(char *)malloc(BUF_SIZE);
        if((buffer==NULL))
        {
            printf("no memory for command string!!\n");
            return -1;
        }
        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "mxp r.info %s", argv[5]);
        ret |= run_command(buffer, 0);

        ret |= run_command("sf probe", 0);
        ret |= run_command("sf erase $(sf_part_start) $(sf_part_size)", 0);

        memset(buffer, 0 , BUF_SIZE);
        sprintf(buffer, "sf write %x $(sf_part_start) %x", image_start, config.u32Size+image_get_header_size());
        ret |= run_command(buffer, 0);

        free(buffer);

        if(!ret)
            printf("Secure writeback \033[1;36m%s\033[m done\n\n", argv[5]);
        //run_command("reset", 0);
    }

    return ret;

}


U_BOOT_CMD(
    aes,  CONFIG_SYS_MAXARGS,    1,    do_aesdma,
    "Control Mstar AES engine",
    "direction image_addr chainmode keytype partition\n\n"
    "\tdirection - enc, dec\n"
    "\timage_addr - image location\n"
    "\tchainmode - ECB, CTR, CBC\n"
    "\tkeytype - CIPHER, EFUSE, HW\n"
    "\tpartition - partition name to program into\n"
);

void *memcpy_4byte(void *dst, const void *src, size_t n)
{
	U32 *pdst = (U32 *)dst;
	const U32 *psrc = (const U32 *)src;
	n = (n%4)? (n/4+1): (n/4);

	for(; n > 0; ++pdst, ++psrc, --n)
	{
		*pdst = *psrc;
	}

	return (dst);
}

void halt(void)
{
	printf("[HALT]\r\n");
	while(1);
}

void chip_flush_miu_pipe(void)
{
	unsigned short dwReadData = 0;
	//toggle the flush miu pipe fire bit
	*(volatile unsigned short *)(0x1F204414) = 0x0;
	*(volatile unsigned short *)(0x1F204414) = 0x1;
    do
	{
		dwReadData = *(volatile unsigned short *)(0x1F204440);
		dwReadData &= BIT12;  //Check Status of Flush Pipe Finish
	} while(dwReadData == 0);
}

#if 0
U8 image_check_encryption(void *hdr)
{
   return (image_get_encryption((image_header_t *)hdr) == 'E' ? 1 : 0);
}
#endif

int do_auth(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	aesdmaConfig config={0};
	u8 uAuthON=0;
	u8 uAesON=0;
	u32 image_start=0;
	int ret=0;

	if(argc<8)
	{
		printf("missing parameters\n");
		return CMD_RET_USAGE;
	}


	if(strncmp(argv[1], "dec", 3) == 0)
	{
		config.bDecrypt=1;
	}
	else if(strncmp(argv[1], "enc", 3) == 0)
	{
		config.bDecrypt=0;
	}
	else
	{
		return CMD_RET_USAGE;
	}

	image_start = (u32)simple_strtoul(argv[2], NULL, 16);

	printf("image_addr = 0x%08X\n", image_start);

	//Check kernel.xz.img magic is 27 05 19 56, first 4 bytes
	if(!image_check_magic((void*)image_start))
	{
		printf("image header check failed, can't get data size\n");
		return CMD_RET_USAGE;
	}
	else
	{   //real image address
		config.u32Size = image_get_data_size((void*)image_start);
		config.u32SrcAddr = config.u32DstAddr = image_start;
		printf("image header check ok, data size=0x%08X\n", config.u32Size); //Data Siz or Data Siz with padding
		printf("data start addr=0x%08X\n", config.u32SrcAddr);
	}

	if(strncmp(argv[3], "ECB", 3) == 0)
	{
		config.eChainMode = E_AESDMA_CHAINMODE_ECB;
		printf("[U-Boot] ChainMode = ECB\n");
	}
	else if(strncmp(argv[3], "CTR", 3) == 0)
	{
		config.eChainMode = E_AESDMA_CHAINMODE_CTR;
		printf("[U-Boot] ChainMode = CTR\n");
	}
	else if(strncmp(argv[3], "CBC", 3) == 0)
	{
		config.eChainMode = E_AESDMA_CHAINMODE_CBC;
		printf("[U-Boot] ChainMode = CBC\n");
	}
    else
    {
		printf("use default chainmode - CBC\n");
		config.eChainMode = E_AESDMA_CHAINMODE_CBC;
    }

	if(strncmp(argv[4], "CIPHER", 6) == 0)
	{
		config.eKeyType = E_AESDMA_KEY_CIPHER;
		config.pu16Key = (U16*)(KEY_CUST_LOAD_ADDRESS+0x210);
		printf("[U-Boot] KeyType = CIPHER\n");
	}
	else if(strncmp(argv[4], "EFUSE", 5) == 0)
	{
		config.eKeyType = E_AESDMA_KEY_EFUSE;
		printf("[U-Boot] KeyType = EFUSE\n");
	}
	else if(strncmp(argv[4], "HW", 2) == 0)
	{
		config.eKeyType = E_AESDMA_KEY_HW;
		printf("[U-Boot] KeyType = HW\n");
	}
	else
	{
		printf("use default keytype - EFUSE\n");
		config.eKeyType = E_AESDMA_KEY_EFUSE;
	}

	if(config.bDecrypt)
	{
		printf("Decrypt setting done!\n");
	}
	else
	{
		printf("Encrypt setting done!\n");
		//image_set_encrypt_flag((U8*)image_start);
	}

	if(strncmp(argv[6], "AUTHON", 6) == 0)
	{
		uAuthON=1;
	}
	else if(strncmp(argv[6], "AUTHOFF", 7) == 0)
	{
		uAuthON=0;
	}
	else
	{
		return CMD_RET_USAGE;
	}


	if(strncmp(argv[7], "AESON", 5) == 0)
	{
		uAesON=1;
	}
	else if(strncmp(argv[7], "AESOFF", 6) == 0)
	{
		uAesON=0;
	}
	else
	{
		return CMD_RET_USAGE;
	}


	if(8 == argc)
	{
		/*Proceed RSA authentication first , then do AES decryption*/
	    if(uAuthON)
	    {
		    if(runAuthenticate(config.u32SrcAddr, config.u32Size+KERNEL_HEAD_SIZE, (U32*)(KEY_CUST_LOAD_ADDRESS)))
		    {
		    	printf("[U-Boot] Authenticate KERNEL pass!\n\r");
		    }
			else
			{
				printf("[U-Boot] Authenticate KERNEL failed!\n\r");
				uAesON=0;
				halt();
			}
	    }
		if(uAesON)
		{
			printf("[U-Boot] Decrypt Kernel\n\r");
			config.u32Size = config.u32Size;
			config.u32SrcAddr = config.u32DstAddr = image_start+KERNEL_HEAD_SIZE;
			printf("SrcAddr 0x%08X DstAddr 0x%08X AES size=0x%08X --> MDrv_AESDMA_Run\n", config.u32SrcAddr, config.u32DstAddr, config.u32Size);
			MDrv_AESDMA_Run(&config);
			printf("[U-Boot] Decrypt AES done!\n\r");
		}
	}

	return ret;
}

U_BOOT_CMD(
     secauth,  CONFIG_SYS_MAXARGS,    1,    do_auth,
     "Control Sstar security authenticate sequence",
     "direction image_addr chainmode keytype partition\n\n"
     "\tdirection - enc, dec\n"
     "\timage_addr - image location\n"
     "\tchainmode - ECB, CTR, CBC\n"
     "\tkeytype - CIPHER, EFUSE, HW\n"
     "\tpartition - partition name to program into\n"
 );
#endif


int do_debug(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u16 *buf = map_sysmem(0x1F001C24, 2);
    //set enable_rx = 0
    *buf = *buf & ~BIT11;
    printf("\ndebug mode on, cmdline is disabled\n\n");
    return 0;
}

U_BOOT_CMD(
    debug,  CONFIG_SYS_MAXARGS,    1,    do_debug,
    "Disable uart rx via PAD_DDCA to use debug tool",
    ""
);

static char uart_help_text[] =
	"uart init [Port] - \n"
	"uart init 1: ttyS1->PAD_VSYNC_OUT& PAD_HSYNC_OUT"
	"uart init 2: ttyS2->PAD_FUART_RTS/ PAD_FUART_CTS"
	"uart init 3: ttyS3->PAD_FUART_TX/ PAD_FUART_RX"
	"uart putChar [char]- \n"
	"uart getChar - \n";
int probe = 0;
extern int ms_uart_init(U8 u8_Port, U32 u32_baudRate);
extern void ms_uart_putc(const char c);
extern int ms_uart_getc (void);

int do_uart (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int dev;
    int baud;
    char c;
    char *cmd;
    cmd = argv[1];

    if(strcmp(cmd, "init")==0)
    {
        if (argc < 3)
        {
            printf("uart init ERROR!\r\n");
            printf("uart init [port] [baudrate]\r\n");
            printf("eg: uart init 3 115200 \r\n");
            return CMD_RET_USAGE;
        }

        dev = (int)simple_strtoul(argv[2], NULL, 10);
        baud = (int)simple_strtoul(argv[3], NULL, 10);

        ms_uart_init(dev, baud);
        probe +=1;
    }
    else if(strcmp(cmd, "putchar")==0)
    {
        if(probe)
        {          
            c = (int)simple_strtoul(argv[2], NULL, 10);
            ms_uart_putc(c);
        }
        else
             printf("please run uart init first!\r\n");
    }
    else if(strcmp(cmd, "getchar")==0)
    {
        if(probe)
        {
            printf("%x\r\n", ms_uart_getc());
        }
        else
             printf("please run uart init first!\r\n");
    }
    else
        printf("error \r\n");
    return 0;
}


U_BOOT_CMD(
	uart, CONFIG_SYS_MAXARGS, 1, do_uart,
	"UART sub-system", uart_help_text
);

int do_setclk(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if(argc != 2)
    {
        return  CMD_RET_USAGE;
    }
    int clk = simple_strtoul(argv[1], NULL, 10);
    printf("clk = ");
    if(clk == 108)
    {
        SETREG8(GET_REG8_ADDR(RIU_BASE_ADDR, 0x1038E2), BIT9);  //mpll_216_force_on
        OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0E41), 0x1C);  //1C:108M, 18:86M 14:72M 10:54M 4:27M 8:36M C:43MH
        SETREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0E41), BIT6);  //glitch mux
        printf("108M\n");
    }
    else if(clk == 86)
    {
        SETREG8(GET_REG8_ADDR(RIU_BASE_ADDR, 0x1038E2), BIT14);  //mpll_86_force_on
        CLRREG8(GET_REG8_ADDR(RIU_BASE_ADDR, 0x1038DA), BIT5);  //86m clock to digpm setting, default off
        OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0E41), 0x18);  //1C:108M, 18:86M 14:72M 10:54M 4:27M 8:36M C:43MH
        SETREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0E41), BIT6);  //glitch mux
        printf("86M\n");
    }
    else if (clk == 54)
    {
        OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0E41), 0x10);  //1C:108M, 18:86M 14:72M 10:54M 4:27M 8:36M C:43MH
        SETREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0E41), BIT6);  //glitch mux
        printf("54M\n");
    }
    else
    {
        printf("unknow clk\n");
        printf("5C:108M, 58:86M 50:54M \n");
        printf("now clk is 0x%x\n",INREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0E41)));
        return 0;
    }
        OUTREG16(GET_REG16_ADDR(REG_ADDR_BASE_PM_GPIO, 0x1), 0x1011);//SPI_CK driving
    return 0;
}

U_BOOT_CMD(
	setclk, CONFIG_SYS_MAXARGS, 1, do_setclk,
	"set spi clk",
	"\nsetclk 54 - set spi clk 54M\n"
	"setclk 86 - set spi clk 86M\n"
	"setclk 108 - set spi clk 108M\n"
);


