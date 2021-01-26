/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef __HAL_PNL_DIS_REG_H__
#define __HAL_PNL_DIS_REG_H__



#define _PNLBIT(_bit_)          (1 << (_bit_))
#define PNL_BITS(_bits_, _val_) ((_PNLBIT(((1)?_bits_)+1)-_PNLBIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define PNL_BITMASK(_bits_)     (_PNLBIT(((1)?_bits_)+1)-_PNLBIT(((0)?_bits_)))


#define CHIP_BASE(type, BankAddr,offset,shift)   (0xFD000000 + (BankAddr)+ (offset<<shift))

//-----------------------------------------------------------------------------------------------
#define DSI_REG_BASE        (0x1A2900UL)
#define DSI_CMDQ_BASE       (0x1A2A00UL)

//-----------------------------------------------------------------------------------------------

#define REG_DSI_START    0x0000         //DSI Start Register
     #define DSI_VM_CMD_START           PNL_BIT16
     #define DSI_SKEWCAL_START          PNL_BIT4
     #define DSI_SLEEPOUT_START         PNL_BIT2
     #define DSI_DSI_START              PNL_BIT0
     //DSI_VM_CMD_START: Starts DSI VM command transmission
    //Set up this bit to start command transmission in video mode. This bit is only available when VM_CMD_EN = 1.
    //0: No effect
    //1: Start

     //DSI_SKEWCAL_START: DSI skew calibration operation
    //Set this bit to start de-skew calibration process for RX. This bit is only available when DSI_BUSY = 0
    //0: No effect
    //1: Start

     //DSI_SLEEPOUT_START: DSI sleep-out operation
    //Set up this bit to wake up DSI from ULPS mode. This bit is only available when SLEEP_MODE = 1.
    //0: No effect
    //1: Start

     //DSI_DSI_START: Starts DSI controller operation
    //Set up this bit to start DSI control.
    //0: No effect
    //1: Start

#define REG_DSI_STATUS    0x0004        //DSI Status Register
    #define DSI_CONTENTION_ERR          PNL_BIT7
     #define DSI_FALSE_CTRL_ERR         PNL_BIT6
     #define DSI_ESC_SYNC_ERR           PNL_BIT5
     #define DSI_ESC_ENTRY_ERR          PNL_BIT4
     #define DSI_BUFFER_UNDERRUN        PNL_BIT1
     //Contention error
    //This error indicates a contention error happened during transmission.

    //False control error
    //This error indicates a LP-Rqst is not followed by a valid escape or turnaround sequence, or a HS-Rqst is not correctly followed by a bridge state from RX

    //Escape mode sync error
    //This error indicates the last bit of a transmission does not match a byte boundary

    //Escape mode entry error
    //This error indicates an invalid escape entry command from RX

    //Buffer underrun error
    //This error indicates the DSI throughput exceeds data throughput and makes invalid data transmission.

#define REG_DSI_INTEN    0x0008             //DSI Interrupt Enable Register
    #define DSI_SKEWCAL_DONE_INT_EN         PNL_BIT11
     #define DSI_VM_VFP_STR_INT_EN          PNL_BIT10
     #define DSI_VM_VACT_STR_INT_EN         PNL_BIT9
     #define DSI_VM_VBP_STR_INT_EN          PNL_BIT8
     #define DSI_TE_TIMEOUT_INT_EN          PNL_BIT7
     #define DSI_SLEEPOUT_DONE_INT_EN       PNL_BIT6
     #define DSI_VM_CMD_DONE_INT_EN         PNL_BIT5
     #define DSI_FRAME_DONE_INT_EN          PNL_BIT4
     #define DSI_VM_DONE_INT_EN             PNL_BIT3
     #define DSI_TE_RDY_INT_EN              PNL_BIT2
     #define DSI_CMD_DONE_INT_EN            PNL_BIT1
     #define DSI_LPRX_RD_RDY_INT_EN         PNL_BIT0
     //Skew-calibration done interrupt
    //This interrupt will be issued when skew-calibration process started by SKEWCAL_START is done.
    //0: Disable
    //1: Enable

     //Video mode VFP period start interrupt
    //This interrupt will be issued when VFP period starts during video mode transmission.
    //0: Disable
    //1: Enable

     //Video mode VACT period start interrupt
    //This interrupt will be issued when VACT period starts during video mode transmission.
    //0: Disable
    //1: Enable

    //Video mode VBP period start interrupt
    //This interrupt will be issued when VBP period starts during video mode transmission.
    //0: Disable
    //1: Enable

    //TE timeout interrupt
    //This interrupt will be issued when the wait time of TE signal exceeds SW-configured threshold.
    //0: Disable
    //1: Enable

    //Enables ULPS sleep-out interrupt
    //The interrupt will be issued when ULPS sleep out procedure is completed.
    //0: Disable
    //1: Enable

    //Enables DSI video mode command packet transmission interrupt
    //This interrupt will be issued when command packet transmission during video mode is completed.
    //0: Disable
    //1: Enable

    //Frame done interrupt
    //This interrupt will be issued when the frame transmission is done, both for video and command mode.
    //0: Disable
    //1: Enable

    //Enables DSI video mode finished interrupt
    //This interrupt will be issued when video mode transmission is finished.
    //0: Disable
    //1: Enable

    //DSI TE ready interrupt
    //This interrupt will be issued when either BTA TE or external TE is received.
    //0: Disable
    //1: Enable

    //Enables DSI command mode finished interrupt
    //This interrupt will be issued when all commands set in command queue are executed.
    //0: Disable
    //1: Enable

    //Enables RX data-ready interrupt
    //This interrupt will be issued when RX data are received through read commands. Enable this interrupt to receive data due to that the read response may be overwritten if another read command exists. An RACK operation should be set after reading data to allow HW continue execution.
    //0: Disable
    //1: Enable

