#include <ms_ir.h>
#include <common.h>

#define IR_MODE_SEL IR_MODE_FULLDECODE

#define REG_IR_CTRL             	     0x40
#define REG_IR_HDC_UPB          		 0x41
#define REG_IR_HDC_LOB          		 0x42
#define REG_IR_OFC_UPB          		 0x43
#define REG_IR_OFC_LOB          		 0x44
#define REG_IR_OFC_RP_UPB       		 0x45
#define REG_IR_OFC_RP_LOB       		 0x46
#define REG_IR_LG01H_UPB        		 0x47
#define REG_IR_LG01H_LOB        		 0x48
#define REG_IR_LG0_UPB          		 0x49
#define REG_IR_LG0_LOB          		 0x4A
#define REG_IR_LG1_UPB          		 0x4B
#define REG_IR_LG1_LOB          		 0x4C
#define REG_IR_SEPR_UPB         		 0x4D
#define REG_IR_SEPR_LOB         		 0x4E
#define REG_IR_TIMEOUT_CYC_L    		 0x4F
#define REG_IR_TIMEOUT_CYC_H             0x50
    #define IR_CCB_CB                    0x9F00//ir_ccode_byte:1+ir_code_bit_num:32
#define REG_IR_SEPR_BIT_FIFO_CTRL        0x51
#define REG_IR_CCODE            		 0x52
#define REG_IR_GLHRM_NUM        		 0x53
#define REG_IR_CKDIV_NUM_KEY_DATA        0x54
#define REG_IR_SHOT_CNT_L       		 0x55
#define REG_IR_SHOT_CNT_H_FIFO_STATUS    0x56
    #define IR_RPT_FLAG                  0x0100
    #define IR_FIFO_EMPTY                0x0200
#define REG_IR_FIFO_RD_PULSE    		 0x58

#define IR_BANK 0x1F007A00

#define READ_WORD(bank)         (*(volatile u16*)(IR_BANK + ((bank)<<2)))
#define WRITE_WORD(bank, val)   (*(volatile u16*)(IR_BANK + ((bank)<<2))) = (u16)(val)


static U8 _mdrv_read_fifo(void)
{
    U8 keyVal = 0;
    U16 regVal = 0;

    keyVal = READ_WORD(REG_IR_CKDIV_NUM_KEY_DATA)>>8;
    regVal = READ_WORD(REG_IR_FIFO_RD_PULSE)|0x01;
    WRITE_WORD(REG_IR_FIFO_RD_PULSE,regVal);

    return keyVal;
}

static void _mdrv_clear_fifo(void)
{
    U16 regVal = 0;

    regVal = READ_WORD(REG_IR_SEPR_BIT_FIFO_CTRL);
    regVal |= 0x01 << 15;
    WRITE_WORD(REG_IR_SEPR_BIT_FIFO_CTRL,regVal);
}

#if (IR_MODE_SEL == IR_MODE_FULLDECODE)
static void _mdrv_get_key_fullmode(struct IR_KeyInfo *keyInfo)
{
    U16 regVal=0;
    U8 currKey=0;

    regVal = READ_WORD(REG_IR_SHOT_CNT_H_FIFO_STATUS);
    if(regVal&IR_FIFO_EMPTY)
    {
        keyInfo->u8Valid = 0;
        return;
    }

    currKey = _mdrv_read_fifo();
    regVal = READ_WORD(REG_IR_SHOT_CNT_H_FIFO_STATUS);
    keyInfo->u8Flag = (regVal&IR_RPT_FLAG)?1:0;
    keyInfo->u8Key = currKey;
    keyInfo->u8Valid = 1;

    while(!(regVal&IR_FIFO_EMPTY))
    {
         currKey = _mdrv_read_fifo();
         regVal = READ_WORD(REG_IR_SHOT_CNT_H_FIFO_STATUS);
    }
    //_mdrv_clear_fifo();
}
#elif(IR_MODE_SEL == IR_MODE_RAWDATA)
#define IR_RAW_DATA_NUM 4
static void _mdrv_get_key_rawmode(struct IR_KeyInfo *keyInfo)
{
    U16 i,regVal=0;
    static int bRepeat=0;
    static U8 preKey = 0;
    static U8 keyCount = 0;
    static U8 rawKey[IR_RAW_DATA_NUM];

    regVal = READ_WORD(REG_IR_SHOT_CNT_H_FIFO_STATUS);
    if(bRepeat == 1 && (regVal&IR_RPT_FLAG) == IR_RPT_FLAG)
    {
        bRepeat = 0;
        keyInfo->u8Valid = 1;
        keyInfo->u8Key = preKey;
        _mdrv_clear_fifo();
        return;
    }
    bRepeat = 1;

    for(i = 0; i < IR_RAW_DATA_NUM; i++)
    {
        regVal = READ_WORD(REG_IR_SHOT_CNT_H_FIFO_STATUS);
        if(regVal&IR_FIFO_EMPTY)
        {
            bRepeat = 0;
            keyInfo->u8Valid = 0;
            return;
        }

        rawKey[keyCount++] = _mdrv_read_fifo();
        if(keyCount == IR_RAW_DATA_NUM)
        {
            keyCount = 0;
            if( (rawKey[0] == IR_HEADER_CODE0)
              &&(rawKey[1] == IR_HEADER_CODE1)
              &&(rawKey[2] == (U8)(~rawKey[3])))
            {
                bRepeat = 0;
                preKey = rawKey[2];
                keyInfo->u8Key = rawKey[2];
                keyInfo->u8Valid = 1;
            }
        }
    }

    _mdrv_clear_fifo();
}
#endif

