#include "iot_api.h"
#include "cust_timer.h"

void cust_timer_set(Timer_Cust *t, int interval)
{
  t->interval = interval;
  t->start = GetMsTimer();
}

void cust_timer_reset(Timer_Cust *t)
{
  t->start += t->interval;
}

void cust_timer_restart(Timer_Cust *t)
{
  t->start = GetMsTimer();
}

int cust_timer_expired(Timer_Cust *t)
{
  return (((int)(GetMsTimer() - t->start) >= (int)t->interval) && (t->start != 0) && (t->interval != 0));
}