#define REG_DSI_INTSTA    0x000C            //DSI Interrupt Status Register
    #define DSI_DSI_BUSY                    PNL_BIT31
    #define DSI_SKEWCAL_DONE_INT_FLAG       PNL_BIT11
    #define DSI_VM_VFP_STR_INT_FLAGN        PNL_BIT10
    #define DSI_VM_VACT_STR_INT_FLAG        PNL_BIT9
    #define DSI_VM_VBP_STR_INT_FLAG         PNL_BIT8
    #define DSI_TE_TIMEOUT_INT_FLAG         PNL_BIT7
    #define DSI_SLEEPOUT_DONE_INT_FLAG      PNL_BIT6
    #define DSI_VM_CMD_DONE_INT_FLAG        PNL_BIT5
    #define DSI_FRAME_DONE_INT_FLAG         PNL_BIT4
    #define DSI_VM_DONE_INT_FLAG            PNL_BIT3
    #define DSI_TE_RDY_INT_FLAG             PNL_BIT2
    #define DSI_CMD_DONE_INT_FLAG           PNL_BIT1
    #define DSI_LPRX_RD_RDY_INT_FLAG        PNL_BIT0

    //DSI busy status
    //0: Idle
    //1: Busy

    //Skew-calibration done interrupt status
    //0: Clear interrupt
    //1: No effect

    //Video mode VFP period start interrupt status
    //0: Clear interrupt
    //1: No effect

    //Video mode VACT period start interrupt status
    //0: Clear interrupt
    //1: No effect

    //Video mode VBP period start interrupt status
    //0: Clear interrupt
    //1: No effect

    //TE timeout interrupt status
    //0: Clear interrupt
    //1: No effect

    //ULPS sleep-out done interrupt status.
    //0: Clear interrupt
    //1: No effect

    //DSI video mode command packet transmission interrupt status
    //0: Clear interrupt
    //1: No effect

    //Frame done interrupt status
    //0: Clear interrupt
    //1: No effect

    //DSI video mode finished interrupt status
    //0: Clear interrupt
    //1: No effect

    //DSI TE ready interrupt status
    //0: Clear interrupt
    //1: No effect

    //DSI command mode finished interrupt status
    //0: Clear interrupt
    //1: No effect

    //RX data-ready interrupt status
    //0: Clear interrupt
    //1: No effect

#define REG_DSI_COM_CON     0x0010  //DSI Common Control Register
    #define DSI_DSI_DUAL_EN         PNL_BIT4
    #define DSI_DPHY_RESET          PNL_BIT2
    #define DSI_DSI_RESET           PNL_BIT0

    //Enables DSI dual module
    //Enable this bit to start dual-DSI display simultaneously.
    //0: Disable dual start
    //1: Enable dual start

    //DIG_MIPI_TX software reset
    //0: De-assert software reset
    //1: Assert software reset

    //DSI module software reset
    //0: De-assert software reset
    //1 Assert software reset

#define REG_DSI_MODE_CON 0x0014                 //DSI Mode Control Register
    #define DSI_SLEEP_MODE                      PNL_BIT20
     #define DSI_C2V_SWITCH_ON                  PNL_BIT19
     #define DSI_V2C_SWITCH_ON                  PNL_BIT18
     #define DSI_MIX_MODE                       PNL_BIT17
     #define DSI_FRAME_MODE                     PNL_BIT16
     #define DSI_MODE_CON_CMD                   PNL_BITS(1:0,0)
     #define DSI_MODE_CON_VID_SYNC_PULSE        PNL_BITS(1:0,1)
     #define DSI_MODE_CON_VID_SYNC_EVENT        PNL_BITS(1:0,2)
     #define DSI_MODE_CON_BURST                 PNL_BITS(1:0,3)
     #define DSI_MODE_MASK                      PNL_BITMASK(1:0)

        //DSI sleep mode for ULPS wake-up operation
    //This mode is used during wake-up stage to leave ULPS. Set this bit to 1 before setting LANE_NUM to enable output data lane to LP-00; then set up SLEEPOUT_START to start the ULPS-exit process.
    //0: Disable
    //1: Enable

    //DSI auto-switch from command mode to video mode
    //Set up this bit to wait for the last TE and start video mode when running in command mode with TE.
    //0: Disable
    //1: Enable

    //DSI auto-switch from video mode to command mode
    //Set up this bit to send the last vsync and switch to command idle when running in video mode.
    //0: Disable
    //1: Assert

    //Mixed mode for command/video transmission
    //Set up this bit to allow command packets in command queue transmitted after every VFP period in video mode.
    //0: Disable
    //1: Enable

    //Single frame mode for video mode
    //Set up this bit to make DSI return to idle after each frame is transmitted in video mode.
    //0: Disable
    //1: Enable

    //DSI mode control
    //Set DSI_EN = 1 when changing this register.
    //0: Command mode
    //1: Sync-pulse video mode
    //2: Sync-event video mode
    //3: Burst video mode

