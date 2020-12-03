/*
* mspi.c- Sigmastar
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
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/dma-mapping.h>
#include "ms_platform.h"

#if defined(CONFIG_MS_PADMUX)
#include "mdrv_padmux.h"
#include "mdrv_puse.h"
#include "gpio.h"
#endif

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

bool gbInitFlag = false;
static struct mutex		hal_mspi_lock;
static bool SUPPORT_DMA = true;

//-------------------------------------------------------------------------------------------------
//  RegbaseAddr  Disc
//-------------------------------------------------------------------------------------------------
#define mspi_dbg 0
#if mspi_dbg == 1
        #define mspi_dbgmsg(args...) printk(args)
#else
        #define mspi_dbgmsg(args...) do{}while(0)
#endif

#define U8 u8
#define U16 u16
#define U32 u32
#define BOOL bool
#define TRUE true
#define FALSE false

#define SUPPORT_SPI_1 0

#define MAX_SUPPORT_BITS    16

#define BANK_TO_ADDR32(b) (b<<9)
#define BANK_SIZE 0x200

#define MS_BASE_REG_RIU_PA 		0x1F000000

#define MSPI0_BANK_ADDR   0x1110
#define MSPI1_BANK_ADDR   0x1111
#define CLK__BANK_ADDR    0x1038
#define CHIPTOP_BANK_ADDR 0x101E
#define MOVDMA_BANK_ADDR 0x100B

#define BASE_REG_MSPI0_ADDR			MSPI0_BANK_ADDR*0x200 //GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x111000)
#define BASE_REG_MSPI1_ADDR			MSPI1_BANK_ADDR*0x200 //GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x111100)
#define BASE_REG_CLK_ADDR			CLK__BANK_ADDR*0x200 //GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x103800)
#define BASE_REG_CHIPTOP_ADDR       CHIPTOP_BANK_ADDR*0x200 //GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x101E00)
//-------------------------------------------------------------------------------------------------
//  Hardware Register Capability
//-------------------------------------------------------------------------------------------------
#define MSPI_WRITE_BUF_OFFSET          0x40
#define MSPI_READ_BUF_OFFSET           0x44
#define MSPI_WBF_SIZE_OFFSET           0x48
#define MSPI_RBF_SIZE_OFFSET           0x48
    // read/ write buffer size
    #define MSPI_RWSIZE_MASK               0xFF
    #define MSPI_RSIZE_BIT_OFFSET          0x8
    #define MAX_READ_BUF_SIZE              0x8
    #define MAX_WRITE_BUF_SIZE             0x8
// CLK config
#define MSPI_CTRL_OFFSET               0x49
#define MSPI_CLK_CLOCK_OFFSET          0x49
    #define MSPI_CLK_CLOCK_BIT_OFFSET      0x08
    #define MSPI_CLK_CLOCK_MASK            0xFF
    #define MSPI_CLK_PHASE_MASK            0x40
    #define MSPI_CLK_PHASE_BIT_OFFSET      0x06
    #define MSPI_CLK_POLARITY_MASK         0x80
	#define MSPI_CLK_POLARITY_BIT_OFFSET   0x07
    #define MSPI_CLK_PHASE_MAX             0x1
    #define MSPI_CLK_POLARITY_MAX          0x1
    #define MSPI_CLK_CLOCK_MAX             0x7
// DC config
#define MSPI_DC_MASK                   0xFF
#define MSPI_DC_BIT_OFFSET             0x08
#define MSPI_DC_TR_START_OFFSET        0x4A
    #define MSPI_DC_TRSTART_MAX            0xFF
#define MSPI_DC_TR_END_OFFSET          0x4A
    #define MSPI_DC_TREND_MAX              0xFF
#define MSPI_DC_TB_OFFSET              0x4B
    #define MSPI_DC_TB_MAX                 0xFF
#define MSPI_DC_TRW_OFFSET             0x4B
    #define MSPI_DC_TRW_MAX                0xFF
// Frame Config
#define MSPI_FRAME_WBIT_OFFSET         0x4C
#define MSPI_FRAME_RBIT_OFFSET         0x4E
    #define MSPI_FRAME_BIT_MAX             0x07
    #define MSPI_FRAME_BIT_MASK            0x07
    #define MSPI_FRAME_BIT_FIELD           0x03
#define MSPI_LSB_FIRST_OFFSET          0x50
#define MSPI_TRIGGER_OFFSET            0x5A
#define MSPI_DONE_OFFSET               0x5B
#define MSPI_DONE_CLEAR_OFFSET         0x5C
#define MSPI_CHIP_SELECT_OFFSET        0x5F

#define MSPI_FULL_DEPLUX_RD_CNT (0x77)
#define MSPI_FULL_DEPLUX_RD00 (0x78)
#define MSPI_FULL_DEPLUX_RD01 (0x78)
#define MSPI_FULL_DEPLUX_RD02 (0x79
#define MSPI_FULL_DEPLUX_RD03 (0x79)
#define MSPI_FULL_DEPLUX_RD04 (0x7a)
#define MSPI_FULL_DEPLUX_RD05 (0x7a)
#define MSPI_FULL_DEPLUX_RD06 (0x7b)
#define MSPI_FULL_DEPLUX_RD07 (0x7b)

#define MSPI_FULL_DEPLUX_RD08 (0x7c)
#define MSPI_FULL_DEPLUX_RD09 (0x7c)
#define MSPI_FULL_DEPLUX_RD10 (0x7d)
#define MSPI_FULL_DEPLUX_RD11 (0x7d)
#define MSPI_FULL_DEPLUX_RD12 (0x7e)
#define MSPI_FULL_DEPLUX_RD13 (0x7e)
#define MSPI_FULL_DEPLUX_RD14 (0x7f)
#define MSPI_FULL_DEPLUX_RD15 (0x7f)

//chip select bit map
    #define MSPI_CHIP_SELECT_MAX           0x07
// control bit
#define MSPI_DONE_FLAG                 0x01
#define MSPI_TRIGGER                   0x01
#define MSPI_CLEAR_DONE                0x01
#define MSPI_INT_ENABLE                0x04
#define MSPI_RESET                     0x02
#define MSPI_ENABLE                    0x01

// clk_mspi0
#define MSPI0_CLK_CFG                   0x33//bit 2 ~bit 3
    #define  MSPI0_CLK_108M         0x00
    #define  MSPI0_CLK_54M          0x04
    #define  MSPI0_CLK_12M          0x08
    #define  MSPI0_CLK_MASK         0x0F
// clk_mspi1
#define MSPI1_CLK_CFG                   0x33 //bit 10 ~bit 11
    #define  MSPI1_CLK_108M         0x0000
    #define  MSPI1_CLK_54M          0x0400
    #define  MSPI1_CLK_12M          0x0800
    #define  MSPI1_CLK_MASK         0x0F00

// clk_mspi
#define MSPI_CLK_CFG                    0x33
    #define MSPI_SELECT_0           0x0000
    #define MSPI_SELECT_1           0x4000
    #define MSPI_CLK_MASK           0xF000
	
//CHITOP 101E mspi mode select
#define MSPI0_MODE          0x0C //bit0~bit1
    #define MSPI0_MODE_MASK 0x07
#define MSPI1_MODE          0x0C //bit4~bit5
    #define MSPI1_MODE_MASK 0x70
#define EJTAG_MODE          0xF
    #define EJTAG_MODE_1    0x01
    #define EJTAG_MODE_2    0x02
    #define EJTAG_MODE_3    0x03
    #define EJTAG_MODE_MASK 0x03

//MOVDMA 100B
#define MOV_DMA_SRC_ADDR_L 0x03
#define MOV_DMA_SRC_ADDR_H 0x04
#define MOV_DMA_DST_ADDR_L 0x05
#define MOV_DMA_DST_ADDR_H 0x06
#define MOV_DMA_BYTE_CNT_L 0x07
#define MOV_DMA_BYTE_CNT_H 0x08
#define DMA_MOVE0_IRQ_CLR  0x28
#define MOV_DMA_IRQ_FINAL_STATUS    0x2A
#define DMA_MOVE0_ENABLE 0x00
#define DMA_RW         0x50 //0 for dma write to device, 1 for dma read from device
    #define DMA_READ 0x01
    #define DMA_WRITE 0x00
#define DMA_DEVICE_MODE 0x51
#define DMA_DEVICE_SEL 0x52

//spi dma
#define MSPI_DMA_DATA_LENGTH_L 0x30
#define MSPI_DMA_DATA_LENGTH_H 0x31
#define MSPI_DMA_ENABLE 0x32
#define MSPI_DMA_RW_MODE 0x33
    #define MSPI_DMA_WRITE 0x00
    #define MSPI_DMA_READ 0x01
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define READ_BYTE(_reg)             (*(volatile u8*)(_reg))
#define READ_WORD(_reg)             (*(volatile u16*)(_reg))
#define READ_LONG(_reg)             (*(volatile u32*)(_reg))
#define WRITE_BYTE(_reg, _val)      {(*((volatile u8*)(_reg))) = (u8)(_val); }
#define WRITE_WORD(_reg, _val)      {(*((volatile u16*)(_reg))) = (u16)(_val); }
#define WRITE_LONG(_reg, _val)      {(*((volatile u32*)(_reg))) = (u32)(_val); }
#define WRITE_WORD_MASK(_reg, _val, _mask)  {(*((volatile u16*)(_reg))) = ((*((volatile u16*)(_reg))) & ~(_mask)) | ((u16)(_val) & (_mask)); }

// read 2 byte
#define MSPI_READ(_reg_)                    READ_WORD(bs->VirtMspBaseAddr + ((_reg_)<<2))
// write 2 byte
#define MSPI_WRITE(_reg_, _val_)            WRITE_WORD(bs->VirtMspBaseAddr + ((_reg_)<<2), (_val_))
//write 2 byte mask
#define MSPI_WRITE_MASK(_reg_, _val_, mask) WRITE_WORD_MASK(bs->VirtMspBaseAddr + ((_reg_)<<2), (_val_), (mask))

#define CLK_READ(_reg_)                     READ_WORD(bs->VirtClkBaseAddr + ((_reg_)<<2))
#define CLK_WRITE(_reg_, _val_)             WRITE_WORD(bs->VirtClkBaseAddr + ((_reg_)<<2), (_val_))

#define CHIPTOP_READ(_reg_)                 READ_WORD(bs->VirtChiptopBaseAddr + ((_reg_)<<2))
#define CHIPTOP_WRITE(_reg_, _val_)         WRITE_WORD(bs->VirtChiptopBaseAddr + ((_reg_)<<2), (_val_))

#define MOVDMA_READ(_reg_)                 READ_WORD(bs->VirtMovdmaBaseAddr + ((_reg_)<<2))
#define MOVDMA_WRITE(_reg_, _val_)         WRITE_WORD(bs->VirtMovdmaBaseAddr + ((_reg_)<<2), (_val_))

#define _HAL_MSPI_ClearDone()               MSPI_WRITE(MSPI_DONE_CLEAR_OFFSET,MSPI_CLEAR_DONE)
#define MAX_CHECK_CNT                       2000

#define MSPI_READ_INDEX 			   0x0
#define MSPI_WRITE_INDEX			   0x1

#define SPI_MIU0_BUS_BASE                  0x20000000
#define SPI_MIU1_BUS_BASE                  0xFFFFFFFF

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MSPI0,
    E_MSPI1,
    E_MSPI_MAX,
}MSPI_CH;


typedef enum
{
    E_MSPI_BIT_MSB_FIRST,
    E_MSPI_BIT_LSB_FIRST,
}MSPI_BitSeq_e;

typedef enum _HAL_CLK_Config
{
    E_MSPI_POL,
    E_MSPI_PHA,
    E_MSPI_CLK
}eCLK_config;

typedef enum _HAL_DC_Config
{
    E_MSPI_TRSTART,
    E_MSPI_TREND,
    E_MSPI_TB,
    E_MSPI_TRW
}eDC_config;

typedef struct
{
    u32 u32Clock;
	u8 u8Clock;
    bool BClkPolarity;
    bool BClkPhase;
    u32 u32MAXClk;
} MSPI_CLKConfig;


typedef struct
{
    u8 u8ClkSpi_cfg;
    u8 u8ClkSpi_DIV;
    u32 u32ClkSpi;
}ST_DRV_MSPI_CLK;

typedef enum
{
    E_MSPI_DBGLV_NONE,    //disable all the debug message
    E_MSPI_DBGLV_INFO,    //information
    E_MSPI_DBGLV_NOTICE,  //normal but significant condition
    E_MSPI_DBGLV_WARNING, //warning conditions
    E_MSPI_DBGLV_ERR,     //error conditions
    E_MSPI_DBGLV_CRIT,    //critical conditions
    E_MSPI_DBGLV_ALERT,   //action must be taken immediately
    E_MSPI_DBGLV_EMERG,   //system is unusable
    E_MSPI_DBGLV_DEBUG,   //debug-level messages
} MSPI_DbgLv;

typedef enum _MSPI_ERRORNOn {
     E_MSPI_OK = 0
    ,E_MSPI_INIT_FLOW_ERROR =1
    ,E_MSPI_DCCONFIG_ERROR =2
    ,E_MSPI_CLKCONFIG_ERROR =4
    ,E_MSPI_FRAMECONFIG_ERROR =8
    ,E_MSPI_OPERATION_ERROR = 0x10
    ,E_MSPI_PARAM_OVERFLOW = 0x20
    ,E_MSPI_MMIO_ERROR = 0x40
    ,E_MSPI_HW_NOT_SUPPORT = 0x80
    ,E_MSPI_NULL
} MSPI_ErrorNo;

typedef struct
{
    MSPI_CH eCurrentCH;
    char *VirtMspBaseAddr;
    char *VirtClkBaseAddr;
    char *VirtChiptopBaseAddr;
    char *VirtMovdmaBaseAddr;
} MSPI_BaseAddr_st;

static MSPI_BaseAddr_st _hal_msp = {
    .eCurrentCH = E_MSPI0,
    //.VirtMspBaseAddr      =  BASE_REG_MSPI0_ADDR,
    //.VirtClkBaseAddr      =  BASE_REG_CLK_ADDR,
    //.VirtChiptopBaseAddr  =  BASE_REG_CHIPTOP_ADDR,
};

typedef enum {
    E_MSPI_MODE0, //CPOL = 0,CPHA =0
    E_MSPI_MODE1, //CPOL = 0,CPHA =1
    E_MSPI_MODE2, //CPOL = 1,CPHA =0
    E_MSPI_MODE3, //CPOL = 1,CPHA =1
    E_MSPI_MODE_MAX,
} MSPI_Mode_Config_e;

typedef struct
{
    u8 u8TrStart;      //time from trigger to first SPI clock
    u8 u8TrEnd;        //time from last SPI clock to transferred done
    u8 u8TB;           //time between byte to byte transfer
    u8 u8TRW;          //time between last write and first read
} MSPI_DCConfig;

typedef struct
{
    u8 u8WBitConfig[8];      //bits will be transferred in write buffer
    u8 u8RBitConfig[8];      //bits Will be transferred in read buffer
} MSPI_FrameConfig;


#define MSTAR_SPI_TIMEOUT_MS	30000
#define MSTAR_SPI_MODE_BITS	(SPI_CPOL | SPI_CPHA /*| SPI_CS_HIGH | SPI_NO_CS | SPI_LSB_FIRST*/)

