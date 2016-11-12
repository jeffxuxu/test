#ifndef __XMEM_H__
#define __XMEM_H__


extern void * malloc(size_t size);
extern void free(void * pblk);
extern void xMemInit();

#define XMEM_FREE_FAILED		"memory free failed!\n"

#endif