#define REG_DSI_TXRX_CON 0x0018             //DSI TX RX Control Register
    #define DSI_RGB_PKT_CNT                 PNL_BITMASK(27:24)
     #define DSI_EXT_TE_TIME_VM             PNL_BITMASK(23:20)
     #define DSI_TE_TIMEOUT_CHK_EN          PNL_BIT19
     #define DSI_TE_WITH_CMD_EN             PNL_BIT18
     #define DSI_TYPE1_BTA_SEL              PNL_BIT17
     #define DSI_HSTX_CKLP_EN               PNL_BIT16
     #define DSI_MAX_RTN_SIZE               PNL_BITMASK(15:12)
     #define DSI_TE_AUTO_SYNC               PNL_BIT11
     #define DSI_EXT_TE_EDGE_SEL            PNL_BIT10
     #define DSI_EXT_TE_EN                  PNL_BIT9
     #define DSI_TE_FREERUN                 PNL_BIT8
     #define DSI_HSTX_BLLP_EN               PNL_BIT7
     #define DSI_HSTX_DIS_EOT               PNL_BIT6
     #define DSI_LANE_NUM                   PNL_BITMASK(5:2)
     #define DSI_VC_NUM                     PNL_BITMASK(1:0)

        //Reverved counter target value for multi-packet in DE period
    //Do not use it in this version.

    //External TE timeout for video mode
    //Unit: Frame

    //Enables TE timeout check mechanism
    //Enable this bit to turn on DSI TE and external TE timeout check mechansim based on wait time of TE_TIMEOUT.
    //0: Disable
    //1: Enable


    //In the tradition design, TE command excutes 'bus turnaround' and ignores other settings in the same command column.
    //Combine the TE bit and other commands if this bit is asserted.
    //0: Disable
    //1: Enable

    //Selects TYPE1 BTA machenism
    //0: TYPE1 BTA by frame
    //1: TYPE1 BTA by packet

    //Enables non-continuous clock lane
    //0: Disable
    //1: Enable

    //Maximum return packet size
    //This register constrains maximum return packet that the slave side will send back to the host.
    //It takes effect after the host sends 'Set Maximum Return Packet Size' packet to slave.

    //Enables TE auto-sync
    //Turn on this register to automatically receive TE signals when command mode is idle. This bit is useful in command mode only.
    //0: Disable TE auto sync
    //1: Enable TE auto sync

    //Selects trigger edge type of external TE
    //0: Rising edge
    //1: Falling edge

    //Enables external TE signal
    //This bit should be set to receive external TE if LPTE pin is used as external TE pin.
    //0: Disable
    //1: Enable

    //TE free-run mode
    //If disabled, SW needs to wait for TE, write RACK and re-trigger DSI_START for the next transmission. If enabled, HW will detect TE and automatically start the next transmission until it is disabled
    //0: Disable free-run mode
    //1: Enable free-run mode

    //Enables null packet transfer in BLLP
    //0: Disable
    //1: Enable

    //Disables end of transmission packet
    //0: Enable EoTp
    //1: Disable EoTp

    //Lane number
    //Set up this bit to turn on PRE_OE to enable lane circuit.
    //4'b0000: Disable all lanes
    //4'b0001: Enable 1 lane
    //4'b0011: Enable 2 lanes
    //4'b0111: Enable 3 lanes
    //4'b1111: Enable 4 lanes

    //Virtual channel parameter in video mode
#define REG_DSI_PSCON 0x001C            //DSI Pixel Stream Control Register
    #define DSI_BYTE_SWAP               PNL_BIT25
     #define DSI_RGB_SWAP               PNL_BIT24
     #define DSI_DSI_PS_SEL             PNL_BITMASK(17:16)
     #define DSI_TDSI_PS_WC             PNL_BITMASK(13:0)

    //Selects byte order
    //For RGB565 type, it swaps bytes between MSB and LSB. For other stream typs, this bit is not used.
    //0: Normal case
    //1: Byte order change

    //Selects order of RGB
    //For all color types, it changes the color order in format of RGB or BGR.
    //0: Normal case
    //1: R/B order change

    //Selects pixel stream type
    //0: Packed pixel stream with 16-bit RGB 5-6-5 format
    //1: Packed pixel stream with 18-bit RGB 6-6-6 format
    //2: Loosely pixel stream with 24-bit RGB 6-6-6 format
    //3: Packed pixel stream with 24-bit RGB 8-8-8 format

    //Word count of long packet in valid pixel data duration
    //Unit: Byte
    //This value must be (H_SIZE*BPP). Take the QVGA display as an example, the value of PS_WC is (240*3) = 720 in decimal.

#define REG_DSI_VSA_NL 0x0020       //DSI Vertical Sync Active Register
    #define DSI_VSA_NL              PNL_BITMASK(11:0)

    //Vertical sync active duration
    //Unit: Line

#define REG_DSI_VBP_NL 0x0024       //DSI Vertical Back Porch Register
    #define DSI_VBP_NL              PNL_BITMASK(11:0)

    //Vertical back porch duration
    //Unit: Line
#define REG_DSI_VFP_NL 0x0028       //DSI Vertical Front Porch Register
    #define DSI_VFP_NL              PNL_BITMASK(11:0)

    //Vertical front porch duration
    //Unit: Line

#define REG_DSI_VACT_NL 0x002C      //DSI Vertical Active Register
    #define DSI_VACT_NL             PNL_BITMASK(11:0)

    //Vertical active duration
    //Configures frame height of pixels in video mode and command mode.

