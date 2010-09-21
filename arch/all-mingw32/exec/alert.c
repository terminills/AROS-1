/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id: alert.c 32172 2009-12-25 11:40:20Z sonic $

    Desc: Display an alert.
    Lang: english
*/

#include <aros/debug.h>
#include <exec/alerts.h>
#include <exec/execbase.h>
#include <exec/rawfmt.h>
#include <proto/exec.h>
#include <proto/kernel.h>
#include <string.h>

#include "etask.h"
#include "exec_intern.h"
#include "exec_util.h"

static UBYTE *const fmtstring = "Task 0x%P - %s\n";
static UBYTE *const errstring = "Error %08lx - ";
static UBYTE *const locstring = "PC 0x%P\n";
static UBYTE *const modstring = "Module %s Segment %u %s (0x%P) Offset 0x%P\n";
static UBYTE *const funstring = "Function %s (0x%P) Offset 0x%P\n";

AROS_LH1(void, Alert,
	 AROS_LHA(ULONG, alertNum, D7),
	 struct ExecBase *, SysBase, 18, Exec)
{
    AROS_LIBFUNC_INIT

    struct Task *task = SysBase->ThisTask;

    /* If we are running in user mode we should first try to report a problem using AROS'
       own way to do it */
    if (!KrnIsSuper())
    {
        alertNum = Exec_UserAlert(alertNum, task, SysBase);
	if (!alertNum)
	    return;
    }

    /* User-mode alert routine failed. We allocate the buffer on stack
       only here in order to avoid excessive stack usage. */
    UBYTE buffer[256], *buf;

    buf = Alert_AddString(buffer, Alert_GetTitle(alertNum));
    *buf++ = '\n';
    FormatAlert(buf, alertNum, task, SysBase);

    /* Display an alert using Windows message box. Disable() before this
       in order to prevent accidental shutdown. */
    Disable();
    D(bug("[Alert] Message:\n%s\n", buffer));
    PD(SysBase).MessageBox(NULL, buffer, "AROS guru meditation", 0x10010); /* MB_ICONERROR|MB_SETFOREGROUND */

    if (alertNum & AT_DeadEnd)
    {
	/* Um, we have to do something here in order to prevent the
	   computer from continuing... */
	PD(SysBase).Reboot(TRUE);
	PD(SysBase).ExitProcess(0);
    }
    Enable();

    AROS_LIBFUNC_EXIT
}
