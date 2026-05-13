/*
 * MIT License
 *
 * Copyright (c) 2025 JinYiCheng
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ipgui_event.h"
#include "ipgui_memory.h"
#include <stdio.h>

static ipgui_input_event_t input_value_sync = { IPGUI_EVT_TYPE_SYNC, IPGUI_EVT_SYN_REPORT, 1 };

__IPGUI_STATIC__ int ipgui_input_attach_handler(ipgui_input_dev_t * dev, ipgui_input_handler_t * handler);

/* set dev prio */
__IPGUI_API__  __IPGUI_INLINE__ void ipgui_input_device_prio_set(ipgui_input_dev_t * dev, unsigned int prio)
{
    dev->prio = prio;
}

/* set event type of device */
__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_event_type_set(ipgui_input_dev_t * dev, unsigned int type)
{
    dev->evtbits[BIT_WORD(type)] |= BIT_MASK(type);
}

/* set event and code of device */
__IPGUI_API__ void ipgui_input_set_capability(ipgui_input_dev_t * dev, unsigned int type, unsigned int code)
{
	switch (type)
    {
        case IPGUI_EVT_TYPE_KEY:
            __set_bit(code, dev->keybits);
            break;
        case IPGUI_EVT_TYPE_REL:
            __set_bit(code, dev->relbits);
            break;
        case IPGUI_EVT_TYPE_ABS:
            // input_alloc_absinfo(dev);
            // if (!dev->absinfo)
            // 	return;
            __set_bit(code, dev->absbits);
            break;
        case IPGUI_EVT_TYPE_MISC:
            __set_bit(code, dev->miscbits);
            break;
        case IPGUI_EVT_TYPE_SW:
            __set_bit(code, dev->swbits);
            break;
        default:
            // ipgui_debug_warning("input_set_capability: unknown type %u (code %u)\n", type, code);
            return;
	}

	ipgui_event_type_set(dev, type);
}

__IPGUI_API__ void ipgui_input_set_capability_composite(ipgui_input_dev_t * dev, ipgui_input_event_t * evts, int len)
{
    while(len --)
        ipgui_input_set_capability(dev, evts[len].type, evts[len].code);
}

__IPGUI_API__ ipgui_err_t ipgui_input_register_handler(ipgui_input_handler_t * handler, ipgui_list_t * dev_list, ipgui_list_t * handler_list)
{
    ipgui_node_t * pos, * temp;
    ipgui_input_dev_t * dev;

    // list_add_tail( &handler->node, &g_input_event_handler_list );
    list_add_tail( &handler->node, handler_list );
    list_head_init(&handler->handle_list);

    // list_for_each_safe(pos, temp, &g_input_device_list)
    list_for_each_safe(pos, temp, dev_list)
    {
        dev = list_entry(pos, ipgui_input_dev_t, node);
        ipgui_input_attach_handler(dev, handler);
    }

    return IPGUI_ERR_OK;
}

/* allocate input device */
__IPGUI_API__ ipgui_input_dev_t * ipgui_input_allocate_device(const char * name)
{
    ipgui_input_dev_t * dev;

    dev = ipgui_mem_alloc(ipgui_smem, sizeof(ipgui_input_dev_t));

    if( dev ){
        for( int i = 0; i < sizeof(ipgui_input_dev_t); i++ )
            ((char *)dev)[i] = 0;
        dev->name = name;
        list_head_init(&dev->node);
        list_head_init(&dev->handle_list);
    }

    return dev;
}

#if defined (IPGUI_BASETYPE_64BIT)
__IPGUI_STATIC__ int bitmap_subset(unsigned long * bitmap1, unsigned long * bitmap2, unsigned int bits)
#else
__IPGUI_STATIC__ int bitmap_subset(unsigned int * bitmap1, unsigned int * bitmap2, unsigned int bits)
#endif
{
    unsigned int i, loop = bits / BITS_PER_WORD;

    for( i = 0; i < loop; ++ i ){
        /* handler map must be the subset of device map
         * otherwise, the other dev event would be
         * sent to the wrong handler 
         */
        if( bitmap1[i] & (~bitmap2[i]) )
            return 0;
    }

    if( bits % BITS_PER_WORD ){
        if((bitmap1[i] & (~bitmap2[i])) & (~BITMAP_LAST_WORD_MASK(bits)))
            return 0;
    }

    return 1;
}

