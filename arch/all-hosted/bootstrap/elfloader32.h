#ifndef ELFLOADER_H_
#define ELFLOADER_H_

/*
 Copyright (C) 2006 - 2010 The AROS Development Team. All rights reserved.
 $Id$
 */

typedef int (*kernel_entry_fun_t)(struct TagItem *);

int AddKernelFile(char *name);
void FreeKernelList(void);
int GetKernelSize(size_t *ro_size, size_t *rw_size);
int LoadKernel(void *ptr_ro, void *ptr_rw, struct KernelBSS *tracker, kernel_entry_fun_t *kernel_entry, void **kernel_debug);

#endif /*ELFLOADER_H_*/
