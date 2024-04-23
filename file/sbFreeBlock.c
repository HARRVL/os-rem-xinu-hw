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

devcall sbFreeBlock(struct superblock *filesystem, int blocknum)
{
    struct freeblock *currentBlockList, *newBlockNode;

    if (NULL == filesystem || blocknum < 0 || blocknum >= filesystem->sb_blocktotal)
    {
        return SYSERR;
    }

    wait(filesystem->sb_freelock);

    currentBlockList = filesystem->sb_freelst;

    if (FREEBLOCKMAX == currentBlockList->fr_count)
    {
        newBlockNode = (struct freeblock *)getmem(sizeof(struct freeblock));
        if (SYSERR == (int)newBlockNode)
        {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }

        newBlockNode->fr_count = 1; // Initialize the count to 1 for the new block
        newBlockNode->fr_free[0] = blocknum; // Add the block number to the free list
        newBlockNode->fr_next = currentBlockList; // Link the new node in front
        filesystem->sb_freelst = newBlockNode; // Set the new node as the head of the list
    }
    else
    {
        currentBlockList->fr_free[currentBlockList->fr_count++] = blocknum;
    }

    signal(filesystem->sb_freelock);


    // TODO:
    // case 1: when the block being free is the only free block in the system, there does nto exist any collextor node
    // case 2: when the collector node that is last in the list is full, designate the newly freed block as a collector node
    // case 3: when exsting free block has room in it and just add the block that is being freed to the end of the tracking array

    // swizzle in all three cases, swizzle a super block only in case 1
    


    return OK;
}