__IPGUI_STATIC__ struct input_device_id * ipgui_input_match_device(ipgui_input_handler_t * handler, ipgui_input_dev_t * dev)
{
    const struct input_device_id * id;

    /* 匹配到第一个或没有遇到标志位就返回 */
    for (id = handler->id_table; id->flags || id->driver_info; id ++)
    {
        if (!bitmap_subset(id->evtbits, dev->evtbits, IPGUI_EVT_TYPE_CNT))
			continue;

		if (!bitmap_subset(id->keybits, dev->keybits, IPGUI_EVT_KEY_CNT))
			continue;

		if (!bitmap_subset(id->relbits, dev->relbits, IPGUI_EVT_REL_CNT))
			continue;

		if (!bitmap_subset(id->absbits, dev->absbits, IPGUI_EVT_ABS_CNT))
			continue;

		if (!bitmap_subset(id->miscbits, dev->miscbits, IPGUI_EVT_MISC_CNT))
			continue;

		if (!bitmap_subset(id->swbits, dev->swbits, IPGUI_EVT_SW_CNT))
			continue;
#if 1
		if (!handler->match || handler->match(handler, dev))
#else
        if (!handler->match)
#endif
			return id;
    }


    return id;
}

/* 关联handler和dev */
__IPGUI_STATIC__ __IPGUI_INLINE__ void ipgui_register_handle(ipgui_input_handle_t * handle)
{
    ipgui_input_handler_t * handler = handle->handler;
    ipgui_input_dev_t * dev = handle->dev;

    list_head_init(&handle->dev_node);
    list_head_init(&handle->handler_node);
    list_add_tail(&handle->dev_node, &dev->handle_list);
    list_add_tail(&handle->handler_node, &handler->handle_list);
}

__IPGUI_STATIC__ int ipgui_input_attach_handler(ipgui_input_dev_t * dev , ipgui_input_handler_t * handler)
{
	struct input_device_id * id;
    ipgui_input_handle_t * handle = NULL;
	int error = 0;

	id = ipgui_input_match_device(handler, dev);
	if (!id)
		return -1;

    /* 有匹配成功的id */
    /* connect handler to device and register handle */
#if defined LINUX_LIKE_STYLE
	error = handler->connect(handler, dev, id);
#else
    if(NULL == (handle = ipgui_mem_alloc(ipgui_smem, sizeof(ipgui_input_handle_t))))
        return -1;

    ipgui_memset((void *)handle, 0, sizeof(ipgui_input_handle_t));
    handle->dev = dev;
    handle->handler = handler;
    ipgui_register_handle(handle);
#endif

	return error;
}

static int device_prio_cmp(void * cont1,void * cont2)
{
    ipgui_input_dev_t * dev1 = (ipgui_input_dev_t *)cont1;
    ipgui_input_dev_t * dev2 = (ipgui_input_dev_t *)cont2;
    
    return (dev1->prio) >= (dev2->prio);
}

__IPGUI_API__ ipgui_err_t ipgui_input_register_device(ipgui_input_dev_t * dev, ipgui_list_t * dev_list, ipgui_list_t * handler_list)
{
    ipgui_input_handler_t * handler;
    ipgui_node_t * pos, * temp;
    int event_num = 0;
    static int node_off = offsetof(ipgui_input_dev_t, node);

    if(( !dev ) || (!dev_list) || (!handler_list))
        return IPGUI_ERR_PARAM;

    /* must register sync event */
    dev->evtbits[BIT_WORD(IPGUI_EVT_TYPE_SYNC)] |= BIT_MASK(IPGUI_EVT_TYPE_SYNC);

    // list_add_tail( &dev->node, &g_input_device_list );
    list_add_tail( &dev->node, dev_list );

    /* for SYNC event */
    event_num += 1;

    /* Make room for KEY and MSC events */
    event_num += 7;

    /* not support multiple touch */
    if( test_bit(IPGUI_EVT_TYPE_ABS, (void *)dev->evtbits) ){
        for( int i = 0; i < IPGUI_EVT_ABS_CNT; ++ i )
        {
            if( test_bit(i, (void *)dev->absbits) )
                event_num += 3;
        }
    }

    if( test_bit(IPGUI_EVT_TYPE_REL, (void *)dev->evtbits) ){
        for( int i = 0; i < IPGUI_EVT_REL_CNT; ++ i )
        {
            if( test_bit(i, (void *)dev->relbits) )
                event_num += 3;
        }
    }

    dev->evt_nums = 0;
    dev->max_evt_nums = event_num;
	dev->evts = ipgui_mem_alloc(ipgui_smem, dev->max_evt_nums * sizeof(*dev->evts));

    if( !dev->evts ){
        return IPGUI_ERR_NOMEM;
    }

    // list_for_each_safe(pos, temp, &g_input_event_handler_list)
    list_for_each_safe(pos, temp, handler_list)
    {
        handler = list_entry(pos, ipgui_input_handler_t, node);
        ipgui_input_attach_handler(dev, handler);
    }

    return IPGUI_ERR_OK;
}