#define DRV_NAME	"spi"
struct mstar_spi {
	void __iomem *regs;
	struct clk *clk;
	int irq;
	int use_dma;
	int xfer_w_dma;
	struct completion done;
	const u8 *tx_buf;
	u8 *rx_buf;
	int len;
	int wsize;
    char *VirtMspBaseAddr;
    char *VirtClkBaseAddr;
    char *VirtChiptopBaseAddr;
    char *VirtMovdmaBaseAddr;
    char u8channel;
    u32  u32spi_mode;
    u8   bits_per_word;
};

static struct spi_board_info mstar_info = {
	.modalias	= "spidev",
};

static void _HAL_MSPI_CheckandSetBaseAddr(MSPI_CH eChannel)
{

#if 0
	if(eChannel == _hal_msp.eCurrentCH)
    {
        return;
    } else if(eChannel ==  E_MSPI0)
    {
        _hal_msp.eCurrentCH = E_MSPI0;
        _hal_msp.VirtMspBaseAddr     = BASE_REG_MSPI0_ADDR;
        _hal_msp.VirtClkBaseAddr     = BASE_REG_CLK_ADDR;
        _hal_msp.VirtChiptopBaseAddr = BASE_REG_CHIPTOP_ADDR,
        printk(KERN_ERR"[Lwc Debug] Set mspi0 base address : %x\n",_hal_msp.VirtMspBaseAddr);
        printk(KERN_ERR"[Lwc Debug] Set clk base address : %x\n",_hal_msp.VirtClkBaseAddr);
    } else if(eChannel ==  E_MSPI1)
    {
        _hal_msp.eCurrentCH = E_MSPI1;
        _hal_msp.VirtMspBaseAddr     = BASE_REG_MSPI1_ADDR;
        _hal_msp.VirtClkBaseAddr     = BASE_REG_CLK_ADDR;
        _hal_msp.VirtChiptopBaseAddr = BASE_REG_CHIPTOP_ADDR,
        printk(KERN_ERR"[Lwc Debug] Set mspi1 base address : %x\n",_hal_msp.VirtMspBaseAddr);
        printk(KERN_ERR"[Lwc Debug] Set clk base address : %x\n",_hal_msp.VirtClkBaseAddr);
    } else
    {
        //DEBUG_MSPI(E_MSPI_DBGLV_ERR,printk("[Mspi Error]FUN:%s MSPI Channel is out of range!\n",__FUNCTION__));
        printk("[Mspi Error]FUN: MSPI Channel is out of range!\n");
        return ;
    }

#endif
}
//------------------------------------------------------------------------------
/// Description : Reset  Frame register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
BOOL HAL_MSPI_Reset_FrameConfig(struct mstar_spi *bs,MSPI_CH eChannel)
{
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);
    // Frame reset
    MSPI_WRITE(MSPI_FRAME_WBIT_OFFSET, 0xFFF);
    MSPI_WRITE(MSPI_FRAME_WBIT_OFFSET+2, 0xFFF);
    MSPI_WRITE(MSPI_FRAME_RBIT_OFFSET, 0xFFF);
    MSPI_WRITE(MSPI_FRAME_RBIT_OFFSET+2, 0xFFF);
    mutex_unlock(&hal_mspi_lock);
    return TRUE;
}

//------------------------------------------------------------------------------
/// Description : MSPI	interrupt enable
/// @param bEnable \b OUT: enable or disable mspi interrupt
/// @return void:
//------------------------------------------------------------------------------
void HAL_MSPI_IntEnable(struct mstar_spi *bs,MSPI_CH eChannel,bool bEnable)
{
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr(eChannel);
    if(bEnable) {
        MSPI_WRITE(MSPI_CTRL_OFFSET,MSPI_READ(MSPI_CTRL_OFFSET)|MSPI_INT_ENABLE);
    } else {
        MSPI_WRITE(MSPI_CTRL_OFFSET,MSPI_READ(MSPI_CTRL_OFFSET)&(~MSPI_INT_ENABLE));
    }
    mutex_unlock(&hal_mspi_lock);
}

#if defined(CONFIG_MS_PADMUX)
static int _MSPI_IsPadSet(MSPI_CH eChannel)
{
    // important: need to modify if more MDRV_PUSE_SPI? defined
    if (eChannel == E_MSPI0 && PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_SPI0_CK) )
    {
        //printk("SPI: %d pad set by padmux driver!!\n", eChannel);
        return TRUE;
    }
    else
        return FALSE;
}
#endif

