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

devcall fileDelete(int fd)
{
    if (isbadfd(fd) || NULL == supertab) return SYSERR;

    struct dentry *deviceEntry = supertab->sb_disk;
    int deviceFileDescriptor = deviceEntry - devtab;

    wait(supertab->sb_dirlock);

    if (NULL == supertab->sb_dirlst) 
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    if (FILE_FREE == filetab[fd].fn_state) return SYSERR;

    filetab[fd].fn_length = 0;
    // filetab[fd].fn_name[0] = '\0';
    filetab[fd].fn_state = FILE_FREE;

    if (SYSERR == sbFreeBlock(supertab, filetab[fd].fn_blocknum))
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    if (SYSERR == seek(deviceFileDescriptor, supertab->sb_dirlst->db_blocknum))
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }
    if (SYSERR == write(deviceFileDescriptor, supertab->sb_dirlst, sizeof(struct dirblock)))
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    signal(supertab->sb_dirlock);
    return OK;
}
