#ifndef LIGHT_H
#define LIGHT_H

#define LIGHT_OPT_REPORT		0x10000000
#define LIGHT_OPT_SET			0x20000000

#if __PID_LIGHT__	

#define COLOR_TEMP_MIN		2700
#define COLOR_TEMP_MAX		6500
#define COLOR_TEMP_WEIGHT	255
#define BRIGHTNESS_WEIGHT	255
			
#define LIGHT_OPT_POWER			0x00000001
#define LIGHT_OPT_BRIGHTNESS	0x00000002
#define LIGHT_OPT_CTEMP			0x00000004
#define LIGHT_OPT_COLOR			0x00000008
#define LIGHT_OPT_LEARNING		0x00000010
#endif



#endif