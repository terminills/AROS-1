/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Open a drawer or launch a program.
*/

#define DEBUG 0
#include <aros/debug.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <utility/tagitem.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <proto/utility.h>
#include <proto/dos.h>

#include <string.h>

#include "workbench_intern.h"
#include "support.h"
#include "support_messages.h"
#include "handler.h"
#include "handler_support.h"
#include "uae_integration.h"

/*** Prototypes *************************************************************/
static BOOL   CLI_LaunchProgram(CONST_STRPTR command, struct TagItem *tags, struct WorkbenchBase *WorkbenchBase);
static STRPTR CLI_BuildCommandLine(CONST_STRPTR command, struct TagItem *tags, struct WorkbenchBase *WorkbenchBase);
static BOOL   WB_LaunchProgram(BPTR lock, CONST_STRPTR name, struct TagItem *tags, struct WorkbenchBase *WorkbenchBase);
static BOOL   WB_BuildArguments(struct WBStartup *startup, BPTR lock, CONST_STRPTR name, struct TagItem *tags, struct WorkbenchBase *WorkbenchBase);
static void   HandleDrawer(STRPTR name, struct WorkbenchBase *WorkbenchBase);
static BOOL   HandleTool(STRPTR name, LONG isDefaultIcon, struct DiskObject *icon, struct TagItem *tags, struct WorkbenchBase *WorkbenchBase);
static BOOL   HandleProject(STRPTR name, LONG isDefaultIcon, struct DiskObject *icon, struct TagItem *tags, struct WorkbenchBase *WorkbenchBase);

/*****************************************************************************

    NAME */

        #include <proto/workbench.h>

        AROS_LH2(BOOL, OpenWorkbenchObjectA,

/*  SYNOPSIS */
        AROS_LHA(STRPTR,           name, A0),
        AROS_LHA(struct TagItem *, tags, A1),

/*  LOCATION */
        struct WorkbenchBase *, WorkbenchBase, 16, Workbench)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

******************************************************************************/
{
    AROS_LIBFUNC_INIT

    BOOL                  success       = FALSE;
    LONG                  isDefaultIcon = 42;

    struct DiskObject    *icon          = GetIconTags
    (
        name,
        ICONGETA_FailIfUnavailable,        FALSE,
        ICONGETA_IsDefaultIcon,     (IPTR) &isDefaultIcon,
        TAG_DONE
    );

    ASSERT_VALID_PTR(name);
    ASSERT_VALID_PTR_OR_NULL(tags);

    D(bug("[WBLIB] OpenWorkbenchObjectA: name = %s\n", name));
    D(bug("[WBLIB] OpenWorkbenchObjectA: isDefaultIcon = %ld\n", isDefaultIcon));

    if( j_uae_running() && is_68k(name) ) 
    {

        D(bug("[WBLIB] OpenWorkbenchObjectA: forward %s to uae\n", name));

        forward_to_uae(tags, name);
        success=TRUE;
    }
    else 
    {
        if (icon != NULL)
        {
            switch (icon->do_Type)
            {
                case WBDISK:
                case WBDRAWER:
                case WBGARBAGE:
                    HandleDrawer(name, WorkbenchBase);
                    break;

                case WBTOOL:
                    success = HandleTool(name, isDefaultIcon, icon, tags, WorkbenchBase);
                    break;

                case WBPROJECT:
                    success = HandleProject(name, isDefaultIcon, icon, tags, WorkbenchBase);
                    break;
            }

            FreeDiskObject(icon);
        }
        else
        {
            /*
                Getting (a possibly default) icon for the path failed, and
                therefore there is no such file. We need to search the default
                search path for an executable of that name and launch it. This
                only makes sense if the name is *not* an (absolute or relative)
                path: that is, it must not contain any ':' or '/'.
            */

            if (strpbrk(name, "/:") == NULL)
            {
                struct CommandLineInterface *cli = Cli();
                if (cli != NULL)
                {
                    BPTR *paths;          /* Path list */
                    BOOL  running = TRUE;

                    /* Iterate over all paths in the path list */
                    for
                    (
                        paths = (BPTR *) BADDR(cli->cli_CommandDir);
                        running == TRUE && paths != NULL;
                        paths = (BPTR *) BADDR(paths[0]) /* next path */
                    )
                    {
                        BPTR cd   = CurrentDir(paths[1]);
                        BPTR lock = Lock(name, SHARED_LOCK);

                        if (lock != NULL)
                        {
                            success = OpenWorkbenchObjectA(name, tags);
                            running = FALSE;

                            UnLock(lock);
                        }

                        CurrentDir(cd);
                    }
                }
            }
        }
    }

    D(bug("[WBLIB] OpenWorkbenchObjectA: success = %d\n", success));

    return success;

    AROS_LIBFUNC_EXIT
} /* OpenWorkbenchObjectA() */

