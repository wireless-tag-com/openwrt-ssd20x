/*
* mhal_sata_host.h- Sigmastar
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

#ifndef _MHAL_SATA_HOST_H_
#define _MHAL_SATA_HOST_H_

#define SSTAR_SATA_DTS_NAME "sstar,sata"

#define SSTAR_RIU_BASE      0xFD000000

#define REG_PM_SLEEP_BASE   0x000E00
#define REG_PM_TOP_BASE      0x001E00
#define REG_CLKGEN_BASE      0x103800

#ifndef BIT	//for Linux_kernel type, BIT redefined in <linux/bitops.h>
    #define BIT(_bit_)                  (1 << (_bit_))
#endif
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

#define REG_PD_XTAL_HV             (REG_PM_SLEEP_BASE + (0x3D<<1))
#define SATA_HV_CRYSTAL_CLK_MASK   BMASK(5:0)

#define REG_CHIP_INFORM_SHADOW     (REG_PM_TOP_BASE + (0x67<<1))

#define REG_CKG_SATA_FCLK          (REG_CLKGEN_BASE + (0x46<<1))
#define REG_CKG_SATA_PM            (REG_CLKGEN_BASE + (0x6C<<1))
#define REG_CKG_SATA_AXI           (REG_CLKGEN_BASE + (0x6E<<1))
#define CKG_SATA_FCLK_PHY_GATED    BIT(0)
#define CKG_SATA_FCLK_PHY_INVERT   BIT(1)
#define CKG_SATA_FCLK_PHY_MASK     BMASK(3:2)
#define CKG_SATA_FCLK_108MHZ       (0 << 2)
#define CKG_SATA_FCLK_GATED        BIT(8)
#define CKG_SATA_FCLK_INVERT       BIT(9)
#define CKG_SATA_FCLK_MASK         BMASK(11:10)
#define CKG_SATA_FCLK_432MHZ       (0 << 2)

#define SATA_GHC_0          0x1A2800
#define SATA_GHC_0_P0       0x1A2840
#define SATA_GHC_0_MISC     0x152500
#define SATA_GHC_0_PHY      0x152600
#define SATA_GHC_0_PHY_ANA  0x152700

#define REG_SATA_PHYD_REG_0A         (SATA_GHC_0_PHY + (0x0A<<1))
#define REG_SATA_PHYD_REG_0B         (SATA_GHC_0_PHY + (0x0B<<1))
#define REG_SATA_PHYD_REG_0E         (SATA_GHC_0_PHY + (0x0E<<1))
#define REG_SATA_PHYD_REG_26         (SATA_GHC_0_PHY + (0x26<<1))
#define REG_SATA_PHYD_REG_2A         (SATA_GHC_0_PHY + (0x2A<<1))
#define REG_SATA_PHYD_REG_3C         (SATA_GHC_0_PHY + (0x3C<<1))
#define REG_SATA_PHYD_REG_3E         (SATA_GHC_0_PHY + (0x3E<<1))
#define REG_SATA_PHYD_REG_40         (SATA_GHC_0_PHY + (0x40<<1))
#define REG_SATA_PHYD_REG_46         (SATA_GHC_0_PHY + (0x46<<1))
#define REG_SATA_PHYD_REG_50         (SATA_GHC_0_PHY + (0x50<<1))
#define REG_SATA_PHYD_REG_61         (SATA_GHC_0_PHY + (0x61<<1))
#define REG_SATA_PHYD_REG_64         (SATA_GHC_0_PHY + (0x64<<1))


#define REG_SATA_PHY_CLK_PMALIVE_SEL (SATA_GHC_0_PHY_ANA + (0x04<<1))
#define REG_SATA_PHY_REG_20          (SATA_GHC_0_PHY_ANA + (0x20<<1))
#define REG_SATA_PHY_REG_30          (SATA_GHC_0_PHY_ANA + (0x30<<1))
#define REG_SATA_PHY_REG_33          (SATA_GHC_0_PHY_ANA + (0x33<<1))
#define REG_SATA_PHY_REG_3E          (SATA_GHC_0_PHY_ANA + (0x3E<<1))
#define REG_SATA_PHY_SYNTH_SLD       (SATA_GHC_0_PHY_ANA + (0x44<<1))
#define REG_SATA_PHY_TXPLL_DET_SW    (SATA_GHC_0_PHY_ANA + (0x60<<1))
#define REG_SATA_PHY_REG_70          (SATA_GHC_0_PHY_ANA + (0x70<<1))

// Bank 0x1A28 is x32 riu(32bit)
#define SATA_GHC_0_ADDRESS_START (SSTAR_RIU_BASE + (0x1A2800 << 1))
#define SATA_GHC_0_ADDRESS_END   (SSTAR_RIU_BASE + (0x1A283F << 1))
#define SATA_GHC_0_P0_ADDRESS_START (SSTAR_RIU_BASE + (0x1A2880 << 1))
#define SATA_GHC_0_P0_ADDRESS_END   (SSTAR_RIU_BASE + (0x1A28BF << 1))
#define SATA_MISC_0_ADDRESS_START  (SSTAR_RIU_BASE + (0x152500 << 1))
#define SATA_MISC_0_ADDRESS_END    (SSTAR_RIU_BASE + (0x1525FE << 1))

#define SATA_SDMAP_RIU_BASE         SSTAR_RIU_BASE //0xFD000000

// MIU interval, the gap between MIU0 and MIU1, chip dependent
#define MIU_INTERVAL_SATA 0x60000000 // for K6

// Local Definition, internal SATA MAC SRAM address
#define AHCI_P0CLB                  0x18001000
#define AHCI_P0FB                   0x18001100
#define AHCI_CTBA0                  0x18001200

#define SATA_PORT_NUM               1
#define SATA_CMD_HEADER_SIZE        0x400
#define SATA_FIS_SIZE               0x100

enum
{
    /* global controller registers */
    SS_HOST_CAP             = 0x00, /* host capabilities */
    SS_HOST_CTL             = 0x04, /* global host control */
    SS_HOST_IRQ_STAT        = 0x08, /* interrupt status */
    SS_HOST_PORTS_IMPL      = 0x0c, /* bitmap of implemented ports */
    SS_HOST_VERSION         = 0x10, /* AHCI spec. version compliancy */
    SS_HOST_CAP2            = 0x24, /* host capabilities, extended */

    /* HOST_CTL bits - HOST_CAP, 0x00 */
    //SS_HOST_RESET           = (1 << 0),  /* reset controller; self-clear */
    //SS_HOST_IRQ_EN          = (1 << 1),  /* global IRQ enable */
    //SS_HOST_AHCI_EN         = (1 << 31), /* AHCI enabled */

    /* Registers for each SATA port */
    SS_PORT_LST_ADDR        = 0x00, /* command list DMA addr */
    SS_PORT_LST_ADDR_HI     = 0x04, /* command list DMA addr hi */
    SS_PORT_FIS_ADDR        = 0x08, /* FIS rx buf addr */
    SS_PORT_FIS_ADDR_HI     = 0x0c, /* FIS rx buf addr hi */
    SS_PORT_IRQ_STAT        = 0x10, /* interrupt status */
    SS_PORT_IRQ_MASK        = 0x14, /* interrupt enable/disable mask */
    SS_PORT_CMD             = 0x18, /* port command */
    SS_PORT_TFDATA          = 0x20, /* taskfile data */
    SS_PORT_SIG             = 0x24, /* device TF signature */
    SS_PORT_SCR_STAT        = 0x28, /* SATA phy register: SStatus */
    SS_PORT_SCR_CTL         = 0x2c, /* SATA phy register: SControl */
    SS_PORT_SCR_ERR         = 0x30, /* SATA phy register: SError */
    SS_PORT_SCR_ACT         = 0x34, /* SATA phy register: SActive */
    SS_PORT_CMD_ISSUE       = 0x38, /* command issue */
    SS_PORT_SCR_NTF         = 0x3c, /* SATA phy register: SNotification */
    SS_PORT_DMA_CTRL        = 0x70, /* SATA phy register: SNotification */

    /* PORT_CMD bits */
    SS_PORT_CMD_ASP         = (1 << 27), /* Aggressive Slumber/Partial */
    SS_PORT_CMD_ALPE        = (1 << 26), /* Aggressive Link PM enable */
    SS_PORT_CMD_ATAPI       = (1 << 24), /* Device is ATAPI */
    SS_PORT_CMD_FBSCP       = (1 << 22), /* FBS Capable Port */
    SS_PORT_CMD_PMP         = (1 << 17), /* PMP attached */
    SS_PORT_CMD_LIST_ON     = (1 << 15), /* cmd list DMA engine running */
    SS_PORT_CMD_FIS_ON      = (1 << 14), /* FIS DMA engine running */
    SS_PORT_CMD_FIS_RX      = (1 << 4), /* Enable FIS receive DMA engine */
    SS_PORT_CMD_CLO         = (1 << 3), /* Command list override */
    SS_PORT_CMD_POWER_ON    = (1 << 2), /* Power up device */
    SS_PORT_CMD_SPIN_UP     = (1 << 1), /* Spin up device */
    SS_PORT_CMD_START       = (1 << 0), /* Enable port DMA engine */

    SS_PORT_CMD_ICC_MASK    = (0xf << 28), /* i/f ICC state mask */
    SS_PORT_CMD_ICC_ACTIVE  = (0x1 << 28), /* Put i/f in active state */
    SS_PORT_CMD_ICC_PARTIAL = (0x2 << 28), /* Put i/f in partial state */
    SS_PORT_CMD_ICC_SLUMBER = (0x6 << 28), /* Put i/f in slumber state */

    /*  Status Error  */
    SS_AHCI_PORT_SERR_RDIE  = (1 << 0),    /* Recovered Data Integrity Error */
    SS_AHCI_PORT_SERR_RCE   = (1 << 1),    /* Recovered Communications Error */
    /* Bit 2 ~ 7 Reserved */
    SS_AHCI_PORT_SERR_TDIE  = (1 << 8),    /* Transient Data Integrity Error */
    SS_AHCI_PORT_SERR_PCDIE = (1 << 9),    /* Persistent    Communication    or    Data    Integrity    Error */
    SS_AHCI_PORT_SERR_PE    = (1 << 10),   /* Protocol  Error */
    SS_AHCI_PORT_SERR_IE    = (1 << 11),   /* Internal  Error */
    /* Bit 15 ~ 12 Reserved */
    SS_AHCI_PORT_SERR_PRDYC = (1 << 16),   /* PhyRdy  Change */
    SS_AHCI_PORT_SERR_PIE   = (1 << 17),   /* Phy  Internal  Error */
    SS_AHCI_PORT_SERR_COMW  = (1 << 18),   /* Comm Wake Detect */
    SS_AHCI_PORT_SERR_TDE   = (1 << 19),   /* 10B  to  8B  Decode  Error  */
    SS_AHCI_PORT_SERR_DE    = (1 << 20),   /* Disparity Error <= Not Use by AHCI  */
    SS_AHCI_PORT_SERR_CRCE  = (1 << 21),   /* CRC Error  */
    SS_AHCI_PORT_SERR_HE    = (1 << 22),   /* Handshake  Error */
    SS_AHCI_PORT_SERR_LSE   = (1 << 23),   /* Link Sequence Error  */
    SS_AHCI_PORT_SERR_TSTE  = (1 << 24),   /* Transport  state  transition  error  */
    SS_AHCI_PORT_SERR_UFIS  = (1 << 25),   /* Unknown FIS Type  */
    SS_AHCI_PORT_SERR_EXC   = (1 << 26),   /* Exchanged :  a  change  in device  presence  has  been  detected */
    /* Bit 31 ~ 27 Reserved */
};

