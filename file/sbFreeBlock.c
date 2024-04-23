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
#include <string.h>

/*------------------------------------------------------------------------
 * Swizzle Functions - Ensure the updated structures are written to disk.
 *------------------------------------------------------------------------
 */

// Forward declarations for helper functions
static devcall swizzleFreeblockNode(struct dentry *devptr, struct freeblock *node);
static devcall swizzleSuperblock(struct superblock *psuper);

// Helper function to swizzle a freeblock node to disk.
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

// Helper function to swizzle the superblock to disk.
static devcall swizzleSuperblock(struct superblock *psuper) {
    if (psuper == NULL || psuper->sb_disk == NULL) {
        return SYSERR;
    }

    int diskfd = psuper->sb_disk - devtab;
    if (seek(diskfd, psuper->sb_blocknum) == SYSERR || 
        write(diskfd, psuper, sizeof(struct superblock)) == SYSERR) {
        return SYSERR;
    }

    return OK;
}

// Function to add a block back into the free list of disk blocks.
devcall sbFreeBlock(struct superblock *psuper, int block) {
    if (psuper == NULL || block < 0 || block >= psuper->sb_blocktotal) {
        return SYSERR;
    }

    wait(psuper->sb_freelock);

    struct freeblock *curr = psuper->sb_freelst, *prev = NULL;
    while (curr != NULL && curr->fr_count == FREEBLOCKMAX) {
        prev = curr;
        curr = curr->fr_next;
    }

    if (curr == NULL) {
        curr = (struct freeblock *)getmem(sizeof(struct freeblock));
        if ((struct freeblock *)SYSERR == curr) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
        memset(curr, 0, sizeof(struct freeblock)); // Important: Initialize the memory
        curr->fr_blocknum = block; // Allocate a new block number for the free list node
        psuper->sb_freelst = curr; // Update head if this is the first node
    }

    curr->fr_free[curr->fr_count++] = block; // Add the block to the current node

    if (swizzleFreeblockNode(psuper->sb_disk, curr) == SYSERR) {
        signal(psuper->sb_freelock);
        return SYSERR;
    }

    if (prev == NULL) { // If this was the first free block, swizzle the superblock too
        if (swizzleSuperblock(psuper) == SYSERR) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
    }

    signal(psuper->sb_freelock);
    return OK;
}
