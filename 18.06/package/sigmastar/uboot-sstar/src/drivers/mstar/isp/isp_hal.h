/*
* isp_hal.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: eroy.yang <eroy.yang@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef ISP_HAL_H
#include <asm/types.h>
#define MS_BASE_REG_RIU_PA					(0x1F000000)
#define BANK_TO_ADDR32(b) (b<<9)
#define BANK_BASE_ADDR(bank) (MS_BASE_REG_RIU_PA+BANK_TO_ADDR32(bank))
typedef enum
{
    RIUBASE_ISP_0 = 0x1302,
    RIUBASE_ISP_1 = 0x1303,
    RIUBASE_ISP_2 = 0x1304,
    RIUBASE_ISP_3 = 0x1305,
    RIUBASE_ISP_4 = 0x1306,
    RIUBASE_ISP_5 = 0x1307,
    RIUBASE_ISP_6 = 0x1308,
    RIUBASE_ISP_7 = 0x1309,
    RIUBASE_ISP_8 = 0x130A,
    RIUBASE_ISP_9 = 0x130B,
    RIUBASE_CHIPTOP = 0x101E,
    RIUBASE_CLKGEN = 0x1038,
    RIUBASE_PADTOP = 0x103C,
    RIUBASE_CSI_MAC = 0x1204,
    RIUBASE_DPHY_CSI_ANA = 0x1202,
    RIUBASE_DPHY_CSI_DIG = 0x1203
} REG_BANKS;

//clock gen
#define OFFSET_CLKGEN_TO_CLK_SENSOR  (0x62<<2)
typedef enum {	//for reg_ckg_sr_mclk_select_clock_source
    SEN_MAIN_CLK_27M     = 0 ,
    SEN_MAIN_CLK_21P6M   = 1 ,
    SEN_MAIN_CLK_12M     = 2 ,
    SEN_MAIN_CLK_5P4M    = 3 ,
    SEN_MAIN_CLK_36M     = 4 ,
    SEN_MAIN_CLK_54M     = 5 ,
    SEN_MAIN_CLK_43P2M   = 6 ,
    SEN_MAIN_CLK_61P7M   = 7 ,
}SENSOR_MAIN_CLK;


/*! @brief Sensor master clock select */
typedef enum {
    CUS_CMU_CLK_5P4M    = 0 ,    /**< Sensor MCLK 5.4MHz */
    CUS_CMU_CLK_12M     = 1 ,     /**< Sensor MCLK 12MHz */
    CUS_CMU_CLK_21P6M   = 2 ,   /**< Sensor MCLK 21.6MHz */
    CUS_CMU_CLK_27M     = 3 ,     /**< Sensor MCLK 27MHz */
    CUS_CMU_CLK_36M     = 4 ,     /**< Sensor MCLK 36MHz */
    CUS_CMU_CLK_43P2M   = 5 ,   /**< Sensor MCLK 43.2MHz */
    CUS_CMU_CLK_54M     = 6 ,     /**< Sensor MCLK 54MHz */
    CUS_CMU_CLK_61P7M   = 7 ,   /**< Sensor MCLK 61.7MHz */
    CUS_CMU_CLK_ISP_MAX = 128,  /**< Reserved , Do not use */
    CUS_CMU_SCL_CLK_36M = 129, /**< Sensor MCLK 36MHz from SCL */
    CUS_CMU_SCL_CLK_27M = 130, /**< Sensor MCLK 36MHz from SCL */
    CUS_CMU_SCL_CLK_37P125M = 131, /**< Sensor MCLK 36MHz from SCL */
	CUS_CMU_SCL_CLK_33M = 132, /**< Sensor MCLK 33MHz from SCL */
	CUS_CMU_SCL_CLK_34M = 133, /**< Sensor MCLK 34MHz from SCL */
	CUS_CMU_SCL_CLK_36P125M = 134, /**< Sensor MCLK 36.125MHz from SCL */
	CUS_CMU_SCL_CLK_21P6M = 135, /**< Sensor MCLK 36.125MHz from SCL */
} CUS_MCLK_FREQ;

typedef struct {
  u32 reg_ckg_sr_disable_clock     :1;				//pixel clock
  u32 reg_ckg_sr_invert_clock    :1;				//pixel clock
  u32 reg_ckg_sr_select_clock_source        :2;		//enum SENSOR_PCLK
  u32                       :4;
  u32 reg_ckg_sr_mclk_disable_clock     :1;		//mclk
  u32 reg_ckg_sr_mclk_invert_clock :1;			//mclk
  u32 reg_ckg_sr_mclk_select_clock_source        :3;//enum SENSOR_MAIN_CLK
  u32                       :19;
} clk_sensor;

#define OFFSET_CHIPTOP_TO_SENSOR_MODE (0x06<<2)
 typedef struct {
   u32 reg_sr_mode           :3;
   u32                       :1;
   u32 reg_sr_i2c_mode       :2;
   u32                       :26;
 } chiptop_sensor_mode;
#endif
