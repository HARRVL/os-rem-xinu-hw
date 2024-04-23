/* sbFreeBlock.c - sbFreeBlock */
#include <xinu.h>
#include <device.h>
#include <memory.h>
#include <disk.h>
#include <file.h>

/*------------------------------------------------------------------------
 * Function to persist (or 'swizzle') a freeblock node to disk.
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

/*------------------------------------------------------------------------
 * Function to persist (or 'swizzle') the superblock to disk.
 *------------------------------------------------------------------------
 */
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

    while (curr != NULL && curr->fr_count == FREEBLOCKMAX) {
        prev = curr;
        curr = curr->fr_next;
    }

    if (curr == NULL) { // Allocate a new node if needed
        curr = (struct freeblock *)getmem(sizeof(struct freeblock));
        if ((struct freeblock *)SYSERR == curr) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
        curr->fr_count = 1;
        curr->fr_free[0] = block;
        curr->fr_next = NULL;

        if (prev != NULL) {
            prev->fr_next = curr; // Link new node at the end of the list
            swizzleFreeblockNode(psuper->sb_disk, prev);
        } else {
            psuper->sb_freelst = curr; // New node is the first in the list
        }
    } else {
        curr->fr_free[curr->fr_count++] = block; // Add the block to the free list
    }

    swizzleFreeblockNode(psuper->sb_disk, curr);

    if (prev == NULL) { // Only swizzle the superblock if it's a new node
        swizzleSuperblock(psuper);
    }

    signal(psuper->sb_freelock);
    return OK;
}


