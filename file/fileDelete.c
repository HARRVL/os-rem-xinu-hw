/* fileDelete.c - fileDelete */
/* Copyright (C) 2008, Marquette University.  All rights reserved. */
/*                                                                 */
/* Modified by                                                     */
/*                                                                 */
/* and                                                             */
/*                                                                 */
/*                                                                 */

#include <kernel.h>
#include <memory.h>
#include <file.h>
#include <string.h>


/*------------------------------------------------------------------------
 * fileDelete - Delete a file.
 *------------------------------------------------------------------------
 */
// devcall fileDelete(int fd)
// {

    // TODO: Unlink this file from the master directory index,
    //  and return its space to the free disk block list.
    //  Use the superblock's locks to guarantee mutually exclusive
    //  access to the directory index.

extern struct dentry *devtab_get(int devnum);

devcall fileDelete(int fd)
{
    if (isbadfd(fd) || NULL == supertab) return SYSERR;

    struct dentry *diskEntryPtr = supertab->superDisk;
    int diskFileDescriptor = diskEntryPtr - deviceTable;

    wait(supertab->dirLock);

    if (NULL == supertab->dirList) 
    {
        signal(supertab->dirLock);
        return SYSERR;
    }

    if (FILE_FREE == fileTab[fileDescriptor].state) return SYSERR;

    fileTab[fileDescriptor].fileLength = 0;
    fileTab[fileDescriptor].fileName[0] = '\0';
    fileTab[fileDescriptor].state = FILE_FREE;

    if (SYSERR == freeBlock(supertab, fileTab[fd].blockNumber))
    {
        signal(supertab->dirLock);
        return SYSERR;
    }

    if (SYSERR == diskSeek(diskFileDescriptor, supertab->dirList->dirBlockNumber))
    {
        signal(supertab->dirLock);
        return SYSERR;
    }
    if (SYSERR == diskWrite(diskFileDescriptor, supertab->dirList, sizeof(struct dirBlock)))
    {
        signal(supertab->dirLock);
        return SYSERR;
    }

    signal(supertab->dirLock);
    return OK;
}
