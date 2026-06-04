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

#include "ipgui_timer.h"
#include "ipgui_memory.h"
#include "ipgui_debug.h"
#include "ipgui_time.h"
#include <math.h>

static ipgui_twhl_mngr_t * kmng1 = NULL;

#define IPGUI_MACRO_POWER(x,n) __IPGUI_MACRO_START \
        int temp = x; \
        for( int i = 0; i < n - 1; i ++) \
            x = x * temp; \
        __IPGUI_MACRO_END

/* find the prime plan */
__IPGUI_STATIC__ ipgui_err_t ipgui_timer_prime_slove(unsigned int unSlotPowerN, unsigned int * punPrimeX, unsigned int * punPrimeN)
{
    int x = 2, y, nProduct;
    int nNeedSz, nTemp = 2147483647;
    * punPrimeN = 0xffffffff;

    while (x * x < unSlotPowerN)
    {
        y = (int)ceil(log((double)unSlotPowerN) / log((double)x));
        nProduct = x * y;
#if IPGUI_TMR_WHEEL_ALLOC_MIN
        nNeedSz = IPGUI_TWHL_SZ * y + x * sizeof(struct list_head);
#endif
        if ( 
            (pow(x, y) >= unSlotPowerN)
#if IPGUI_TMR_WHEEL_ALLOC_MIN
            && (nNeedSz < nTemp)
#else
            && ( y < (* punPrimeN) )
#endif
         ) {
            * punPrimeX = x;
            * punPrimeN = y;
#if IPGUI_TMR_WHEEL_ALLOC_MIN
            nTemp = nNeedSz;
#endif
        }
        x++;
    }
    if (* punPrimeN == 0xffffffff)
        return IPGUI_ERR_NOK;

    return IPGUI_ERR_OK;
}

/* create timer wheel manager */
__IPGUI_API__ ipgui_err_t ipgui_timer_manager_create(ipgui_tick_t Preci, ipgui_tick_t MaxTick, ipgui_twhl_mngr_t ** pstMngr)
{
    unsigned int unSlotPowerN, unSlotNum;
    unsigned int unPrimeX= 0, unPrimeN = 0;
    unsigned int unPerWheelSize;
    int nIdx;

    /* 参数检查替换为debug_log */
    if (!Preci || !MaxTick || (MaxTick % Preci))
        return IPGUI_ERR_PARAM;
    //ipgui_debug_assert(!Preci || !MaxTick || (MaxTick % Preci), "err param");

    (* pstMngr)    = (ipgui_twhl_mngr_t *)0;
    unSlotPowerN   = MaxTick / Preci;
    ipgui_timer_prime_slove(unSlotPowerN, &unPrimeX, &unPrimeN);
    unSlotNum      = unPrimeX * unPrimeN;
    unPerWheelSize = IPGUI_TWHL_SZ + unPrimeX * IPGUI_LIST_SIZE;
    
    if (NULL == ((* pstMngr) = (ipgui_twhl_mngr_t *)ipgui_mem_alloc(ipgui_smem, IPGUI_TWHL_MNGR_SZ + unPrimeN * sizeof(ipgui_tmr_whl_t *))))
        goto __return;

    for (nIdx = 0; nIdx < unPrimeN; nIdx ++)
    {
        if (NULL == (((* pstMngr)->pstWhls[nIdx]) = (ipgui_tmr_whl_t *)ipgui_mem_alloc(ipgui_smem, unPerWheelSize)))
        {
            goto __return;
        }

        for (int i = 0; i < unPrimeX; i ++)
            list_head_init(&(((* pstMngr)->pstWhls[nIdx])->stTmrList[i]));
    }

    /* init timer manager 
     * calculate recommended max tick 
     */
    (* pstMngr)->Preci    = Preci;
    (* pstMngr)->ubWhlNum = unPrimeN;
    (* pstMngr)->ubPerWhlSlotNum = unPrimeX;
    IPGUI_MACRO_POWER(unPrimeX, unPrimeN);
    (* pstMngr)->TickMax  = unPrimeX * Preci;
    (* pstMngr)->bValid   = 1;

    return IPGUI_ERR_OK;
__return:
    if (*pstMngr)
        ipgui_mem_free(ipgui_smem, (void *)(* pstMngr));
    for (int i = 0; i < nIdx; i ++)
        ipgui_mem_free(ipgui_smem, (void *)(* pstMngr)->pstWhls[nIdx]);
    return IPGUI_ERR_MEM;
}

