/* sbFreeBlock.c - sbFreeBlock */
#include <xinu.h>
#include <device.h>
#include <memory.h>
#include <disk.h>
#include <file.h>

// swizzle
static devcall swizzleFreeblockNode(struct dentry *devptr, struct freeblock *node) {
    if (node == NULL || devptr == NULL) {
        return SYSERR;
    }
    int diskfd = devptr - devtab;
    return (seek(diskfd, node->fr_blocknum) == SYSERR || write(diskfd, node, sizeof(struct freeblock)) == SYSERR) ? SYSERR : OK;
}


//swizzleSuperBlock
static devcall swizzleSuperblock(struct superblock *sb) {
    if (sb == NULL || sb->sb_disk == NULL) {
        return SYSERR;
    }
    int diskfd = sb->sb_disk - devtab;
    return (seek(diskfd, sb->sb_blocknum) == SYSERR || write(diskfd, sb, sizeof(struct superblock)) == SYSERR) ? SYSERR : OK;
}


// freeblock
devcall sbFreeBlock(struct superblock *filesystem, int blocknum) {
    kprintf("Entering sbFreeBlock with blocknum: %d\n", blocknum); // Debugging output

    if (NULL == filesystem || blocknum < 0 || blocknum >= filesystem->sb_blocktotal) {
        kprintf("Error: Invalid parameters.\n"); // Error message
        return SYSERR;
    }

    wait(filesystem->sb_freelock);

    struct freeblock *currentBlockList = filesystem->sb_freelst;
    if (currentBlockList == NULL) {
        kprintf("Free list is currently empty.\n");
    }

    struct freeblock *lastBlock = NULL;
    while (currentBlockList->fr_next != NULL) {
        lastBlock = currentBlockList;
        currentBlockList = currentBlockList->fr_next;
    }

    if (!currentBlockList) {
        kprintf("All existing blocks are full, allocating a new block node.\n");
        currentBlockList = (struct freeblock *)getmem(sizeof(struct freeblock));
        if ((struct freeblock *)SYSERR == currentBlockList) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }
        currentBlockList->fr_count = 0;
        currentBlockList->fr_next = NULL;
        if (lastBlock) {
            lastBlock->fr_next = currentBlockList;
        } else {
            filesystem->sb_freelst = currentBlockList;
        }
    }

    currentBlockList->fr_free[currentBlockList->fr_count++] = blocknum;
    kprintf("Added block %d to free list node with starting block %d.\n", blocknum, currentBlockList->fr_blocknum);

    if (swizzleFreeblockNode(filesystem->sb_disk, currentBlockList) == SYSERR) {
        kprintf("Failed to swizzle the free block node.\n");
        signal(filesystem->sb_freelock);
        return SYSERR;
    }

    if (lastBlock == NULL) { // This was the first block added when the list was empty
        kprintf("Swizzling superblock as this was the first free block added.\n");
        if (swizzleSuperblock(filesystem) == SYSERR) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }
    }

    signal(filesystem->sb_freelock);
    kprintf("Exiting sbFreeBlock successfully.\n");
    return OK;
}


