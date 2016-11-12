#include "types.h"
#include "iot_api.h"
#include "xmem_2ndlev.h"
#include "xmem_1stlev.h"
#include "xmem.h"

#if (ATCMD_RECOVERY_SUPPORT != 1)
static uint8 mem_init_flag=0;
void xMemInit()
{
	xMem1stLevInit();
	xMem2ndLevInit();
	mem_init_flag=1;	
	return;
}

void * malloc(size_t size)
{
	if(!mem_init_flag){
		xMemInit();
	}
 
	if(size<=XMEM_BALLANCE_SIZE){
		return xMem2ndLevAlloc(size);
	}else{
		return xMem1stLevAlloc(size);
	}
}

void free(void *ptr)
{
	if(xMem2ndLevFree(ptr)&&xMem1stLevFree(ptr)){//firest 2ndLev, then 1stLev, avoid 2ndLev MemPool start addr equals 1stLev MemBlock start addr
		IoT_uart_output(XMEM_FREE_FAILED,strlen(XMEM_FREE_FAILED));
	}
	return;
}


#else
void * malloc(size_t size){return NULL;}
void free(void * pblk){}
void xMemInit(){}
#endif

