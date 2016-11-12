#include <stdio.h>
#include "iot_api.h"
#include "time.h"

TIME_SYNC timeSYNC;

struct _tm * my_localtime_r(const INT64 *srctime, struct _tm *tm_time)
{
	long int n32_Pass4year,n32_hpery;

	// ÿ���µ�����? ������
	const static char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	// һ���Сʱ��
	const static int ONE_YEAR_HOURS = 8760; // 365 * 24 (������)

	//����ʱ��8*60*60 �̶�����ʱ��
	long time = *srctime;
	time=time+28800;
	tm_time->tm_isdst=0;
	if(time < 0)
	{
		time = 0;
	}

	//ȡ��ʱ��
	tm_time->tm_sec=(int)(time % 60);
	time /= 60;
	 
	//ȡ����ʱ��
	tm_time->tm_min=(int)(time % 60);
	time /= 60;
	 
	//��������
	tm_time->tm_wday=(time/24+4)%7;
	 
	//ȡ��ȥ���ٸ����꣬ÿ������ 1461*24 Сʱ
	 n32_Pass4year=((unsigned int)time / (1461L * 24L));
	 
	//�������
	tm_time->tm_year=(n32_Pass4year << 2)+70;
	 
	//������ʣ�µ�Сʱ��
	time %= 1461L * 24L;

	//��������һ�������
	tm_time->tm_yday=(time/24)%365;

	//У������Ӱ�����ݣ�����һ����ʣ�µ�Сʱ��
	for (;;)
	{
		//һ���Сʱ��
		n32_hpery = ONE_YEAR_HOURS;

		//�ж�����
		if ((tm_time->tm_year & 3) == 0)
		{
		//�����꣬һ�����24Сʱ����һ��
			n32_hpery += 24;
		}

		if (time < n32_hpery)
		{
			break;
		}

		tm_time->tm_year++;
		time -= n32_hpery;
	}

	//Сʱ��
	tm_time->tm_hour=(int)(time % 24);

	//һ����ʣ�µ�����
	time /= 24;

	//�ٶ�Ϊ����
	time++;

	//У��������������·ݣ�����
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
	//��������
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