/*
* mdrv_system.h- Sigmastar
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
#include "../include/mdrv_system_st.h"
#include "mst_platform.h"

#ifndef _DRV_SYSTEM_H_
#define _DRV_SYSTEM_H_


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define SYS_BOARD_NAME_MAX          32                                  ///< Maximum length of board name
#define SYS_PLATFORM_NAME_MAX       32                                  ///< Maximum length of playform name

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

/// System output pad switch type
typedef enum
{
    E_SYS_PAD_MSD5010_SM2_IIC2,                                         ///< 5010 SM2, IIC2
    E_SYS_PAD_MSD5011_SM2_IIC2,                                         ///< 5011 SM2, IIC2
    E_SYS_PAD_MSD5015_GPIO,                                             ///< 5015 GPIO
    E_SYS_PAD_MSD5018_SM2,                                              ///< 5018 SM2
} SYS_PadType;

/// System information
typedef struct
{
    /// Software information
    struct
    {
        U8                          Board[SYS_BOARD_NAME_MAX];          ///< Board name
        U8                          Platform[SYS_PLATFORM_NAME_MAX];    ///< Platform name
    } SWLib;
} SYS_Info;

/// Memory mapping type
typedef enum
{
    E_SYS_MMAP_LINUX_BASE,      //0
    E_SYS_MMAP_BIN_MEM,         //1
    E_SYS_MMAP_MAD_BASE,        //2
    E_SYS_MMAP_SCALER_DNR_BUF,  //3
    E_SYS_MMAP_RLD_BUF,         //4
    E_SYS_MMAP_MVD_SW,          //5
    E_SYS_MMAP_VD_3DCOMB,       //6
    E_SYS_MMAP_VE,              //7
    E_SYS_MMAP_TTX_BUF,         //8
    E_SYS_MMAP_MPOOL,           //9
    E_SYS_MMAP_EMAC_MEM,        //10
    E_SYS_MMAP_LINUX_MEM,       //11
    E_SYS_MMAP_SVD,             //12
    E_SYS_MMAP_SVD_ALL,         //13
    E_SYS_MMAP_MVD_FB,          //14
    E_SYS_MMAP_MVD_BS,          //15
    E_SYS_MMAP_POSD0_MEM,       //16
    E_SYS_MMAP_POSD1_MEM,       //17
    E_SYS_MMAP_TSP, // samuel, 20081107 //18
    E_SYS_MMAP_AUDIO_CLIP_MEM, // samuel, 20081107  //19
    E_SYS_MMAP_MBOX_SHM,                //20
    E_SYS_MMAP_CHAKRA_SUBSYSTEM,        //21
    E_SYS_MMAP_CHAKRA_FW,           //22
#ifdef CONFIG_MSTAR_KIP
    E_SYS_MMAP_AEON_SHM,               // 23
#endif
    E_SYS_MMAP_NUMBER,          //23

} SYS_Memory_Mapping;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
BOOL MDrv_System_Init(void);
//BOOL MDrv_System_SwitchPad(SYS_PadType ePadType);
void MDrv_System_WDTEnable(BOOL bEnable);
void MDrv_System_WDTClear(void);
BOOL MDrv_System_WDTLastStatus(void);
void MDrv_System_WDTSetTime(U32 u32Ms);
void MDrv_System_ResetChip(void);
void MDrv_System_ResetCPU(void);


PMST_PANEL_INFO_t MDrv_SYS_GetPanelInfo(void);

void MDrv_SYS_PowerDown(int src);
void MDrv_SYS_PD_ADC_R(B16 bStatus);
void MDrv_SYS_PD_ADC_G(B16 bStatus);
void MDrv_SYS_PD_ADC_B(B16 bStatus);
void MDrv_SYS_PD_ADC_Y(B16 bStatus);
void MDrv_SYS_PD_GMC_P(B16 bStatus);
void MDrv_SYS_PD_GMC_Y(B16 bStatus);
void MDrv_SYS_PD_GMC_C(B16 bStatus);
void MDrv_SYS_PD_CVBS_Buffer(B16 bStatus);
void MDrv_SYS_PD_DAC_CVBS(B16 bStatus);
void MDrv_SYS_PD_DAC(B16 bStatus);
void MDrv_SYS_PD_FB_DAC(B16 bStatus);
void MDrv_SYS_PD_DAC_RGB(B16 bStatus);
void MDrv_SYS_PD_Audio(B16 bStatus);
void MDrv_SYS_PD_LVDS(B16 bStatus);
void MDrv_SYS_PD_VD(B16 bStatus);
void MDrv_SYS_PD_SVD(B16 bStatus);
void MDrv_SYS_PD_MVD_M4V(B16 bStatus);
void MDrv_SYS_PD_TSP(B16 bStatus);
void MDrv_SYS_PD_VE(B16 bStatus);
void MDrv_SYS_PD_RVD(B16 bStatus);
void MDrv_SYS_PD_STRLD(B16 bStatus);
void MDrv_SYS_PD_AEON(B16 bStatus);
void MDrv_SYS_PD_GOPG2(B16 bStatus);


#ifdef CONFIG_MSTAR_SPI_FLASH
U32 MDrv_SYS_SPI_READ(U32 arg);
U32 MDrv_SYS_SPI_WRITE(U32 arg);
U32 MDrv_SYS_SPI_ERASE(U32 arg);
U32 MDrv_SYS_SPI_ERASE_SECTOR(U32 arg);
U32 MDrv_SYS_SPI_SIZE_DETECT(U32 arg);
U32 MDrv_SYS_SPI_READ_STATUS(U32 arg);
U32 MDrv_SYS_SPI_WRITE_STATUS(U32 arg);
U32 MDrv_SYS_SPI_INIT(void);
U32 MDrv_SYS_SPI_WRITE_PROTECT(U32 arg);
U32 MDrv_SYS_SPI_ERASE_ALL(void);
U32 MDrv_SYS_SPI_GetFlash_INFO(U32 arg);
#endif


unsigned int MDrv_SYS_GetDRAMLength(void);

#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
extern int MDrv_SYS_GetMMAP(int type, unsigned int *addr, unsigned int *len);
#elif defined(CONFIG_ARM64)
extern int MDrv_SYS_GetMMAP(int type, u64 *addr, u64 *len);
#endif

#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)

U32 MDrv_SYS_SetPanelInfo(U32 arg);
void MDrv_SYS_GetPanelRes(U32 arg);
void MDrv_SYS_GetPanelHStart(U32 argv);
void MDrv_SYS_GetGFXGOPPipelineDelay(U32 argv);

void MDrv_SYS_ReadGeneralRegister(U32 arg);
void MDrv_SYS_WriteGeneralRegister(U32 arg);
void MDrv_SYS_LoadAeon(U32 arg);
void MDrv_SYS_ResetAeon(U32 arg);
void MDrv_SYS_EnableAeon(void);
void MDrv_SYS_DumpAeonMessage(void);
void MDrv_SYS_DisableAeon(void);
void MDrv_SYS_SwitchUart(U32 arg);
U32 MDrv_SYS_IsAeonEnable(U32 arg);

void MDrv_SYS_SetNexusPID(U32 argv);
void MDrv_SYS_GetNexusPID(U32 argv);

U32 MDrv_SYS_PCMCIA_WRITE(U32 arg, BOOL bFromUser);
U32 MDrv_SYS_PCMCIA_READ(U32 arg, BOOL bFromUser);
U32 MDrv_SYS_PCMCIA_READ_DATA(U32 arg, BOOL bFromUser);
//U32 MDrv_SYS_PCMCIA_WRITE_DATA(U32 arg, BOOL bFromUser);

void MDrv_SYS_GetMBoxShareMemory(U32 argv);

void MDrv_SYS_GetMsBinInfo(U32 argv);
void MDrv_SYS_GetMIU1Base(U32 argv);
void MDrv_SYS_GetMIU1BusBase(U32 argv);
void MDrv_SYS_ForceUpgradeOADByDRAM(U32 arg);
void MDrv_SYS_ForceUpgradeENVByDRAM(U32 arg);
void MDrv_SYS_PrintMsg(U32 arg);

U32 MDrv_SYS_GetRawUART(U32 arg);
void MDrv_SYS_ReloadAeon( U32 arg ) ;
U32 MDrv_SYS_Timer(U32 arg) ;
U32 MDrv_SYS_RegOP(U32 arg);
extern void MDrv_SYS_MMAP_Dump( void ) ;
U32 MDrv_SYS_HotelMode(U32 arg) ;
U32 MDrv_SYS_HotelModePrintf(U32 arg) ;

void MDrv_SYS_ChangeUart( U32 arg );

void MDrv_SYS_SetGFXGOPIndex(U32 argv);
void MDrv_SYS_GetGFXGOPIndex(U32 argv);

void MDrv_SYS_SetDisplayControllerSeparated(U32 argv);
U32  MDrv_SYS_GetDisplayControllerSeparated(void);
void MDrv_SYS_IsDisplayControllerSeparated(U32 argv);

void MDrv_SYS_SetNexus(U32 argv);
void MDrv_SYS_HasNexus(U32 argv);


U32 MDrv_SYS_SPI_LOAD(U32 arg); //20100120 Terry, SPI Load Code

#elif defined(CONFIG_ARM64)

U32 MDrv_SYS_SetPanelInfo(unsigned long arg);
void MDrv_SYS_GetPanelRes(unsigned long arg);
void MDrv_SYS_GetPanelHStart(unsigned long argv);
void MDrv_SYS_GetGFXGOPPipelineDelay(unsigned long argv);

void MDrv_SYS_ReadGeneralRegister(unsigned long arg);
void MDrv_SYS_WriteGeneralRegister(unsigned long arg);
void MDrv_SYS_LoadAeon(unsigned long arg);
void MDrv_SYS_ResetAeon(unsigned long arg);
void MDrv_SYS_EnableAeon(void);
void MDrv_SYS_DumpAeonMessage(void);
void MDrv_SYS_DisableAeon(void);
void MDrv_SYS_SwitchUart(unsigned long arg);
U32 MDrv_SYS_IsAeonEnable(unsigned long arg);

void MDrv_SYS_SetNexusPID(unsigned long argv);
void MDrv_SYS_GetNexusPID(unsigned long argv);

U32 MDrv_SYS_PCMCIA_WRITE(unsigned long arg, BOOL bFromUser);
U32 MDrv_SYS_PCMCIA_READ(unsigned long arg, BOOL bFromUser);
U32 MDrv_SYS_PCMCIA_READ_DATA(unsigned long arg, BOOL bFromUser);
//U32 MDrv_SYS_PCMCIA_WRITE_DATA(unsigned long arg);

void MDrv_SYS_GetMBoxShareMemory(unsigned long argv);

void MDrv_SYS_GetMsBinInfo(unsigned long argv);
void MDrv_SYS_GetMIU1Base(unsigned long argv);
void MDrv_SYS_GetMIU1BusBase(unsigned long argv);
void MDrv_SYS_ForceUpgradeOADByDRAM(unsigned long arg);
void MDrv_SYS_ForceUpgradeENVByDRAM(unsigned long arg);
void MDrv_SYS_PrintMsg(unsigned long arg);

U32 MDrv_SYS_GetRawUART(unsigned long arg);
void MDrv_SYS_ReloadAeon(unsigned long arg ) ;
U32 MDrv_SYS_Timer(unsigned long arg) ;
U32 MDrv_SYS_RegOP(unsigned long arg);
extern void MDrv_SYS_MMAP_Dump( void ) ;
U32 MDrv_SYS_HotelMode(unsigned long arg) ;
U32 MDrv_SYS_HotelModePrintf(unsigned long arg) ;

void MDrv_SYS_ChangeUart(unsigned long arg );

void MDrv_SYS_SetGFXGOPIndex(unsigned long argv);
void MDrv_SYS_GetGFXGOPIndex(unsigned long argv);

void MDrv_SYS_SetDisplayControllerSeparated(unsigned long argv);
U32  MDrv_SYS_GetDisplayControllerSeparated(void);
void MDrv_SYS_IsDisplayControllerSeparated(unsigned long argv);

void MDrv_SYS_SetNexus(unsigned long argv);
void MDrv_SYS_HasNexus(unsigned long argv);


U32 MDrv_SYS_SPI_LOAD(unsigned long arg); //20100120 Terry, SPI Load Code

#endif

void MDrv_SYS_ReadMemory(void);
void MDrv_SYS_FlushMemory(void);

unsigned int MDrv_SYS_GetPowerStates(void);
unsigned int MDrv_SYS_GetGPIOIR(void);
unsigned int MDrv_SYS_GetGPIOIRType(void);

#endif // _DRV_SYSTEM_H_
