/*
* part_mxp.c- Sigmastar
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


#include <common.h>
#include <malloc.h>
#include "part_mxp.h"
#include "asm/arch/mach/platform.h"


#define MXP_ALLOC(x)            malloc(x)
#define MXP_FREE(x)             free(x)
#define MXP_MSG(format,...)     printf(format, ##__VA_ARGS__)

static int mxp_table_size=0;
static int mxp_record_count=0;
static mxp_manager* mxp;
static mxp_record*  mxp_parts=NULL;
static const int mxp_offset[] = {0xA000/*40K*/, 0xC000/*48K*/, 0xF000/*60K*/, 0x10000/*64K*/, 0x20000/*128K*/, 0x40000/*256K*/, 0x80000/*512K*/};
int mxp_base;


static int chk_magic_seq(int seq)
{

    if( mxp_parts[seq].magic_prefix[0]=='M' &&
        mxp_parts[seq].magic_prefix[1]=='X' &&
        mxp_parts[seq].magic_prefix[2]=='P' &&
        mxp_parts[seq].magic_prefix[3]=='T' &&
        mxp_parts[seq].magic_suffix[0]=='T' &&
        mxp_parts[seq].magic_suffix[1]=='P' &&
        mxp_parts[seq].magic_suffix[2]=='X' &&
        mxp_parts[seq].magic_suffix[3]=='M' )
    {
        return 0;
    }

    return -1;
}

static void fill_init_record(mxp_record* rec)
{
    rec->magic_prefix[0]='M';
    rec->magic_prefix[1]='X';
    rec->magic_prefix[2]='P';
    rec->magic_prefix[3]='T';
    rec->magic_suffix[0]='T';
    rec->magic_suffix[1]='P';
    rec->magic_suffix[2]='X';
    rec->magic_suffix[3]='M';
    rec->type=MXP_PART_TYPE_TAG;
    rec->status=MXP_PART_STATUS_EMPTY;
    rec->format=MXP_PART_FORMAT_NONE;
    rec->version=1;
    memset(rec->backup,0x00,sizeof(rec->backup));
    memset(rec->name,0x00,sizeof(rec->name));

}

int mxp_init(int size,mxp_manager* manager)
{
    int i=0;
    mxp_table_size=size;
    mxp=manager;

    if(mxp_parts!=NULL)
    {
        MXP_FREE(mxp_parts);
    }

    mxp_parts=MXP_ALLOC(mxp_table_size);
    if(!mxp_parts)
    {
        MXP_MSG("ERROR!! Failed to allocate memory with mxp_table_size:%d\n",mxp_table_size);
        return -1;
    }

    for(i=0; i<sizeof(mxp_offset)/sizeof(mxp_offset[0]);i++)
    {
        mxp_base=mxp_offset[i];
        mxp->read_table_bytes(mxp_base,mxp_parts,mxp_table_size);

        if(0==mxp_check_table_magic())
        {
            MXP_MSG("MXP found at mxp_offset[%d]=0x%08X, size=0x%X\n", i, mxp_offset[i], mxp_table_size);
            return 0;
        }
    }
    mxp_base = mxp_offset[1];
    MXP_MSG("MXP not found, use default base 0x%08X\n", mxp_base);
    return 0;

}


int mxp_check_record_magic(int index)
{
    return chk_magic_seq(index);
}


int mxp_check_table_magic(void)
{
    int count=(mxp_table_size/sizeof(mxp_record));
    if((0==chk_magic_seq(0)) && (0==chk_magic_seq(count-1)))
    {
        return 0;
    }
    return -1;
}



int mxp_init_table(void){
    int count=(mxp_table_size/sizeof(mxp_record));

    memset(mxp_parts,0xFF,mxp_table_size);

    fill_init_record(&mxp_parts[0]);
    fill_init_record(&mxp_parts[count-1]);

    mxp_record_count=0;

    return mxp->write_table_bytes(mxp_base,mxp_parts,mxp_table_size);

}


int mxp_get_total_record_count(void)
{
    return mxp_record_count;
}

int mxp_save_table(void)
{
    return mxp->write_table_bytes(mxp_base,mxp_parts,mxp_table_size);
}