enum
{
    E_PORT_SPEED_MASK           = (0xF << 4),
    E_PORT_SPEED_NO_RESTRICTION = (0x0 < 4),
    E_PORT_SPEED_GEN1           = (0x1 << 4),
    E_PORT_SPEED_GEN2           = (0x2 << 4),
    E_PORT_SPEED_GEN3           = (0x3 << 4),

    E_PORT_DET_MASK             = (0xF << 0),  //Device  Detection  Initialization
    E_PORT_DET_NODEVICE_DETECT  = 0x0,
    E_PORT_DET_HW_RESET         = 0x1, // Cause HW send initial sequence
    E_PORT_DET_DISABLE_PHY      = 0x4, // Put PHY into Offline
    E_PORT_DET_DEVICE_NOEST     = 0x1, // not established
    E_PORT_DET_DEVICE_EST       = 0x3,  // established
    E_PORT_DET_PHY_OFFLINE      = 0x4, // Put PHY into Offline

    DATA_COMPLETE_INTERRUPT     = (1 << 31),

    /* PORT_IRQ_{STAT,MASK} bits */
    //PORT_IRQ_COLD_PRES          = (1 << 31), /* cold presence detect */
    //PORT_IRQ_TF_ERR             = (1 << 30), /* task file error */
    //PORT_IRQ_HBUS_ERR           = (1 << 29), /* host bus fatal error */
    //PORT_IRQ_HBUS_DATA_ERR      = (1 << 28), /* host bus data error */
    //PORT_IRQ_IF_ERR             = (1 << 27), /* interface fatal error */
    //PORT_IRQ_IF_NONFATAL        = (1 << 26), /* interface non-fatal error */
    //PORT_IRQ_OVERFLOW           = (1 << 24), /* xfer exhausted available S/G */
    //PORT_IRQ_BAD_PMP            = (1 << 23), /* incorrect port multiplier */

