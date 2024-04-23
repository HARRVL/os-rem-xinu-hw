/* sbFreeBlock.c - sbFreeBlock */
/* Copyright (C) 2008, Marquette University. All rights reserved. */

#include <xinu.h>
#include <device.h>
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
    int diskfd = devptr - devtab;
    if (seek(diskfd, node->fr_blocknum) == SYSERR ||
        write(diskfd, node, sizeof(struct freeblock)) == SYSERR) {
        return SYSERR;
    }
    return OK;
}

static devcall swizzleSuperblock(struct superblock *sb) {
    if (sb == NULL || sb->sb_disk == NULL) {
        return SYSERR;
    }
    int diskfd = sb->sb_disk - devtab;
    if (seek(diskfd, sb->sb_blocknum) == SYSERR ||
        write(diskfd, sb, sizeof(struct superblock)) == SYSERR) {
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

    wait(psuper->sb_freelock);

    struct freeblock *curr = psuper->sb_freelst, *prev = NULL;

    // Traverse to find an appropriate node or the end of the list
    while (curr != NULL && curr->fr_count >= FREEBLOCKMAX) {
        prev = curr;
        curr = curr->fr_next;
    }

    if (curr == NULL) { // If no node was found or all are full, create a new one
        curr = (struct freeblock *)getmem(sizeof(struct freeblock));
        if (curr == (struct freeblock *)SYSERR) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
        // Manually initialize the new freeblock node
        curr->fr_blocknum = psuper->sb_blocktotal; // Optionally use a new block number
        curr->fr_count = 1;
        curr->fr_free[0] = block;
        curr->fr_next = NULL;

        if (prev != NULL) {
            prev->fr_next = curr; // Link the new block node
        } else {
            psuper->sb_freelst = curr; // This is the first node
        }
    } else {
        // Add the block to the existing node
        curr->fr_free[curr->fr_count++] = block;
    }

    // Swizzle changes to disk for both the freeblock node and superblock if needed
    if (swizzleFreeblockNode(psuper->sb_disk, curr) == SYSERR ||
        (prev == NULL && swizzleSuperblock(psuper) == SYSERR)) {
        signal(psuper->sb_freelock);
        return SYSERR;
    }

    signal(psuper->sb_freelock);
    return OK;
}