//
int mxp_save_table_from_mem(u32 mem_address)
{
    mxp_storage_info info;
    mxp_record* mxps=(mxp_record*)((void *)mem_address);
    int count=(mxp_table_size/sizeof(mxp_record));
    int i=0;
    if((0!=chk_magic_seq(0)) || (0!=chk_magic_seq(count-1)))
    {
        return -1;
    }

    mxp->get_storage_info(&info);
    for(i=0;i<(count-1);i++)
    {

        if(mxps[i].type==MXP_PART_TYPE_TAG)
        {
            break;
        }

        if(((u64)mxps[i].start+(u64)mxps[i].size)>((u64)info.size))
        {
            mxps[i].size=(((u64)info.size)-(u64)mxps[i].start);
        }

    }

    return mxp->write_table_bytes(mxp_base,(void*)mem_address,mxp_table_size);
}
//
//


int mxp_set_record(mxp_record* part)
{
//  mxp_record found;

    mxp_storage_info info;
    int count=mxp_table_size/sizeof(mxp_record);
    int index=mxp_get_record_index((char *)(part->name));
    if(-1==index) //not found
    {
        if(mxp_record_count < (count-1))
        {
            index=mxp_record_count;
        }
        else
        {
            MXP_MSG("ERROR!! No vancacy to add new mxp:%s\n",part->name);
            return -2;
        }
    }


    MXP_MSG("set mxp:%s to index %d\n",part->name,index);
    memcpy(&mxp_parts[index],part,sizeof(mxp_record));

    mxp->get_storage_info(&info);

    if(((u64)mxp_parts[index].start+(u64)mxp_parts[index].size)>((u64)info.size))
    {
        mxp_parts[index].size=(((u64)info.size)-(u64)mxp_parts[index].start);
    }

    mxp_save_table();

    return 0;

}

//0:success -1:error
int mxp_get_record_by_index(int index,mxp_record* part)
{

    if(index>=mxp_record_count)
    {
        MXP_MSG("ERROR!! index:%d is out of bound:%d\n",index,mxp_record_count);
        return -1;
    }
    memcpy(part,&mxp_parts[index],sizeof(mxp_record));
    return 0;

}


int mxp_delete_record_by_index(int index)
{

    if(index>=mxp_record_count)
    {
        MXP_MSG("ERROR!! index:%d is out of bound:%d\n",index,mxp_record_count);
        return -1;
    }
    memcpy(&mxp_parts[index],&mxp_parts[index+1],(mxp_table_size-sizeof(mxp_record)*(index+1)));

    mxp_record_count-=1;
    return 0;

}

int mxp_set_record_status(int index,int status)
{
    if(index>=mxp_record_count)
    {
        MXP_MSG("ERROR!! index:%d is out of bound:%d\n",index,mxp_record_count);
        return -1;
    }
    mxp_parts[index].status=status;
    mxp->update_table_byte(mxp_base+((sizeof(mxp_record)*(index))+(sizeof(mxp_record)-5)),status);
    return 0;
}

//0~n:success -1:not found
int mxp_get_record_index(char* name)
{
    int i=0;
    for(i=0;i<(mxp_record_count);i++)
    {
//      mxp->read_bytes(sizeof(mxp_record)*(i+1),&mxp_parts[i+1],sizeof(mxp_record));

        if(0==strcmp((const char*)(mxp_parts[i].name),((const char*)name)))
        {
//          memcpy(part,&mxp_parts[i+1],sizeof(mxp_record));
            return i;
        }
    }

    return -1;

}

//0 -1:not found
int mxp_get_record_by_name(char* name,mxp_record* part)
{
    int i=0;
    for(i=0;i<(mxp_record_count);i++)
    {

        if(0==strcmp((const char*)(mxp_parts[i].name),((const char*)name)))
        {
            memcpy(part,&mxp_parts[i],sizeof(mxp_record));
            return 0;
        }
    }

    return -1;

}

int mxp_load_table(void){

    int count=mxp_table_size/sizeof(mxp_record);
    int i=0;

    if(mxp_parts!=NULL)
    {
        MXP_FREE(mxp_parts);
        mxp_parts=NULL;
    }

    mxp_record_count=0;
    mxp_parts=MXP_ALLOC(mxp_table_size);
    if(!mxp_parts)
    {
        MXP_MSG("ERROR!! Failed to allocate memory with mxp_table_size:%d\n",mxp_table_size);
        return -1;
    }
    memset(mxp_parts,0xFF,mxp_table_size);

    for(i=0;i<count-1;i++)
    {
        mxp->read_table_bytes(mxp_base+sizeof(mxp_record)*i,&mxp_parts[i],sizeof(mxp_record));

        if(mxp_parts[i].type==MXP_PART_TYPE_TAG)
        {
            break;
        }
    }

    mxp_record_count=i;

    // read last part
    mxp->read_table_bytes(sizeof(mxp_record)*(count-1),&mxp_parts[count-1],sizeof(mxp_record));

    return 0;
}