    //PORT_IRQ_PHYRDY             = (1 << 22), /* PhyRdy changed */
    //PORT_IRQ_DEV_ILCK           = (1 << 7), /* device interlock */
    //PORT_IRQ_CONNECT            = (1 << 6), /* port connect change status */
    //PORT_IRQ_SG_DONE            = (1 << 5), /* descriptor processed */
    //PORT_IRQ_UNK_FIS            = (1 << 4), /* unknown FIS rx'd */
    //PORT_IRQ_SDB_FIS            = (1 << 3), /* Set Device Bits FIS rx'd */
    //PORT_IRQ_DMAS_FIS           = (1 << 2), /* DMA Setup FIS rx'd */
    //PORT_IRQ_PIOS_FIS           = (1 << 1), /* PIO Setup FIS rx'd */
    //PORT_IRQ_D2H_REG_FIS        = (1 << 0), /* D2H Register FIS rx'd */

    //PORT_IRQ_FREEZE     = PORT_IRQ_HBUS_ERR |
    //    PORT_IRQ_IF_ERR |
    //    PORT_IRQ_CONNECT |
    //    PORT_IRQ_PHYRDY |
    //    PORT_IRQ_UNK_FIS |
    //    PORT_IRQ_BAD_PMP,
    //PORT_IRQ_ERROR      = PORT_IRQ_FREEZE |
    //    PORT_IRQ_TF_ERR |
    //    PORT_IRQ_HBUS_DATA_ERR,
    //DEF_PORT_IRQ        = PORT_IRQ_ERROR | PORT_IRQ_SG_DONE |
    //    PORT_IRQ_SDB_FIS | PORT_IRQ_DMAS_FIS |
    //    PORT_IRQ_PIOS_FIS | PORT_IRQ_D2H_REG_FIS,
};