/****************************************************************************/

static STRPTR CLI_BuildCommandLine
(
    CONST_STRPTR command, struct TagItem *tags,
    struct WorkbenchBase *WorkbenchBase
)
{
    const struct TagItem *tstate   = tags;
    const struct TagItem *tag      = NULL;
    BPTR            lastLock = NULL;
    STRPTR          buffer   = NULL;
    ULONG           length   = strlen(command) + 3 /* NULL + 2 '"' */;

    /*-- Calculate length of resulting string ------------------------------*/
    while ((tag = NextTagItem(&tstate)) != NULL)
    {
        switch (tag->ti_Tag)
        {
            case WBOPENA_ArgLock:
                lastLock = (BPTR) tag->ti_Data;
                break;

            case WBOPENA_ArgName:
                if (lastLock != NULL)
                {
                    BPTR cd   = CurrentDir(lastLock);
                    BPTR lock = Lock((STRPTR) tag->ti_Data, ACCESS_READ);
                    if (lock != NULL)
                    {
                        STRPTR path = AllocateNameFromLock(lock);
                        if (path != NULL)
                        {
                            length += 3 /* space + 2 '"' */ + strlen(path);
                            FreeVec(path);
                        }

                        UnLock(lock);
                    }

                    CurrentDir(cd);
                }
                break;
        }
    }

    /*-- Allocate space for command line string ----------------------------*/
    buffer = AllocVec(length, MEMF_ANY);

    if (buffer != NULL)
    {
        buffer[0] = '\0';

        /*-- Build command line --------------------------------------------*/
        strcat(buffer, "\"");
        strcat(buffer, command);
        strcat(buffer, "\"");

        tstate = tags; lastLock = NULL;
        while ((tag = NextTagItem(&tstate)) != NULL )
        {
            switch (tag->ti_Tag)
            {
                case WBOPENA_ArgLock:
                    lastLock = (BPTR) tag->ti_Data;
                    break;

                case WBOPENA_ArgName:
                    if (lastLock != NULL)
                    {
                        BPTR cd   = CurrentDir(lastLock);
                        BPTR lock = Lock((STRPTR) tag->ti_Data, ACCESS_READ);
                        if (lock != NULL)
                        {
                            STRPTR path = AllocateNameFromLock(lock);
                            if (path != NULL)
                            {
                                strcat(buffer, " \"");
                                strcat(buffer, path);
                                strcat(buffer, "\"");
                                FreeVec(path);
                            }

                            UnLock(lock);
                        }

                        CurrentDir(cd);
                    }
                    break;
            }
        }
    }
    else
    {
        SetIoErr(ERROR_NO_FREE_STORE);
    }

    return buffer;
}

/****************************************************************************/

static BOOL CLI_LaunchProgram
(
    CONST_STRPTR command, struct TagItem *tags,
    struct WorkbenchBase *WorkbenchBase
)
{
    BPTR   input       = NULL;
    STRPTR commandline = NULL;
    IPTR                stacksize = WorkbenchBase->wb_DefaultStackSize;
    LONG                priority = 0;
    struct TagItem      *foundTag = NULL;

    input = Open("CON:////Output Window/CLOSE/AUTO/WAIT", MODE_OLDFILE);
    if (input == NULL) goto error;

    commandline = CLI_BuildCommandLine(command, tags, WorkbenchBase);
    if (commandline == NULL) goto error;

    if (tags)
    {
        foundTag = FindTagItem(NP_StackSize, tags);
        if (foundTag != NULL)
        {
            stacksize = foundTag->ti_Data;
        }

        foundTag = FindTagItem(NP_Priority, tags);
        if (foundTag != NULL)
        {
            priority = foundTag->ti_Data;
        }
    }

    if
    (
        /*
            Launch the program. Note that there is no advantage of doing this
            in the handler, since we need to wait for the return value anyway
            (and thus we cannot cut down the blocking time by returning before
            the program is loaded from disk).
        */

        SystemTags
        (
            commandline,
            SYS_Asynch,          TRUE,
            SYS_Input,    (IPTR) input,
            SYS_Output,   (IPTR) NULL,
            SYS_Error,    (IPTR) NULL,
            NP_StackSize,        stacksize,
            NP_Priority,         priority,
            TAG_DONE
        ) == -1
    )
    {
        goto error;
    }
    FreeVec(commandline);

    return TRUE;

error:
    if (input != NULL) Close(input);
    if (commandline != NULL) FreeVec(commandline);

    return FALSE;
}

