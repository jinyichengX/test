#include "ipgui_norm_queue.h"
#include "ipgui_memory.h"

#define QUEUE_HEAD_ITEM_PTR(q) (q->pool + (q->head * q->item_size))
#define QUEUE_HEAD_PREV_ITEM_PTR(q) (q->pool + ((q->head - 1) * q->item_size))
#define QUEUE_TAIL_ITEM_PTR(q) (q->pool + (q->tail * q->item_size))

/* increase queue length */
static inline void ipgui_norm_queue_item_increase(ipgui_norm_queue_t * q)
{
    q->tail = (q->tail + 1) % q->item_cnt;
    q->length ++;
}

/* decrease queue length */
static inline void ipgui_norm_queue_item_decrease(ipgui_norm_queue_t * q)
{
    q->head = (q->head + 1) % q->item_cnt;
    q->length --;
}

/* write queue item */
static inline void ipgui_norm_queue_item_write(ipgui_norm_queue_t * q, void * src)
{
    ipgui_memcpy(QUEUE_TAIL_ITEM_PTR(q), src, q->item_size);
}

/* read queue item */
static inline void ipgui_norm_queue_item_read(ipgui_norm_queue_t * q, void * des)
{
    ipgui_memcpy(des, QUEUE_HEAD_ITEM_PTR(q), q->item_size);
}

/* check if queue is full */
static inline s32_t ipgui_norm_queue_full(ipgui_norm_queue_t * q)
{
    return (q->length == q->item_cnt);
}

/* check if queue is empty */
static inline s32_t ipgui_norm_queue_empty(ipgui_norm_queue_t * q)
{
    return (q->length == 0);
}

/* init queue */
/* 为确保程序的鲁棒性最好从全局的对象池中申请ipc对象 */
ipgui_err_t ipgui_norm_queue_init(ipgui_norm_queue_t * q, void * pool, u16_t item_cnt, u16_t item_size)
{
    ipgui_err_t err = IPGUI_ERR_OK;

    /* init queue */
    q->length = 0;
    q->head = q->tail = 0;
    q->item_cnt = item_cnt;
    q->item_size = item_size;
    q->pool = (u8_t *)pool;

    return IPGUI_ERR_OK;
}

/* post message */
ipgui_err_t ipgui_norm_queue_post(ipgui_norm_queue_t * q, void * item, u16_t size)
{
    if( size != q->item_size )
        return IPGUI_ERR_NOK;

    if (ipgui_norm_queue_full(q)) {
        return IPGUI_ERR_NOK;
    }

    /* post message */
    ipgui_norm_queue_item_write(q, item);
    /* update queue length and tail pointer */
    ipgui_norm_queue_item_increase(q);

    return IPGUI_ERR_OK;
}

/* fetch message */
ipgui_err_t ipgui_norm_queue_fetch(ipgui_norm_queue_t * q, void * item)
{    
    if (ipgui_norm_queue_empty(q)) {
        return IPGUI_ERR_NOK;
    }
    /* fetch message */
    ipgui_norm_queue_item_read(q, item);
    /* update queue length and head pointer */
    ipgui_norm_queue_item_decrease(q);

    return IPGUI_ERR_OK;
}
