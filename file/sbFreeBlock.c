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
    return (seek(diskfd, node->fr_blocknum) == SYSERR || write(diskfd, node, sizeof(struct freeblock)) == SYSERR) ? SYSERR : OK;
}

static devcall swizzleSuperblock(struct superblock *sb) {
    if (sb == NULL || sb->sb_disk == NULL) {
        return SYSERR;
    }
    int diskfd = sb->sb_disk - devtab;
    return (seek(diskfd, sb->sb_blocknum) == SYSERR || write(diskfd, sb, sizeof(struct superblock)) == SYSERR) ? SYSERR : OK;
}

devcall sbFreeBlock(struct superblock *psuper, int block) {
    if (psuper == NULL || block < 0 || block >= psuper->sb_blocktotal) {
        return SYSERR;
    }

    wait(psuper->sb_freelock);

    struct freeblock *curr = psuper->sb_freelst, *prev = NULL;
    while (curr != NULL && curr->fr_count >= FREEBLOCKMAX) {
        prev = curr;
        curr = curr->fr_next;
    }

    if (curr == NULL) {
        curr = (struct freeblock *)getmem(sizeof(struct freeblock));
        if (curr == (struct freeblock *)SYSERR) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
        curr->fr_count = 1;
        curr->fr_free[0] = block;
        curr->fr_next = NULL;
        if (prev) {
            prev->fr_next = curr;
        } else {
            psuper->sb_freelst = curr;
        }
    } else {
        curr->fr_free[curr->fr_count++] = block;
    }

    swizzleFreeblockNode(psuper->sb_disk, curr);
    if (prev == NULL) { // Swizzle the superblock only if the list was previously empty
        swizzleSuperblock(psuper);
    }

    signal(psuper->sb_freelock);
    return OK;
}


