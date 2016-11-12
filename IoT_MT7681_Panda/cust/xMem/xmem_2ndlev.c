#include "types.h"
#include "iot_api.h"
#include "xmem_2ndlev.h"

/*
xMem is only for sta mode
*/
#if (ATCMD_RECOVERY_SUPPORT != 1)

xMemPool xMemPoolList[3]={0};
extern uint32 __OS_Heap_Start;

void xMem2ndLevPoolInit (xMemPool * pmem,void *addr, uint8 nblks, uint16 blksize)
{
    UINT8     *pblk;
    void     **plink;
    UINT32     i;

    if (addr ==NULL||nblks < 2||blksize < sizeof(void *)||pmem == NULL) return;
    plink = (void **)addr;  
    pblk  = (UINT8 *)addr + blksize;
    for (i = 0; i < (nblks - 1); i++) 
 	{
         *plink = (void *)pblk;
         plink  = (void **)pblk;
         pblk   = pblk + blksize;
     }
    *plink  = (void *)0;
    pmem->addr     = addr;
    pmem->freeList = addr;
    pmem->nfree    = nblks;
    pmem->nblks	= nblks;
    pmem->blkSize  = blksize;
	pmem->next=NULL;

	//Printf_High("pool:%u@%u,next:%u@%u,freelist:%u@%u,blkSize:%d@%u,nblks:%d@%u,nfree:%d@%u\n",
		//pmem,&pmem,pmem->next,&pmem->next,pmem->freeList,&pmem->freeList,pmem->blkSize,&pmem->blkSize,pmem->nblks,&pmem->nblks,pmem->nfree,&pmem->nfree);
	return;
}


xMemPool * xMem2ndLevPoolAppend(xMemPool * pmem)
{
	xMemPool * pmemtail,*pmemnew=NULL;
	void *ptr;

	pmemtail=pmem;
	while(pmemtail->next) pmemtail=pmemtail->next;
	pmemnew=(xMemPool *)xMem1stLevAlloc(sizeof(xMemPool));
	if(pmemnew){		
		ptr=xMem1stLevAlloc(pmem->blkSize*(pmem->nblks/2));
		if(ptr){		
			xMem2ndLevPoolInit(pmemnew,ptr,(pmem->nblks/2),pmem->blkSize);
			pmemtail->next=pmemnew;
		}
	}
	return pmemnew;
}

void xMem2ndLevInit()
{
	void * ptr;
	ptr=xMem1stLevAlloc(XMEM_POOL_4BYTES_INIT_SIZE+XMEM_POOL_8BYTES_INIT_SIZE+XMEM_POOL_16BYTES_INIT_SIZE);
	if(ptr){
		xMem2ndLevPoolInit(&xMemPoolList[0],ptr,XMEM_POOL_4BYTES_LEN,XMEM_4BYTES_BLOCK_SIZE);
		xMem2ndLevPoolInit(&xMemPoolList[1],ptr+XMEM_POOL_4BYTES_INIT_SIZE,XMEM_POOL_8BYTES_LEN,XMEM_8BYTES_BLOCK_SIZE);
		xMem2ndLevPoolInit(&xMemPoolList[2],ptr+XMEM_POOL_4BYTES_INIT_SIZE+XMEM_POOL_8BYTES_INIT_SIZE,XMEM_POOL_16BYTES_LEN,XMEM_16BYTES_BLOCK_SIZE);	
	}
}
#if	(ATCMD_DEVICE_INFO_DUMP)
void xMem2ndLevInfoDump()
{
	uint8 i;
	xMemPool * pmem;
	Printf_High("-----xMem2ndLev Info-----\n");
	for(i=0;i<3;i++){
		pmem=&xMemPoolList[i];
		while(pmem){
			Printf_High("size:%d,free:%d,total:%d\n",pmem->blkSize,pmem->nfree,pmem->nblks);
			pmem=pmem->next;
		}
	}
	Printf_High("-----xMem2ndLev Info-----\n");
}
#endif

void * xMallocGet(xMemPool * pmem,size_t size)
{
	void	  *pblk;
	xMemPool *pmemiter;
	
	if (pmem == NULL||size>pmem->blkSize)	return NULL;

	pmemiter=pmem;

	while(pmemiter){
		if (pmemiter->nfree > 0){
			break;
		}
		
		pmemiter=pmemiter->next;
	}
	
	if(pmemiter==NULL)pmemiter=xMem2ndLevPoolAppend(pmem);

	if(pmemiter){
		pblk= pmemiter->freeList;
		pmemiter->freeList = *(void **)pblk;
		pmemiter->nfree--;
		return (pblk); 		
	}
				
	return NULL;   

}


void * xMem2ndLevAlloc(size_t size)
{
	void * ptr;

	if(size>8){
		ptr=xMallocGet(&xMemPoolList[2],size);
	}else if(size>4){
		ptr=xMallocGet(&xMemPoolList[1],size);
	}else{
		ptr=xMallocGet(&xMemPoolList[0],size);
	}

	if(ptr==NULL)
		IoT_uart_output(XMEM_2NDLEV_ALLOC_FAILED,strlen(XMEM_2NDLEV_ALLOC_FAILED));

	return ptr;
}


void  xMallocPut (xMemPool  *pmem, void *pblk)
{
    if (pmem == NULL||pblk == NULL||pmem->nfree >= pmem->nblks)  return ;

    *(void **)pblk = pmem->freeList;
    pmem->freeList = pblk;
    pmem->nfree++;

	return; 
}


uint8 xMem2ndLevFree(void * pblk)
{
	int i;
	UINT32 start,end,p;
	xMemPool  *pmem,*pmemprev=NULL;

	if (pblk == NULL)   return 0;

	for(i=0;i<3;i++){
		pmem=&xMemPoolList[i];
		while(pmem){
			p=(UINT32)pblk;
			start=(UINT32)pmem->addr;
			end=start+pmem->blkSize*pmem->nblks;
			if(p>=start&&p<end){
				xMallocPut(pmem,pblk);
				if(pmem->nfree==pmem->nblks&&pmemprev){
					pmemprev->next=pmem->next;
					xMem1stLevFree(pmem->addr);
					xMem1stLevFree(pmem);
				}
				return 0;
			}
			pmemprev=pmem;
			pmem=pmem->next;		
		}
	}
	return 1;
}


#endif

