#ifndef ipgui_timer_h
#define ipgui_timer_h

#include "ipgui_utils.h"
#include "ipgui_types.h"
#include "ipgui_list.h"
#include "ipgui_defs.h"

IPGUI_HEADER_BEGIN  _______________MARKER_______________

#define IPGUI_TMR_WHEEL_ALLOC_MIN 0
#define IPGUI_TIMER_ALLOW_SUSPEND 1 /* 允许暂停定时器 */

typedef struct ipgui_timer_st ipg_tmr_t;
typedef void (* pfCallback_t)(ipg_tmr_t *, void *);

typedef struct ipgui_timer_st
{
    ipgui_tick_t unPeriod;          /* period of timer */
    unsigned int unLiveRound;       /* keep alive round(-1 : forever) */
    ipgui_tick_t Expire;            /* expire time */
    ipgui_tick_t CurSlotStay;       /* current slot stay time */
    ipgui_tick_t unDelay;           /* delay some ticks to start up */
    pfCallback_t pfCallback;        /* callback when time up */
    void * pvPrvdata;               /* argument for callback */
#if IPGUI_TIMER_ALLOW_SUSPEND == 1
    int suspend;                    /* is suspend? */
#endif
    ipgui_node_t stLink;            /* list to link timer */
}ipg_tmr_t;

typedef struct ipgui_timer_whl_conf_st
{
    unsigned int unSlotNum;         /* wheel number */
    unsigned int unCurSlot;         /* current slot index */
}ipgui_tmr_whl_conf_t;

/* struct of timer wheel */
typedef struct ipgui_tmr_wheeler_st
{
    unsigned int unSlotNum;         /* wheel number */
    unsigned int unCurSlot;         /* current slot index */
    ipgui_list_t stTmrList[];       /* list to link timer */
}ipgui_tmr_whl_t;

typedef struct ipgui_tmr_wheel_manager_st
{
    ipgui_tick_t TickMax;           /* max tick */
    ipgui_tick_t Preci;             /* precision of wheel */
    unsigned char ubWhlNum;         /* wheel number */
    unsigned char ubPerWhlSlotNum;  /* per wheel slot number */
    char bValid;                    /* is using? */
    ipgui_tmr_whl_t * pstWhls[];    /* wheel idx */
}ipgui_twhl_mngr_t;

#define IPGUI_TWHL_MNGR_SZ  sizeof(ipgui_twhl_mngr_t)
#define IPGUI_TWHL_SZ       sizeof(ipgui_tmr_whl_t)

extern __IPGUI_API__ ipgui_err_t ipgui_timer_init(ipg_tmr_t *, ipgui_tick_t, unsigned int, pfCallback_t, void *);
extern __IPGUI_API__ ipgui_err_t ipgui_timer_create(ipg_tmr_t **, ipgui_tick_t, unsigned int, pfCallback_t, void *);
extern __IPGUI_API__ ipgui_err_t ipgui_timer_destroy(ipg_tmr_t *);
extern __IPGUI_API__ ipgui_err_t ipgui_timer_start(ipg_tmr_t *, ipgui_twhl_mngr_t *, ipgui_tick_t);
extern __IPGUI_API__ ipgui_err_t ipgui_timer_restart(ipg_tmr_t *, ipgui_twhl_mngr_t *);
extern __IPGUI_API__ ipgui_err_t ipgui_timer_manager_create(ipgui_tick_t, ipgui_tick_t, ipgui_twhl_mngr_t **);
extern __IPGUI_API__ ipgui_err_t ipgui_timer_manager_destroy(ipgui_twhl_mngr_t **);
extern __IPGUI_API__ ipgui_err_t ipgui_timer_loop(ipgui_twhl_mngr_t *, ipgui_tick_t);
extern __IPGUI_API__ ipgui_err_t ipgui_timer_start_def(ipg_tmr_t *, ipgui_tick_t);
extern __IPGUI_API__ void        ipgui_loop_def(ipgui_tick_t);
IPGUI_HEADER_END    _______________MARKER_______________
#endif