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
{
    struct freeblock *freeblk;
    struct freeblock *collector;
    int diskfd;

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

    // Assuming 'devtab_get' returns a pointer to a 'struct disk' and that it is properly declared.
    // You need to include the header file where 'devtab_get' is declared or define the function if missing.
    struct disk *pdisk = (struct disk *)devtab_get(psuper->sb_disk->dvnum);
    if (NULL == pdisk)
    {
        signal(psuper->sb_freelock);
        return SYSERR;
    }

    // Assuming 'disk_current' or a similar correctly named member holds the current block information.
    diskfd = pdisk->disk_current;

    // Write this information to the disk
    // You need to replace 'seek' and 'write' with the actual function calls to interact with the disk.
    if (SYSERR == seek(diskfd, collector->fr_blocknum))
    {
        signal(psuper->sb_freelock);
        return SYSERR;
    }
    if (SYSERR == write(diskfd, collector, sizeof(struct freeblock)))
    {
        signal(psuper->sb_freelock);
        return SYSERR;
    }

    // Signal semaphore to end mutual exclusion
    signal(psuper->sb_freelock);

    return OK;
}