/* 如果没有定时器在运行，返回1，否则返回0 */
__IPGUI_STATIC__ int no_timer_is_running(ipgui_twhl_mngr_t * pstMngr)
{
    for (int i = 0; i < pstMngr->ubWhlNum; i ++)
    {
        for (int j = 0; j < pstMngr->ubPerWhlSlotNum; j ++)
            if (!list_empty(&(pstMngr->pstWhls[i]->stTmrList[j])))
                return 0;
    }

    return 1;
}

/* destroy timer wheel manager */
__IPGUI_API__ ipgui_err_t ipgui_timer_manager_destroy(ipgui_twhl_mngr_t ** pstMngr)
{
    (*pstMngr)->bValid = 0;
    for (int i = 0; i < (* pstMngr)->ubWhlNum; i ++)
        ipgui_mem_free(ipgui_smem, (void *)(* pstMngr)->pstWhls[i]);
    ipgui_mem_free(ipgui_smem, (void *)(* pstMngr));
    *pstMngr = NULL;

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_timer_init(ipg_tmr_t * pstTimer, ipgui_tick_t unPeriod, unsigned int unLiveRound, pfCallback_t pfCallback, void * pvPrvdata)
{

    if(!pstTimer || !unLiveRound || !unPeriod)
        return IPGUI_ERR_PARAM;

    pstTimer->unDelay       = 0;
    pstTimer->unPeriod      = unPeriod;
    pstTimer->pfCallback    = pfCallback;
    pstTimer->unLiveRound   = unLiveRound;
    pstTimer->pvPrvdata     = pvPrvdata;
    // pstTimer->unRoundToCall = 0;
    list_head_init(&pstTimer->stLink);

    return IPGUI_ERR_OK;
}

__IPGUI_API__ ipgui_err_t ipgui_timer_create(ipg_tmr_t ** pstTimer, ipgui_tick_t unPeriod, unsigned int unLiveRound, pfCallback_t pfCallback, void * pvPrvdata)
{
    if (!pstTimer || !unLiveRound || !unPeriod)
        return IPGUI_ERR_PARAM;

    if ((*pstTimer = (ipg_tmr_t *)ipgui_mem_alloc(ipgui_smem, sizeof(ipg_tmr_t))) == ((ipg_tmr_t *)0) )
        return IPGUI_ERR_MEM;
    return ipgui_timer_init(* pstTimer, unPeriod, unLiveRound, pfCallback, pvPrvdata);
}

__IPGUI_API__ ipgui_err_t ipgui_timer_destroy(ipg_tmr_t * pstTimer)
{
    if (!pstTimer)
        return IPGUI_ERR_PARAM;
    ipgui_mem_free(ipgui_smem, (void *)pstTimer);
}

/* insert to lower level wheel */
__IPGUI_STATIC__ void ipgui_timer_lock_target(ipgui_twhl_mngr_t * pstMngr, ipg_tmr_t * pstTimer, unsigned int * punSlotIdx, ipgui_tmr_whl_t ** ppstWhl)
{
    int nExpire = pstTimer->Expire;
    int nLockedWhl = 0, nCompensate = 0, nTemp, nLastOff = 1;
    int nTickMax = pstMngr->ubPerWhlSlotNum;
    nExpire /= pstMngr->Preci;/* 先归一化 */

    /* 这里的所有乘/除/取余运算都可以使用查表法优化 */
    while ((nExpire /= pstMngr->ubPerWhlSlotNum) > 0) {
        nLockedWhl ++;
    }

    /* lock wheel first */
    * ppstWhl = pstMngr->pstWhls[nLockedWhl];

    if (nLockedWhl)
    {
        IPGUI_MACRO_POWER(nTickMax, nLockedWhl);
        nTemp = pstMngr->ubPerWhlSlotNum;
        nCompensate += pstMngr->ubPerWhlSlotNum - pstMngr->pstWhls[0]->unCurSlot;

        for (int i = 1; i < nLockedWhl; i++) {
            IPGUI_MACRO_POWER(nTemp, i);
            nCompensate += (pstMngr->ubPerWhlSlotNum - pstMngr->pstWhls[i]->unCurSlot - 1) * nTemp;
        }

        nExpire = pstTimer->Expire - nCompensate;
        while (nExpire /= nTickMax) {
            nLastOff ++;    
        }
        * punSlotIdx = ((* ppstWhl)->unCurSlot + nLastOff) % pstMngr->ubPerWhlSlotNum;
        pstTimer->CurSlotStay = (nCompensate + (nLastOff - 1) * nTickMax) * pstMngr->Preci;
    }
    else {
        nLastOff = pstTimer->Expire / pstMngr->Preci;
        * punSlotIdx = ((* ppstWhl)->unCurSlot + nLastOff) % pstMngr->ubPerWhlSlotNum;
        pstTimer->CurSlotStay = nLastOff * pstMngr->Preci;
    }

    pstTimer->Expire -= pstTimer->CurSlotStay;
}

__IPGUI_API__ ipgui_err_t ipgui_timer_start(ipg_tmr_t * pstTimer, ipgui_twhl_mngr_t * pstMngr, ipgui_tick_t unDelay)
{
    ipgui_tick_t SchdLine;
    ipgui_tmr_whl_t * pstWhl;
    unsigned int unSlotIdx;
    unsigned int unSkipRound;

    /* 参数检查替换为ipgui_debug_assert(...) */
    if ((unDelay > IPGUI_TIME_TICK_MAX) || (unDelay % pstMngr->Preci))
        return IPGUI_ERR_PARAM;
        
    if (pstTimer->unPeriod % pstMngr->Preci)
        return IPGUI_ERR_PARAM;
    //ipgui_debug_assert();

    pstTimer->unDelay = 0;

    /* 参数检查替换为ipgui_debug_assert(...) */
    if ((pstTimer->unDelay += unDelay) > pstMngr->TickMax)
        return IPGUI_ERR_OVERFLOW;

    pstTimer->Expire = unDelay;

    /* add to timer wheel */
    ipgui_timer_lock_target(pstMngr, pstTimer, &unSlotIdx, &pstWhl);
    list_add_tail(&pstTimer->stLink, &pstWhl->stTmrList[unSlotIdx]);

    return IPGUI_ERR_OK;
}

/* can be event api */
__IPGUI_API__ ipgui_err_t ipgui_timer_stop(ipg_tmr_t * pstTimer)
{
    //if(pstTimer->Expire)/* 加上这行会segment fault */
        list_del(&pstTimer->stLink);
    return IPGUI_ERR_OK;
}

/* restart timer */
__IPGUI_API__ ipgui_err_t ipgui_timer_restart(ipg_tmr_t * pstTimer, ipgui_twhl_mngr_t * pstMngr)
{
    ipgui_timer_stop(pstTimer);
    ipgui_timer_start(pstTimer, pstMngr, pstTimer->unPeriod);
    return IPGUI_ERR_OK;
}

/* reload timer */
__IPGUI_STATIC__ __IPGUI_INLINE__ ipgui_err_t ipgui_timer_reload(ipg_tmr_t * pstTimer, ipgui_twhl_mngr_t * pstMngr)
{
    if (pstTimer->unLiveRound == IPGUI_TIME_FOREVER)
    {
        ipgui_timer_stop(pstTimer);
        return ipgui_timer_start(pstTimer, pstMngr, pstTimer->unPeriod);
    }
    else if ((-- pstTimer->unLiveRound) > 0)
    {
        ipgui_timer_stop(pstTimer);
        return ipgui_timer_start(pstTimer, pstMngr, pstTimer->unPeriod);
    }
    else
    {
        ipgui_timer_stop(pstTimer);
        return IPGUI_ERR_OK;
    }
}

#if IPGUI_TIMER_ALLOW_SUSPEND == 1
/* 定时器的暂停功能暂时没用到 */
__IPGUI_API__ ipgui_err_t ipgui_timer_set_suspend(ipg_tmr_t * pstTimer, int suspend)
{
    pstTimer->suspend = !!suspend;
}
#endif

/* get next timeout tick */
__IPGUI_API__ ipgui_tick_t ipgui_next_timeout(ipgui_twhl_mngr_t * pstMngr)
{
    ipgui_list_t * pstIndex;

    for (int i = 0; i < pstMngr->ubWhlNum; ++ i) {

    }

    return (ipgui_tick_t)0;
}

/* this api must be called in a loop */
__IPGUI_API__ ipgui_err_t ipgui_timer_loop(ipgui_twhl_mngr_t * pstMngr, ipgui_tick_t unPassTick)
{
    ipg_tmr_t * pstTimer;
    struct list_head * pstLinkBuck;
    ipgui_tmr_whl_t * pstWhl;
    int nWhlIdx = -1;
    unsigned int unCurSlot;
    int loop = 0;

    //ipgui_debug_assert(pstMngr->Preci == unPassTick, "unPassTick is not equal with Preci");

    while (loop ++ < pstMngr->ubWhlNum)
    {
        pstWhl = pstMngr->pstWhls[++ nWhlIdx];
        unCurSlot = (++ pstWhl->unCurSlot) % pstMngr->ubPerWhlSlotNum;
        pstLinkBuck = &pstWhl->stTmrList[unCurSlot];
        pstWhl->unCurSlot = unCurSlot;

        if (list_empty(pstLinkBuck) && unCurSlot)
            break;

        while (!list_empty_careful(pstLinkBuck))
        {
            pstTimer = list_entry(pstLinkBuck->next, ipg_tmr_t, stLink);
            if (pstTimer->pfCallback && !nWhlIdx)
            {
                pstTimer->pfCallback( pstTimer, pstTimer->pvPrvdata );
                ipgui_timer_reload(pstTimer, pstMngr);
            }
            else {
                list_del(pstLinkBuck->next);
                ipgui_timer_start(pstTimer, pstMngr, pstTimer->Expire);
            }
        }
        if (unCurSlot)
            break;
    }

    return IPGUI_ERR_OK;
}

//????
__IPGUI_API__ void ipgui_timer_handler(ipgui_twhl_mngr_t * pstMngr)
{
    ipgui_timer_loop(pstMngr, ipgui_tick_passed_last());
}


__IPGUI_INIT__ ipgui_err_t ipgui_timer_moudle_init(void)
{
    return ipgui_timer_manager_create((ipgui_tick_t)1, (ipgui_tick_t)1000, &kmng1);
}

__IPGUI_API__ ipgui_err_t ipgui_timer_start_def(ipg_tmr_t * pstTimer, ipgui_tick_t unDelay)
{
    return ipgui_timer_start(pstTimer, kmng1, unDelay);
}

__IPGUI_API__ void ipgui_loop_def(ipgui_tick_t unPassTick)
{
    if( kmng1 && kmng1->bValid )
    {
        ipgui_timer_loop(kmng1, unPassTick);
    }
    else
    {
        ipgui_dbg_warning("timer manager is not valid!\r\n");
    }
}
