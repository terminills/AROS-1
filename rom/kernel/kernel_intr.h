#include <proto/exec.h>

/* Main scheduler entry points */
void core_ExitInterrupt(regs_t *regs);
void core_SysCall(int sc, regs_t *regs);

/* CPU-specific wrappers. Need to be implemented in CPU-specific parts */
void cpu_Switch(regs_t *regs);
void cpu_Dispatch(regs_t *regs);

/* This constant can be redefined this in arch-specific includes */
#ifndef _CUSTOM
#define _CUSTOM NULL
#endif

/* Call exec interrupt vector, if present */
static inline void core_Cause(unsigned char n, unsigned int mask)
{
    struct IntVector *iv = &SysBase->IntVects[n];

    /* If the SoftInt vector in SysBase is set, call it. It will do the rest for us */
    if (iv->iv_Code)
        AROS_UFC5(void, iv->iv_Code,
		  AROS_UFCA(ULONG, mask, D1),
		  AROS_UFCA(APTR, _CUSTOM, A0),
		  AROS_UFCA(APTR, iv->iv_Data, A1),
		  AROS_UFCA(APTR, iv->iv_Code, A5),
		  AROS_UFCA(struct ExecBase *, SysBase, A6));
}

/* Call exec trap handler, if possible */
static inline int core_Trap(ULONG code, void *regs)
{
    /* exec.library Alert() is inoperative without KernelBase */
    if (SysBase && KernelBase)
    {
	void (*trapHandler)(ULONG, void *) = SysBase->TaskTrapCode;
        struct Task *t = SysBase->ThisTask;

        if (t)
	{
	    if (t->tc_TrapCode)
		trapHandler = t->tc_TrapCode;
	}

	if (trapHandler)
	{
	    trapHandler(code, regs);
	    return 1;
	}
    }
    return 0;
}