#define REG_DSI_LFR_CON 0x0030          //DSI Low Frame Rate Control Register
    #define DSI_LFR_SKIP_NUM            PNL_BITMASK(13:8)
    #define DSI_LFR_VSE_DIS             PNL_BIT6
    #define DSI_LFR_UPDATE              PNL_BIT5
    #define DSI_LFR_EN                  PNL_BIT4
    #define DSI_LFR_TYPE                PNL_BITMASK(3:2)
    #define DSI_LFR_MODE                PNL_BITMASK(1:0)

    //Low frame rate skip frame number
    //Configure this register to control the number of frames to skip in LFR static mode.

    //Disables low frame rate VSE
    //Set this bit to 1 to disable Vsync End packet transmission when low frame rate type is vsync only.
    //0: Enable VSE packet
    //1: Disable VSE packet

    //Low frame rate update in dynamic mode
    //Set this bit from 0 to 1 to send one normal frame in LFR dynamic mode. In both modes, updating frames will reset the skip frame counter.
    //0: No effect
    //1: Update frame

    //Enables low frame rate
    //Set this bit to 1 to enable LFR (low frame rate) mode.
    //0: Disable
    //1: Enable

    //Low frame rate transmission type
    //Selects packet types to be transmitted in skip frame
    //0: LP mode (no transmission)
    //1: Vsync only
    //2: Hsync only
    //3: Vsync + Hsync

    //Low frame rate mode
    //Selects which mode is used for LFR control. Static mode allows DSI  to send one normal frame per skip frames based on DSI_LFR_SKIP_NUM autometically, and dynamic mode allows software to control when to send normal frame manually.
    //0: Disable
    //1: Static mode
    //2: Dynamic mode
    //3: Both

#define REG_DSI_LFR_STA 0x0034          //DSI Low Frame Rate Status Register
    #define DSI_LFR_SKIP_STA            PNL_BIT8
    #define DSI_LFR_SKIP_CNT            PNL_BITMASK(5:0)

    //Low frame rate skip frame status
    //0: Normal frame
    //1: Skip frame

    //Low frame rate skip frame counter
    //Shows current frame count of skip frames in static mode

#define REG_DSI_HSA_WC 0x0050       //DSI Horizontal Sync Active Word Count Register
    #define DSI_HSA_WC              PNL_BITMASK(11:0)

    //Word count of horizontal sync active duration
    //Follow the program sequence and configure sync pulse mode.

#define REG_DSI_HBP_WC 0x0054       //DSI Horizontal Back Porch Word Count Register
    #define DSI_HBP_WC              PNL_BITMASK(11:0)

    //Word count of horizontal sync back porch duration
    //Follow the program sequence and configure all video mode.

#define REG_DSI_HFP_WC 0x0058        //DSI Horizontal Front Porch Word Count Register
    #define DSI_HFP_WC               PNL_BITMASK(11:0)

    //Word count of horizontal sync front porch duration
    //Follow the program sequence and configure all video mode.

#define REG_DSI_BLLP_WC 0x005C      //DSI Horizontal Blanking or Low Power Mode Word Count Register
    #define DSI_BLLP_WC             PNL_BITMASK(11:0)

    //Word count of horizontal blanking or low power mode duration
    //Follow the program sequence and configure burst mode.

#define REG_DSI_CMDQ_CON 0x0060     //DSI Command Queue Control Register
    #define DSI_CMDQ_SIZE           PNL_BITMASK(7:0)

    //Number of commands in command queue
    //Range: 1 ~ 127

#define REG_DSI_HSTX_CKLP_WC  0x0064    //DSI HSTX Clock Low-power Mode Word Count Register
    #define DSI_HSTX_CKLP_WC_AUTO       PNL_BIT16
    #define DSI_HSTX_CKLP_WC            PNL_BITMASK(15:2)

    //Automatic calculation for HSTX_CKLP_WC
    //Word count of non-continuous clock lane counter
    //Sets up HSTX clock low-power period when HSTX_CKLP_EN = 1. Refer to programming guide for details on the usage.

#define REG_DSI_HSTX_CKLP_WC_AUTO_RESULT    0x0068  //DSI HSTX Clock Low-power Mode Automatic Calculation result
    #define DSI_HSTX_CKLP_WC_AUTO_RESULT            PNL_BITMASK(15:0)

    //DSI HSTX clock low-power mode automatic calculation result
#define REG_DSI_RX_DATA03    0x0074     //DSI Receive Packet Data Byte 0 ~ 3 Register
    #define DSI_RX_BYTE3                PNL_BITMASK(31:24)
    #define DSI_RX_BYTE2                PNL_BITMASK(23:16)
    #define DSI_RX_BYTE1                PNL_BITMASK(15:8)
    #define DSI_RX_BYTE0                PNL_BITMASK(7:0)

#define REG_DSI_RX_DATA47    0x0078     //DSI Receive Packet Data Byte 4 ~ 7 Register
    #define DSI_RX_BYTE7                PNL_BITMASK(31:24)
    #define DSI_RX_BYTE6                PNL_BITMASK(23:16)
    #define DSI_RX_BYTE5                PNL_BITMASK(15:8)
    #define DSI_RX_BYTE4                PNL_BITMASK(7:0)

#define REG_DSI_RX_DATA8B    0x007C     //DSI Receive Packet Data Byte 8 ~ 11 Register
    #define DSI_RX_BYTEB                PNL_BITMASK(31:24)
    #define DSI_RX_BYTEA                PNL_BITMASK(23:16)
    #define DSI_RX_BYTE9                PNL_BITMASK(15:8)
    #define DSI_RX_BYTE8                PNL_BITMASK(7:0)

#define REG_DSI_RX_DATAC    0x0080      //DSI Receive Packet Data Byte 12 ~ 15 Register
    #define DSI_RX_BYTEF                PNL_BITMASK(31:24)
    #define DSI_RX_BYTEE                PNL_BITMASK(23:16)
    #define DSI_RX_BYTED                PNL_BITMASK(15:8)
    #define DSI_RX_BYTEC                PNL_BITMASK(7:0)

