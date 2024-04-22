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
    struct dentry *pdev;
    int devtab;

    // Error check the file descriptor (fd)
    if (isbadfd(fd))
    {
        return SYSERR;
    }

    // Lock the directory block for mutual exclusion
    wait(supertab->sb_dirlock);

    // Error check the directory list
    struct filenode *fnode = &supertab->sb_dirlst->db_fnodes[fd];
    if (NULL == fnode || FILE_USED != fnode->fn_state)
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    // Reset values of the file
    memset(fnode->fn_name, 0, FNAMLEN + 1);
    fnode->fn_state = FILE_FREE;
    fnode->fn_length = 0;
    fnode->fn_cursor = 0;

    // Remove data from hard drive with sbFreeBlock
    if (SYSERR == sbFreeBlock(supertab, fnode->fn_blocknum))
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    // Assuming device entry retrieval and superblock updates are handled correctly
    // You might need to adjust or remove this section if devtab_get causes issues
    pdev = devtab_get(supertab->sb_disk->dvnum);
    if (pdev == NULL)
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    devtab = pdev->dvnum; // Retrieve the device number, assuming pdev points to the correct dentry

    // Write updated superblock back to disk
    if (seek(devtab, SUPERBLOCKNUM) == SYSERR || write(devtab, supertab, sizeof(struct superblock)) == SYSERR)
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    // Signal semaphore to end mutual exclusion
    signal(supertab->sb_dirlock);

    return OK;
}
