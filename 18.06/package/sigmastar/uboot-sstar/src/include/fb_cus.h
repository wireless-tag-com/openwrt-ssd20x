#ifndef _FB_CUS_H_
#define _FB_CUS_H_
#include <jffs2/load_kernel.h>

#define MAX_PARTNAME_LEN 20
typedef struct part_ops_s
{
    char partname[MAX_PARTNAME_LEN];
    int (*erase)(const char *partname);
    int (*write)(const char *partname, void *download_buffer, unsigned int file_size);
    int (*probe_info)(const char *partname, struct part_info *part_info);
} part_ops_t;

int fastboot_cus_get_part_info(const char *part_name,
                struct part_info *part_info, char *response);
void fastboot_cus_flash_write(const char *cmd, void *download_buffer,
			unsigned int download_bytes, char *response);
void fastboot_cus_erase(const char *cmd, char *response);

#endif
