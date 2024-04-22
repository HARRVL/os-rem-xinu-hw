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

// TODO: Add the block back into the filesystem's list of
    //  free blocks.  Use the superblock's locks to guarantee
    //  mutually exclusive access to the free list, and write
    //  the changed free list segment(s) back to disk. int diskfd struct freeblock * pointer return dev call 

devcall sbFreeBlock(struct superblock *psuper, int block)
{
    struct freeblock *collector, *newnode;

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

    // Check if the current collector node is full
    if (collector->fr_count >= FREEBLOCKMAX)
    {
        // Allocate memory for a new collector node
        newnode = (struct freeblock *)getmem(sizeof(struct freeblock));
        if ((int)newnode == SYSERR)
        {
            signal(psuper->sb_freelock);
            return SYSERR;
        }

        // Initialize the new collector node
        newnode->fr_count = 1; // Since we're adding the first block
        newnode->fr_free[0] = block;
        newnode->fr_next = collector; // Link to the current collector
        psuper->sb_freelst = newnode; // Update the superblock to point to the new node
    }
    else
    {
        // Add to the next available index in the current collector node
        collector->fr_free[collector->fr_count++] = block;
    }

    // Signal semaphore to end mutual exclusion
    signal(psuper->sb_freelock);

    return OK;
}

