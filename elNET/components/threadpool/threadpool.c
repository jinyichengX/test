/*
 * MIT License
 *
 * Copyright (c) 2024 YiChengJin
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

#include "threadpool.h"
#include <stdint.h>

/* execute task here */
void * thread_execute_unit(void * args)
{
    struct list_head * node;
    uexec_t * uexec;
    threadpool_t * threadpool = (threadpool_t *)args;

    while(1)
    {
        node = NULL;
        /* wait for task */
        queuev2_pend(&threadpool->task_queue, node, 0xffffffff);

        if(!node)
            continue;

        /* execute user task */
        uexec = list_entry(node, uexec_t, node);
        uexec->exec(uexec->args);

        free(uexec);
    }

    return NULL;
}

/* init thread pool */
int threadpool_init(threadpool_t * pool, uint16_t thread_num)
{
    int idx;

    if(( !thread_num ) || ( !pool ))
        return -1;

    /* create thread obj pointers */
    if(NULL == (pool->pthread_ary = (thread_t **)malloc(sizeof(thread_t) * thread_num)))
        goto _threadpool_init_fail;

    if(0 != (int)queuev2_init(pool->task_queue))
        goto _threadpool_init_fail;

    /* create threads for serving user task */
    for(idx = 0; idx < thread_num; idx ++){
      pool->pthread_ary[idx] = sys_thread_create(thread_execute_unit, (void *)pool);
      if( NULL == pool->pthread_ary[idx] )
          goto _threadpool_init_fail;

      pool->pthread_ary[idx]->pool = pool;
    }

    return 0;
_threadpool_init_fail:
    if( pool->pthread_ary != NULL )
        free(pool->pthread_ary);
    return -1;
}

/* add task to thread pool */
int threadpool_work_add(threadpool_t * pool, thread_exec_t exec_func, void * args)
{
    uexec_t * exec = (uexec_t *)malloc(sizeof(uexec_t));
    if( exec == NULL )
       return -1; 

    /* init task */
    exec->func = exec_func;
    exec->args = args;

    /* add task to queue */
    queuev2_post(pool->task_queue, &exec->node);

    return 0;
}

/* destroy thread pool */
void threadpool_destroy(threadpool_t *pool)
{
    return;
}