#define REG_DSI_RX_RACK    0x0084       //DSI Read Data Acknowledge Register
    #define DSI_RACK_BYPASS             PNL_BIT1
    #define DSI_RACK                    PNL_BIT0

    //Enables RX read acknowledge bypass
    //Set this bit to enable to ignore RACK from SW and continue next commands.
    //1: Does not check RACK
    //0: Check RACK

    //Acknowledges RX read
    //When a read command is executed and read data are received completely, the LPRX_RD_RDY interrupt will be issued. After read from the RX_DATA buffer, set up this bit to continue to the next command.
    //1: Acknowledge
    //0: No effect

#define REG_DSI_RX_TRIG_STA    0x0088       //DSI Receiver Status Register
    #define DSI_RX_POINTER                  PNL_BITMASK(11:8)
    #define DSI_RX_LPDT                     PNL_BIT6
    #define DSI_DIRECTION                   PNL_BIT5
    #define DSI_RX_ULPS                     PNL_BIT4
    #define DSI_RX_TRIG_3                   PNL_BIT3
    #define DSI_RX_TRIG_2                   PNL_BIT2
    #define DSI_RX_TRIG_1                   PNL_BIT1
    #define DSI_RX_TRIG_0                   PNL_BIT0

    //RX data count

    //RX low power data transmission state

    //Escape turnaround direction
    //Current bus direction of Data Lane 0. If set to 1, there will be reverse direction transmission on Data Lane 0 in low power mode; otherwise, it will be a forward direction transmission.
    //1: Reverse direction
    //0: Forward direction

    //RX ULPS (ultra-low power state)
    //Entry pattern is 00011110.

    //Reserved by DSI specification
    //Entry pattern is 10100000.

    //Acknowledge
    //Entry pattern is 00100001.

    //TE
    //Entry pattern is 01011101.

    //Remote application reset
    //Entry pattern is 01100010.

#define REG_DSI_MEM_CONTI    0x0090     //DSI Memory Continue Command Register
    #define DSI_DSI_RWMEM_CONTI         PNL_BITMASK(15:0)

    //Read/Write memory continue command
#define REG_DSI_FRM_BC    0x0094        //DSI Frame Byte Count Register
    #define DSI_FRM_BC                  PNL_BITMASK(20:0)

    //Frame buffer byte count
    //The total number of byte is expected to be read for type 3 command.

#define REG_DSI_V3D_CON    0x0098       //DSI Vsync 3D Control Register

#define REG_DSI_TIME_CON0    0x00A0     //DSI Timing Control 0 Register
    #define DSI_SKEWCAL_PRD             PNL_BITMASK(31:16)
    #define DSI_ULPS_WAKEUP_PRD         PNL_BITMASK(15:0)

    //Skew calibration period
    //Cycle count for skew-calibration period defined in DPHY 1.2 spec to continuously send out calibration patterns.
    //Total time = (SKEWCAL_PRD*1024*DSI clock cycle time)
    //Default value: 45us under 130MHz DSI byte clock


    //ULPS wakeup period
    //Cycle count for ultra-low power state (ULPS) wake-up during ULPS-exit sequence.
    //Total wait time = (ULPS_WAKEUP_PRD*1024*DSI clock cycle time)
    //Default value: 1ms under 130MHz DSI byte clock

#define REG_DSI_TIME_CON1    0x00A4         //DSI Timing Control 1 Register
    #define DSI_PREFETCH_EN                 PNL_BIT31
    #define DSI_PREFETCH_TIME               PNL_BITMASK(30:16)
    #define DSI_TE_TIMEOUT_PRD              PNL_BITMASK(15:0)

    //Enables prefetch for GCE to get DSI SOF prior to DE start
    //0: Disable
    //1: Enable

    //Prefetch pulse time

    //TE timeout check period
    //Cycle count to check TE timeout and issue timeout interrupt when waiting for TE signal.
    //Total wait time = (TE_TIMEOUT_PRD*16384*DSI clock cycle time)
    //Default value: 1sec under 130MHz DSI byte clock

#define REG_DSI_PHY_LCPAT    0x0100         //DSI PHY Lane Clock Pattern Register
    #define DSI_LC_HSTX_CK_PAT              PNL_BITMASK(7:0)

    //Clock lane HSTX clock pattern
    //Configure this field to generate HSTX clock pattern and change polarity. Default: Rising edge pulse first

#define REG_DSI_PHY_LCCON    0x0104         //DSI PHY Lane Clock Control Register
    #define DSI_LC_WAKEUP_EN                PNL_BIT2
    #define DSI_LC_ULPM_EN                  PNL_BIT1
    #define DSI_LC_HSTX_EN                  PNL_BIT0

    //Enables clock lane wake-up
    //Make the clock lane wake-up from ultra-low power mode. Make sure DSI_EN = 1 when setting up this register.

    //Enables clock lane ULPS
    //Make the clock lane go to ultra-low power mode. Make sure DSI_EN = 1 when setting up this register.

    //Enables clock lane HS mode
    //Starts clock lane high speed transmission.

