/*
    Copyright � 2015, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <aros/kernel.h>
#include <aros/symbolsets.h>
#include "kernel_intern.h"

static void bcm2708_init(void)
{
#if (0)
    // TODO:
    // How to identify broadcom IP's?
    // Expose this as a seprate subsystem (like PCI?)
    D(bug("[KRN:BCM2708] Integrated Peripherals -:\n"));
    for (ptr = ARM_PERIIOBASE; ptr < (ARM_PERIIOBASE + ARM_PERIIOSIZE); ptr += ARM_PRIMECELLPERISIZE)
    {
        unsigned int perihreg = (*(volatile unsigned int *)(ptr + 0xFF0) & 0xFF) | (*(volatile unsigned int *)(ptr + 0xFF4) & 0xFF) << 8 | (*(volatile unsigned int *)(ptr + 0xFF8) & 0xFF) << 16 | (*(volatile unsigned int *)(ptr + 0xFFC) & 0xFF) << 24;
        if (perihreg == ARM_PRIMECELLID)
        {
            perihreg = (*(volatile unsigned int *)(ptr + 0xFE0) & 0xFF) | (*(volatile unsigned int *)(ptr + 0xFE4) & 0xFF) << 8 | (*(volatile unsigned int *)(ptr + 0xFE8) & 0xFF) << 16 | (*(volatile unsigned int *)(ptr + 0xFEC) & 0xFF) << 24;
            unsigned int manu = (perihreg & (0x7F << 12)) >> 12;
            unsigned int prod = (perihreg & 0xFFF);
            unsigned int rev = (perihreg & (0xF << 20)) >> 20;
            unsigned int config = (perihreg & (0x7F << 24)) >> 24;
            D(bug("[KRN:BCM2708]           0x%p: manu %x, prod %x, rev %d, config %d\n", ptr, manu, prod, rev, config));
        }
/*        else
        {
            if (perihreg)
            {
                D(bug("[Kernel] PlatformPostInit:           0x%p: PrimeCellID != %08x\n", ptr, perihreg));
            }
        }*/
    }
#endif
}

static void bcm2708_toggle_led(int LED, int state)
{
    if (__arm_periiobase == BCM2836_PERIPHYSBASE)
    {
        int pin = 35;
        IPTR gpiofunc = GPSET1;

        if (LED == ARM_LED_ACTIVITY)
            pin = 47;

        if (state == ARM_LED_ON)
            gpiofunc = GPCLR1;

        *(volatile unsigned int *)gpiofunc = (1 << (pin-32));
    }
    else
    {
        // RasPi 1 only allows us to toggle the activity LED
        if (state)
            *(volatile unsigned int *)GPCLR0 = (1 << 16);
        else
            *(volatile unsigned int *)GPSET0 = (1 << 16);
    }
}

static void bcm2708_gputimer_handler(unsigned int timerno, void *unused1)
{
    unsigned int stc, cs;

    DIRQ(bug("[KRN:BCM2708] %s(%d)\n", __PRETTY_FUNCTION__, timerno));

    /* Aknowledge our timer interrupt */
    cs = *((volatile unsigned int *)(SYSTIMER_CS));
    cs &= ~ (1 << timerno);
    *((volatile unsigned int *)(SYSTIMER_CS)) = cs;

    /* Signal the Exec VBlankServer */
    if (SysBase && (SysBase->IDNestCnt < 0)) {
        core_Cause(INTB_VERTB, 1L << INTB_VERTB);
    }

    /* Refresh our timer interrupt */
    stc = *((volatile unsigned int *)(SYSTIMER_CLO));
    stc += VBLANK_INTERVAL;
    *((volatile unsigned int *)(SYSTIMER_CS)) = cs | (1 << timerno);
    *((volatile unsigned int *)(SYSTIMER_C0 + (timerno * 4))) = stc;

    DIRQ(bug("[BCM2708] %s: Done..\n", __PRETTY_FUNCTION__));
}

static bcm2708_init_gputimer(struct KernelBase *KernelBase)
{
    struct IntrNode *GPUTimerHandle;
    unsigned int stc;

    D(bug("[KRN:BCM2708] %s(%012p)\n", __PRETTY_FUNCTION__, KernelBase));

    if ((GPUTimerHandle = AllocMem(sizeof(struct IntrNode), MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
    {
        D(bug("[KRN:BCM2708] %s: IntrNode @ 0x%p:\n", __PRETTY_FUNCTION__, GPUTimerHandle));
        D(bug("[KRN:BCM2708] %s: Using GPUTimer %d for VBlank\n", __PRETTY_FUNCTION__, VBLANK_TIMER));

        GPUTimerHandle->in_Handler = bcm2708_gputimer_handler;
        GPUTimerHandle->in_HandlerData = VBLANK_TIMER;
        GPUTimerHandle->in_HandlerData2 = KernelBase;
        GPUTimerHandle->in_type = it_interrupt;
        GPUTimerHandle->in_nr = IRQ_TIMER0 + VBLANK_TIMER;

        ADDHEAD(&KernelBase->kb_Interrupts[IRQ_TIMER0 + VBLANK_TIMER], &GPUTimerHandle->in_Node);

        D(bug("[KRN:BCM2708] %s: Enabling Hardware IRQ.. \n", __PRETTY_FUNCTION__));

        stc = *((volatile unsigned int *)(SYSTIMER_CLO));
        stc += VBLANK_INTERVAL;
        *((volatile unsigned int *)(SYSTIMER_CS)) = (1 << VBLANK_TIMER);
        *((volatile unsigned int *)(SYSTIMER_C0 + (VBLANK_TIMER * 4))) = stc;

        ictl_enable_irq(IRQ_TIMER0 + VBLANK_TIMER, KernelBase);
    }

    D(bug("[KRN:BCM2708] %s: Done.. \n", __PRETTY_FUNCTION__));

    return GPUTimerHandle;
}


static IPTR bcm2708_probe(struct ARM_Implementation *krnARMImpl, struct TagItem *msg)
{
    //TODO: really detect if we are running on a broadcom 2835/2836
    if (krnARMImpl->ARMI_Family == 7) /*  bcm2836 uses armv7 */
        __arm_periiobase = BCM2836_PERIPHYSBASE;
    else
        __arm_periiobase = BCM2835_PERIPHYSBASE;

    krnARMImpl->ARMI_InitTimer = &bcm2708_init_gputimer;
    krnARMImpl->ARMI_LED_Toggle = &bcm2708_toggle_led;

    return TRUE;
}

ADD2SET(bcm2708_probe, ARMPLATFORMS, 0);
