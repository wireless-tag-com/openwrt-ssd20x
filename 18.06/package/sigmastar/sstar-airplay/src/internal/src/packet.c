#include "packet.h"
#include "player.h"

int packet_queue_init(packet_queue_t *q)
{
    memset(q, 0, sizeof(packet_queue_t));

    CheckFuncResult(pthread_mutex_init(&q->mutex, NULL));
    CheckFuncResult(pthread_cond_init(&q->cond,NULL));

    q->abort_request = 0;
    return 0;
}


// 写队列尾部。pkt是一包还未解码的音频数据
int packet_queue_put(packet_queue_t *q, AVPacket *pkt)
{
    AVPacketList *pkt_list;
    
    /*if (av_packet_make_refcounted(pkt) < 0)
    {
        printf("[pkt] is not refrence counted\n");
        return -1;
    }*/
    
    pthread_mutex_lock(&q->mutex);
    
    pkt_list = (AVPacketList *)av_malloc(sizeof(AVPacketList));
    
    if (!pkt_list)
    {
        return -1;
    }
    
    pkt_list->pkt = *pkt;
    pkt_list->next = NULL;

    if (!q->last_pkt)   // 队列为空
    {
        q->first_pkt = pkt_list;
    }
    else
    {
        q->last_pkt->next = pkt_list;
    }
    q->last_pkt = pkt_list;
    q->nb_packets++;
    q->size += pkt_list->pkt.size + sizeof(*pkt_list);
    
    // 发个条件变量的信号：重启等待q->cond条件变量的一个线程
    pthread_cond_signal(&q->cond);

    pthread_mutex_unlock(&q->mutex);
    return 0;
}

// 读队列头部。
int packet_queue_get(packet_queue_t *q, AVPacket *pkt, int block)
{
    AVPacketList *p_pkt_node;
    int ret;

    pthread_mutex_lock(&q->mutex);

    while (1)
    {
        if(q->abort_request)
        {
            ret = -1;
            break;
        }
        p_pkt_node = q->first_pkt;
        if (p_pkt_node)             // 队列非空，取一个出来
        {
            q->first_pkt = p_pkt_node->next;
            if (!q->first_pkt)
            {
                q->last_pkt = NULL;
            }
            q->nb_packets--;
            q->size -= p_pkt_node->pkt.size + sizeof(*p_pkt_node);
            *pkt = p_pkt_node->pkt;
            av_freep(&p_pkt_node);
            ret = 1;

            break;
        }
        else if (!block)            // 队列空且阻塞标志无效，则立即退出
        {
            ret = 0;
            break;
        }
        else                        // 队列空且阻塞标志有效，则等待
        {
            printf("\033[33;2mno pkt wait pktnb: %d\033[0m\n",q->nb_packets);
            pthread_cond_wait(&q->cond, &q->mutex);
        }
    }
    pthread_mutex_unlock(&q->mutex);
    return ret;
}

int packet_queue_put_nullpacket(packet_queue_t *q, int stream_index)
{
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

void packet_queue_flush(packet_queue_t *q)
{
    AVPacketList *pkt, *pkt1;

    pthread_mutex_lock(&q->mutex);
    for (pkt = q->first_pkt; pkt; pkt = pkt1) {
        pkt1 = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->size = 0;
    q->duration = 0;
    pthread_mutex_unlock(&q->mutex);
}

void packet_queue_destroy(packet_queue_t *q)
{
    packet_queue_flush(q);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

void packet_queue_abort(packet_queue_t *q)
{
    pthread_mutex_lock(&q->mutex);

    q->abort_request = 1;
    printf("send pkt signal\n");
    pthread_cond_signal(&q->cond);

    pthread_mutex_unlock(&q->mutex);
}

