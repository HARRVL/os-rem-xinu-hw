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
 * Function to write a freeblock node to disk (swizzle).
 *------------------------------------------------------------------------
 */
devcall swizzleFreeblockNode(struct dentry *devptr, struct freeblock *node, int blocknum) {
    int diskfd = devptr - devtab; // Assuming devtab is an array of dentry
    seek(diskfd, blocknum); // Seek to the block's location on disk
    return write(diskfd, node, sizeof(struct freeblock)); // Write the new node
}

/*------------------------------------------------------------------------
 * Function to write the superblock to disk (swizzle).
 *------------------------------------------------------------------------
 */
devcall swizzleSuperblock(struct superblock *sb) {
    struct dentry *devptr = sb->sb_disk;
    int diskfd = devptr - devtab; // Assuming devtab is an array of dentry
    seek(diskfd, sb->sb_blocknum); // Seek to the superblock's location
    return write(diskfd, sb, sizeof(struct superblock)); // Write the superblock
}

/*------------------------------------------------------------------------
 * sbFreeBlock - Add a block back into the free list of disk blocks.
 *------------------------------------------------------------------------
 */
devcall sbFreeBlock(struct superblock *filesystem, int blocknum) {
    struct freeblock *current = filesystem->sb_freelst, *prev = NULL;

    if (NULL == filesystem || blocknum < 0 || blocknum >= filesystem->sb_blocktotal) {
        return SYSERR;
    }

    wait(filesystem->sb_freelock);

    // Debugging output
    printf("Attempting to free block %d\n", blocknum);

    // Traverse to the end of the free list or find a spot with available space
    while (current != NULL && current->fr_count >= FREEBLOCKMAX) {
        prev = current;
        current = current->fr_next;
    }

    if (current == NULL) { // Need to create a new free block node
        current = (struct freeblock *)getmem(sizeof(struct freeblock));
        if (current == NULL) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }
        current->fr_count = 0;
        current->fr_blocknum = blocknum; // Assigning a new block number
        current->fr_next = NULL;
        if (prev != NULL) {
            prev->fr_next = current;
        } else {
            filesystem->sb_freelst = current; // This is now the head of the list
        }
    }

    // Add the block to the free list
    current->fr_free[current->fr_count++] = blocknum;
    printf("Block %d added to free list node with count %d\n", blocknum, current->fr_count);

    // Swizzle the changes to the disk
    if (swizzleFreeblockNode(filesystem->sb_disk, current, current->fr_blocknum) == SYSERR) {
        printf("Failed to swizzle free block node.\n");
        signal(filesystem->sb_freelock);
        return SYSERR;
    }

    if (swizzleSuperblock(filesystem) == SYSERR) {
        printf("Failed to swizzle superblock.\n");
        signal(filesystem->sb_freelock);
        return SYSERR;
    }

    signal(filesystem->sb_freelock);
    return OK;
}