static unsigned int ipgui_input_to_handler(ipgui_input_handle_t * handle, ipgui_input_event_t * evts, unsigned int count)
{
    ipgui_input_handler_t * handler = handle->handler;
	ipgui_input_event_t * end = evts;
	ipgui_input_event_t * e;

	if (handler->filter)
    {
		for (e = evts; e != evts + count; e ++)
        {
            /* user can filter some event */
            if(handler->filter(handle, e->type, e->code, e->value))
                continue;
                
			if (end != e)
				* end = * e;
			end ++;
		}
		count = end - evts;
	}

	if ( !count )
		return 0;

	if (handler->events)
    {
		handler->events(handle, evts, count);
    }
	else if (handler->event)
    {
		for (e = evts; e != evts + count; e ++)
        {
			handler->event(handle, e->type, e->code, e->value);
        }
    }
    
	return count;
}

__IPGUI_STATIC__ void ipgui_input_pass_events(ipgui_input_dev_t * dev, ipgui_input_event_t * evts, unsigned int count)
{
    ipgui_input_handle_t * handle;
    ipgui_node_t * pos, * temp;
    list_for_each_safe(pos, temp, &dev->handle_list)
    {
        handle = list_entry(pos, ipgui_input_handle_t, dev_node);
        ipgui_input_to_handler(handle, evts, count);
    }
}

#define INPUT_IGNORE_EVENT	    0
#define INPUT_PASS_TO_HANDLERS	1
#define INPUT_PASS_TO_DEVICE	2
#define INPUT_SLOT		        4
#define INPUT_FLUSH		        8
#define INPUT_PASS_TO_ALL	(INPUT_PASS_TO_HANDLERS | INPUT_PASS_TO_DEVICE)

/* is event supported */
__IPGUI_STATIC__ __IPGUI_INLINE__ int ipgui_is_event_supported(unsigned int type_or_code, void * bm, unsigned int max)
{
    if(test_bit((int)type_or_code, bm ) && (max >= type_or_code))
        return 1;
    return 0;
}