/****************************************************************************/

static BOOL WB_BuildArguments
(
    struct WBStartup *startup, BPTR lock, CONST_STRPTR name, struct TagItem *tags,
    struct WorkbenchBase *WorkbenchBase
)
{
    const struct TagItem *tstate   = tags,
                   *tag      = NULL;
    BPTR            lastLock = NULL;
    struct WBArg   *args     = NULL;

    /*-- Calculate the number of arguments ---------------------------------*/
    startup->sm_NumArgs = 1;
    while ((tag = NextTagItem(&tstate)) != NULL)
    {
        switch (tag->ti_Tag)
        {
            case WBOPENA_ArgLock:
                lastLock = (BPTR) tag->ti_Data;
                break;

            case WBOPENA_ArgName:
                /*
                    Filter out args where both lock AND name are NULL, since
                    they are completely worthless to the application.
                */
                if (lastLock != NULL || (STRPTR) tag->ti_Data != NULL)
                {
                    startup->sm_NumArgs++;
                }
                break;
        }
    }

    /*-- Allocate memory for the arguments ---------------------------------*/
    args = AllocMem(sizeof(struct WBArg) * startup->sm_NumArgs, MEMF_ANY | MEMF_CLEAR);
    if (args != NULL)
    {
        LONG i = 0;

        startup->sm_ArgList = args;

        /*-- Build the argument list ---------------------------------------*/
        if
        (
               (args[i].wa_Lock = DupLock(lock)) == NULL
            || (args[i].wa_Name = StrDup(name))  == NULL
        )
        {
            goto error;
        }
        i++;

        tstate = tags; lastLock = NULL;
        while ((tag = NextTagItem(&tstate)) != NULL)
        {
            switch (tag->ti_Tag)
            {
                case WBOPENA_ArgLock:
                    lastLock = (BPTR) tag->ti_Data;
                    break;

                case WBOPENA_ArgName:
                    /*
                        Filter out args where both lock AND name are NULL,
                        since they are completely worthless to the application.
                    */
                    if (lastLock != NULL || (STRPTR) tag->ti_Data != NULL)
                    {
                        STRPTR name = (STRPTR) tag->ti_Data;

                        /* Duplicate the lock */
                        if (lastLock != NULL)
                        {
                            args[i].wa_Lock = DupLock(lastLock);

                            if (args[i].wa_Lock == NULL)
                            {
                                D(bug("[WBLIB] WB_BuildArguments: Failed to duplicate lock!\n"));
                                goto error;
                                break;
                            }
                        }
                        else
                        {
                            args[i].wa_Lock = NULL;
                        }

                        /* Duplicate the name */
                        if (name != NULL)
                        {
                            args[i].wa_Name = StrDup(name);

                            if (args[i].wa_Name == NULL)
                            {
                                D(bug("[WBLIB] WB_BuildArguments: Failed to duplicate string!\n"));
                                goto error;
                                break;
                            }
                        }
                        else
                        {
                            args[i].wa_Name = NULL;
                        }

                        i++;
                    }
                    break;
            }
        }

        return TRUE;
    }
    else
    {
        D(bug("[WBLIB] WB_BuildArguments: Failed to allocate memory for argument array\n"));
    }

error:
    D(bug("[WBLIB] WB_BuildArguments: Freeing resources after error...\n"));
    /* Free allocated resources */
    if (args != NULL)
    {
        int i;

        for (i = 0; i < startup->sm_NumArgs; i++)
        {
            if (args[i].wa_Lock != NULL) UnLock(args[i].wa_Lock);
            if (args[i].wa_Name != NULL) FreeVec(args[i].wa_Name);
        }

        FreeMem(args, sizeof(struct WBArg) * startup->sm_NumArgs);
    }

    startup->sm_NumArgs = 0;
    startup->sm_ArgList = NULL;

    return FALSE;
}

/****************************************************************************/

