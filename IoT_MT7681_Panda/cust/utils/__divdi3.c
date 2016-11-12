/*
 * arch/i386/libgcc/__divdi3.c
 */
#include "iot_api.h"
#include "types.h"

extern UINT64 __udivmoddi4(UINT64 num, UINT64 den, UINT64 * rem);

INT64 __divdi3(INT64 num, INT64 den)
{
	int minus = 0;
	INT64 v;

	if (num < 0) {
		num = -num;
		minus = 1;
	}
	if (den < 0) {
		den = -den;
		minus ^= 1;
	}

	v = __udivmoddi4(num, den, NULL);
	if (minus)
		v = -v;

	return v;
}