void HAL_MSPI_Init(struct mstar_spi *bs,MSPI_CH eChannel,u8 u8Mode)
{
    u16 TempData;
    //init  MSP
    //DEBUG_MSPI(E_MSPI_DBGLV_INFO,printk("HAL_MSPI_Init\n"));
    mspi_dbgmsg("HAL_MSPI_Init : Channel=%d spi mode=%d\n",eChannel,u8Mode);
    mutex_init(&hal_mspi_lock);
    if((eChannel > E_MSPI1) || (u8Mode < 1))
    {
        return;
    }
    else if((eChannel == E_MSPI0) && (u8Mode > 6))
    {
        return;
    }
#if SUPPORT_SPI_1
    else if((eChannel == E_MSPI1) && (u8Mode > 6))
    {
        return;
    }
#endif
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr(eChannel);

    MSPI_WRITE(MSPI_CTRL_OFFSET,MSPI_READ(MSPI_CTRL_OFFSET)|(MSPI_RESET|MSPI_ENABLE));

    if(eChannel == E_MSPI0)
    {
        // CLK SETTING
        TempData = CLK_READ(MSPI0_CLK_CFG);
        TempData &= ~(MSPI0_CLK_MASK);
        TempData |= MSPI0_CLK_108M;
        CLK_WRITE(MSPI0_CLK_CFG, TempData);

#if defined(CONFIG_MS_PADMUX)
        if (0 == mdrv_padmux_active() ||
            FALSE == _MSPI_IsPadSet(E_MSPI0) )
#endif
        {
            //printk("SPI: %d pad set by SPI driver!!\n", eChannel);
            //select mspi mode
            TempData = CHIPTOP_READ(MSPI0_MODE);
            TempData &= ~(MSPI0_MODE_MASK);
            TempData |= u8Mode;
            CHIPTOP_WRITE(MSPI0_MODE,TempData);

            //Disable jtag mode   // IO PAD conflict turn off jtag
            TempData = CHIPTOP_READ(EJTAG_MODE);
            if((u8Mode == 4 && TempData == EJTAG_MODE_1) ||
               (u8Mode == 3 && TempData == EJTAG_MODE_3) ){
                TempData &= ~(EJTAG_MODE_MASK);
                CHIPTOP_WRITE(EJTAG_MODE,TempData);
            }
        }
    }
#if SUPPORT_SPI_1
   else if (eChannel == E_MSPI1)
    {
        // CLK SETTING
        TempData = CLK_READ(MSPI1_CLK_CFG);
        TempData &= ~(MSPI1_CLK_MASK);
        TempData |= MSPI1_CLK_108M;
        CLK_WRITE(MSPI1_CLK_CFG, TempData);

#if defined(CONFIG_MS_PADMUX)
        if (0 == mdrv_padmux_active() ||
            FALSE == _MSPI_IsPadSet(E_MSPI1) )
#endif
        {
            //select mspi mode
            TempData = CHIPTOP_READ(MSPI1_MODE);
            TempData &= ~(MSPI1_MODE_MASK);
            TempData |= (u8Mode << 4);
            CHIPTOP_WRITE(MSPI1_MODE,TempData);
        }
    }
#endif
    mutex_unlock(&hal_mspi_lock);
    return;
}
//------------------------------------------------------------------------------
/// Description : config spi transfer timing
/// @param ptDCConfig    \b OUT  struct pointer of bits of buffer tranferred to slave config
/// @return NONE
//------------------------------------------------------------------------------
void HAL_MSPI_SetDcTiming (struct mstar_spi *bs,MSPI_CH eChannel, eDC_config eDCField, U8 u8DCtiming)
{
    U16 u16TempBuf = 0;
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr(eChannel);
    switch(eDCField) {
    case E_MSPI_TRSTART:
        u16TempBuf = MSPI_READ(MSPI_DC_TR_START_OFFSET);
        u16TempBuf &= (~MSPI_DC_MASK);
        u16TempBuf |= u8DCtiming;
        MSPI_WRITE(MSPI_DC_TR_START_OFFSET, u16TempBuf);
        break;
    case E_MSPI_TREND:
        u16TempBuf = MSPI_READ(MSPI_DC_TR_END_OFFSET);
        u16TempBuf &= MSPI_DC_MASK;
        u16TempBuf |= u8DCtiming << MSPI_DC_BIT_OFFSET;
        MSPI_WRITE(MSPI_DC_TR_END_OFFSET, u16TempBuf);
        break;
    case E_MSPI_TB:
        u16TempBuf = MSPI_READ(MSPI_DC_TB_OFFSET);
        u16TempBuf &= (~MSPI_DC_MASK);
        u16TempBuf |= u8DCtiming;
        MSPI_WRITE(MSPI_DC_TB_OFFSET, u16TempBuf);
        break;
    case E_MSPI_TRW:
        u16TempBuf = MSPI_READ(MSPI_DC_TRW_OFFSET);
        u16TempBuf &= MSPI_DC_MASK;
        u16TempBuf |= u8DCtiming << MSPI_DC_BIT_OFFSET;
        MSPI_WRITE(MSPI_DC_TRW_OFFSET, u16TempBuf);
        break;
    }

    mutex_unlock(&hal_mspi_lock);
}

static void _HAL_MSPI_RWBUFSize(struct mstar_spi *bs,BOOL Direct, U8 Size)
{
    U16 u16Data = 0;
    u16Data = MSPI_READ(MSPI_RBF_SIZE_OFFSET);
    //printk("===RWBUFSize:%d\n",Size);
    if(Direct == MSPI_READ_INDEX)
    {
        u16Data &= MSPI_RWSIZE_MASK;
        u16Data |= Size << MSPI_RSIZE_BIT_OFFSET;
    }
    else
    {
        u16Data &= ~MSPI_RWSIZE_MASK;
        u16Data |= Size;
    }
    MSPI_WRITE(MSPI_RBF_SIZE_OFFSET, u16Data);
}
//------------------------------------------------------------------------------
/// Description : check MSPI operation complete
/// @return TRUE :  operation complete
/// @return FAIL : failed timeout
//------------------------------------------------------------------------------
static U16 _HAL_MSPI_CheckDone(struct mstar_spi *bs)
{
/*
    U16 uCheckDoneCnt = 0;
    U16 uDoneFlag = 0;
    while(uCheckDoneCnt < MAX_CHECK_CNT) {
        uDoneFlag = MSPI_READ(MSPI_DONE_OFFSET);
        if(uDoneFlag & MSPI_DONE_FLAG) {
            printk("Done flag success!!!!!!!!!!!!!!!!!!!!!!!!\n");
            return TRUE;
        }
       // printk("...");
        uCheckDoneCnt++;
    }
    //DEBUG_MSPI(E_MSPI_DBGLV_ERR,printk("ERROR:MSPI Operation Timeout!!!!!\n"));
    printk("ERROR:MSPI Operation asasTimeout!!!!!\n");
    return FALSE;
*/

	return MSPI_READ(MSPI_DONE_OFFSET);
}

//------------------------------------------------------------------------------
/// Description : SPI chip select enable and disable
/// @param Enable \b OUT: enable or disable chip select
//------------------------------------------------------------------------------
static void _HAL_MSPI_ChipSelect(struct mstar_spi *bs,BOOL Enable ,U8 eSelect)
{
    U16 regdata = 0;
    U8 bitmask = 0;
    regdata = MSPI_READ(MSPI_CHIP_SELECT_OFFSET);
    if(Enable) {
        bitmask = ~(1 << eSelect);
        regdata &= bitmask;
    } else {
        bitmask = (1 << eSelect);
        regdata |= bitmask;
    }
    MSPI_WRITE(MSPI_CHIP_SELECT_OFFSET, regdata);
}

//------------------------------------------------------------------------------
/// Description : Trigger MSPI operation
/// @return TRUE  : operation success
/// @return FALSE : operation timeout
//------------------------------------------------------------------------------
#ifdef _EN_MSPI_INTC_
BOOL _HAL_MSPI_Trigger(struct mstar_spi *bs)
{
	unsigned int timeout;
    // trigger operation
	reinit_completion(&bs->done);
    MSPI_WRITE(MSPI_TRIGGER_OFFSET,MSPI_TRIGGER);


	timeout = wait_for_completion_timeout(&bs->done,
    msecs_to_jiffies(MSTAR_SPI_TIMEOUT_MS));

    MSPI_WRITE(MSPI_RBF_SIZE_OFFSET,0x0);

    if (!timeout) {
        mspi_dbgmsg("timeout\n");
        //goto out;
    }
    else {
        if(bs->xfer_w_dma){ // clear MOVDMA finish status
            u16 intsrc = MOVDMA_READ(MOV_DMA_IRQ_FINAL_STATUS);
            MOVDMA_WRITE(DMA_MOVE0_IRQ_CLR, intsrc);
        }
    }

    // check operation complete
//    if(!_HAL_MSPI_CheckDone()) {
//        return FALSE;
//    }
    // clear done flag
//    _HAL_MSPI_ClearDone();
    // reset read/write buffer size
//    MSPI_WRITE(MSPI_RBF_SIZE_OFFSET,0x0);
    return TRUE;
}
#else
#define HW_MSPI_WAIT_TIMEOUT      (30000)
BOOL _HAL_MSPI_Trigger(struct mstar_spi *bs)
{
    U16 volatile time_remain = HW_MSPI_WAIT_TIMEOUT;
   // trigger operation
    MSPI_WRITE(MSPI_TRIGGER_OFFSET,MSPI_TRIGGER);


    while (time_remain--) {

        if (_HAL_MSPI_CheckDone(bs) == 1) { //done
            if(bs->xfer_w_dma){ // clear MOVDMA finish status
                u16 intsrc = MOVDMA_READ(MOV_DMA_IRQ_FINAL_STATUS);
                MOVDMA_WRITE(DMA_MOVE0_IRQ_CLR, intsrc);
            }
            _HAL_MSPI_ClearDone();  // for debug
            mspi_dbgmsg("<<<<<<<<<<<<<<<<<<< SPI_Done >>>>>>>>>>>>>>>>>\n");
            break;
        }
    }

    MSPI_WRITE(MSPI_RBF_SIZE_OFFSET,0x0);

    if (!time_remain) {
        mspi_dbgmsg("timeout\n");
        return FALSE;
    }
    return TRUE;
}
#endif
//-------------------------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b IN :pointer to receive data from MSPI read buffer
/// @param u16Size \ b OTU : read data size
/// @return TRUE  : read data success
/// @return FALSE : read data fail
//-------------------------------------------------------------------------------------------------
BOOL HAL_MSPI_Read(struct mstar_spi *bs, MSPI_CH eChannel, U8 *pData, U16 u16Size)
{
    U8  u8Index = 0;
    U16  u16TempBuf = 0;
    U16 i =0, j = 0;
    U8 shift;
    U8  isMsbBitMode = (bs->bits_per_word % 8) && !(bs->u32spi_mode & SPI_LSB_FIRST);

    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);
    for(i = 0; i < u16Size; i+= MAX_READ_BUF_SIZE) {
        u16TempBuf = u16Size - i;
        if(u16TempBuf > MAX_READ_BUF_SIZE) {
            j = MAX_READ_BUF_SIZE;
        } else {
            j = u16TempBuf;
        }
        _HAL_MSPI_RWBUFSize(bs,MSPI_READ_INDEX, j);

        _HAL_MSPI_Trigger(bs);

        for(u8Index = 0; u8Index < j; u8Index++) {

            if(u8Index & 1) {
                u16TempBuf = MSPI_READ((MSPI_READ_BUF_OFFSET + (u8Index >> 1)));
                //DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("read Buf data %x index %d\n",u16TempBuf, u8Index));
                if (isMsbBitMode)
                {
                    if (bs->bits_per_word < 8)
                    {
                        shift = 8 - bs->bits_per_word;
                        pData[u8Index]   = u16TempBuf >> (8 + shift);
                        pData[u8Index-1] = (u16TempBuf & 0xFF) >> shift;
                    }
                    else  //bits_per_word=9~15
                    {
                        shift = 16 - bs->bits_per_word;
                        pData[u8Index] = u16TempBuf >> 8;
                        pData[u8Index-1] = (u16TempBuf & 0xFF) >> shift;
                    }
                }
                else {
                    pData[u8Index] = u16TempBuf >> 8;
                    pData[u8Index-1] = u16TempBuf & 0xFF;
                }
            } else if(u8Index == (j -1)) {
                u16TempBuf = MSPI_READ((MSPI_READ_BUF_OFFSET + (u8Index >> 1)));
                //DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("read Buf data %x index %d\n",u16TempBuf, u8Index));
                if (isMsbBitMode) {
                    pData[u8Index] = (u16TempBuf & 0xFF) >> (8 - bs->bits_per_word);
                }
                else {
                    pData[u8Index] = u16TempBuf & 0xFF;
                }
            }

            //printk("******************* read Buf data %x index %d\n",u16TempBuf, u8Index);
        }
        pData+= j;
    }

    mutex_unlock(&hal_mspi_lock);
    return TRUE;
}

