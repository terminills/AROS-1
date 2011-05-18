#undef MDEBUG
#define MDEBUG 1

#include <aros/debug.h>

#define MUNGWALL_HEADER_ID 0x1ADEBCA1

struct MungwallHeader
{   
    struct MinNode  mwh_node;
    ULONG   	    mwh_magicid;	/* ID for validation			     */
    BOOL	    mwh_fault;		/* Fault state indicator		     */
    IPTR   	    mwh_allocsize;	/* Allocation size (requested by the caller) */
    APTR	    mwh_pool;		/* Pool to which the allocation belongs	     */
    const char	   *mwh_AllocFunc;	/* Name of API allocator function	     */
    struct Task	   *mwh_Owner;		/* Allocator task			     */
    APTR	    mwh_Caller;		/* Caller address			     */
};

/*
 * Total size of mungwall header needs to be a multiple of MEMCHUNK_TOTAL
 * for keeping proper alignment.
 */
#define MUNGWALLHEADER_SIZE  (AROS_ROUNDUP2(sizeof(struct MungwallHeader), sizeof(struct MemChunk)))
#define MUNGWALL_BLOCK_SHIFT (MUNGWALLHEADER_SIZE + MUNGWALL_SIZE)
#define MUNGWALL_TOTAL_SIZE  (MUNGWALLHEADER_SIZE + MUNGWALL_SIZE * 2)

APTR MungWall_Build(APTR res, APTR pool, IPTR origSize, ULONG requirements, const char *function, APTR caller, struct ExecBase *SysBase);
APTR MungWall_Check(APTR memoryBlock, IPTR byteSize, const char *function, APTR caller, APTR stack, struct ExecBase *SysBase);
void MungWall_Scan(APTR pool, const char *function, APTR caller, APTR stack, struct ExecBase *SysBase);
