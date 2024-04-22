/* sbFreeBlock.c - sbFreeBlock */
/* Copyright (C) 2008, Marquette University.  All rights reserved. */
/*                                                                 */
/* Modified by                                                     */
/*                                                                 */
/* and                                                             */
/*                                                                 */
/*                                                                 */

#include <kernel.h>
#include <device.h>
#include <memory.h>
#include <disk.h>
#include <file.h>

/*------------------------------------------------------------------------
 * sbFreeBlock - Add a block back into the free list of disk blocks.
 *------------------------------------------------------------------------
 */
devcall sbFreeBlock(struct superblock *psuper, int block)

    // TODO: Add the block back into the filesystem's list of
    //  free blocks.  Use the superblock's locks to guarantee
    //  mutually exclusive access to the free list, and write
    //  the changed free list segment(s) back to disk.
{
    struct freeblock *freeblk;
    struct freeblock *collector;
    int devtab;

    // Error check if superblock is null
    if (NULL == psuper)
    {
        return SYSERR;
    }

    // Error check block number
    if ((block < 0) || (block >= psuper->sb_blocktotal))
    {
        return SYSERR;
    }

    // Lock free list for mutual exclusion
    wait(psuper->sb_freelock);

    // Get the first collector node
    collector = psuper->sb_freelst;

    // Case: if the disk is completely full
    if (0 == collector->fr_count)
    {
        // Create a new collector node
        // (Note: Actual implementation of adding collector nodes is needed)
    }
    else
    {
        // Add to the next available index in collector node
        collector->fr_free[collector->fr_count++] = block;
    }

    // Write this information to the disk
    devtab = ((struct disk *)devtab_get(psuper->sb_disk->dvnum))->disk_blocknum;

    seek(devtab, collector->fr_blocknum);
    write(devtab, collector, sizeof(struct freeblock));

    // Signal semaphore to end mutual exclusion
    signal(psuper->sb_freelock);

    return OK;
}

