/*
 * WARNING!!!
 * This test relies on some particular internal functioning of kernel memory allocator!
 * You'll have to change the code if you change something in kernel!
 */

#include <aros/kernel.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <proto/kernel.h>

#include <stdio.h>

/* Include private kernel.resource stuff, for DumpState() */
struct KernelBase;
#include "../../rom/kernel/memory_intern.h"

#define PAGES_NUM 14

/* This alignment stuff is copy-pasted from rom/exec/memory.h */
#define RTPOTx4(a)      ((a)>2?4:(a)>1?2:1)

#define RTPOTx10(a)     ((a)>=4?RTPOTx4(((a)+3)/4)*4:RTPOTx4(a))

#define RTPOTx100(a)    \
    ((a)>=0x10?RTPOTx10(((a)+0xf)/0x10)*0x10:RTPOTx10(a))

#define ROUNDUP_TO_POWER_OF_TWO(a)      RTPOTx100(a)

#define MEMCHUNK_TOTAL \
    ROUNDUP_TO_POWER_OF_TWO(AROS_WORSTALIGN>sizeof(struct MemChunk)? \
    AROS_WORSTALIGN:sizeof(struct MemChunk))

#define MEMHEADER_TOTAL \
    ((sizeof(struct MemHeader)+MEMCHUNK_TOTAL-1)&~(MEMCHUNK_TOTAL-1))

/*
 * Print status of all pages in the specified MemHeader.
 * '#' means allocated page, '.' means free page
 */
void DumpState(struct MemHeader *mh)
{
    struct BlockHeader *head = (struct BlockHeader *)mh->mh_First;
    ULONG p;

    printf("Page map (%u total):\n", head->size);

    for (p = 0; p < head->size; p++)
        printf(head->map[p] ? "." : "#");

    printf("\n");
}

int main(void)
{
#ifdef KrnGetSystemAttr

    APTR KernelBase;
    ULONG page;
    struct MemHeader *TestArea;
    ULONG TestLength;
    struct MemChunk *mc;
    APTR region1, region2, region3;

    KernelBase = OpenResource("kernel.resource");
    if (!KernelBase)
    {
	printf("Failed to open kernel.resource!\n");
	return 1;
    }

    page = KrnGetSystemAttr(KATTR_PageSize);
    printf("System page size: %u (0x%08X)\n", page, page);
    if (!page)
    {
	printf("This system does not have page size specified!\n"
	       "kernel.resource memory allocator will not work!\n");
	return 1;
    }

    TestLength = PAGES_NUM * page;
    TestArea = AllocMem(TestLength, MEMF_ANY);
    printf("Allocated test region (%u bytes) at 0x%p\n", TestLength, TestArea);
    if (!TestArea)
    {
	printf("Failed to allocate test region!\n");
	return 1;
    }

    /* Compose a MemHeader */
    TestArea->mh_Node.ln_Succ = NULL;
    TestArea->mh_Node.ln_Type = NT_MEMORY;
    TestArea->mh_Node.ln_Name = "Kernel allocator test area";
    TestArea->mh_Node.ln_Pri  = 127;			/* This MemHeader must be the first in the list, otherwise KrnFreePages() will find a wrong one */
    TestArea->mh_Attributes   = MEMF_CHIP|MEMF_FAST;	/* Prevent normal AllocMem() requests from even looking at this region */
    TestArea->mh_Lower        = TestArea;
    TestArea->mh_Upper        = TestArea->mh_Lower + TestLength - 1;
    TestArea->mh_First        = TestArea->mh_Lower + MEMHEADER_TOTAL;
    TestArea->mh_Free         = TestLength - MEMHEADER_TOTAL;

    mc = TestArea->mh_First;
    mc->mc_Next  = NULL;
    mc->mc_Bytes = TestArea->mh_Free;

    /* Give up the area to kernel allocator */
    KrnInitMemory(TestArea);
    if (mc->mc_Next || mc->mc_Bytes)
    {
	printf("KrnInitMemory() failed:\n"
	       "  mc_Next  is 0x%p\n"
	       "  mc_Bytes is %u\n",
	       mc->mc_Next, mc->mc_Bytes);
	goto exit;
    }

    /*
     * Insert the area into system list.
     * We do it manually because in future AddMemList() will call KrnInitMemory() itself.
     */
    Forbid();
    Enqueue(&SysBase->MemList, &TestArea->mh_Node);
    Permit();

    printf("Allocating region1 (two read-write pages)...\n");
    region1 = KrnAllocPages(2 * page, MEMF_CHIP|MEMF_FAST, MAP_Readable|MAP_Writable);
    printf("region1 at 0x%p\n", region1);
    DumpState(TestArea);

    printf("Freeing region1...\n");
    KrnFreePages(region1, 2 * page);
    printf("Done!\n");
    DumpState(TestArea);

    printf("Allocating region1 (3 read-only pages)...\n");
    region1 = KrnAllocPages(3 * page, MEMF_CHIP|MEMF_FAST, MAP_Readable);
    printf("region1 at 0x%p\n", region1);
    DumpState(TestArea);

    printf("Allocating region2 (4 write-only ;-) pages)...\n");
    region2 = KrnAllocPages(4 * page, MEMF_CHIP|MEMF_FAST, MAP_Writable);
    printf("region2 at 0x%p\n", region2);
    DumpState(TestArea);

    printf("Attempting to allocate MEMF_24BITDMA page...\n");
    region3 = KrnAllocPages(page, MEMF_CHIP|MEMF_FAST|MEMF_24BITDMA, MAP_Readable);
    printf("Region at 0x%p\n", region3);
    if (region3)
    {
	printf("WARNING!!! This should have been NULL!\n");
	KrnFreePages(region3, page);
    }

    printf("Freeing region1...\n");
    KrnFreePages(region1, 3 * page);
    printf("Done!\n");
    DumpState(TestArea);

    printf("Freeing region2...\n");
    KrnFreePages(region2, 4 * page);
    printf("Done!\n");
    DumpState(TestArea);

    printf("Allocating region1 (one read-write page)...\n");
    region1 = KrnAllocPages(page, MEMF_CHIP|MEMF_FAST, MAP_Readable|MAP_Writable);
    printf("region1 at 0x%p\n", region1);
    DumpState(TestArea);
 
    printf("Freeing region1...\n");
    KrnFreePages(region1, page);
    printf("Done!\n");
    DumpState(TestArea);
 
exit:
    if (TestArea->mh_Node.ln_Succ)
    {
	Forbid();
	Remove(&TestArea->mh_Node);
	Permit();
    }
    FreeMem(TestArea, TestLength);

#else
    printf("The test can't be built for this kernel.resource implementation\n");
#endif

    return 0;
}
