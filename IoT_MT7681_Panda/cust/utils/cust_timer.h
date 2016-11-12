#ifndef CUST_TIMER_H
#define CUST_TIMER_H
#include "uip_timer.h"

typedef struct t_Timer_Cust {
  int start;
  int interval;
}Timer_Cust;


void cust_timer_set(Timer_Cust *t, int interval) XIP_ATTRIBUTE(".xipsec0");
void cust_timer_reset(Timer_Cust *t) XIP_ATTRIBUTE(".xipsec0");
void cust_timer_restart(Timer_Cust *t) XIP_ATTRIBUTE(".xipsec0");
int cust_timer_expired(Timer_Cust *t) XIP_ATTRIBUTE(".xipsec0");


#endif
