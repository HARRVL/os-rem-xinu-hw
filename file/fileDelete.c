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

/*------------------------------------------------------------------------
 * fileDelete - Delete a file.
 *------------------------------------------------------------------------
 */
devcall fileDelete(int fd)
{
    struct dentry *phw;
    int devtab;

    // Error check the file descriptor (fd)
    if (isbadfd(fd))
    {
        return SYSERR;
    }

    // Error check superblock
    if (NULL == supertab)
    {
        return SYSERR;
    }

    // Lock the directory block for mutual exclusion
    wait(supertab->sb_dirlock);

    // Error check the directory list
    if (NULL == supertab->sb_dirlst)
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    // Get a pointer to the file's directory entry
    struct filenode *fnode = &supertab->sb_dirlst->db_fnodes[fd];

    // Error check file state
    if (FILE_USED != fnode->fn_state)
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    // Reset values of the file (name, state, length, etc.)
    bzero(fnode->fn_name, FNAMLEN + 1);
    fnode->fn_state = FILE_FREE;
    fnode->fn_length = 0;
    fnode->fn_cursor = 0;

    // Remove data from hard drive with sbFreeBlock and error check
    if (SYSERR == sbFreeBlock(supertab, fnode->fn_blocknum))
    {
        signal(supertab->sb_dirlock);
        return SYSERR;
    }

    // Update hard drive info with seek and write
    phw = supertab->sb_disk->dvnum;
    devtab = ((struct disk *)devtab_get(phw))->disk_blocknum;

    seek(devtab, supertab->sb_blocknum);
    write(devtab, supertab, sizeof(struct superblock));

    // Signal semaphore to end mutual exclusion
    signal(supertab->sb_dirlock);

    return OK;
}