//------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b OUT :pointer to write  data to MSPI write buffer
/// @param u16Size \ b OTU : write data size
/// @return TRUE  : write data success
/// @return FALSE : wirte data fail
//------------------------------------------------------------------------------
BOOL HAL_MSPI_Write(struct mstar_spi *bs, MSPI_CH eChannel, U8 *pData, U16 u16Size)
{
    U8  u8Index = 0, u8TempBuf;
    U16 u16TempBuf = 0;
    U8 shift;
    U8  isMsbBitMode = (bs->bits_per_word % 8) && !(bs->u32spi_mode & SPI_LSB_FIRST);

    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);

    // bit mode: to send 1 0000 0101
    //     we get data[0]=0x01,data[1]=0x05 from users
    //     We must fill wbuf[0]=0x80,wbuf[1]=0x05 for HW MSB bit mode
    for(u8Index = 0; u8Index < u16Size; u8Index++) {
        if(u8Index & 1) {
            if (isMsbBitMode) {
                if (bs->bits_per_word < 8)
                {
                    shift = 8 - bs->bits_per_word;
                    u16TempBuf = (pData[u8Index] << (8+shift)) | (pData[u8Index-1] << shift);
                }
                else  //bits_per_word=9~15
                {
                    shift = 16 - bs->bits_per_word;
                    u16TempBuf = (pData[u8Index] << 8) | (pData[u8Index-1] << shift);
                }
            }
            else {
                u16TempBuf = (pData[u8Index] << 8) | pData[u8Index-1];
            }
            //DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("write Buf data %x index %d\n",u16TempBuf, u8Index));
            MSPI_WRITE((MSPI_WRITE_BUF_OFFSET + (u8Index >> 1)),u16TempBuf);
        } else if(u8Index == (u16Size -1)) {
            if (isMsbBitMode) {
                 u8TempBuf = pData[u8Index] << (8 - bs->bits_per_word);
             }
             else {
                 u8TempBuf = pData[u8Index];
             }
            //DEBUG_MSPI(E_MSPI_DBGLV_DEBUG,printk("write Buf data %x index %d\n",u8TempBuf, u8Index));
            MSPI_WRITE((MSPI_WRITE_BUF_OFFSET + (u8Index >> 1)),u8TempBuf);
        }
    }

    _HAL_MSPI_RWBUFSize(bs,MSPI_WRITE_INDEX, u16Size);
    _HAL_MSPI_Trigger(bs);
    // set write data size
    mutex_unlock(&hal_mspi_lock);
    return TRUE;
}

BOOL HAL_MSPI_FullDuplex(struct mstar_spi *bs , MSPI_CH eChannel, U8 *txdata, U8 *rxdata, U16 u16size)
{
    U16 fullDeplux_rd_cnt = 0;
    U16 *u16val = (U16*)rxdata;
    U16  i = 0;
    BOOL bRet = FALSE;

    /* tx */
    bRet = HAL_MSPI_Write(bs, eChannel, txdata, u16size);

    /*read duplux buffer*/
    fullDeplux_rd_cnt  = MSPI_READ(MSPI_FULL_DEPLUX_RD_CNT)&0xFF;

    for(i = 0; i < fullDeplux_rd_cnt/2; i++)
    {
        u16val[i]  = MSPI_READ(MSPI_FULL_DEPLUX_RD00+i);
    }

    if(fullDeplux_rd_cnt%2)
    {
        rxdata[fullDeplux_rd_cnt - 1]  = ((MSPI_READ(MSPI_FULL_DEPLUX_RD00 + fullDeplux_rd_cnt/2)) &0xFF);
    }

    return bRet;
}

//------------------------------------------------------------------------------
/// Description : Reset  CLK register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
BOOL HAL_MSPI_Reset_CLKConfig(struct mstar_spi *bs,MSPI_CH eChannel)
{
    U16 Tempbuf;
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);
    //reset clock
    Tempbuf = MSPI_READ(MSPI_CTRL_OFFSET);
    Tempbuf &= 0x3F;
    MSPI_WRITE(MSPI_CTRL_OFFSET, Tempbuf);
    mutex_unlock(&hal_mspi_lock);
    return TRUE;
}

U8 HAL_MSPI_DCConfigMax(eDC_config eDCField)
{
    switch(eDCField)
    {
    case E_MSPI_TRSTART:
        return MSPI_DC_TRSTART_MAX;
    case E_MSPI_TREND:
        return MSPI_DC_TREND_MAX;
    case E_MSPI_TB:
        return MSPI_DC_TB_MAX;
    case E_MSPI_TRW:
        return MSPI_DC_TRW_MAX;
    default:
        return 0;
    }
}

//------------------------------------------------------------------------------
/// Description : Set TrStart timing of DC config
/// @return E_MSPI_OK :
/// @return >1 : failed to set TrStart timing
//------------------------------------------------------------------------------
static U8 _MDrv_DC_TrStartSetting(struct mstar_spi *bs,U8 u8Channel,U8 TrStart)
{
    U8 u8TrStartMax;
    U8 errnum = E_MSPI_OK;

    u8TrStartMax = HAL_MSPI_DCConfigMax(E_MSPI_TRSTART);
    if(TrStart > u8TrStartMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetDcTiming(bs,(MSPI_CH)u8Channel,E_MSPI_TRSTART ,TrStart);
    return errnum;
}

//------------------------------------------------------------------------------
/// Description : Reset  DC register setting of MSPI
/// @param NONE
/// @return TRUE  : reset complete
//------------------------------------------------------------------------------
BOOL HAL_MSPI_Reset_DCConfig(struct mstar_spi *bs,MSPI_CH eChannel)
{
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);
    //DC reset
    MSPI_WRITE(MSPI_DC_TR_START_OFFSET, 0x00);
    MSPI_WRITE(MSPI_DC_TB_OFFSET, 0x00);
    mutex_unlock(&hal_mspi_lock);
    return TRUE;
}

//------------------------------------------------------------------------------
/// Description : Set MSPI chip select
/// @param u8CS \u8 OUT: MSPI chip select
/// @return void:
//------------------------------------------------------------------------------
void HAL_MSPI_SetChipSelect(struct mstar_spi *bs,MSPI_CH eChannel, BOOL Enable, U8  u8CS)
{
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);
    _HAL_MSPI_ChipSelect(bs, Enable, u8CS);
    mutex_unlock(&hal_mspi_lock);
}
// add to sync code from utopia for localdimming to set clk
void MDrv_MSPI_ChipSelect(struct mstar_spi *bs,U8 u8Channel,  BOOL Enable, U8 u8CS)
{
    HAL_MSPI_SetChipSelect(bs,(MSPI_CH)u8Channel,Enable,u8CS);
    return;
}

U32 HAL_MSPI_SelectCLK(struct mstar_spi *bs,U8 u8Channel) //Enable DMA CLK
{
    u16 TempData;

    TempData = CLK_READ(MSPI_CLK_CFG);
    if(u8Channel == E_MSPI0)//mspi0
    {
        // CLK SETTING
        TempData &= ~(MSPI_CLK_MASK);
        TempData |= MSPI_SELECT_0;
    }
    else if(u8Channel == E_MSPI1)//mspi1
    {
        // CLK SETTING
        TempData &= ~(MSPI_CLK_MASK);
        TempData |= MSPI_SELECT_1;
    }
    CLK_WRITE(MSPI_CLK_CFG, TempData);
    
    return 0;
}

//-------------------------------------------------------------------------------------------------
/// Description : read data from MSPI
/// @param pData \b IN :pointer to receive data from MSPI read buffer
/// @param u16Size \ b OTU : read data size
/// @return the errorno of operation
//-------------------------------------------------------------------------------------------------
U8 MDrv_MSPI_Read(struct mstar_spi *bs, U8 u8Channel, U8 *pData, U16 u16Size)
{
    //MSPI_ErrorNo errorno = E_MSPI_OK;

    U8  u8Index = 0;
    U8  u8TempFrameCnt=0;
    U8  U8TempLastFrameCnt=0;
    int  ret = 0;
    if(pData == NULL) {
        return E_MSPI_NULL;
    }
    u8TempFrameCnt = u16Size/MAX_WRITE_BUF_SIZE; //Cut data to frame by max frame size
    U8TempLastFrameCnt = u16Size%MAX_WRITE_BUF_SIZE; //Last data less than a MAX_WRITE_BUF_SIZE fame
    for (u8Index = 0; u8Index < u8TempFrameCnt; u8Index++) {
        ret = HAL_MSPI_Read(bs,(MSPI_CH)u8Channel,pData+u8Index*MAX_WRITE_BUF_SIZE,MAX_WRITE_BUF_SIZE);
        if (!ret) {
            return E_MSPI_OPERATION_ERROR;
        }
    }
    if(U8TempLastFrameCnt) {
        ret = HAL_MSPI_Read(bs,(MSPI_CH)u8Channel,pData+u8TempFrameCnt*MAX_WRITE_BUF_SIZE,U8TempLastFrameCnt);
    }
    if (!ret) {
        return E_MSPI_OPERATION_ERROR;
    }
    return E_MSPI_OK;
}
//------------------------------------------------------------------------------
/// Description : write data from MSPI
/// @param pData \b OUT :pointer to write  data to MSPI write buffer
/// @param u16Size \ b OTU : write data size
/// @return the errorno of operation
//------------------------------------------------------------------------------
U8 MDrv_MSPI_Write(struct mstar_spi *bs,U8 u8Channel, U8 *pData, U16 u16Size)
{
    U8  u8Index = 0;
    U8  u8TempFrameCnt=0;
    U8  U8TempLastFrameCnt=0;
    BOOL  ret = false;
    u8TempFrameCnt = u16Size/MAX_WRITE_BUF_SIZE; //Cut data to frame by max frame size
    U8TempLastFrameCnt = u16Size%MAX_WRITE_BUF_SIZE; //Last data less than a MAX_WRITE_BUF_SIZE fame
    for (u8Index = 0; u8Index < u8TempFrameCnt; u8Index++)
    {
        ret = HAL_MSPI_Write(bs, (MSPI_CH)u8Channel,pData+u8Index*MAX_WRITE_BUF_SIZE,MAX_WRITE_BUF_SIZE);
        if (!ret) {
            return E_MSPI_OPERATION_ERROR;
        }
    }

    if(U8TempLastFrameCnt)
    {
        ret = HAL_MSPI_Write(bs, (MSPI_CH)u8Channel,pData+u8TempFrameCnt*MAX_WRITE_BUF_SIZE,U8TempLastFrameCnt);
    }

    if (!ret) {
        return E_MSPI_OPERATION_ERROR;
    }
    return E_MSPI_OK;
}