/* 这里是真正传递事件的地方 */
__IPGUI_STATIC__ ipgui_err_t ipgui_input_handle_event(ipgui_input_dev_t * dev, unsigned int type, unsigned int code, int value)
{
    ipgui_input_event_t * evt;

    int disposition = INPUT_IGNORE_EVENT;

    switch (type) 
    {
        case IPGUI_EVT_TYPE_SYNC:
            switch (code) 
            {
                case IPGUI_EVT_SYN_REPORT:
                    disposition = INPUT_PASS_TO_HANDLERS | INPUT_FLUSH;
                    break;
                case IPGUI_EVT_SYN_MT_REPORT:
                    disposition = INPUT_PASS_TO_HANDLERS;
                    break;
                default:
                    break;
            }
            break;

        case IPGUI_EVT_TYPE_KEY:
            if (ipgui_is_event_supported(code, dev->keybits, IPGUI_EVT_KEY_MAX))
                disposition = INPUT_PASS_TO_HANDLERS;

            break;

        case IPGUI_EVT_TYPE_REL:
            if (ipgui_is_event_supported(code, dev->relbits, IPGUI_EVT_REL_MAX))
                disposition = INPUT_PASS_TO_HANDLERS;
            break;

        case IPGUI_EVT_TYPE_ABS:
            if (ipgui_is_event_supported(code, dev->absbits, IPGUI_EVT_ABS_MAX))
                // disposition = input_handle_abs_event(dev, code, &value);
                disposition = INPUT_PASS_TO_HANDLERS;
            break;

        case IPGUI_EVT_TYPE_MISC:
            if (ipgui_is_event_supported(code, dev->miscbits, IPGUI_EVT_MISC_MAX))
                disposition = INPUT_PASS_TO_ALL;
            break;

        case IPGUI_EVT_TYPE_SW:
            if (ipgui_is_event_supported(code, dev->swbits, IPGUI_EVT_SW_MAX))
                disposition = INPUT_PASS_TO_HANDLERS;
            break;

        default:
            break;
	}

    /* record except flush event */
	if (disposition & INPUT_PASS_TO_HANDLERS) {

		if (disposition & INPUT_SLOT) {
		    evt = &dev->evts[dev->evt_nums++];
			evt->type = IPGUI_EVT_TYPE_ABS;
			evt->code = IPGUI_EVT_ABS_MT;
			evt->value = dev->mt->slot;
		}

		evt = &dev->evts[dev->evt_nums++];
		evt->type = type;
		evt->code = code;
		evt->value = value;
	}

    /* report */
    if(disposition & INPUT_FLUSH){
		if (dev->evt_nums >= 2)
			ipgui_input_pass_events(dev, dev->evts, dev->evt_nums);
		dev->evt_nums = 0;
	} 
    else if(dev->evt_nums >= dev->max_evt_nums - 2){
		dev->evts[dev->evt_nums++] = input_value_sync;
		ipgui_input_pass_events(dev, dev->evts, dev->evt_nums);
		dev->evt_nums = 0;
	}

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_input_event(ipgui_input_dev_t * dev, unsigned int type, unsigned int code, int value)
{
    /* if device support this type event */
    if( ipgui_is_event_supported(type, (void *)dev->evtbits, IPGUI_EVT_TYPE_MAX) )
    {
        return ipgui_input_handle_event(dev, type, code, value);
    }
    return IPGUI_ERR_EVT_NOT_SUPPORTED;
}

/* 下面7个API在输入设备驱动中调用 */
__IPGUI_API__ ipgui_err_t ipgui_input_report_sync(ipgui_input_dev_t * dev, unsigned int code, int value)
{
    return  ipgui_input_event(dev, IPGUI_EVT_TYPE_SYNC, code, value);
}

__IPGUI_API__ ipgui_err_t ipgui_input_report_key(ipgui_input_dev_t * dev, unsigned int code, int value)
{
    return  ipgui_input_event(dev, IPGUI_EVT_TYPE_KEY, code, value);
}

__IPGUI_API__ ipgui_err_t ipgui_input_report_rel(ipgui_input_dev_t * dev, unsigned int code, int value)
{
    return  ipgui_input_event(dev, IPGUI_EVT_TYPE_REL, code, value);
}

 __IPGUI_API__ ipgui_err_t ipgui_input_report_abs(ipgui_input_dev_t * dev, unsigned int code, int value)
{
    return  ipgui_input_event(dev, IPGUI_EVT_TYPE_ABS, code, value);
}

__IPGUI_API__ ipgui_err_t ipgui_input_report_misc(ipgui_input_dev_t * dev, unsigned int code, int value)
{
    return  ipgui_input_event(dev, IPGUI_EVT_TYPE_MISC, code, value);
}

__IPGUI_API__ ipgui_err_t ipgui_input_report_sw(ipgui_input_dev_t * dev, unsigned int code, int value)
{
    return  ipgui_input_event(dev, IPGUI_EVT_TYPE_SW, code, value);
}

__IPGUI_API__ ipgui_err_t ipgui_input_sync(ipgui_input_dev_t * dev)
{
    return  ipgui_input_event(dev, IPGUI_EVT_TYPE_SYNC, IPGUI_EVT_SYN_REPORT, 1);
}

__IPGUI_API__ ipgui_err_t ipgui_input_report_composite(ipgui_input_dev_t * dev, ipgui_input_event_t * evts, unsigned int count)
{
    ipgui_input_event_t * evt;

    while( count-- )
    {
        evt = evts ++;
        if(IPGUI_ERR_EVT_NOT_SUPPORTED == ipgui_input_event(dev, evt->type, evt->code, evt->value))
            return IPGUI_ERR_EVT_NOT_SUPPORTED;
    }
    ipgui_input_sync(dev);
    return IPGUI_ERR_OK;
}

__IPGUI_STATIC__ __IPGUI_INLINE__ ipgui_err_t ipgui_unregister_handle(ipgui_input_handle_t * handle)
{
    list_del(&handle->dev_node);
    list_del(&handle->handler_node);
    ipgui_mem_free(ipgui_smem, (void *)handle);

    return IPGUI_ERR_OK;
}

/* unregister device */
__IPGUI_API__ ipgui_err_t ipgui_input_unregister_device(ipgui_input_dev_t * dev)
{
    ipgui_node_t * pos, * temp;
    //ipgui_debug_assert( device_registered( device ), "device not registered" );

    /* flush event buffer */
    ipgui_input_sync(dev);
    list_del(&dev->node);
    list_for_each_safe(pos, temp, &dev->handle_list) {
        ipgui_input_handle_t * handle = list_entry(pos, ipgui_input_handle_t, dev_node);
        ipgui_unregister_handle(handle);
    }

    return IPGUI_ERR_OK;
}

/* unregister device */
__IPGUI_API__ ipgui_err_t ipgui_input_free_device(ipgui_input_dev_t * dev)
{
    ipgui_mem_free(ipgui_smem, (void *)dev->evts);
    ipgui_mem_free(ipgui_smem, (void *)dev);

    return IPGUI_ERR_OK;
}

/* unregister handler */
__IPGUI_API__ ipgui_err_t ipgui_input_unregister_handler(ipgui_input_handler_t * handler)
{
    ipgui_node_t * pos, * temp;
    //ipgui_debug_assert( handler_registered( handler ), "handler not registered" );

    list_for_each_safe(pos, temp, &handler->handle_list) {
        ipgui_input_handle_t * handle = list_entry(pos, ipgui_input_handle_t, handler_node);
        ipgui_unregister_handle(handle);
    }
    list_del(&handler->node);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ void ipgui_register_event_poll(ipgui_input_dev_t * dev, event_poll epoll, void * args)
{
    dev->epoll_args = args;
    dev->epoll = epoll;
}