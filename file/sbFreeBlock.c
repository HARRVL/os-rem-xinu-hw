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
    struct freeblock *lastBlock = NULL;

    // Traverse to the last block or the block with available space
    while (currentBlockList != NULL && currentBlockList->fr_count == FREEBLOCKMAX) {
        lastBlock = currentBlockList;
        currentBlockList = currentBlockList->fr_next;
    }

    // Case where all nodes are full or no nodes exist
    if (currentBlockList == NULL || currentBlockList->fr_count == FREEBLOCKMAX) {
        kprintf("Allocating a new free block node as all are full or non-existent.\n");
        struct freeblock *newBlock = (struct freeblock *)getmem(sizeof(struct freeblock));
        if ((struct freeblock *)SYSERR == newBlock) {
            signal(filesystem->sb_freelock);
            return SYSERR;
        }

        // Initialize the new block node
        newBlock->fr_count = 1; // It will immediately contain the new block
        newBlock->fr_free[0] = blocknum;
        newBlock->fr_next = NULL;
        newBlock->fr_blocknum = blocknum; // Adjust this as needed

        if (lastBlock) {
            lastBlock->fr_next = newBlock;
        } else {
            filesystem->sb_freelst = newBlock; // This is now the first node if none existed before
        }

        currentBlockList = newBlock; // Set current to new node for swizzling
    } else {
        // Add block to the current node
        currentBlockList->fr_free[currentBlockList->fr_count++] = blocknum;
    }

    // Swizzle the current node to save changes
    if (swizzleFreeblockNode(filesystem->sb_disk, currentBlockList) == SYSERR) {
        kprintf("Failed to swizzle the free block node.\n");
        signal(filesystem->sb_freelock);
        return SYSERR;
    }

    // If this was the first block added to an empty list, also swizzle the superblock
    if (filesystem->sb_freelst == currentBlockList && currentBlockList->fr_count == 1) {
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