#define REG_DSI_PHY_LD0CON    0x0108    //DSI PHY Lane 0 Control Register
    #define DSI_L0_RX_FILTER_EN         PNL_BIT4
    #define DSI_LX_ULPM_AS_L0           PNL_BIT3
    #define DSI_L0_WAKEUP_EN            PNL_BIT2
    #define DSI_L0_ULPM_EN              PNL_BIT1
    #define DSI_L0_RM_TRIG_EN           PNL_BIT0

    //Enables Data Lane 0 RX glitch filter
    //If the MIPI RX signal from peripheral is not stable, set up this bit to filter the signal glitches.

    //Enables Data Lane 1/2/3 ULPS mode
    //Set up this bit to make Data Lane 1/2/3 enter ULPS accompanied by Data Lane 0.

    //Enables Data Lane 0 wake-up
    //Make Data Lane 0 wake up from ultra-low power mode.

    //Enables Data Lane 0 ULPS
    //Make Data Lane 0 enter ultra-low power mode.

    //Enables Data Lane 0 remote application trigger
    //Send application trigger to slave side.

#define REG_DSI_PHY_SYNCON    0x010C            //DSI PHY Sync Control Register
    #define DSI_HS_DB_SYNC_EN                   PNL_BIT24
    #define DSI_HS_SKEWCAL_PAT                  PNL_BITMASK(23:16)
    #define DSI_HS_SYNC_CODE2                   PNL_BITMASK(15:8)
    #define DSI_HS_SYNC_CODE                    PNL_BITMASK(7:0)

    //Enables HSTX double-byte sync code
    //Set this bit to 1 if 16-bit sync code is to be sent. The LSB is HS_SYNC_CODE; the MSB is HS_SYNC_CODE2.

    //HSTX skew calibration pattern
    //Set up this bit to transmit continuous pattern code for skew calibration.


    //HSTX sync code 2
    //If a sync code with 16-bit length is sent, fill out this field.

    //HSTX sync code
    //Sync code byte sent for high-speed transmission
#define REG_DSI_PHY_TIMCON0    0x0110       //DSI PHY Timing Control 0 Register
    #define DSI_DA_HS_TRAIL                 PNL_BITMASK(31:24)
    #define DSI_DA_HS_ZERO                  PNL_BITMASK(23:16)
    #define DSI_DA_HS_PREP                  PNL_BITMASK(15:8)
    #define DSI_LPX                         PNL_BITMASK(7:0)

    //Control for timing parameter: T_HS-Trail
    //Control for timing parameter: T_HS-Zero
    //Control for timing parameter: T_HS-Prepare
    //Control for timing parameter: T_LPX
#define REG_DSI_PHY_TIMCON1    0x0114       //DSI PHY Timing Control 1 Register
    #define DSI_DA_HS_EXIT                  PNL_BITMASK(31:24)
    #define DSI_TA_GET                      PNL_BITMASK(23:16)
    #define DSI_TA_SURE                     PNL_BITMASK(15:8)
    #define DSI_TA_GO                       PNL_BITMASK(7:0)

    //Control for timing parameter: T_HS-Exit for data lane
    //Control for timing parameter: T_TA-Get
    //Control for timing parameter: T_TA-Sure
    //Control for timing parameter: T_TA-Go
#define REG_DSI_PHY_TIMCON2    0x0118       //DSI PHY Timing Control 2 Register
    #define DSI_CLK_HS_TRAIL                PNL_BITMASK(31:24)
    #define DSI_CLK_HS_ZERO                 PNL_BITMASK(23:16)
    #define DSI_DA_HS_SYNC                  PNL_BITMASK(15:8)
    #define DSI_CONT_DET                    PNL_BITMASK(7:0)

    //Control for timing parameter: T_CLK-Trail
    //Control for timing parameter: T_CLK-Zero
    //Control for timing parameter: T_HS-Sync
    //Control for contention detection cycle
#define REG_DSI_PHY_TIMCON3    0x011C    //DSI PHY Timing Control 3 Register
    #define DSI_CLK_HS_EXIT                PNL_BITMASK(23:16)
    #define DSI_CLK_HS_POST                PNL_BITMASK(15:8)
    #define DSI_CLK_HS_PREP                PNL_BITMASK(7:0)


    //Control for timing parameter: T_HS-Exit for clock lane
    //Control for timing parameter: T_CLK-Post
    //Control for timing parameter: T_CLK-Prepare

#define REG_DSI_VM_CMD_CON    0x0130    //DSI Video Mode Command Packet Control Register
    #define DSI_CM_DATA_1               PNL_BITMASK(31:24)
    #define DSI_CM_DATA_0               PNL_BITMASK(23:16)
    #define DSI_CM_DATA_ID              PNL_BITMASK(15:8)
    #define DSI_TS_VFP_EN               PNL_BIT5
    #define DSI_TS_VBP_EN               PNL_BIT4
    #define DSI_TS_VSA_EN               PNL_BIT3
    #define DSI_TIME_SEL                PNL_BIT2
    #define DSI_LONG_PKT                PNL_BIT1
    #define DSI_VM_CMD_EN               PNL_BIT0

    //Command packet data byte 1
    //If short packet, the byte will be used as DATA 1. If long packet, this registers should be 0 because word count should be less than 16 bytes.

    //Command packet data byte 0
    //If short packet, the byte will be used as DATA 0. If long packet, this registers should be used as Word Count 0 for long packet.

    //Command packet data ID
    //Note: The packet data ID should contain both data type and VC.


    //Command packet is allowable in VFP period
    //0: Disable
    //1: Enable

    //Command packet is allowable in VBP period
    //0: Disable
    //1: Enable

    //Command packet is allowable in VSA period
    //0: Disable
    //1: Enable

    //Timing interval for transmission
    //Allowable timing interval for command transmission in BLLP period. Select RGB if the HBP interval length is shorter than long packet length to be sent.
    //0: HBP timing interval
    //1: RGB timing interval

    //Command packet type
    //0: Short packet
    //1: Long packet

    //Enables video mode command packet transmission
    //0: Disable
    //1: Enable

