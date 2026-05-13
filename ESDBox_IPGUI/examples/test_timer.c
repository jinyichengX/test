#include <stdio.h>
#include "ipgui_timer.h"
ipg_tmr_t * tmr1,*tmr2,*tmr3,*tmr4,* tmr5,*tmr6,*tmr7,*tmr8;

unsigned long tick1 = 0,tick2=0;
unsigned long tick3 = 0,tick4=0;
ipgui_twhl_mngr_t *mng1;
void callback(ipg_tmr_t * tmr, void * a)
{
    tick2 = GetTickCount();
    
    printf("timer1 callback,stamp = %d\n",tick2-tick1);
    tick1 = tick2;
}
void callback1(ipg_tmr_t * tmr, void * a)
{
        tick4 = GetTickCount();
    printf("timer2 callback,stamp = %d\n",tick4-tick3);
    tick3 = tick4;
}
int main(void)
{
    ipgui_timer_manager_create(1, 1000, &mng1);

    ipgui_timer_create(&tmr1, 650, 999, callback, (void *)tmr1);
    ipgui_timer_create(&tmr2, 102, 999, callback, (void *)tmr2);
    ipgui_timer_create(&tmr3, 48, 100, callback, (void *)tmr3);
    ipgui_timer_create(&tmr4, 688, 150, callback1, (void *)tmr4);

    ipgui_timer_create(&tmr5, 190, 109, callback, (void *)tmr5);
    ipgui_timer_create(&tmr6, 202, 1046, callback, (void *)tmr6);
    ipgui_timer_create(&tmr7, 23, 89564, callback, (void *)tmr7);
    ipgui_timer_create(&tmr8, 999, 1, callback, (void *)tmr8);

    ipgui_timer_start(tmr1, mng1, 199);
    ipgui_timer_start(tmr2, mng1, 206);
    ipgui_timer_start(tmr3, mng1, 300);
    ipgui_timer_start(tmr4, mng1, 458);
    ipgui_timer_start(tmr5, mng1, 15);
    ipgui_timer_start(tmr6, mng1, 479);
    ipgui_timer_start(tmr7, mng1, 223);
    ipgui_timer_start(tmr8, mng1, 100);
    tick1=GetTickCount();
        tick3 = GetTickCount();

    while(1){
        ipgui_timer_loop(mng1,10);
        Sleep(1);
    }
    return 0;
}