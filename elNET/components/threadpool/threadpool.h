#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "plat.h"
#include "queue.h"
typedef struct _threadpool_context_st threadpool_t;

/* define thread num */
#define THREAD_NUM_IN_POOL 10

/* execute callback */
typedef int (*thread_exec_t)(threadpool_t *, void * para);

typedef struct usr_exec_context_st
{
    struct list_head node;
    thread_exec_t exec;
    void * args;
}uexec_t;

typedef struct
{
    sys_thread_t pthread;
    threadpool_t * pool;
}thread_t;

typedef struct _threadpool_context_st {
    thread_t ** pthread_ary;
    queue_v2_t task_queue;
}threadpool_t;

#endif