#include "ping.h"
#include "el_icmpv4.h"
#include "el_ipaddr.h"
#include "dbg.h"
#include "plat.h"

timer_t ping_timeout_timer;

void ping_timeout_timer_handler(timer_t * timer,void * args);

void ping_init(void)
{
    net_timer_add(&ping_timeout_timer,
                  ping_timeout_timer_handler,
                  RELOAD_FOREVER,
                  IP_TIMER_CALLBACK_TIME,
                  &ping_timeout_timer);
}

net_err_t ping_generate(uint8_t * des, 
                    uint16_t id, 
                    uint16_t seq, 
                    uint8_t * data, 
                    uint16_t len)
{
    nbuf_t * nbuf;
    net_err_t err;

    err = nbuf_alloc(&nbuf, len);
    if (err)
        goto __return;

    err = nbuf_write(nbuf, data, len);
    if (err) {
        printk("error: unexpected error!\n");
        goto __return;
    }

    err = icmpv4_echo_request_out(nbuf, des, id, seq);
    if (err)
        goto __return;

__return:
    if (nbuf) nbuf_free(nbuf);
    return err ? NET_ERR_NOK : NET_ERR_OK;
}

static const char * default_data = "0123456789ABCDEF0123456789ABCDEF";
net_err_t ping_send(uint8_t * des, 
                    uint16_t id, 
                    uint16_t seqs)
{
    ping_generate(des, id, seqs, default_data, strlen(default_data));   
}

void ping_timeout_timer_handler(timer_t * timer, void * args)
{
    printk("请求超时\r\n");
}

void ping_ctx_init(ping_t * ping,
                    ip4addr_t * des,
                    ip4addr_t * src,
                    int count,
                    int data_len)
{
    ping->des = des;
    ping->src = src;
    ping->count = count;
    ping->data_len = strlen(default_data);
    ping->period = 1000;
    ping->timeout = 10000;
    ip4addr2str(ping->des_ip_str, des);
}

void ping_start(ping_t * ping)
{
    /* timer start */
    net_timerst_init(&ping->timer, 
                     ping_periodic_handler,\
                     ping->count,
                     ping->period,
                     ping);
    net_timer_insert(&ping->timer);

    /* record echo request out tick */
    ping->out_tick = (size_t)sys_tick_now();
}

void ping_periodic_handler(timer_t * timer, void * args)
{
    ping_t * ping = (ping_t *)args;

    printk("来自 %s 的回复: 字节=%d 时间=%dms TTL=%d\r\n", 
            ping->des_ip_str, 
            ping->data_len, 
            ping->in_tick - ping->out_tick, 
            IPV4_HEAD_TTL_DEFAULT);
}