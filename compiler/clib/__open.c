/*
    Copyright 2001 AROS - The Amiga Research OS
    $Id$

    Desc: file descriptors handling internals
    Lang: english
*/

#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/filesystem.h>
#include <aros/symbolsets.h>

#include "__errno.h"


#include "__open.h"

void *__stdfiles[3];

int __numslots = 0;
fdesc **__fd_array = NULL;

fdesc *__getfdesc(register int fd)
{
    return ((__numslots>=fd) && (fd>=0))?__fd_array[fd]:NULL;
}

void __setfdesc(register int fd, fdesc *fdesc)
{
    __fd_array[fd] = fdesc;
}

int __getfirstfd(register int startfd)
{
    for (
	;
	startfd < __numslots && __fd_array[startfd];
	startfd++
    );

    return startfd;
}

int __getfdslot(int wanted_fd)
{
    if (wanted_fd>=__numslots)
    {
        void *tmp;

        tmp = malloc((wanted_fd+1)*sizeof(fdesc *));

	if (!tmp) return -1;

	if (__fd_array)
	{
	    CopyMem(__fd_array, tmp, __numslots*sizeof(fdesc *));
	    free(__fd_array);
     	}

	__fd_array = tmp;

	bzero(__fd_array + __numslots, (wanted_fd - __numslots + 1) * sizeof(fdesc *));
	__numslots = wanted_fd+1;
    }
    else if (wanted_fd<0)
        return -1;
    else if (__fd_array[wanted_fd])
    	close(wanted_fd);

    return wanted_fd;
}

int __open(int wanted_fd, const char *pathname, int flags, int mode)
{
    BPTR fh = NULL, lock = NULL;
    fdesc *currdesc = NULL;
    struct FileInfoBlock *fib = NULL;

    int  openmode;

    /* filter out invalid modes */
    switch (flags & (O_CREAT|O_TRUNC|O_EXCL))
    {
    	case O_EXCL:
    	case O_EXCL|O_TRUNC:
            errno = EINVAL;
            return -1;
    }

    switch (flags & O_ACCMODE)
    {
        case O_RDONLY:
	    openmode = FMF_READ;
	    break;

        case O_WRONLY:
    	    openmode = FMF_WRITE;
	    break;

    	case O_RDWR:
	    openmode = FMF_WRITE | FMF_READ;
	    break;

     	default:
	    errno = EINVAL;
	    return -1;
    }

    if (flags & O_TRUNC) openmode |= FMF_CLEAR;
    if (flags & O_CREAT) openmode |= FMF_CREATE;

    currdesc = malloc(sizeof(fdesc));
    if (!currdesc) goto err;

    wanted_fd = __getfdslot(wanted_fd);
    if (wanted_fd == -1) goto err;

    lock = Lock((char *)pathname, SHARED_LOCK);
    if (!lock)
    {
	if
	(
	    (IoErr() != ERROR_OBJECT_NOT_FOUND) ||
	    /* if the file doesn't exist and the flag O_CREAT is not set return an error*/
	    (IoErr() == ERROR_OBJECT_NOT_FOUND && !(flags & O_CREAT))
        )
	{
	    errno = IoErr2errno(IoErr());
	    goto err;
        }
    }
    else
    {
	fib = AllocDosObject(DOS_FIB, NULL);
	/*if the file exists, but O_EXCL is set, then return an error. */
    	if (flags & O_EXCL)
    	{
	    errno = EEXIST;
	    goto err;
    	}

        if (!fib)
        {
    	    errno = IoErr2errno(IoErr());
	    goto err;
        }

	if (!Examine(lock, fib))
    	{
    	    errno = IoErr2errno(IoErr());
	    goto err;
	}

	#warning implement softlink handling

	/* Check if it's a directory or a softlink.
	 Softlinks are not handled yet, though */
	if (fib->fib_DirEntryType > 0)
	{
	    /* A directory cannot be opened for writing */
	    if (openmode & FMF_WRITE)
	    {
	    	errno = EISDIR;
	        goto err;
            }

	    fh = lock;
            FreeDosObject(DOS_FIB, fib);

	    goto success;
        }
        FreeDosObject(DOS_FIB, fib);
    }

    /* the file exists and it's not a directory or the file doesn't exist */

    if (!(fh = Open ((char *)pathname, openmode)) )
    {
	errno = IoErr2errno (IoErr ());
	goto err;
    }

    if (lock) UnLock(lock);

success:
    currdesc->fh        = fh;
    currdesc->flags     = flags;
    currdesc->opencount = 1;

    __setfdesc(wanted_fd, currdesc);

    return wanted_fd;

err:
    if (fib) FreeDosObject(DOS_FIB, fib);
    if (currdesc) free(currdesc);
    if (fh && fh != lock) Close(fh);
    if (lock) UnLock(lock);

    return -1;
}


#warning perhaps this has to be handled in a different way...
void __init_stdfiles(void)
{
    struct Process *me;
    fdesc *indesc=NULL, *outdesc=NULL, *errdesc=NULL;
    int res = __getfdslot(2);

    if
    (
        res == -1                          ||
	!(indesc  = malloc(sizeof(fdesc))) ||
	!(outdesc = malloc(sizeof(fdesc))) ||
	!(errdesc = malloc(sizeof(fdesc)))
    )
    {
    	SetIoErr(ERROR_NO_FREE_STORE);
    	exit(20);
    }


    me = (struct Process *)FindTask (NULL);
    indesc->fh  = __stdfiles[STDIN_FILENO]  = Input();
    outdesc->fh = __stdfiles[STDOUT_FILENO] = Output();
    errdesc->fh = __stdfiles[STDERR_FILENO] = me->pr_CES ? me->pr_CES : me->pr_COS;

    indesc->flags  = O_RDONLY;
    outdesc->flags = O_WRONLY | O_APPEND;
    errdesc->flags = O_WRONLY | O_APPEND;

    indesc->opencount = outdesc->opencount = errdesc->opencount = 1;

    __fd_array[STDIN_FILENO]  = indesc;
    __fd_array[STDOUT_FILENO] = outdesc;
    __fd_array[STDERR_FILENO] = errdesc;
}


void __exit_stdfiles(void)
{
    int i = __numslots;
    while (i)
    {
	close(--i);
    }
}

ADD2INIT(__init_stdfiles, 2);
ADD2EXIT(__exit_stdfiles, 2);
