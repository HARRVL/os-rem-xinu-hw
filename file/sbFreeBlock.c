/* sbFreeBlock.c - sbFreeBlock */
/* Copyright (C) 2008, Marquette University.  All rights reserved. */
/*                                                                 */
/* Modified by   HR and MP                                         */
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
 * Function to persist (or 'swizzle') a freeblock node to disk.
 *------------------------------------------------------------------------
 */
devcall swizzleFreeblockNode(struct dentry *devptr, struct freeblock *node) {
    int diskfd = devptr - devtab; // Obtain the disk descriptor
    int blocknum = node->fr_blocknum; // Get the block number to swizzle to

    // Seek to the block's location on disk and write the node
    if ((seek(diskfd, blocknum) == SYSERR) || (write(diskfd, node, sizeof(struct freeblock)) == SYSERR)) {
        return SYSERR;
    }

    return OK;
}

/*------------------------------------------------------------------------
 * Function to persist (or 'swizzle') the superblock to disk.
 *------------------------------------------------------------------------
 */
devcall swizzleSuperblock(struct superblock *sb) {
    struct dentry *devptr = sb->sb_disk;
    int diskfd = devptr - devtab; // Obtain the disk descriptor

    // Seek to the superblock's location and write the superblock
    if ((seek(diskfd, sb->sb_blocknum) == SYSERR) || (write(diskfd, sb, sizeof(struct superblock)) == SYSERR)) {
        return SYSERR;
    }

    return OK;
}

/*------------------------------------------------------------------------
 * sbFreeBlock - Add a block back into the free list of disk blocks.
 *------------------------------------------------------------------------
 */
devcall sbFreeBlock(struct superblock *filesystem, int blocknum) {
    struct freeblock *currentBlockList, *newBlockNode;

    if (NULL == filesystem || blocknum < 0 || blocknum >= filesystem->sb_blocktotal) {
        return SYSERR;
    }

    wait(filesystem->sb_freelock);

    currentBlockList = filesystem->sb_freelst;

    if (FREEBLOCKMAX == currentBlockList->fr_count) {
        // The current free block node list is full, create a new block node
        newBlockNode = (struct freeblock *)getmem(sizeof(struct freeblock));
        if (SYSERR == (int)newBlockNode) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }

        // Initialize the new free block node
        newBlockNode->fr_count = 0; // No free blocks yet in this node
        newBlockNode->fr_next = NULL; // It's the last node in the list
        newBlockNode->fr_blocknum = blocknum; // Set the block number

        // Link the new node with the current free block list
        currentBlockList->fr_next = newBlockNode;

        // Swizzle the new free block node to disk
        if (SYSERR == swizzleFreeblockNode(filesystem->sb_disk, newBlockNode)) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }
    } else {
        // There's room in the current free block node list, add the block
        currentBlockList->fr_free[currentBlockList->fr_count++] = blocknum;
    }

    // If this is the only free block, also swizzle the superblock
    if (1 == filesystem->sb_freelst->fr_count && filesystem->sb_freelst->fr_free[0] == blocknum) {
        if (SYSERR == swizzleSuperblock(filesystem)) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }
    }

    signal(filesystem->sb_freelock);

    return OK;
}
