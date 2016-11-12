#ifndef __XMEM_2NDLEV_H__
#define __XMEM_2NDLEV_H__
#include "types.h"
#include "stddef.h"
#include "..\cJSON\cJSON.h"


#define XMEM_4BYTES_BLOCK_SIZE	((UINT_32)4)
#define XMEM_8BYTES_BLOCK_SIZE	((UINT_32)8)
#define XMEM_16BYTES_BLOCK_SIZE	((UINT_32)16)

#define XMEM_POOL_4BYTES_LEN		((UINT_32)32)
#define XMEM_POOL_8BYTES_LEN		((UINT_32)64)
#define XMEM_POOL_16BYTES_LEN		((UINT_32)32)

#define XMEM_DEBUG	0
#if XMEM_DEBUG==1
#define XMEM_LOG Printf_High
#else
#define XMEM_LOG(...)
#endif


#define XMEM_POOL_4BYTES_INIT_SIZE (XMEM_4BYTES_BLOCK_SIZE*XMEM_POOL_4BYTES_LEN)
#define XMEM_POOL_8BYTES_INIT_SIZE (XMEM_8BYTES_BLOCK_SIZE*XMEM_POOL_8BYTES_LEN)
#define XMEM_POOL_16BYTES_INIT_SIZE (XMEM_16BYTES_BLOCK_SIZE*XMEM_POOL_16BYTES_LEN)


typedef struct GNU_PACKED t_xMemPool{
	struct t_xMemPool *next;
	void * freeList;
	void * addr;
	uint8  nfree;
	uint8  nblks;
	uint16  blkSize;
}xMemPool;

extern void * xMem2ndLevAlloc(size_t size);
extern uint8 xMem2ndLevFree(void * pblk);
extern void xMem2ndLevInit();

#define XMEM_2NDLEV_ALLOC_FAILED "2nd level memory alloc failed!\n"

#endif