BOOL MDrv_MSPI_FullDuplex(struct mstar_spi *bs, U8 u8Channel, U8 *txdata, U8 *rxdata, U16 u16size)
{
    U8  i = 0;
    U8  frmcnt = 0;
    U8  lastfrmsize = 0;

    frmcnt = u16size/MAX_WRITE_BUF_SIZE;
    lastfrmsize = u16size%MAX_WRITE_BUF_SIZE;

    for (i = 0; i < frmcnt; i++)
    {
        if(!HAL_MSPI_FullDuplex(bs, (MSPI_CH)u8Channel, txdata + i * MAX_WRITE_BUF_SIZE, rxdata + i * MAX_WRITE_BUF_SIZE, MAX_WRITE_BUF_SIZE))
        {
            return FALSE;
        }
    }

    if(!lastfrmsize)
        return TRUE;

    return HAL_MSPI_FullDuplex(bs, (MSPI_CH)u8Channel, txdata + frmcnt * MAX_WRITE_BUF_SIZE, rxdata + frmcnt * MAX_WRITE_BUF_SIZE, lastfrmsize);
}


U8 MDrv_MSPI_DMA_Read(struct mstar_spi *bs, U8 u8Channel, U8 *pData, U16 u16Size)
{
    dma_addr_t data_addr;

    if(pData == NULL) {
        return E_MSPI_NULL;
    }
    mutex_lock(&hal_mspi_lock);

    HAL_MSPI_SelectCLK(bs, u8Channel);

    MOVDMA_WRITE( DMA_RW, DMA_READ); // 1 for dma read from device

    MSPI_WRITE(MSPI_DMA_ENABLE, 0x01);
    MSPI_WRITE(MSPI_DMA_RW_MODE, MSPI_DMA_READ);

    MSPI_WRITE(MSPI_DMA_DATA_LENGTH_L, u16Size & 0xFFFF );
    MSPI_WRITE(MSPI_DMA_DATA_LENGTH_H, (u16Size>>16)& 0x00FF); // 24bit

    MOVDMA_WRITE(MOV_DMA_BYTE_CNT_L, u16Size & 0xFFFF );
    MOVDMA_WRITE(MOV_DMA_BYTE_CNT_H, u16Size>>16 );

    data_addr=dma_map_single(NULL, pData, u16Size, DMA_TO_DEVICE);
    if(data_addr > SPI_MIU1_BUS_BASE)
        data_addr -= SPI_MIU1_BUS_BASE;
    else
        data_addr -= SPI_MIU0_BUS_BASE;

    MOVDMA_WRITE(MOV_DMA_DST_ADDR_L, data_addr & 0x0000FFFF );
    MOVDMA_WRITE(MOV_DMA_DST_ADDR_H, data_addr >>16 );

    MOVDMA_WRITE(0x00,0x01);//dma enable
    _HAL_MSPI_ChipSelect(bs,1,0);//enable chip select for device0  (pulled low)
    _HAL_MSPI_RWBUFSize(bs,MSPI_READ_INDEX, 0); //spi length = 0
    _HAL_MSPI_Trigger(bs);
    _HAL_MSPI_ChipSelect(bs,0,0);//disable chip select for device0  (pulled high)

    mutex_unlock(&hal_mspi_lock);
    return E_MSPI_OK;
}

U8 MDrv_MSPI_DMA_Write(struct mstar_spi *bs,U8 u8Channel, U8 *pData, U16 u16Size)
{
    dma_addr_t data_addr;
    mutex_lock(&hal_mspi_lock);

    //printk("### MDrv_MSPI_DMA_Write\r\n");

    HAL_MSPI_SelectCLK(bs, u8Channel);

    MOVDMA_WRITE( DMA_RW, DMA_WRITE );//0 for dma write to device
    MOVDMA_WRITE( DMA_DEVICE_MODE, 0x0001 ); // 1 to enable dma device mode
    MOVDMA_WRITE( DMA_DEVICE_SEL, bs->u8channel); //0 select mspi0 , 1 select mspi1

    MSPI_WRITE(MSPI_DMA_ENABLE, 0x01);
    MSPI_WRITE(MSPI_DMA_RW_MODE, MSPI_DMA_WRITE);

    MSPI_WRITE(MSPI_DMA_DATA_LENGTH_L, u16Size & 0xFFFF );
    MSPI_WRITE(MSPI_DMA_DATA_LENGTH_H, u16Size>>16 );

    MOVDMA_WRITE(MOV_DMA_BYTE_CNT_L, u16Size & 0xFFFF );
    MOVDMA_WRITE(MOV_DMA_BYTE_CNT_H, u16Size>>16 );

    data_addr=dma_map_single(NULL, pData, u16Size, DMA_FROM_DEVICE);
    if(data_addr > SPI_MIU1_BUS_BASE)
        data_addr -= SPI_MIU1_BUS_BASE;
    else
        data_addr -= SPI_MIU0_BUS_BASE;

    Chip_Flush_MIU_Pipe();

    MOVDMA_WRITE(MOV_DMA_SRC_ADDR_L, data_addr & 0x0000FFFF );
    MOVDMA_WRITE(MOV_DMA_SRC_ADDR_H, data_addr >>16);


    MOVDMA_WRITE(0x00,0x01);//dma enable
    _HAL_MSPI_ChipSelect(bs,1,0);//enable chip select for device0  (pulled low)
    _HAL_MSPI_RWBUFSize(bs,MSPI_WRITE_INDEX, 0);
    _HAL_MSPI_Trigger(bs);
    _HAL_MSPI_ChipSelect(bs,0,0);//disable chip select for device0  (pulled high)
	
    mutex_unlock(&hal_mspi_lock);
    return E_MSPI_OK;
}


u8 MDrv_MSPI_Init(struct mstar_spi *bs,u8 u8Channel,u8 u8Mode)
{
    u8 errorno = E_MSPI_OK;
    HAL_MSPI_Init(bs,(MSPI_CH)u8Channel,u8Mode);
    HAL_MSPI_IntEnable(bs,(MSPI_CH)u8Channel,true);
    gbInitFlag = true;
    return errorno;
}
//------------------------------------------------------------------------------
/// Description : Set TrEnd timing of DC config
/// @return E_MSPI_OK :
/// @return >1 : failed to set TrEnd timing
//------------------------------------------------------------------------------
static U8 _MDrv_DC_TrEndSetting(struct mstar_spi *bs,U8 u8Channel,U8 TrEnd)
{
    U8 u8TrEndMax;
    U8 errnum = E_MSPI_OK;

    u8TrEndMax = HAL_MSPI_DCConfigMax(E_MSPI_TREND);
    if(TrEnd > u8TrEndMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetDcTiming(bs,(MSPI_CH)u8Channel,E_MSPI_TREND ,TrEnd);
    return errnum;

}
//------------------------------------------------------------------------------
/// Description : Set TB timing of DC config
/// @return E_MSPI_OK :
/// @return >1 : failed to set TB timing
//------------------------------------------------------------------------------
static U8 _MDrv_DC_TBSetting(struct mstar_spi *bs,U8 u8Channel,U8 TB)
{
    U8 u8TBMax;
    U8 errnum = E_MSPI_OK;

    u8TBMax = HAL_MSPI_DCConfigMax(E_MSPI_TB);
    if(TB > u8TBMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetDcTiming(bs,(MSPI_CH)u8Channel,E_MSPI_TB ,TB);
    return errnum;

}
//------------------------------------------------------------------------------
/// Description : Set TRW timing of DC config
/// @return E_MSPI_OK :
/// @return >1 : failed to set TRW timging
//------------------------------------------------------------------------------
static U8 _MDrv_DC_TRWSetting(struct mstar_spi *bs,U8 u8Channel,U8 TRW)
{
    U8 u8TRWMax;
    U8 errnum = E_MSPI_OK;

    u8TRWMax = HAL_MSPI_DCConfigMax(E_MSPI_TRW);
    if(TRW > u8TRWMax)
        errnum = E_MSPI_PARAM_OVERFLOW;
    else
        HAL_MSPI_SetDcTiming(bs,(MSPI_CH)u8Channel,E_MSPI_TRW ,TRW);
    return errnum;

}
//------------------------------------------------------------------------------
/// Description : config spi transfer timing
/// @param ptDCConfig    \b OUT  struct pointer of transfer timing config
/// @return E_MSPI_OK : succeed
/// @return E_MSPI_DCCONFIG_ERROR : failed to config transfer timing
//------------------------------------------------------------------------------
U8 MDrv_MSPI_DCConfig(struct mstar_spi *bs,U8 u8Channel, MSPI_DCConfig *ptDCConfig)
{
    U8 errnum = E_MSPI_OK;
    //check init
    if(!gbInitFlag)
        return E_MSPI_INIT_FLOW_ERROR;

    if(ptDCConfig == NULL)
    {
        HAL_MSPI_Reset_DCConfig(bs,(MSPI_CH)u8Channel);
        return E_MSPI_OK;
    }
    errnum = _MDrv_DC_TrStartSetting(bs,u8Channel,ptDCConfig->u8TrStart);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    errnum = _MDrv_DC_TrEndSetting(bs,u8Channel,ptDCConfig->u8TrEnd);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    errnum = _MDrv_DC_TBSetting(bs,u8Channel,ptDCConfig->u8TB);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    errnum = _MDrv_DC_TRWSetting(bs,u8Channel,ptDCConfig->u8TRW);
    if(errnum != E_MSPI_OK)
        goto ERROR_HANDLE;
    return E_MSPI_OK;

ERROR_HANDLE:
    errnum |= E_MSPI_DCCONFIG_ERROR;
    return errnum;
}

void HAL_MSPI_SetCLKTiming(struct mstar_spi *bs,MSPI_CH eChannel, eCLK_config eCLKField, U8 u8CLKVal)
{
    U16 u16TempBuf = 0;
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);
    switch(eCLKField) {
    case E_MSPI_POL:
        u16TempBuf = MSPI_READ(MSPI_CLK_CLOCK_OFFSET);
        u16TempBuf &= ~(MSPI_CLK_POLARITY_MASK);
        u16TempBuf |= u8CLKVal << MSPI_CLK_POLARITY_BIT_OFFSET;
        break;
    case E_MSPI_PHA:
        u16TempBuf = MSPI_READ(MSPI_CLK_CLOCK_OFFSET);
        u16TempBuf &= ~(MSPI_CLK_PHASE_MASK);
        u16TempBuf |= u8CLKVal << MSPI_CLK_PHASE_BIT_OFFSET;
        break;
    case E_MSPI_CLK:
        u16TempBuf = MSPI_READ(MSPI_CLK_CLOCK_OFFSET);
        u16TempBuf &= MSPI_CLK_CLOCK_MASK;
        u16TempBuf |= u8CLKVal << MSPI_CLK_CLOCK_BIT_OFFSET;
        break;
    }
    MSPI_WRITE(MSPI_CLK_CLOCK_OFFSET, u16TempBuf);

    mutex_unlock(&hal_mspi_lock);
}

void HAL_MSPI_SetLSB(struct mstar_spi *bs,MSPI_CH eChannel, BOOL enable)
{
    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);

    MSPI_WRITE(MSPI_LSB_FIRST_OFFSET, enable);

    mutex_unlock(&hal_mspi_lock);
}