#define REG_DSI_VM_CMD_DATA0    0x0134      //DSI Video Mode Command Packet Data Byte 0~3 Register
    #define DSI_LONG_PKT_WORD03             PNL_BITMASK(31:0)

    //Command packet word
    //Command packet data byte 0~3 of long packet for video mode transmission

#define REG_DSI_VM_CMD_DATA4    0x0138      //DSI Video Mode Command Packet Data Byte 4~7 Register
    #define DSI_LONG_PKT_WORD47             PNL_BITMASK(31:0)

    //Command packet word
    //Command packet data byte 4~7 of long packet for video mode transmission
#define REG_DSI_VM_CMD_DATA8    0x013C      //DSI Video Mode Command Packet Data Byte 8~11 Register
    #define DSI_LONG_PKT_WORD8B             PNL_BITMASK(31:0)

    //Command packet word
    //Command packet data byte 8~11 of long packet for video mode transmission
#define REG_DSI_VM_CMD_DATAC    0x0140      //DSI Video Mode Command Packet Data Byte 12~15 Register
    #define DSI_LONG_PKT_WORDCF             PNL_BITMASK(31:0)

    //Command packet word
    //Command packet data byte 12~15 of long packet for video mode transmission

#define REG_DSI_CKSM_OUT    0x0144          //DSI Checksum Output
    #define DSI_ACC_CHKSUM                  PNL_BITMASK(31:16)
    #define DSI_PKT_CHKSUM                  PNL_BITMASK(15:0)

    //Accumulated packet checksum
    //Checksum value of sequentially accumulated packets for System Level Test (SLT)

    //Transmitted packet checksum
    //Checksum value of the last transmitted packet for System Level Test (SLT)

#define REG_DSI_STATE_DBG0    0x0148        //DSI State Machine Debug 0
    #define DSI_DPHY_HS_TX_STATE_C          PNL_BITMASK(20:16)
    #define DSI_DPHY_CTL_STATE_C            PNL_BITMASK(8:0)

    //DSI DPHY clock lane HSTX state monitor
    //DSI DPHY clock lane control state monitor

#define REG_DSI_STATE_DBG1    0x014C        //DSI State Machine Debug 1
    #define DSI_DPHY_ESC_STATE_0            PNL_BITMASK(31:24)
    #define DSI_DPHY_HS_TX_STATE_0          PNL_BITMASK(20:16)
    #define DSI_DPHY_CTL_STATE_0            PNL_BITMASK(14:0)

    //DSI DPHY Data Lane 0 escape state monitor
    //DSI DPHY Data Lane 0 HSTX state monitor
    //DSI DPHY Data Lane 0 control state monitor

#define REG_DSI_STATE_DBG2    0x0150            //DSI State Machine Debug 2
    #define DSI_DPHY_TA_R2T_STATE               PNL_BITMASK(28:24)
    #define DSI_DPHY_TA_T2R_STATE               PNL_BITMASK(20:16)
    #define DSI_DPHY_RX_ESC_STATE               PNL_BITMASK(9:0)

    //DSI DPHY Data Lane 0 turnaround RX to TX state monitor
    //DSI DPHY Data Lane 0 turnaround TX to RX state monitor
    //DSI DPHY Data Lane 0 LPRX escape state monitor

#define REG_DSI_STATE_DBG3    0x0154             //DSI State Machine Debug 3
    #define DSI_DPHY_HS_TX_STATE_2               PNL_BITMASK(28:24)
    #define DSI_DPHY_CTL_STATE_2                 PNL_BITMASK(20:16)
    #define DSI_DPHY_HS_TX_STATE_1               PNL_BITMASK(12:8)
    #define DSI_DPHY_CTL_STATE_1                 PNL_BITMASK(4:0)

    //DSI DPHY Data Lane 2 HSTX state monitor
    //DSI DPHY Data Lane 2 control state monitor
    //DSI DPHY Data Lane 1 HSTX state monitor
    //DSI DPHY Data Lane 1 control state monitor

#define REG_DSI_STATE_DBG4    0x0158            //DSI State Machine Debug 4
    #define DSI_DPHY_HS_TX_STATE_3               PNL_BITMASK(12:8)
    #define DSI_DPHY_CTL_STATE_3                 PNL_BITMASK(4:0)

    //DSI DPHY Data Lane 3 HSTX state monitor
    //DSI DPHY Data Lane 3 control state monitor
#define REG_DSI_STATE_DBG5    0x015C            //DSI State Machine Debug 5
    #define DSI_WAKEUP_STATE                    PNL_BITMASK(31:28)
    #define DSI_TIMER_BUSY                      PNL_BIT16
    #define DSI_TIMER_COUNTER                   PNL_BITMASK(15:0)

    //DSI sleep-out state monitor
    //DSI sleep mode/TE timeout timer busy status
    //DSI sleep mode/TE timeout timer counter

#define REG_DSI_STATE_DBG6    0x0160            //DSI State Machine Debug 6
    #define DSI_CMDQ_STATE                      PNL_BITMASK(22:16)
    #define DSI_CMCTL_STATE                     PNL_BITMASK(14:0)

    //DSI command queue state monitor
    //DSI command control state monitor

