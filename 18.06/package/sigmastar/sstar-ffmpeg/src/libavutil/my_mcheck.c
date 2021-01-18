#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <execinfo.h>
#include <pthread.h>
#include <malloc.h>


#include "libavutil/my_mcheck.h"
#include "libavutil/list.h"

#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

static int total_size, peak_size;


typedef struct mcheck_stl_s
{
    void *paddr;
    unsigned long size;
    char **bt;
    unsigned long bt_size;
    struct list_head list;
    int index;
} mcheck_stl_t;

typedef struct MI_SYS_BufferQueue_s
{
    pthread_mutex_t mutex;
    struct list_head list;
    unsigned long queue_buf_count;
    unsigned long init;
} mcheck_queue_t;

mcheck_queue_t mcheck_queue = {0};

static inline void _mcheck_init(void)
{
    mcheck_queue_t *pst_queue = &mcheck_queue;
    assert(pst_queue);

    if(!pst_queue->init)
    {
        pthread_mutex_init(&pst_queue->mutex,NULL);
        pthread_mutex_lock(&pst_queue->mutex);
        INIT_LIST_HEAD(&pst_queue->list);
        pst_queue->queue_buf_count = 0;
        pst_queue->init = 1;
        pthread_mutex_unlock(&pst_queue->mutex);
    }
    mallopt(M_MXFAST, 0);
}


static inline void _mcheck_deinit(void)
{
    mcheck_queue_t *pst_queue = &mcheck_queue;
    assert(pst_queue);

    pthread_mutex_lock(&pst_queue->mutex);
    memset(pst_queue, 0, sizeof(*pst_queue));
    pst_queue->init = 0;
    pthread_mutex_unlock(&pst_queue->mutex);
}

static inline void _mcheck_try_init(void)
{
    _mcheck_init();
}

static inline void _mcheck_add_queue_tail(mcheck_stl_t *pst_stl, mcheck_queue_t *pst_queue)
{
    _mcheck_try_init();

    assert(pst_queue && pst_stl);
    assert(list_empty(&pst_stl->list));
    pthread_mutex_lock(&pst_queue->mutex);
    list_add_tail(&pst_stl->list, &pst_queue->list);
    pst_queue->queue_buf_count++;
    total_size += pst_stl->size;
    assert(pst_queue->queue_buf_count > 0);
    // printf("+:%p, addr:%p, size:%lu, bt(%p, %lu)\n", pst_stl, pst_stl->paddr, pst_stl->size, pst_stl->bt, pst_stl->bt_size);
    pthread_mutex_unlock(&pst_queue->mutex);
}

static inline void _mcheck_add_queue_head(mcheck_stl_t *pst_stl, mcheck_queue_t *pst_queue)
{
    _mcheck_try_init();

    assert(pst_queue && pst_stl);
    assert(list_empty(&pst_stl->list));

    pthread_mutex_lock(&pst_queue->mutex);
    list_add_tail(&pst_stl->list, &pst_queue->list);
    pst_queue->queue_buf_count++;
    total_size += pst_stl->size;
    assert(pst_queue->queue_buf_count > 0);
    pthread_mutex_unlock(&pst_queue->mutex);
}

static inline mcheck_stl_t* _mcheck_find_remove_from_queue(void *index, mcheck_queue_t *pst_queue)
{
    mcheck_stl_t *pst_stl, *n;

    assert(pst_queue);
    assert(index);
    _mcheck_try_init();

    pthread_mutex_lock(&pst_queue->mutex);
    list_for_each_entry_safe(pst_stl, n, &pst_queue->list, list)
    {
        if (pst_stl->paddr == index)
        {
            list_del(&pst_stl->list);
            INIT_LIST_HEAD(&pst_stl->list);
            assert(mcheck_queue.queue_buf_count > 0);
            mcheck_queue.queue_buf_count--;
            peak_size = FFMAX(peak_size, total_size);
            //printf("runing alloc size = %d\n", total_size);
            total_size -= pst_stl->size;
            total_size = FFMAX(total_size, 0);
            // printf("-:%p, addr:%p, size:%lu, bt(%p, %lu)\n", pst_stl, pst_stl->paddr, pst_stl->size, pst_stl->bt, pst_stl->bt_size);
            pthread_mutex_unlock(&pst_queue->mutex);
            return pst_stl;
        }
    }
    pthread_mutex_unlock(&pst_queue->mutex);
    return NULL;
}

static void mcheck_init(void)
{
    _mcheck_init();
}

static void mcheck_deinit(void)
{
    _mcheck_deinit();
}

void mcheck_creat_record(void *addr, unsigned long size)
{
    mcheck_stl_t *pst_stl;
    void *array[10];

    pst_stl = malloc(sizeof(*pst_stl));
    assert(pst_stl);
    memset(pst_stl, 0, sizeof(*pst_stl));
    INIT_LIST_HEAD(&pst_stl->list);
    pst_stl->paddr = addr;
    pst_stl->size = size;

    pst_stl->bt_size = backtrace(array, 10);
    pst_stl->bt = backtrace_symbols(array, pst_stl->bt_size);
    if (pst_stl->bt == NULL)
    {
        perror("backtrace_symbols");
        exit(-1);
    }

    _mcheck_add_queue_tail(pst_stl, &mcheck_queue);
}

void mcheck_destory_record(void *addr)
{
    mcheck_stl_t *pst_stl = NULL;

    if(addr)
    {
        pst_stl = _mcheck_find_remove_from_queue(addr, &mcheck_queue);
        assert(pst_stl);
        if(pst_stl->bt)
        {
            free(pst_stl->bt);
            pst_stl->bt_size = 0;
        }
        
        free(pst_stl);
        pst_stl = NULL;
    }
}

void av_mcheck_dump(void)
{
#if ENABLE_MCHECK
    mcheck_stl_t *pos, *n;
    mcheck_queue_t *pst_queue = &mcheck_queue;
    unsigned long total = 0;
    int i;

    assert(pst_queue);
    _mcheck_try_init();

    printf("**************************** [[ START:  mcheck_leak total count:%12lu ]] *****************************\n", pst_queue->queue_buf_count);

    list_for_each_entry_safe(pos, n, &pst_queue->list, list)
    {
        assert(pos);
        printf("++++++++++++++++mcheck_leak_addr:%p mcheck_leak_size:%lu+++++++++++++++++\n", pos->paddr, pos->size);
        total += pos->size;
        printf("Obtained %lu stack frames.\n", pos->bt_size);
        for (i = 0; i < pos->bt_size; i++)
        {
            printf("%s\n", pos->bt[i]);
        }
        mcheck_destory_record(pos->paddr);
    }

    mcheck_deinit();

    printf("********************************** [total_size = %d, peak_size = %d] *********************************\n", total_size, peak_size);
    printf("**************************** [[ END: mcheck_leak total size:%12lu Bytes]] ****************************\n", total);
    total_size = 0;
    peak_size = 0;

    printf("****************************** [dump status before] **************************************************\n");
    malloc_stats();
    printf("******************************************************************************************************\n");

    malloc_trim(0);

    printf("****************************** [dump status after] ***************************************************\n");
    malloc_stats();
    printf("******************************************************************************************************\n");
#endif
}