#define NUM_SPI_CKG             4
#define NUM_SPI_CLKDIV          8
#define NUM_SPI_CLKRATES        32                  //NUM_SPI_CKG * NUM_SPI_CLKDIVRATE
static U8 clk_spi_ckg[NUM_SPI_CKG] = {108, 54, 12, 144};
static U16 clk_spi_div[NUM_SPI_CLKDIV] = {2, 4, 8, 16, 32, 64, 128, 256};
static ST_DRV_MSPI_CLK clk_buffer[NUM_SPI_CLKRATES];

U32 HAL_MSPI_CLK_Config(struct mstar_spi *bs,U8 u8Chanel,U32 u32MspiClk)
{
    U8 i = 0;
    U8 j= 0;
    U16 TempData = 0;
    U32 clk =0;
    ST_DRV_MSPI_CLK temp;
    if(u8Chanel >=2)
        return FALSE;
    memset(&temp,0,sizeof(ST_DRV_MSPI_CLK));
    memset(&clk_buffer,0,sizeof(ST_DRV_MSPI_CLK)*NUM_SPI_CLKRATES);
    for(i = 0;i < NUM_SPI_CKG;i++)//clk_spi_m_p1
    {
        for(j = 0;j<NUM_SPI_CLKDIV;j++)//spi div
        {
            clk = clk_spi_ckg[i]*1000000;
            clk_buffer[j+8*i].u8ClkSpi_cfg = i;
            clk_buffer[j+8*i].u8ClkSpi_DIV = j ;
            clk_buffer[j+8*i].u32ClkSpi = clk/clk_spi_div[j];
        }
    }
    for(i = 0;i<NUM_SPI_CLKRATES;i++)
    {
        for(j = i;j<NUM_SPI_CLKRATES;j++)
        {
            if(clk_buffer[i].u32ClkSpi > clk_buffer[j].u32ClkSpi)
            {
                memcpy(&temp,&clk_buffer[i],sizeof(ST_DRV_MSPI_CLK));

                memcpy(&clk_buffer[i],&clk_buffer[j],sizeof(ST_DRV_MSPI_CLK));

                memcpy(&clk_buffer[j],&temp,sizeof(ST_DRV_MSPI_CLK));
            }
        }
    }
    for(i = 0;i<NUM_SPI_CLKRATES;i++)
    {
        if(u32MspiClk <= clk_buffer[i].u32ClkSpi)
        {
            break;
        }
    }
    if (NUM_SPI_CLKRATES == i)
    {
        i--;
    }
    //match Closer clk
    else if ((i) && ((u32MspiClk - clk_buffer[i-1].u32ClkSpi)<(clk_buffer[i].u32ClkSpi - u32MspiClk)))
    {
        i -= 1;
    }
    mspi_dbgmsg("[Lwc Debug] u8ClkSpi_P1 =%d\n",clk_buffer[i].u8ClkSpi_cfg);
    mspi_dbgmsg("[Lwc Debug] u8ClkSpi_DIV =%d\n",clk_buffer[i].u8ClkSpi_DIV);
    mspi_dbgmsg("[Lwc Debug] u32ClkSpi = %d\n",clk_buffer[i].u32ClkSpi);

    if(u8Chanel == E_MSPI0)//mspi0
    {
        // CLK SETTING
        TempData = CLK_READ(MSPI0_CLK_CFG);
        TempData &= ~(MSPI0_CLK_MASK);
        TempData |= (clk_buffer[i].u8ClkSpi_cfg<<2);
        CLK_WRITE(MSPI0_CLK_CFG, TempData);
    }
#if SUPPORT_SPI_1
    else if(u8Chanel == E_MSPI1)//mspi1
    {
        // CLK SETTING
        TempData = CLK_READ(MSPI1_CLK_CFG);
        TempData &= ~(MSPI1_CLK_MASK);
        TempData |= (clk_buffer[i].u8ClkSpi_cfg<<10);
        CLK_WRITE(MSPI1_CLK_CFG, TempData);
    }
#endif
    TempData = MSPI_READ(MSPI_CLK_CLOCK_OFFSET);
    TempData &= MSPI_CLK_CLOCK_MASK;
    TempData |= clk_buffer[i].u8ClkSpi_DIV << MSPI_CLK_CLOCK_BIT_OFFSET;
    MSPI_WRITE(MSPI_CLK_CLOCK_OFFSET, TempData);

    return clk_buffer[i].u32ClkSpi;
}


//------------------------------------------------------------------------------
/// Description : config spi clock setting
/// @param ptCLKConfig    \b OUT  struct pointer of clock config
/// @return E_MSPI_OK : succeed
/// @return E_MSPI_CLKCONFIG_ERROR : failed to config spi clock
//------------------------------------------------------------------------------
U8  MDrv_MSPI_SetMode(struct mstar_spi *bs,U8 u8Channel, MSPI_Mode_Config_e eMode)
{
    if (eMode >= E_MSPI_MODE_MAX) {
        return E_MSPI_PARAM_OVERFLOW;
    }

    switch (eMode) {
    case E_MSPI_MODE0:
        HAL_MSPI_SetCLKTiming(bs,(MSPI_CH)u8Channel, E_MSPI_POL ,false);
        HAL_MSPI_SetCLKTiming(bs,(MSPI_CH)u8Channel, E_MSPI_PHA ,false);

        break;
    case E_MSPI_MODE1:
        HAL_MSPI_SetCLKTiming(bs,(MSPI_CH)u8Channel, E_MSPI_POL ,false);
        HAL_MSPI_SetCLKTiming(bs,(MSPI_CH)u8Channel, E_MSPI_PHA ,true);
        break;
    case E_MSPI_MODE2:
        HAL_MSPI_SetCLKTiming(bs,(MSPI_CH)u8Channel, E_MSPI_POL ,true);
        HAL_MSPI_SetCLKTiming(bs,(MSPI_CH)u8Channel, E_MSPI_PHA ,false);
        break;
    case E_MSPI_MODE3:
        HAL_MSPI_SetCLKTiming(bs,(MSPI_CH)u8Channel, E_MSPI_POL ,true);
        HAL_MSPI_SetCLKTiming(bs,(MSPI_CH)u8Channel, E_MSPI_PHA ,true);
        break;
    default:
        HAL_MSPI_Reset_CLKConfig(bs,(MSPI_CH)u8Channel);
        return E_MSPI_OPERATION_ERROR;
    }

    return E_MSPI_OK;
}

void HAL_MSPI_SetPerFrameSize(struct mstar_spi *bs,MSPI_CH eChannel, BOOL bDirect, U8 u8BufOffset, U8 u8PerFrameSize)
{
    U8 u8Index = 0;
    U16 u16TempBuf = 0;
    U8 u8BitOffset = 0;
    U16 u16regIndex = 0;

    mutex_lock(&hal_mspi_lock);
    _HAL_MSPI_CheckandSetBaseAddr( eChannel);
    if(bDirect == MSPI_READ_INDEX) {
        u16regIndex = MSPI_FRAME_RBIT_OFFSET;
    } else {
        u16regIndex = MSPI_FRAME_WBIT_OFFSET;
    }
    if(u8BufOffset >=4) {
        u8Index++;
        u8BufOffset -= 4;
    }
    u8BitOffset = u8BufOffset * MSPI_FRAME_BIT_FIELD;
    u16TempBuf = MSPI_READ(u16regIndex+ u8Index);
    u16TempBuf &= ~(MSPI_FRAME_BIT_MASK << u8BitOffset);
    u16TempBuf |= u8PerFrameSize << u8BitOffset;
    MSPI_WRITE((u16regIndex + u8Index), u16TempBuf);
    mutex_unlock(&hal_mspi_lock);

}

//------------------------------------------------------------------------------
/// Description : config spi transfer timing
/// @param ptDCConfig    \b OUT  struct pointer of bits of buffer tranferred to slave config
/// @return E_MSPI_OK : succeed
/// @return E_MSPI_FRAMECONFIG_ERROR : failed to config transfered bit per buffer
//------------------------------------------------------------------------------
U8 MDrv_MSPI_FRAMEConfig(struct mstar_spi *bs,U8 u8Channel, MSPI_FrameConfig  *ptFrameConfig)
{
    U8 errnum = E_MSPI_OK;
    U8 u8Index = 0;

    if(ptFrameConfig == NULL) {
        HAL_MSPI_Reset_FrameConfig(bs,(MSPI_CH)u8Channel);
        return E_MSPI_OK;
    }
    // read buffer bit config
    for(u8Index = 0; u8Index < MAX_READ_BUF_SIZE; u8Index++) {
        if(ptFrameConfig->u8RBitConfig[u8Index] > MSPI_FRAME_BIT_MAX) {
            errnum = E_MSPI_PARAM_OVERFLOW;
        } else {
            HAL_MSPI_SetPerFrameSize(bs,(MSPI_CH)u8Channel, MSPI_READ_INDEX,  u8Index, ptFrameConfig->u8RBitConfig[u8Index]);
        }
    }
    //write buffer bit config
    for(u8Index = 0; u8Index < MAX_WRITE_BUF_SIZE; u8Index++) {
        if(ptFrameConfig->u8WBitConfig[u8Index] > MSPI_FRAME_BIT_MAX) {
            errnum = E_MSPI_PARAM_OVERFLOW;
        } else {
            HAL_MSPI_SetPerFrameSize(bs,(MSPI_CH)u8Channel, MSPI_WRITE_INDEX,  u8Index, ptFrameConfig->u8WBitConfig[u8Index]);
        }
    }
    return errnum;
}