#define REG_DSI_STATE_DBG7    0x0164            //DSI State Machine Debug 7
    #define DSI_VSA_PERIOD                      PNL_BIT15
    #define DSI_VBP_PERIOD                      PNL_BIT14
    #define DSI_VACT_PERIOD                     PNL_BIT13
    #define DSI_VFP_PERIOD                      PNL_BIT12
    #define DSI_VMCTL_STATE                     PNL_BITMASK(10:0)

    //DSI video mode VSA indication
    //DSI video mode VBP indication
    //DSI video mode VACT indication
    //DSI video mode VFP indication
    //DSI video mode control state monitor

#define REG_DSI_STATE_DBG8    0x0168            //DSI State Machine Debug 8
    #define DSI_WORD_COUNTER                    PNL_BITMASK(13:0)
    //DSI packet word count monitor
#define REG_DSI_STATE_DBG9    0x016C            //DSI State Machine Debug 9
    #define DSI_LINE_COUNTER                    PNL_BITMASK(21:0)
    //DSI packet line count monitor
#define REG_DSI_DEBUG_SEL    0x0170             //DSI Debug Output Selection
    #define DSI_CHKSUM_REC_EN                   PNL_BIT8
    #define DSI_DEBUG_OUT_SEL                   PNL_BITMASK(4:0)

    //Enables checksum record
    //Set this bit to enable checksum record circuit to obtain PKT_CHKSUM and ACC_CHKSUM in DSI_CKSM_OUT register.

    //Selects DSI debug out
    //This register selects which group of internal signal is sent to debug output port. It is only for hardware debug purpose and not described here.

#define REG_DSI_RESERVED    0x0174              //Reserved Register
    #define DSI_RESERVED                        PNL_BITMASK(31:0)

/*--------------------*/
/*    BIST Control */
/*--------------------*/
    //DSI reserved register for HW usage
#define REG_DSI_BIST_PATTE_RN    0x0178         //DSI Bist Pattern Register
    #define DSI_BIST_PATTERN                    PNL_BITMASK(31:0)
    //DSI BIST test register for HW verification
#define REG_DSI_BIST_CON        0x017C          //DSI Bist Control Register

    #define DSI_BIST_TIMIING                    PNL_BITMASK(23:16)
    #define DSI_BIST_LANE_NUM                   PNL_BITMASK(11:8)
    #define DSI_SELF_PAT_MODE                   PNL_BIT6
    #define DSI_BIST_HS_FREE                    PNL_BIT4
    #define DSI_BIST_SPECIFIED_PATTERN          PNL_BIT3
    #define DSI_BIST_FIX_PATTERN                PNL_BIT2
    #define DSI_BIST_ENABLE                     PNL_BIT1
    #define DSI_BIST_MODE                       PNL_BIT0

    //BIST timing control for MIPI signal
    //BIST lane number
    //4'b0000: Disable all lanes
    //4'b0001: Enable 1 lane
    //4'b0011: Enable 2 lanes
    //4'b0111: Enable 3 lanes
    //4'b1111: Enable 4 lanes

    //Enables the self-pattern test mode to show built-in pattens according to configuration of BIST_PATTERN value
    //0: Disable
    //1: Enable

    //Enables BIST free run test
    //0: Normal mode
    //1: Free-run mode

    //Enables BIST specified pattern
    //0: Not specified pattern
    //1: Specified pattern

    //Enables BIST fix pattern
    //0: Not fix pattern
    //1: Fix pattern

    //Start BIST mode
    //0: No effect
    //1: Trigger

    //Enables BIST mode
    //0: Disable
    //1: Enable

#define REG_DSI_VM_CMD_DATA10        0x0180         //DSI Video Mode Command Packet Data Byte 16~19 Register
    #define DSI_VM_LONG_PKT_WORD16_19               PNL_BITMASK(31:0)
#define REG_DSI_VM_CMD_DATA14        0x0184         //DSI Video Mode Command Packet Data Byte 20~23 Register
    #define DSI_VM_LONG_PKT_WORD20_23               PNL_BITMASK(31:0)
#define REG_DSI_VM_CMD_DATA18        0x0188         //DSI Video Mode Command Packet Data Byte 24~27 Register
    #define DSI_VM_LONG_PKT_WORD24_27               PNL_BITMASK(31:0)
#define REG_DSI_VM_CMD_DATA1C        0x018C         //DSI Video Mode Command Packet Data Byte 28~31 Register
    #define DSI_VM_LONG_PKT_WORD28_31               PNL_BITMASK(31:0)


#define REG_DSI_CMDQ_N        0x0200                //DSI Command Queue
    #define DSI_DATA_1                              PNL_BITMASK(31:24)
    #define DSI_DATA_0                              PNL_BITMASK(23:16)
    #define DSI_DATA_ID                             PNL_BITMASK(15:8)
    #define DSI_RESV                                PNL_BITMASK(7:6)
    #define DSI_TE                                  PNL_BIT5
    #define DSI_CL                                  PNL_BIT4
    #define DSI_HS                                  PNL_BIT3
    #define DSI_BTA                                 PNL_BIT2
    #define DSI_TYPE                                PNL_BITMASK(1:0)

    //Data byte 1 of command

    //Data byte 0 of command

    //Data ID of command

    //Reserved

    //Enables internal or external TE
    //0: Disable
    //1: Enable

    //Selects DCS byte
    //0: 1-byte DCS
    //1: 2-byte DCS

    //Enables high-speed transmission
    //0: LPTX transmission
    //1: HSTX transmission

    //Enables BTA
    //0: Disable
    //1: Enable

    //Command types
    //0: Type-0 command
    //1: Type-1 command
    //2: Type-2 command
    //3: Type-3 command






#endif /* __HAL_PNL_MIPI_MIPI_DSIREG_H__ */
