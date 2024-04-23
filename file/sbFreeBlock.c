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

    // TODO:
    // case 1: when the block being free is the only free block in the system, there does nto exist any collextor node
    // case 2: when the collector node that is last in the list is full, designate the newly freed block as a collector node
    // case 3: when exsting free block has room in it and just add the block that is being freed to the end of the tracking array

    // swizzle in all three cases, swizzle a super block only in case 1

/*------------------------------------------------------------------------
 * Function to swizzle a freeblock node to disk.
 *------------------------------------------------------------------------
 */
devcall swizzleFreeblockNode(struct dentry *devptr, struct freeblock *node) {
    int diskfd = devptr - devtab; // Calculate disk file descriptor from device pointer
    int blocknum = node->fr_blocknum; // Get block number for the free list node

    // Write the updated freeblock node to the disk
    seek(diskfd, blocknum);
    return write(diskfd, node, sizeof(struct freeblock));
}

/*------------------------------------------------------------------------
 * Function to swizzle the superblock to disk.
 *------------------------------------------------------------------------
 */
devcall swizzleSuperblock(struct dentry *devptr, struct superblock *sb) {
    int diskfd = devptr - devtab; // Calculate disk file descriptor from device pointer
    int blocknum = sb->sb_blocknum; // Superblock's disk block number

    // Write the updated superblock to the disk
    seek(diskfd, blocknum);
    return write(diskfd, sb, sizeof(struct superblock));
}

/*------------------------------------------------------------------------
 * sbFreeBlock - Add a block back into the free list of disk blocks.
 *------------------------------------------------------------------------
 */
devcall sbFreeBlock(struct superblock *psuper, int block) {
    struct freeblock *freeblk;
    struct dirblock *dirblk;
    int diskfd;

    // Error checks
    if (NULL == psuper || block < 0 || block >= psuper->sb_blocktotal) {
        return SYSERR;
    }

    // Acquire semaphore for exclusive access to the free list
    wait(psuper->sb_freelock);

    // Retrieve the first freeblock node
    freeblk = psuper->sb_freelst;

    // Handle the three cases based on the status of the free list
    if (NULL == freeblk) { // Case 1: Free list is empty
        // Create a new freeblock node as it is the first one
        freeblk = (struct freeblock *)getmem(sizeof(struct freeblock));
        if ((struct freeblock *)SYSERR == freeblk) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
        freeblk->fr_blocknum = block;
        freeblk->fr_count = 1;
        freeblk->fr_free[0] = block;
        freeblk->fr_next = NULL;
        psuper->sb_freelst = freeblk;
        // Swizzle the superblock since we're adding the first free block
        swizzleSuperblock(psuper->sb_disk, psuper);
    } else { // Case 2 or 3: Free list is not empty
        // Traverse to the last freeblock node or the one with space available
        while (freeblk->fr_next != NULL && freeblk->fr_count >= FREEBLOCKMAX) {
            freeblk = freeblk->fr_next;
        }
        // If the last node is full, create a new node
        if (freeblk->fr_count >= FREEBLOCKMAX) {
            struct freeblock *newblk = (struct freeblock *)getmem(sizeof(struct freeblock));
            if ((struct freeblock *)SYSERR == newblk) {
                signal(psuper->sb_freelock);
                return SYSERR;
            }
            newblk->fr_blocknum = psuper->sb_blocktotal++; // Use a new block for this node
            newblk->fr_count = 1;
            newblk->fr_free[0] = block;
            newblk->fr_next = NULL;
            freeblk->fr_next = newblk;
            freeblk = newblk;
        } else { // Add to the existing freeblock node
            freeblk->fr_free[freeblk->fr_count++] = block;
        }
        // Swizzle the updated freeblock node
        swizzleFreeblockNode(psuper->sb_disk, freeblk);
    }

    // Release semaphore
    signal(psuper->sb_freelock);

    return OK;
}
