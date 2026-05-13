#ifndef PING_H
#define PING_H

#include "el_ipv4.h"
#include "timer.h"
#ifdef __cplusplus
extern "C" {
#endif

#define PING_TIMEOUT_CALLBACK_TIME (1 * 1000)     /* PING超时定时器回调时间(ms) */

typedef struct
{
    int timeout;            /* tick to report ping time out */
    int period;             /* tick to send ping packet periodically */

    size_t out_tick;        /* system tick to send ping packet */
    size_t in_tick;         /* system tick to receive ping packet */

    int exp_id;             /* the expected ping id *//* 期望下一个收到的回送请求的id */
    int exp_seq;            /* the expected ping sequence number *//* 期望下一个收到的回送请求的序号 */

    ip4addr_t * des;        /* destination ipv4 address */
    char des_ip_str[16];    /* destination ipv4 address string,for example 10.246.41.45\0 */
    ip4addr_t * src;        /* source ipv4 address. 
                             * if not set, router will be used 
                             */
    int data_len;           /* ping data length */
    int count;              /* ping count, count = 0xffffffff means endless */
    timer_t timer;          /* ping timer */
}ping_t;

#ifdef __cplusplus
}
#endif

#endif