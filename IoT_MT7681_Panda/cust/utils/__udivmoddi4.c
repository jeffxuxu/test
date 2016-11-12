#include "iot_api.h"
#include "types.h"


UINT64 __udivmoddi4(UINT64 num, UINT64 den, UINT64 * rem_p)
{
	UINT64 quot = 0, qbit = 1;

	if (den == 0) {
		return 0;	/* If trap returns... */
	}

	/* Left-justify denominator and count shift */
	while ((INT64) den >= 0) {
		den <<= 1;
		qbit <<= 1;
	}

	while (qbit) {
		if (den <= num) {
			num -= den;
			quot += qbit;
		}
		den >>= 1;
		qbit >>= 1;
	}

	if (rem_p)
		*rem_p = num;

	return quot;
}