static BOOL WB_LaunchProgram
(
    BPTR lock, CONST_STRPTR name, struct TagItem *tags,
    struct WorkbenchBase *WorkbenchBase
)
{
    struct WBStartup        *startup = NULL;
    struct WBCommandMessage *message = NULL;
    struct TagItem          *stackTag = NULL;
    struct TagItem          *prioTag = NULL;

    /*-- Allocate memory for messages --------------------------------------*/
    startup = CreateWBS();
    if (startup == NULL)
    {
        D(bug("[WBLIB] WB_LaunchProgram: Failed to allocate memory for startup message\n"));
        SetIoErr(ERROR_NO_FREE_STORE);
        goto error;
    }

    message = CreateWBCM(WBCM_TYPE_LAUNCH);
    if (message == NULL)
    {
        D(bug("[WBLIB] WB_LaunchProgram: Failed to allocate memory for launch message\n"));
        SetIoErr(ERROR_NO_FREE_STORE);
        goto error;
    }

    if (tags)
    {
        stackTag = FindTagItem(NP_StackSize, tags);
        prioTag = FindTagItem(NP_Priority, tags);

        if (stackTag || prioTag)
        {
            message->wbcm_Tags =  AllocateTagItems(3);

            message->wbcm_Tags[0].ti_Tag = stackTag ? stackTag->ti_Tag : TAG_IGNORE;
            message->wbcm_Tags[0].ti_Data = stackTag->ti_Data;

            message->wbcm_Tags[1].ti_Tag = prioTag ? prioTag->ti_Tag : TAG_IGNORE;
            message->wbcm_Tags[1].ti_Data = prioTag->ti_Data;

            message->wbcm_Tags[2].ti_Tag = TAG_DONE;
        }
    }

    /*-- Build the arguments array -----------------------------------------*/
    if (!WB_BuildArguments(startup, lock, name, tags, WorkbenchBase))
    {
        D(bug("[WBLIB] WB_LaunchProgram: Failed to build arguments\n"));
        goto error;
    }

    /*-- Send message to handler -------------------------------------------*/
    /* NOTE: The handler will deallocate all resources of this message! */
    message->wbcm_Data.Launch.Startup = startup;

    PutMsg(&(WorkbenchBase->wb_HandlerPort), (struct Message *) message);

    D(bug("[WBLIB]  WB_LaunchProgram: Success\n"));

    return TRUE;

error:
    if (startup != NULL) DestroyWBS(startup);
    if (message != NULL) DestroyWBCM(message);

    D(bug("[WBLIB] WB_LaunchProgram: Failure\n"));

    return FALSE;
}

/****************************************************************************/

static void HandleDrawer(STRPTR name, struct WorkbenchBase *WorkbenchBase)
{
    /*
        Since it's a directory or volume, tell the Workbench
        Application to open the corresponding drawer.
    */

    D(bug("[WBLIB] OpenWorkbenchObjectA: it's a DISK, DRAWER or GARBAGE\n"));

    struct WBCommandMessage *wbcm     = NULL;
    struct WBHandlerMessage *wbhm     = NULL;
    CONST_STRPTR             namecopy = NULL;

    if
    (
           (wbcm     = CreateWBCM(WBCM_TYPE_RELAY)) != NULL
        && (wbhm     = CreateWBHM(WBHM_TYPE_OPEN))  != NULL
        && (namecopy = StrDup(name))                != NULL

    )
    {
        wbhm->wbhm_Data.Open.Name     = namecopy;
        wbcm->wbcm_Data.Relay.Message = wbhm;

        PutMsg(&(WorkbenchBase->wb_HandlerPort), (struct Message *) wbcm);
    }
    else
    {
        FreeVec((STRPTR)namecopy);
        DestroyWBHM(wbhm);
        DestroyWBCM(wbcm);
    }
}

/****************************************************************************/

