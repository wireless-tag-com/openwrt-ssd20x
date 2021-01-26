/*
* part_mxp.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: alterman.lin <alterman.lin@sigmastar.com.tw>
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

#ifndef _PART_MXP_H_
#define _PART_MXP_H_


#define MXP_PART_MAGIC_PREFIX           "MXPT"
#define MXP_PART_MAGIC_SUFFIX           "TPXM"

#define MXP_STORAGE_TYPE_FLASH          0x01

#define MXP_PART_TYPE_TAG               0x00
#define MXP_PART_TYPE_NORMAL            0x01
#define MXP_PART_TYPE_MTD               0x02

#define MXP_PART_FORMAT_NONE            0x00


#define MXP_PART_STATUS_UPGRADING       0xF7
#define MXP_PART_STATUS_READY           0xF6
#define MXP_PART_STATUS_ACTIVE          0xF5
#define MXP_PART_STATUS_INACTIVE        0xF4
#define MXP_PART_STATUS_EMPTY           0x00

typedef struct {

    u8      magic_prefix[4];//MXPT
    u8      version;
    u8      type;
    u8      format;
    u8      padding;
    u64     start;
    u64     size;
    u32     block;
    u32     block_count;
    u8      name[16]; //should be 0 terminated
    u8      backup[16]; //should be 0 terminated
    u8      hash[32];
    u8      reserved[23];
    u32     crc32;
    u8      status;
    u8      magic_suffix[4];//TPXM

} mxp_record;

typedef struct {
    u8 type;
    u64 size;
}mxp_storage_info;

typedef struct{
    int     (*read_table_bytes)(int offset,void* buf,int len);
    int     (*write_table_bytes)(int offset,void* buf,int len);
    int     (*update_table_byte)(int offset,char b);
    int     (*get_storage_info)(mxp_storage_info* info);
}mxp_manager;


//0:success -1:error
int mxp_set_record_status(int index,int status);

//0:success -1:error
int mxp_init(int size,mxp_manager* manager);

//0:success -1:error
int mxp_check_record_magic(int index);

//0:success -1:error
int mxp_check_table_magic(void);

//0:success -1:error
int mxp_init_table(void);

//0:success -1:error
int mxp_get_total_record_count(void);

//0:success -1:error
int mxp_set_record(mxp_record* rec);

//0:success -1:error
int mxp_get_record_by_index(int index,mxp_record* rec);
int mxp_delete_record_by_index(int index);
//0~n:success -1:not found
int mxp_get_record_index(char* name);


int mxp_save_table(void);
int mxp_load_table(void);


#endif
