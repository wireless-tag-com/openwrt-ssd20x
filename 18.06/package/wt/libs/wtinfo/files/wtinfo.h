#ifndef __WTINFO_H__
#define __WTINFO_H__

const char *wtinfo_get_val(void *info, const char *key);

void *wtinfo_init(void);
void wtinfo_deinit(void *info);

#endif
