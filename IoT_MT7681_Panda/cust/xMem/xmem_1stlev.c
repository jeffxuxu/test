#include "types.h"
#include "iot_api.h"
#include "xmem_1stlev.h"

#if (ATCMD_RECOVERY_SUPPORT != 1)


static pxMemBlock xMemBlkList;

extern unsigned long _RAM_SIZE;
extern unsigned long _BSS_END;
extern uint32 __OS_Heap_Start;

void xMem1stLevInit()
{
	if((uint32)&_RAM_SIZE-(uint32)&_BSS_END<XMEM_POOL_MIN_SIZE){
		IoT_uart_output(XMEM_NOTENOUGH_WARNING,strlen(XMEM_NOTENOUGH_WARNING));
		while(1);
	}
	//init memory pool 
	__OS_Heap_Start += XMEM_POOL_MIN_SIZE;	
	xMemBlkList=(pxMemBlock)&_BSS_END;
	xMemBlkList->blksize=XMEM_POOL_MIN_SIZE-XMEM_BLOCK_SIZE;
	xMemBlkList->next=NULL;
	xMemBlkList->free=1;
	return;
}


void * xMem1stLevAlloc(size_t size)
{
	pxMemBlock blkprev=NULL,blk=NULL,blknew=NULL,blkalloc=NULL;
	void *ptr=NULL;
	int16 allocsize,remainsize;

	blk=xMemBlkList;
	remainsize=0x7FFF;
	blkalloc=NULL;
	allocsize=size+((size%4)==0?0:(4-size%4));

	while(blk){
		if(blk->free&&blk->blksize>=(allocsize+XMEM_BLOCK_SIZE)&&(blk->blksize-allocsize)<remainsize){
			remainsize=blk->blksize-allocsize;
			blkalloc=blk;
		}
		blkprev=blk;
		blk=blk->next;	
	}

	if(blkalloc){
		if(remainsize!=0x7FFF&&remainsize>=(XMEM_BLOCK_SIZE+XMEM_BALLANCE_SIZE)){
			blknew=(pxMemBlock)((void *)blkalloc+allocsize+XMEM_BLOCK_SIZE);
			blknew->blksize=remainsize-XMEM_BLOCK_SIZE;
			blknew->free=1;
			blknew->next=blkalloc->next;
			blkalloc->next=blknew;
			blkalloc->blksize=allocsize;
		}
		
		blkalloc->free=0;
		ptr=(void*)blkalloc+XMEM_BLOCK_SIZE;			
	}else if((__OS_Heap_Start+XMEM_BLOCK_SIZE+allocsize)<((uint32)&_RAM_SIZE)){
		blkalloc=(pxMemBlock)__OS_Heap_Start;
		__OS_Heap_Start += (XMEM_BLOCK_SIZE+allocsize);			
		blkalloc->blksize=allocsize;
		blkalloc->free=0;
		blkalloc->next=blkprev->next;
		blkprev->next=blkalloc;		
		ptr=(void*)blkalloc+XMEM_BLOCK_SIZE;
	}

	if(ptr==NULL)
		IoT_uart_output(XMEM_1STLEV_ALLOC_FAILED,strlen(XMEM_1STLEV_ALLOC_FAILED));
	return ptr;
}

uint8 xMem1stLevFree(void *ptr){
	pxMemBlock blkpriv=NULL,blk=NULL,blkfree=NULL,blkfreepriv=NULL;
	uint8 merge;

	
	blkfree=xMemBlkList;
	while(blkfree){
		if((void*)blkfree+XMEM_BLOCK_SIZE==ptr){
			blkfree->free=1;
//may move this block of code to memory collection
			if(__OS_Heap_Start>((uint32)&_BSS_END+XMEM_POOL_MIN_SIZE)&&(uint32)(ptr+blkfree->blksize)==__OS_Heap_Start){
				blkfreepriv->next=blkfree->next;
				__OS_Heap_Start-=(blkfree->blksize+XMEM_BLOCK_SIZE);
				blk=xMemBlkList;

				while(blk){
					if(!blk->free){
						blkpriv=blk;
					}
					blk=blk->next;					
				}
				blk=blkpriv->next;
				if((void*)blk>=((uint32)&_BSS_END+XMEM_POOL_MIN_SIZE)){
					blkpriv->next=NULL;
					__OS_Heap_Start=(uint32)blk;
				}
			}else{			
				blk=xMemBlkList;
				while(blk
					&&((((void*)blkfree<((uint32)&_BSS_END+XMEM_POOL_MIN_SIZE))&&((void*)blk<((uint32)&_BSS_END+XMEM_POOL_MIN_SIZE)))||
						(((void*)blkfree>=((uint32)&_BSS_END+XMEM_POOL_MIN_SIZE))&&((void*)blk>=((uint32)&_BSS_END+XMEM_POOL_MIN_SIZE))))
					){
					if(blk->free&&(void*)blk+blk->blksize+XMEM_BLOCK_SIZE==(void *)blkfree){
						blk->blksize+=(blkfree->blksize+XMEM_BLOCK_SIZE);
						if(blkfreepriv) blkfreepriv->next=blkfree->next;
						blkfree=blk;
						continue;
					}else if(blk->free&&(void *)blkfree+blkfree->blksize+XMEM_BLOCK_SIZE==(void *)blk){
						blkfree->blksize+=(blk->blksize+XMEM_BLOCK_SIZE);
						if(blkpriv) blkpriv->next=blk->next;
					}			
					blkpriv=blk;
					blk=blk->next;
				}
			}
//end			
			return 0;
		}
		blkfreepriv=blkfree;
		blkfree=blkfree->next;
	}

	return 1;
}


#if	(ATCMD_DEVICE_INFO_DUMP)
void xMem1stLevInfoDump()
{
	pxMemBlock pmemblk;
	pmemblk=xMemBlkList;
	Printf_High("-----xMem1stLev Info-----\n");	
	while(pmemblk){
		Printf_High("blk:%u,blksize:%d,blknext:%u,free:%d\n",(uint32)pmemblk,pmemblk->blksize,(uint32)pmemblk->next,pmemblk->free);
		pmemblk=pmemblk->next;
	}
	Printf_High("-----xMem1stLev Info-----\n");		
	return;
}
#endif

void xMemLeakCheck()
{
}
#endif