u8 _mdrv_get_key(u8 *key_value)
{
    struct IR_KeyInfo ir_keyInfo;

    memset((void *)&ir_keyInfo, 0, sizeof(struct IR_KeyInfo));

    #if (IR_MODE_SEL == IR_MODE_FULLDECODE)
    _mdrv_get_key_fullmode(&ir_keyInfo);
    #elif (IR_MODE_SEL == IR_MODE_RAWDATA)
    _mdrv_get_key_rawmode(&ir_keyInfo);
    #endif

    if(ir_keyInfo.u8Valid)
    {
        *key_value = ir_keyInfo.u8Key;
        return 0;
    }
    else
    {
        *key_value = 0;
        return 1;
    }

}

static void _mdrv_set_timing(void)
{
    // header code upper/lower bound
    WRITE_WORD(REG_IR_HDC_UPB,IR_HDC_UPB);
    WRITE_WORD(REG_IR_HDC_LOB, IR_HDC_LOB);

    // off code upper/lower bound
    WRITE_WORD(REG_IR_OFC_UPB, IR_OFC_UPB);
    WRITE_WORD(REG_IR_OFC_LOB, IR_OFC_LOB);

    // off code repeat upper/lower bound
    WRITE_WORD(REG_IR_OFC_RP_UPB, IR_OFC_RP_UPB);
    WRITE_WORD(REG_IR_OFC_RP_LOB, IR_OFC_RP_LOB);

    // logical 0/1 high upper/lower bound
    WRITE_WORD(REG_IR_LG01H_UPB, IR_LG01H_UPB);
    WRITE_WORD(REG_IR_LG01H_LOB, IR_LG01H_LOB);

    // logical 0 upper/lower bound
    WRITE_WORD(REG_IR_LG0_UPB, IR_LG0_UPB);
    WRITE_WORD(REG_IR_LG0_LOB, IR_LG0_LOB);

    // logical 1 upper/lower bound
    WRITE_WORD(REG_IR_LG1_UPB, IR_LG1_UPB);
    WRITE_WORD(REG_IR_LG1_LOB, IR_LG1_LOB);

    // timeout cycles
    WRITE_WORD(REG_IR_TIMEOUT_CYC_L, IR_RP_TIMEOUT&0xFFFF);
	//set up ccode bytes and code bytes/bits num
    WRITE_WORD(REG_IR_TIMEOUT_CYC_H, IR_CCB_CB | 0x30UL | ((IR_RP_TIMEOUT >> 16)&0x0F));

    WRITE_WORD(REG_IR_CKDIV_NUM_KEY_DATA, IR_CKDIV_NUM);
}

int _mdrv_ir_init(void)
{
    U16 regVal = 0;

    _mdrv_set_timing();

    WRITE_WORD(REG_IR_CCODE, (IR_HEADER_CODE1<<8)|IR_HEADER_CODE0);
    WRITE_WORD(REG_IR_CTRL, 0x01BF);
    WRITE_WORD(REG_IR_GLHRM_NUM, 0x0804);
    WRITE_WORD(REG_IR_SEPR_BIT_FIFO_CTRL,0x0F00);

    if(IR_MODE_SEL == IR_MODE_RAWDATA)
    {
        WRITE_WORD(REG_IR_CTRL, 0x01B3);
        WRITE_WORD(REG_IR_GLHRM_NUM, READ_WORD(REG_IR_GLHRM_NUM)|(0x02<<12));
        WRITE_WORD(REG_IR_FIFO_RD_PULSE,READ_WORD(REG_IR_FIFO_RD_PULSE)|0x20); //wakeup key sel
    }
    else if(IR_MODE_SEL == IR_MODE_FULLDECODE)
    {
        WRITE_WORD(REG_IR_GLHRM_NUM, READ_WORD(REG_IR_GLHRM_NUM)|(0x03<<12));
        WRITE_WORD(REG_IR_FIFO_RD_PULSE,READ_WORD(REG_IR_FIFO_RD_PULSE)|0x20); //wakeup key sel
    }
    else
    {
        WRITE_WORD(REG_IR_GLHRM_NUM, READ_WORD(REG_IR_GLHRM_NUM)|(0x01<<12));
        if(IR_TYPE_SEL == IR_TYPE_RCMM)
        {
            regVal = READ_WORD(REG_IR_SEPR_BIT_FIFO_CTRL)|(0x01<<12);
            WRITE_WORD(REG_IR_SEPR_BIT_FIFO_CTRL, regVal);
        }
        else
        {
            #ifdef IR_INT_NP_EDGE_TRIG
            regVal = READ_WORD(REG_IR_SEPR_BIT_FIFO_CTRL)|(0x03<<12);
            WRITE_WORD(REG_IR_SEPR_BIT_FIFO_CTRL, regVal);
            #else
            WRITE_WORD(REG_IR_SEPR_BIT_FIFO_CTRL,0x2F00);//[10:8]: FIFO depth, [11]:Enable FIFO full
            #endif
        }
    }

    #if(IR_MODE_SEL==IR_MODE_SWDECODE)
        _mdrv_set_sw_decode(1);
        WRITE_WORD(REG_IR_CKDIV_NUM_KEY_DATA,0x00CF);
    #endif

    #if((IR_MODE_SEL==IR_MODE_RAWDATA)||(IR_MODE_SEL==IR_MODE_FULLDECODE)||(IR_MODE_SEL==IR_MODE_SWDECODE))
        _mdrv_clear_fifo();
    #endif

    return 0;
}