static BOOL HandleTool
(
    STRPTR name, LONG isDefaultIcon, struct DiskObject *icon,
    struct TagItem *tags, struct WorkbenchBase *WorkbenchBase
)
{
    /*
        It's an executable. Before I launch it, I must check
        whether it is a Workbench program or a CLI program.
    */

    BOOL success = FALSE;

    D(bug("[WBLIB] OpenWorkbenchObjectA: it's a TOOL\n"));

    if
    (
           !isDefaultIcon
        && FindToolType(icon->do_ToolTypes, "CLI") == NULL
    )
    {
        /* It's a Workbench program */
        BPTR lock = Lock(name, ACCESS_READ);

        D(bug("[WBLIB] OpenWorkbenchObjectA: it's a WB program\n"));

        if (lock != NULL)
        {
            BPTR parent = ParentDir(lock);

            if (parent != NULL)
            {
                IPTR stacksize = icon->do_StackSize;

                if (stacksize < WorkbenchBase->wb_DefaultStackSize)
                    stacksize = WorkbenchBase->wb_DefaultStackSize;

                /* check for TOOLPRI */
                LONG priority = 0;
                STRPTR prio_tt = FindToolType(icon->do_ToolTypes, "TOOLPRI");
                if (prio_tt)
                {
                    StrToLong(prio_tt, &priority);
                    if (priority < -128)
                        priority = -128;
                    if (priority > 127)
                        priority = 127;
                }
                
                D(bug("[WBLIB] OpenWorkbenchObjectA: stack size: %d Bytes, priority %d\n", stacksize, priority));

                struct TagItem wbp_Tags[] =
                {
                    { NP_StackSize,   stacksize },
                    { NP_Priority,    priority  },
                    { TAG_MORE, (IPTR)tags      },
                    { TAG_DONE,       0         }
                };

                if (tags == NULL)
                    wbp_Tags[2].ti_Tag = TAG_IGNORE;

                success = WB_LaunchProgram
                (
                    parent, FilePart(name), wbp_Tags, WorkbenchBase
                );

                if (!success)
                {
                    /*
                        Fallback to launching it as a CLI program.
                        Most likely it will also fail, but we
                        might get lucky.
                    */
                    success = CLI_LaunchProgram(name, wbp_Tags, WorkbenchBase);
                }

                UnLock(parent);
            }

            UnLock(lock);
        }
    }
    else
    {
        /* It's a CLI program */

        D(bug("[WBLIB] OpenWorkbenchObjectA: it's a CLI program\n"));

        success = CLI_LaunchProgram(name, tags, WorkbenchBase);
    }

    return success;
}

/****************************************************************************/

static BOOL HandleProject
(
    STRPTR name, LONG isDefaultIcon, struct DiskObject *icon,
    struct TagItem *tags, struct WorkbenchBase *WorkbenchBase
)
{
    /* It's a project; try to launch it via its default tool. */

    BOOL success = FALSE;

    D(bug("[WBLIB] OpenWorkbenchObjectA: it's a PROJECT\n"));
    D(bug("[WBLIB] OpenWorkbenchObjectA: default tool: %s\n", icon->do_DefaultTool));

    if
    (
           icon->do_DefaultTool != NULL
        && strlen(icon->do_DefaultTool) > 0
    )
    {
        BPTR lock = NULL, parent = NULL;

        lock = Lock(name, ACCESS_READ);
        if (lock != NULL)
            parent = ParentDir(lock);
        if (parent != NULL)
        {
            IPTR stacksize = icon->do_StackSize;

            if (stacksize < WorkbenchBase->wb_DefaultStackSize)
                stacksize = WorkbenchBase->wb_DefaultStackSize;

            /* check for TOOLPRI */
            LONG priority = 0;
            STRPTR prio_tt = FindToolType(icon->do_ToolTypes, "TOOLPRI");
            if (prio_tt)
            {
                StrToLong(prio_tt, &priority);
                if (priority < -128)
                    priority = -128;
                if (priority > 127)
                    priority = 127;
            }
            
            D(bug("[WBLIB] OpenWorkbenchObjectA: stack size: %d Bytes, priority %d\n", stacksize, priority));

            struct TagItem tags2[] =
            {
                { NP_StackSize    ,        stacksize      },
                { NP_Priority     ,        priority       },
                { WBOPENA_ArgLock , (IPTR) parent         },
                { WBOPENA_ArgName , (IPTR) FilePart(name) },
                { TAG_MORE        , (IPTR) tags           },
                { TAG_DONE        ,        0              }
            };

            if (tags == NULL)
                tags2[4].ti_Tag = TAG_IGNORE;
            
            if (FindToolType(icon->do_ToolTypes, "CLI") == NULL)
            {
                BPTR lock2 = NULL, parent2 = NULL;

                lock2 = Lock(icon->do_DefaultTool, ACCESS_READ);
                if (lock2 != NULL)
                    parent2 = ParentDir(lock2);
                if (parent2 != NULL)
                {
                    success = WB_LaunchProgram
                    (
                        parent2, FilePart(icon->do_DefaultTool), tags2, WorkbenchBase
                    );
                }
                UnLock(parent2);
                UnLock(lock2);
            }
            else
            {
                success = CLI_LaunchProgram
                (
                    icon->do_DefaultTool, tags, WorkbenchBase
                );
            }

            UnLock(parent);
            UnLock(lock);
        }
    }

    if (!success)
    {
        // FIXME: open execute command?
    }
    
    return success;
}
