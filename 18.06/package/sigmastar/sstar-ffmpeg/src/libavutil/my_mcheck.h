#ifndef  AVUTIL_MY_MCHECK_H
#define  AVUTIL_MY_MCHECK_H

#include <pthread.h>

#define  ENABLE_MCHECK      0

//void mcheck_init(void);
//void mcheck_try_init(void);
//void mcheck_deinit(void);
//void mcheck_add_queue_tail(mcheck_stl_t *pst_stl, mcheck_queue_t *pst_queue);
//void mcheck_add_queue_head(mcheck_stl_t *pst_stl, mcheck_queue_t *pst_queue);
//mcheck_stl_t* mcheck_find_remove_from_queue(void *index, mcheck_queue_t *pst_queue);

void mcheck_creat_record(void *addr, unsigned long size);
void mcheck_destory_record(void *addr);
void av_mcheck_dump(void);

#endif


