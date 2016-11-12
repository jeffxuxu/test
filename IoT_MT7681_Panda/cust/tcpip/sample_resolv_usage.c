#include "uip.h"
#include "sample_resolv_usage.h"
#include "resolv.h"
#include "uip_timer.h"

struct sample_state {
	u8_t state;
};

#define SAMPLE_STATE_INIT 1
#define SAMPLE_STATE_DNS_QUERYING 2
#define SAMPLE_STATE_DONE 3

static struct sample_state ss;
//static struct timer dns_timer;

void resolv_found(char *name, u16_t *ipaddr)
{
	if (name == NULL) {
		return;
	}
	ss.state = SAMPLE_STATE_DONE;
#if CFG_SUPPORT_DNS
	udp_client_resolv(ipaddr);
#endif
	return;
}