/*
 *          Host Controller MISC Register
 */
enum
{
    SATA_MISC_CFIFO_ADDRL     =((0x10 <<1 )<<1),
    SATA_MISC_CFIFO_ADDRH     =((0x11 <<1 )<<1),
    SATA_MISC_CFIFO_WDATAL    =((0x12 <<1 )<<1),
    SATA_MISC_CFIFO_WDATAH    =((0x13 <<1 )<<1),
    SATA_MISC_CFIFO_RDATAL    =((0x14 <<1 )<<1),
    SATA_MISC_CFIFO_RDATAH    =((0x15 <<1 )<<1),
    SATA_MISC_CFIFO_RORW      =((0x16 <<1 )<<1),
    SATA_MISC_CFIFO_ACCESS    =((0x17 <<1 )<<1),
    SATA_MISC_ACCESS_MODE     =((0x18 <<1 )<<1),
    SATA_MISC_AMBA_MUXRST     =((0x21 <<1 )<<1),
    SATA_MISC_HBA_LADDR       =((0x24 <<1 )<<1),
    SATA_MISC_HBA_HADDR       =((0x25 <<1 )<<1),
    SATA_MISC_CMD_LADDR       =((0x26 <<1 )<<1),
    SATA_MISC_CMD_HADDR       =((0x27 <<1 )<<1),
    SATA_MISC_DATA_ADDR       =((0x28 <<1 )<<1),
    SATA_MISC_ENRELOAD        =((0x29 <<1 )<<1),
    SATA_MISC_AMBA_ARBIT      =((0x2A <<1 )<<1),
    SATA_MISC_AMBA_PGEN       =((0x30 <<1 )<<1),
    SATA_MISC_AGEN_F_VAL      =((0x35 <<1 )<<1),
    SATA_MISC_HOST_SWRST      =((0x50 <<1 )<<1),
    SATA_MISC_HOST_NEAR       =((0x51 <<1 )<<1),
    SATA_MISC_FPGA_EN         =((0x55 <<1 )<<1),
};

#define INT_SATA_PHY_RST_DONE                  BIT0
#define INT_SATA_PHY_COWAKE                    BIT1
#define INT_SATA_PHY_COMINIT                   BIT2
#define INT_SATA_PHY_SIG_DET                   BIT3
#define INT_SATA_PHY_CALIBRATED                BIT5
#define INT_SATA_PHY_RX_DATA_VLD_PRE_0         BIT6
#define INT_SATA_SYMBOL_LOCK                   BIT7
#define INT_SATA_PHY_RXPLL_LOCK                BIT8
#define INT_SATA_PHY_RXPLL_FREQ_DET_DONE_FLAG  BIT9
#define INT_SATA_PHY_RXPLL_FREQ_LOCK_FLAG      BIT10
#define INT_SATA_PHY_RXPLL_FREQ_UNLOCK_FLAG    BIT11
#define INT_SATA_PHY_TXPLL_LOCK                BIT12
#define INT_SATA_PHY_TXPLL_FREQ_DET_DONE_FLAG  BIT13
#define INT_SATA_PHY_TXPLL_FREQ_LOCK_FLAG      BIT14
#define INT_SATA_PHY_TXPLL_FREQ_UNLOCK_FLAG    BIT15

void MHal_SATA_Clock_Config(phys_addr_t misc_base, phys_addr_t port_base, bool enable);
void MHal_SATA_HW_Inital(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base);
void MHal_SATA_Setup_Port_Implement(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base);
phys_addr_t MHal_SATA_bus_address(phys_addr_t phy_address);
u32 MHal_SATA_get_max_speed(void);

#endif
