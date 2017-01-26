/*
    Copyright � 2017, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
*/

#include <aros/kernel.h>
#include <proto/exec.h>

#include <inttypes.h>
#include <string.h>

#include <kernel_base.h>
#include <kernel_cpu.h>
#include <kernel_debug.h>
#include <kernel_interrupts.h>
#include <kernel_objects.h>

/* We use own implementation of bug(), so we don't need aros/debug.h */
#define D(x) 

/*****************************************************************************

            Register an Interrupt Controller in Kernelbase. Assign an ID (in ln_Type)
            returns -1 on failure.
            
*****************************************************************************/

icintrid_t krnAddInterruptController(struct KernelBase *KernelBase, struct IntrController *intController)
{
    struct IntrController *regContr;
    icid_t icid = 0;
    
    ForeachNode(&KernelBase->kb_ICList, regContr)
    {
        if (!strcmp(intController->ic_Node.ln_Name, regContr->ic_Node.ln_Name))
        {
            /* Already registered, return its ID */
            regContr->ic_Node.ln_Pri++;
            
            D(bug("[Kernel] %s: controller id #%d, use count %d\n", __func__, regContr->ic_Node.ln_Type, regContr->ic_Node.ln_Pri));

            return (icintrid_t)((regContr->ic_Node.ln_Type << 8) | regContr->ic_Node.ln_Pri);
        }
    }
    intController->ic_Node.ln_Pri = 1;                                                                                  /* first user */
    intController->ic_Node.ln_Type = KernelBase->kb_ICTypeBase++;

    if (intController->ic_Register)
        icid = intController->ic_Register(KernelBase);
    else
        icid = intController->ic_Node.ln_Type;

    if (icid == -1)
        return -1;

    AddTail(&KernelBase->kb_ICList, &intController->ic_Node);

    D(bug("[Kernel] %s: new controller id #%d = '%s'\n", __func__, intController->ic_Node.ln_Type, intController->ic_Node.ln_Name));

    return (icintrid_t)((icid << 8) | intController->ic_Node.ln_Pri);
}

/*****************************************************************************/

struct IntrController *krnFindInterruptController(struct KernelBase *KernelBase, ULONG ICType)
{
    struct IntrController *intContr;
    ForeachNode(&KernelBase->kb_ICList, intContr)
    {
        if (intContr->ic_Type == ICType)
        {
            return intContr;
        }
    }
    return NULL;
}

/*****************************************************************************/

BOOL krnInitInterrupt(struct KernelBase *KernelBase, icid_t irq, icid_t icid, icid_t icinstance)
{
    if (KernelBase->kb_Interrupts[irq].lh_Type == KBL_INTERNAL)
    {
        KernelBase->kb_Interrupts[irq].lh_Type = icid;
        KernelBase->kb_Interrupts[irq].l_pad = icinstance;
        return TRUE;
    }
    return FALSE;
}


/*****************************************************************************

            Initialize the registered Interrupt Controllers.
            
*****************************************************************************/

int krnInitInterruptControllers(struct KernelBase *KernelBase)
{
    struct IntrController *regContr;
    int cnt = 0;

    ForeachNode(&KernelBase->kb_ICList, regContr)
    {
        if (regContr->ic_Init)
        {
            if (regContr->ic_Init(KernelBase, regContr->ic_Node.ln_Pri))
            {
                regContr->ic_Flags |= ICF_READY;
                cnt += regContr->ic_Node.ln_Pri;
            }
        }
    }
    return cnt;
}
