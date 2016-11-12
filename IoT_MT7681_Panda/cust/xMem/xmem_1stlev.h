#ifndef __XMEM_1STLEV_H__
#define __XMEM_1STLEV_H__
#include "types.h"
#include "stddef.h"

typedef struct GNU_PACKED t_xMemBlock{
	struct t_xMemBlock * next;
	int16 blksize;
	uint8 free;
	uint8 reserve;
}xMemBlock,*pxMemBlock;

#define XMEM_BLOCK_SIZE sizeof(xMemBlock)


#define XMEM_BALLANCE_SIZE	16
#define XMEM_POOL_MIN_SIZE	(1024*3)


#define XMEM_NOTENOUGH_WARNING	"Not Enough Space for Dynamic Memory Allocation\n"
#define XMEM_1STLEV_ALLOC_FAILED "1st level memory alloc failed!\n"


extern void xMem1stLevInit();
extern void * xMem1stLevAlloc(size_t size);
extern uint8 xMem1stLevFree(void *ptr);
#endif