// add to sync code from utopia for localdimming to set clk
U32 MDrv_MSPI_SetCLK(struct mstar_spi *bs,U8 u8Channel, U32 u32MspiClk)
{
	int u32SpiClk;
	u32SpiClk = HAL_MSPI_CLK_Config(bs,(MSPI_CH)u8Channel,u32MspiClk);
    return u32SpiClk;
}

void mspi_config(struct mstar_spi *bs,u8 u8Channel)
{
    MSPI_DCConfig stDCConfig;
    MSPI_FrameConfig stFrameConfig;
    MSPI_Mode_Config_e mspimode = E_MSPI_MODE0;
    stDCConfig.u8TB = 0;
    stDCConfig.u8TrEnd = 0x0;
    stDCConfig.u8TrStart = 0x0;
    stDCConfig.u8TRW = 0;

    memset(&stFrameConfig,0x07,sizeof(MSPI_FrameConfig));
    MDrv_MSPI_Init(bs,u8Channel,bs->u32spi_mode);
    MDrv_MSPI_DCConfig(bs,u8Channel, &stDCConfig);
    MDrv_MSPI_SetMode(bs,u8Channel, mspimode);
    MDrv_MSPI_FRAMEConfig(bs,u8Channel,&stFrameConfig);
    MDrv_MSPI_SetCLK(bs,u8Channel,54000000); //200000 CLK
    MDrv_MSPI_ChipSelect(bs,u8Channel,0,0);
    HAL_MSPI_SetLSB(bs,u8Channel, 0);
    return;
}

#ifdef _EN_MSPI_INTC_
static irqreturn_t mstar_spi_interrupt(int irq, void *dev_id)
{
	struct spi_master *master = dev_id;
	struct mstar_spi *bs = spi_master_get_devdata(master);
	int uDoneFlag = 0;
	//static int i=0;

	uDoneFlag = _HAL_MSPI_CheckDone(bs);

	if(uDoneFlag == 1){

        complete(&bs->done);
        mspi_dbgmsg("<<<<<<<<<<<<<<<<<<< SPI_0 Done >>>>>>>>>>>>>>>>>\n");
	}else{
            //printk("<<<<<<<<<<<<<<<<<<< SPI Fail >>>>>>>>>>>>>>>>>\n");
	}

	_HAL_MSPI_ClearDone();

    return IRQ_HANDLED;
}
#endif

#if SUPPORT_SPI_1
static irqreturn_t mstar_spi_interrupt_spi1(int irq, void *dev_id)
{
	struct spi_master *master = dev_id;
	struct mstar_spi *bs = spi_master_get_devdata(master);
	int uDoneFlag = 0;
	//static int i=0;

	uDoneFlag = _HAL_MSPI_CheckDone(bs);

	if(uDoneFlag == 1){

        complete(&bs->done);
        mspi_dbgmsg("<<<<<<<<<<<<<<<<<<< SPI_1 Done >>>>>>>>>>>>>>>>>\n");
	}else{
            //printk("<<<<<<<<<<<<<<<<<<< SPI Fail >>>>>>>>>>>>>>>>>\n");
	}
	_HAL_MSPI_ClearDone();

    return IRQ_HANDLED;
}
#endif

static int mstar_spi_start_transfer(struct spi_device *spi, struct spi_transfer *tfr)
{
    struct mstar_spi *bs = spi_master_get_devdata(spi->master);

    //printk("[mstar_spi_start_transfer]\n");

    mspi_dbgmsg("All = %x\n",spi->mode);
    mspi_dbgmsg("SPI mode = %d\n",spi->mode & 0x03);
    mspi_dbgmsg("LSB first = %d\n",spi->mode & 0x08);

    //reinit_completion(&bs->done);
    bs->tx_buf = tfr->tx_buf;
    bs->rx_buf = tfr->rx_buf;
    bs->len = tfr->len;

    MDrv_MSPI_ChipSelect(bs,_hal_msp.eCurrentCH,1,0);

    /*if(tfr->speed_hz != NULL){
         MDrv_MSPI_SetCLK(_hal_msp.eCurrentCH,tfr->speed_hz);
    }*/

    if((bs->tx_buf != NULL) && (bs->rx_buf != NULL))
    {
        MDrv_MSPI_FullDuplex(bs, _hal_msp.eCurrentCH, (U8 *)bs->tx_buf, (U8 *)bs->rx_buf, (U16)bs->len);
        return 0;
    }else if(bs->tx_buf != NULL){
        mspi_dbgmsg("bs->tx_buf=%x,%x\n",bs->tx_buf[0],bs->tx_buf[1]);

        if(bs->xfer_w_dma)
            MDrv_MSPI_DMA_Write(bs, spi->master->bus_num,(U8 *)bs->tx_buf,(U16)bs->len);
        else
            MDrv_MSPI_Write(bs, _hal_msp.eCurrentCH,(U8 *)bs->tx_buf,(U16)bs->len);
    }else if(bs->rx_buf != NULL){
        if(bs->xfer_w_dma)
            MDrv_MSPI_DMA_Read(bs,spi->master->bus_num,(U8 *)bs->rx_buf,(U16)bs->len);
        else
            MDrv_MSPI_Read(bs,_hal_msp.eCurrentCH,(U8 *)bs->rx_buf,(U16)bs->len);

        mspi_dbgmsg("bs->rx_buf=%x,%x\n",bs->rx_buf[0],bs->rx_buf[1]);
    }
    //printk("bs->len=%d\n",bs->len);

    return 0;
}

static int mstar_spi_finish_transfer(struct spi_device *spi,
		struct spi_transfer *tfr, bool cs_change)
{
	struct mstar_spi *bs = spi_master_get_devdata(spi->master);

	//printk("[mstar_spi_finish_transfer] cs_change=%d\n",cs_change);

#if 1

	if (tfr->delay_usecs)
		udelay(tfr->delay_usecs);

	if (cs_change){
		/* Clear TA flag */
		MDrv_MSPI_ChipSelect(bs,_hal_msp.eCurrentCH,0,0);
		//MSPI_WRITE(MSPI_CHIP_SELECT_OFFSET, 0xFFFF);
	}
#endif
	return 0;
}

static int mstar_spi_set_framecfg(struct mstar_spi *bs, int bits_per_word)
{
    MSPI_FrameConfig stFrameConfig;
    int i;

    if (bits_per_word > MAX_SUPPORT_BITS)
    {
         return -EINVAL;
    }
    else if (bits_per_word > 8)
    {
        for (i = 0; i < MAX_WRITE_BUF_SIZE; i+=2)
        {
            stFrameConfig.u8WBitConfig[i]   = bits_per_word - 8 -1;
            stFrameConfig.u8WBitConfig[i+1] = 8 -1;
        }
        for (i = 0; i < MAX_READ_BUF_SIZE; i+=2)
        {
            stFrameConfig.u8RBitConfig[i]   = bits_per_word - 8 -1;
            stFrameConfig.u8RBitConfig[i+1] = 8 -1;
        }
    }
    else
    {
        for (i = 0; i < MAX_WRITE_BUF_SIZE; i++)
        {
            stFrameConfig.u8WBitConfig[i]   = bits_per_word -1;
        }
        for (i = 0; i < MAX_READ_BUF_SIZE; i++)
        {
            stFrameConfig.u8WBitConfig[i]   = bits_per_word -1;
        }
    }
    MDrv_MSPI_FRAMEConfig(bs,bs->u8channel,&stFrameConfig);

    return 0;
}

static int mstar_spi_setup(struct spi_device *spi)
{
	struct mstar_spi *bs = spi_master_get_devdata(spi->master);

	MDrv_MSPI_SetMode(bs,bs->u8channel, spi->mode & MSTAR_SPI_MODE_BITS);
	HAL_MSPI_SetLSB(bs,bs->u8channel,(spi->mode & SPI_LSB_FIRST)>>3);
	spi->max_speed_hz = MDrv_MSPI_SetCLK(bs,bs->u8channel,spi->max_speed_hz);
	bs->u32spi_mode = spi->mode & MSTAR_SPI_MODE_BITS;

	// setup transfer bit mask
    bs->xfer_w_dma = (spi->bits_per_word % 8 ==0) ? bs->use_dma : false;
    bs->bits_per_word = spi->bits_per_word;
    if (spi->bits_per_word > MAX_SUPPORT_BITS) {
        return -EINVAL;
    }
    else if (spi->bits_per_word > 8) {
        bs->wsize = 2;
    }
    else {
        bs->wsize = 1;
    }
	mstar_spi_set_framecfg(bs, spi->bits_per_word);

    mspi_dbgmsg("<~~~~~~~~~~~~~~~~>SETUP :%x,speed:%d channel:%d\n",spi->mode,spi->max_speed_hz,bs->u8channel);
    return 0;
}

static int mstar_spi_transfer_one(struct spi_master *master,
		struct spi_message *mesg)
{
	struct mstar_spi *bs = spi_master_get_devdata(master);
	struct spi_transfer *tfr;
	struct spi_device *spi = mesg->spi;
	int err = 0;
	bool cs_change;

	//mspi_dbgmsg("[mstar_spi_transfer_one]\n");

	list_for_each_entry(tfr, &mesg->transfers, transfer_list) {

	    if (tfr->len % bs->wsize != 0) {
            mspi_dbgmsg("invalid transfer len\n");
            goto out;
        }

		err = mstar_spi_start_transfer(spi, tfr);
		if (err){
			mspi_dbgmsg("start_transfer err\n");
			goto out;
		}

		cs_change = tfr->cs_change ||
			list_is_last(&tfr->transfer_list, &mesg->transfers);

		err = mstar_spi_finish_transfer(spi, tfr, cs_change);
		if (err){
			mspi_dbgmsg("finish transfer err\n");
			goto out;
		}
		mesg->actual_length += bs->len;//(tfr->len - bs->len);
		mspi_dbgmsg("transfered:%d\n",mesg->actual_length);
	}

out:
	/* Clear FIFOs, and disable the HW block */
	mesg->status = err;
	spi_finalize_current_message(master);

