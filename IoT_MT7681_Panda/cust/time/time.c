#include <stdio.h>
#include "iot_api.h"
#include "time.h"

TIME_SYNC timeSYNC;

struct _tm * my_localtime_r(const INT64 *srctime, struct _tm *tm_time)
{
	long int n32_Pass4year,n32_hpery;

	// 每个月的天数? 非闰年
	const static char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	// 一年的小时数
	const static int ONE_YEAR_HOURS = 8760; // 365 * 24 (非闰年)

	//计算时差8*60*60 固定北京时间
	long time = *srctime;
	time=time+28800;
	tm_time->tm_isdst=0;
	if(time < 0)
	{
		time = 0;
	}

	//取秒时间
	tm_time->tm_sec=(int)(time % 60);
	time /= 60;
	 
	//取分钟时间
	tm_time->tm_min=(int)(time % 60);
	time /= 60;
	 
	//计算星期
	tm_time->tm_wday=(time/24+4)%7;
	 
	//取过去多少个四年，每四年有 1461*24 小时
	 n32_Pass4year=((unsigned int)time / (1461L * 24L));
	 
	//计算年份
	tm_time->tm_year=(n32_Pass4year << 2)+70;
	 
	//四年中剩下的小时数
	time %= 1461L * 24L;

	//计算在这一年的天数
	tm_time->tm_yday=(time/24)%365;

	//校正闰年影响的年份，计算一年中剩下的小时数
	for (;;)
	{
		//一年的小时数
		n32_hpery = ONE_YEAR_HOURS;

		//判断闰年
		if ((tm_time->tm_year & 3) == 0)
		{
		//是闰年，一年则多24小时，即一天
			n32_hpery += 24;
		}

		if (time < n32_hpery)
		{
			break;
		}

		tm_time->tm_year++;
		time -= n32_hpery;
	}

	//小时数
	tm_time->tm_hour=(int)(time % 24);

	//一年中剩下的天数
	time /= 24;

	//假定为闰年
	time++;

	//校正润年的误差，计算月份，日期
	if ((tm_time->tm_year & 3) == 0)
	{
		if (time > 60)
		{
			time--;
		}
		else
		{
			if (time == 60)
			{
				tm_time->tm_mon = 1;
				tm_time->tm_mday = 29;
				return tm_time;
			}
		}
	}
	//计算月日
	for (tm_time->tm_mon = 0;Days[tm_time->tm_mon] < time;tm_time->tm_mon++)
	{
		time -= Days[tm_time->tm_mon];
	}
	tm_time->tm_mday = (int)(time);
	return tm_time;
}
#if 0
//19:49:59
int main()
{
	struct _tm tm;
	long t = 1421286599;
	my_localtime_r(&t, &tm);
	printf("%d:%d:%d  %d %d %d xingqi:%d\n", tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_wday);
	return 0;
}
#endif