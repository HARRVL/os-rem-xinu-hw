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
    kprintf("Entering sbFreeBlock with blocknum: %d\n", blocknum);

    if (filesystem == NULL || blocknum < 0 || blocknum >= filesystem->sb_blocktotal) {
        kprintf("Error: Invalid parameters.\n");
        return SYSERR;
    }

    wait(filesystem->sb_freelock);

    struct freeblock *current = filesystem->sb_freelst;
    struct freeblock *prev = NULL;

    // Traverse to find the last block in the list or a block with available space
    while (current != NULL && current->fr_count == FREEBLOCKMAX) {
        prev = current;
        current = current->fr_next;
    }

    // Allocate a new block if necessary
    if (current == NULL || current->fr_count >= FREEBLOCKMAX) {
        struct freeblock *newBlock = (struct freeblock *)getmem(sizeof(struct freeblock));
        if (newBlock == SYSERR) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }
        newBlock->fr_count = 0;
        newBlock->fr_next = NULL;

        if (prev != NULL) {
            prev->fr_next = newBlock;
        } else {
            filesystem->sb_freelst = newBlock; // First node in the list
        }
        current = newBlock;
    }

    // Add the block number to the free list
    current->fr_free[current->fr_count++] = blocknum;

    // Swizzle the free block node to disk
    if (swizzleFreeblockNode(filesystem->sb_disk, current) == SYSERR) {
        kprintf("Failed to swizzle the free block node.\n");
        signal(filesystem->sb_freelock);
        return SYSERR;
    }

    kprintf("Exiting sbFreeBlock successfully. Block %d freed.\n", blocknum);
    signal(filesystem->sb_freelock);
    return OK;
}