	return 0;
}

static int mstar_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct mstar_spi *bs;
	int err;
	unsigned int u4IO_PHY_BASE;
	unsigned int u4spi_bank[5];
#ifdef _EN_MSPI_INTC_
	int irq;
#if SUPPORT_SPI_1
    struct spi_master *master_spi1;
    int irq2;
#endif
#endif
    struct clk *clk;
    struct clk_hw *hw_parent;

    mspi_dbgmsg("<<<<<<<<<<<<<<<<< Probe >>>>>>>>>>>>>>>\n");

	master = spi_alloc_master(&pdev->dev, sizeof(*bs));
	if (!master) {
		mspi_dbgmsg( "spi_alloc_master() failed\n");
		dev_err(&pdev->dev, "spi_alloc_master() failed\n");
		return -ENOMEM;
	}
#if SUPPORT_SPI_1
	master_spi1 = spi_alloc_master(&pdev->dev, sizeof(*bs));
	if (!master_spi1) {
		mspi_dbgmsg( "spi_alloc_master() failed\n");
		dev_err(&pdev->dev, "spi_alloc_master() failed\n");
		return -ENOMEM;
	}
#endif
	mspi_dbgmsg( "spi_alloc_master\n");
	platform_set_drvdata(pdev, master);
#if SUPPORT_SPI_1
	platform_set_drvdata(pdev, master_spi1);
#endif

	master->mode_bits = MSTAR_SPI_MODE_BITS;
	master->bits_per_word_mask = SPI_BPW_RANGE_MASK(1, MAX_SUPPORT_BITS);
	master->num_chipselect = 3;
	master->transfer_one_message = mstar_spi_transfer_one;
	master->dev.of_node = pdev->dev.of_node;
	master->setup = mstar_spi_setup;
	master->max_speed_hz = 54000000;
	master->min_speed_hz = 46875;
	master->bus_num = 0;

#if SUPPORT_SPI_1
	master_spi1->mode_bits = MSTAR_SPI_MODE_BITS;
	master_spi1->bits_per_word_mask = SPI_BPW_RANGE_MASK(1, MAX_SUPPORT_BITS);
	master_spi1->num_chipselect = 3;
	master_spi1->transfer_one_message = mstar_spi_transfer_one;
	master_spi1->dev.of_node = pdev->dev.of_node;
	master_spi1->setup = mstar_spi_setup;
	master_spi1->max_speed_hz = 54000000;
	master_spi1->min_speed_hz = 46875;
	master_spi1->bus_num = 1;
#endif

#ifdef _EN_MSPI_INTC_
    irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    mspi_dbgmsg("[MSPI] Request IRQ: %d\n", irq);
    if (request_irq(irq, mstar_spi_interrupt, 0, "MSPI_0 interrupt", (void*)master) == 0)
        mspi_dbgmsg("[MSPI] MSPI_0 interrupt registered\n");
    else
        mspi_dbgmsg("[MSPI] MSPI_0 interrupt failed");
#if SUPPORT_SPI_1
    irq2 = irq_of_parse_and_map(pdev->dev.of_node, 1);
    mspi_dbgmsg("[MSPI] Request IRQ: %d\n", irq2);
    if (request_irq(irq2, mstar_spi_interrupt_spi1, 0, "MSPI_1 interrupt", (void*)master_spi1) == 0)
        mspi_dbgmsg("[MSPI] MSPI_1 interrupt registered\n");
    else
        mspi_dbgmsg("[MSPI] MSPI_1 interrupt failed");
#endif
#endif

    of_property_read_u32(pdev->dev.of_node, "io_phy_addr", &u4IO_PHY_BASE);
    of_property_read_u32_array(pdev->dev.of_node, "banks", (unsigned int*)u4spi_bank, 5);

    //_hal_msp.eCurrentCH = E_MSPI1;
    //_hal_msp.VirtMspBaseAddr = (char*)ioremap(BANK_TO_ADDR32(u4spi_bank[_hal_msp.eCurrentCH])+u4IO_PHY_BASE, BANK_SIZE);
    //_hal_msp.VirtClkBaseAddr     = (char*)ioremap(BANK_TO_ADDR32(u4spi_bank[2])+u4IO_PHY_BASE, BANK_SIZE);
    //_hal_msp.VirtChiptopBaseAddr =(char*)ioremap(BANK_TO_ADDR32(u4spi_bank[3])+u4IO_PHY_BASE, BANK_SIZE);

    mspi_dbgmsg("u4IO_PHY_BASE %x\n",u4IO_PHY_BASE);
    mspi_dbgmsg("u4spi_bank[0] %x\n",u4spi_bank[0]);
    mspi_dbgmsg("u4spi_bank[1] %x\n",u4spi_bank[1]);
    mspi_dbgmsg("u4spi_bank[2] %x\n",u4spi_bank[2]);
    mspi_dbgmsg("u4spi_bank[3] %x\n",u4spi_bank[3]);
    mspi_dbgmsg("u4spi_bank[4] %x\n",u4spi_bank[4]);

	bs = spi_master_get_devdata(master);
	init_completion(&bs->done);

    bs->VirtMspBaseAddr = (char*)ioremap(BANK_TO_ADDR32(u4spi_bank[0])+u4IO_PHY_BASE, BANK_SIZE);
    bs->VirtClkBaseAddr = (char*)ioremap(BANK_TO_ADDR32(u4spi_bank[2])+u4IO_PHY_BASE, BANK_SIZE);
    bs->VirtChiptopBaseAddr =(char*)ioremap(BANK_TO_ADDR32(u4spi_bank[3])+u4IO_PHY_BASE, BANK_SIZE);
    bs->VirtMovdmaBaseAddr =(char*)ioremap(BANK_TO_ADDR32(u4spi_bank[4])+u4IO_PHY_BASE, BANK_SIZE);

    bs->u8channel = E_MSPI0;

    of_property_read_u32(pdev->dev.of_node, "spi0_mode", &bs->u32spi_mode);
    bs->use_dma = false;
    if (SUPPORT_DMA)
    {
      of_property_read_u32(pdev->dev.of_node, "dma", &bs->use_dma);
    }

	/* initialise the hardware */
    mspi_config(bs,0);

#if SUPPORT_SPI_1
	bs = spi_master_get_devdata(master_spi1);
	init_completion(&bs->done);
    bs->VirtMspBaseAddr = (char*)ioremap(BANK_TO_ADDR32(u4spi_bank[1])+u4IO_PHY_BASE, BANK_SIZE);
    bs->VirtClkBaseAddr = (char*)ioremap(BANK_TO_ADDR32(u4spi_bank[2])+u4IO_PHY_BASE, BANK_SIZE);
    bs->VirtChiptopBaseAddr =(char*)ioremap(BANK_TO_ADDR32(u4spi_bank[3])+u4IO_PHY_BASE, BANK_SIZE);
    bs->VirtMovdmaBaseAddr =(char*)ioremap(BANK_TO_ADDR32(u4spi_bank[4])+u4IO_PHY_BASE, BANK_SIZE);
    bs->u8channel = E_MSPI1;

    of_property_read_u32(pdev->dev.of_node, "spi1_mode", &bs->u32spi_mode);
    bs->use_dma = false;
    if (SUPPORT_DMA)
    {
        of_property_read_u32(pdev->dev.of_node, "dma", &bs->use_dma);
    }

    /* initialise the hardware */
    mspi_config(bs,1);
#endif

    //2. set clk
    clk = of_clk_get(pdev->dev.of_node, 0);
    if(IS_ERR(clk))
    {
        err = PTR_ERR(clk);
        mspi_dbgmsg("[%s]: of_clk_get failed\n", __func__);
    }
    else
    {
        /* select clock mux */
        hw_parent = clk_hw_get_parent_by_index(__clk_get_hw(clk), 0);  // select clock mux=0
        mspi_dbgmsg( "[%s]parent_num:%d parent[0]:%s\n", __func__,
                clk_hw_get_num_parents(__clk_get_hw(clk)), clk_hw_get_name(hw_parent));
        clk_set_parent(clk, hw_parent->clk);

        clk_prepare_enable(clk);
        mspi_dbgmsg("[mspi] clk_prepare_enable\n");
    }

	err = devm_spi_register_master(&pdev->dev, master);
	if (err) {
		mspi_dbgmsg( "could not register SPI_0 master: %d\n", err);
		dev_err(&pdev->dev, "could not register SPI master: %d\n", err);
		goto out_master_put;
	}
#if SUPPORT_SPI_1
	err = devm_spi_register_master(&pdev->dev, master_spi1);
	if (err) {
		mspi_dbgmsg( "could not register SPI_1 master: %d\n", err);
		dev_err(&pdev->dev, "could not register SPI master: %d\n", err);
		goto out_master_put;
	}
#endif
	spi_new_device(master, &mstar_info);
#if SUPPORT_SPI_1
	spi_new_device(master_spi1, &mstar_info);
#endif
	return 0;

//out_clk_disable:
//	clk_disable_unprepare(bs->clk);
out_master_put:
	spi_master_put(master);
	mspi_dbgmsg( "((((((((((( err:%d\n", err);
	return err;
}

static int mstar_spi_remove(struct platform_device *pdev)
{
    struct clk *clk;

#if 0

	struct spi_master *master = platform_get_drvdata(pdev);
	struct mstar_spi *bs = spi_master_get_devdata(master);

	/* Clear FIFOs, and disable the HW block */

	clk_disable_unprepare(bs->clk);
#endif

    clk = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR(clk))
    {
        mspi_dbgmsg( "[SAR] Fail to get clk!\n" );

    }
    else
    {
        clk_disable_unprepare(clk);
        clk_put(clk);
    }

	return 0;
}

static const struct of_device_id mstar_spi_match[] = {
	{ .compatible = "sstar_spi", },
	{}
};
MODULE_DEVICE_TABLE(of, mstar_spi_match);

static struct platform_driver mstar_spi_driver = {
	.driver		= {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= mstar_spi_match,
	},
	.probe		= mstar_spi_probe,
	.remove		= mstar_spi_remove,
};
module_platform_driver(mstar_spi_driver);

MODULE_DESCRIPTION("SPI controller driver for Broadcom BCM2835");
MODULE_AUTHOR("Chris Boot <bootc@bootc.net>");
MODULE_LICENSE("GPL v2");
