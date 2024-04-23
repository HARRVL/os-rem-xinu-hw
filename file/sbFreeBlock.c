/* sbFreeBlock.c - sbFreeBlock */
/* Copyright (C) 2008, Marquette University.  All rights reserved. */
/*                                                                 */
/* Modified by                                                     */
/*                                                                 */
/* and                                                             */
/*                                                                 */
/*                                                                 */

/* sbFreeBlock.c - sbFreeBlock */
#include <xinu.h>
#include <memory.h>
#include <disk.h>
#include <file.h>

/*------------------------------------------------------------------------
 * Swizzle Functions - Ensure the updated structures are written to disk.
 *------------------------------------------------------------------------
 */
static devcall swizzleFreeblockNode(struct dentry *devptr, struct freeblock *node) {
    if (node == NULL || devptr == NULL) {
        return SYSERR;
    }

    int diskfd = devptr - devtab; // Calculate disk file descriptor from device pointer
    // Ensure block number is within valid range if needed

    // Write the updated freeblock node to the disk
    if (seek(diskfd, node->fr_blocknum) == SYSERR || 
        write(diskfd, node, sizeof(struct freeblock)) == SYSERR) {
        return SYSERR;
    }

    return OK;
}

static devcall swizzleSuperblock(struct superblock *psuper) {
    if (psuper == NULL || psuper->sb_disk == NULL) {
        return SYSERR;
    }

    int diskfd = psuper->sb_disk - devtab; // Calculate disk file descriptor from device pointer
    // Ensure block number is within valid range if needed

    // Write the updated superblock to the disk
    if (seek(diskfd, psuper->sb_blocknum) == SYSERR || 
        write(diskfd, psuper, sizeof(struct superblock)) == SYSERR) {
        return SYSERR;
    }

    return OK;
}

/*------------------------------------------------------------------------
 * sbFreeBlock - Add a block back into the free list of disk blocks.
 *------------------------------------------------------------------------
 */

devcall sbFreeBlock(struct superblock *psuper, int block) {
    if (psuper == NULL || block < 0 || block >= psuper->sb_blocktotal) {
        return SYSERR;
    }

    // Acquire semaphore for exclusive access to the free list
    wait(psuper->sb_freelock);

    // Start at the first freeblock node and find the correct position to insert
    struct freeblock *curr = psuper->sb_freelst, *prev = NULL;

    while (curr != NULL && curr->fr_count == FREEBLOCKMAX) {
        prev = curr;
        curr = curr->fr_next;
    }

    if (curr == NULL) { // Need to allocate a new freeblock node
        curr = (struct freeblock *)getmem(sizeof(struct freeblock));
        if ((struct freeblock *)SYSERR == curr) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
        curr->fr_count = 0;
        curr->fr_next = NULL;
        // The new block should be swizzled to an existing empty block or new one
        if (prev != NULL) {
            prev->fr_next = curr; // Link the new block to the previous one
            // Consider swizzling the previous node to update its next pointer
            swizzleFreeblockNode(psuper->sb_disk, prev);
        } else {
            psuper->sb_freelst = curr; // This is the first node in the free list
        }
    }

    // Add the block to the current freeblock node
    curr->fr_free[curr->fr_count++] = block;

    // Swizzle the freeblock node to persist the change
    if (swizzleFreeblockNode(psuper->sb_disk, curr) == SYSERR) {
        signal(psuper->sb_freelock);
        return SYSERR;
    }

    // If this was the first free block added to an empty list, swizzle the superblock too
    if (prev == NULL) {
        if (swizzleSuperblock(psuper) == SYSERR) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
    }

    // Release semaphore
    signal(psuper->sb_freelock);
    return OK;
}
