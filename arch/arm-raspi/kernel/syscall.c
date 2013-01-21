/*
    Copyright � 2013, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <inttypes.h>
#include <aros/kernel.h>
#include <aros/libcall.h>
#include <stddef.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/kernel.h>

#include "kernel_cpu.h"
#include "kernel_intern.h"
#include "kernel_scheduler.h"
#include "kernel_intr.h"
#include "kernel_syscall.h"

extern char * __text_start;
extern char * __text_end;

#ifndef _CUSTOM
#define _CUSTOM NULL
#endif

#define DREGS(x)
#define D(x)

/*
    __vectorhand_swi:

    warning : this code will be changes to stoe spsr_svc, and lr_svc onto the system mode stack
    and then switch into that mode - and at the end jump back into svc mode, before returning from the exception

    r0 = passed to c handler, r1/r2 = temp
*/
asm (
    ".set	MODE_SUPERVISOR, 0x13          \n"
    ".set	MODE_SYSTEM, 0x1f              \n"

    ".globl __vectorhand_swi                   \n"
    ".type __vectorhand_swi,%function          \n"
    "__vectorhand_swi:                         \n"
    "           sub     sp, sp, #5*4           \n" // make space to store callers cpsr, pc, lr, sp, and ip
    "           stmfd   sp!, {r0-r11}          \n" // store untouched registers to pass to c handler ..
    "           mov     r0, sp                 \n" // r0 = registers r0-r12 on the stack
    "           mrs     r2, spsr               \n" // store spsr above registers
    "           str     r2, [sp, #16*4]        \n"
    "           str     lr, [sp, #15*4]        \n" // store the lr as the callers pc
    "           str     ip, [sp, #13*4]        \n" // store callers stack pointer ..
    "           ldr     r1, [ip, #-8]          \n" // store lr passed in via the callers stack
    "           str     r1, [sp, #14*4]        \n"
    "           ldr     ip, [ip, #-4]          \n" // store callers ip
    "           str     ip, [sp, #12*4]        \n"
    "           ldr     r1, [sp, #1*4]         \n" // restore r1 ..
    "           ldr     r2, [sp, #2*4]         \n" // .. and r2 ..
    "           mov     fp, #0                 \n" // clear fp(??)
    "           bl      handle_syscall         \n"
    "           ldr     r0, [sp, #13*4]        \n" // get task_sp
    "           ldr     r1, [sp, #12*4]        \n" // get task_ip
    "           str     r1, [r0, #-4]          \n" // push task_lp into task_sp
    "           ldr     r1, [sp, #14*4]        \n" // get task_lr
    "           str     r1, [r0, #-8]          \n" // push task_lr into task_sp
    "           ldr     lr, [sp, #15*4]        \n" // put task_pc into lr
    "           ldr     r2, [sp, #16*4]        \n" // restore task_cpsr
    "           msr     spsr_c, r2             \n"
    "           add     sp, sp, #1*4           \n" // skip r0 (contains our return value = task_sp)
    "           ldmfd   sp!, {r1-r11}          \n" // restore remaining task_registers
    "           add     sp, sp, #5*4           \n" // correct the stack pointer .. 
    "           movs    pc, lr                 \n" // ..and return
);

void handle_syscall(void *regs)
{
    register unsigned int addr;
    register unsigned int swi_no;
    struct ExceptionContext *ctx;
    struct Task *thisTask;

    /* We determine the SWI number by reading in the return address
       from the link register, subtract the instruction from it and
       obtain the value from there.  we also use this to check if
       we have been called from outwith the kernel's code (illegal!)
     */

    addr = ((uint32_t *)regs)[15];
    addr -= 4;
    swi_no = *((unsigned int *)addr) & 0x00ffffff;

    D(bug("[KRN] ## SWI %d @ 0x%p\n", swi_no, addr));

    if (((char*)addr < &__text_start) || ((char*)addr >= &__text_end))
    {
        D(bug("[KRN] ## SWI : ILLEGAL ACCESS!\n"));
        return;
    }
    if (swi_no <= 0x0a || swi_no == 0x100)
    {
        if ((thisTask = SysBase->ThisTask) != NULL)
        {
            D(bug("[KRN] SWI invoked in '%s'", thisTask->tc_Node.ln_Name));
            if ((ctx = thisTask->tc_UnionETask.tc_ETask->et_RegFrame) != NULL)
            {
                int i;
                
                D(bug(", ExceptionContext @ 0x%p", ctx));
                DREGS(bug("\n"));
                for (i = 0; i < 12; i++)
                {
                    ctx->r[i] = ((uint32_t *)regs)[i];
                    DREGS(bug("[KRN]      r%02d: 0x%08x\n", i, ctx->r[i]));
                }
                ctx->ip = ((uint32_t *)regs)[12];
                DREGS(bug("[KRN] (ip) r12: 0x%08x\n", ctx->ip));
                ctx->sp = ((uint32_t *)regs)[13];
                DREGS(bug("[KRN] (sp) r13: 0x%08x\n", ctx->sp));
                ctx->lr = ((uint32_t *)regs)[14];
                DREGS(bug("[KRN] (lr) r14: 0x%08x\n", ctx->lr));
                ctx->pc = ((uint32_t *)regs)[15];
                DREGS(bug("[KRN] (pc) r15: 0x%08x\n", ctx->pc));
                ctx->cpsr = ((uint32_t *)regs)[16];
                DREGS(bug("[KRN]     cpsr: 0x%08x", ctx->cpsr));
                thisTask->tc_SPReg = ctx->sp;
            }
            D(bug("\n"));
        }
    
        switch (swi_no)
        {
            case SC_CLI:
            {
                D(bug("[KRN] ## CLI...\n"));
                if (ctx) ctx->cpsr |= 0x80;
                ((uint32_t *)regs)[16] |= 0x80;
                break;
            }

            case SC_STI:
            {
                D(bug("[KRN] ## STI...\n"));
                if (ctx) ctx->cpsr &= ~0x80;
                ((uint32_t *)regs)[16] &= ~0x80;
                break;
            }

            case SC_SUPERSTATE:
            {
                D(bug("[KRN] ## SUPERSTATE...\n"));
                break;
            }

            case SC_ISSUPERSTATE:
            {
                D(bug("[KRN] ## ISSUPERSTATE...\n"));
                break;
            }

            case SC_RTAS:
            {
                break;
            }

            case SC_INVALIDATED:
            {
                break;
            }

            case SC_REBOOT:
            {
                D(bug("[KRN] ## REBOOT...\n"));
                asm volatile ("mov pc, #0\n"); // Jump to the reset vector..
                break;
            }
            default:
                core_SysCall(swi_no, regs);
                break;
        }
    }
    else
    {
        D(bug("[KRN] ## SWI : ILLEGAL SWI!\n"));
        return;
    }

    D(bug("[KRN] ## SWI returning ..\n"));
}
