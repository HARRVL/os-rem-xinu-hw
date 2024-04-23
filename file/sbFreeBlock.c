/* sbFreeBlock.c - sbFreeBlock */
/* Copyright (C) 2008, Marquette University.  All rights reserved. */
/*                                                                 */
/* Modified by  Hector Reyes and Max Pena                          */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*                                                                 */

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

#include <kernel.h>
#include <device.h>
#include <memory.h>
#include <disk.h>
#include <file.h>

devcall swizzleFreeblockNode(struct dentry *devptr, struct freeblock *node, int blocknum);
devcall swizzleSuperblock(struct superblock *sb);

devcall sbFreeBlock(struct superblock *filesystem, int blocknum) {
    struct freeblock *current = filesystem->sb_freelst, *prev = NULL;

    if (NULL == filesystem || blocknum < 0 || blocknum >= filesystem->sb_blocktotal) {
        return SYSERR;
    }

    wait(filesystem->sb_freelock);

    // Find the appropriate place to add the new block
    while (current != NULL && current->fr_count == FREEBLOCKMAX) {
        prev = current;
        current = current->fr_next;
    }

    if (current == NULL) { // All blocks full, or no blocks yet
        current = (struct freeblock *)getmem(sizeof(struct freeblock));
        if (current == NULL) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }
        current->fr_count = 0;
        current->fr_next = NULL;
        current->fr_blocknum = filesystem->sb_blocktotal++; // Use a new block for metadata storage

        if (prev) {
            prev->fr_next = current;
        } else {
            filesystem->sb_freelst = current;
        }
    }

    current->fr_free[current->fr_count++] = blocknum;
    if (swizzleFreeblockNode(filesystem->sb_disk, current, current->fr_blocknum) == SYSERR) {
        signal(filesystem->sb_freelock);
        return SYSERR;
    }

    if (filesystem->sb_freelst->fr_count == 1 && filesystem->sb_freelst == current) {
        if (swizzleSuperblock(filesystem) == SYSERR) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }
    }

    signal(filesystem->sb_freelock);
    return OK;
}
