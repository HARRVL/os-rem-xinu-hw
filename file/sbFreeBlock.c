// edited by Hector Reyes and Max Pena
/* sbFreeBlock.c - sbFreeBlock */
#include <xinu.h>
#include <device.h>
#include <memory.h>
#include <disk.h>
#include <file.h>

/**
 * Swizzles a freeblock node's next pointer for disk storage
 * and writes the freeblock node to disk.
 * 
 * @param diskfd The file descriptor for the disk device
 * @param freeblk Pointer to the freeblock to be swizzled and written to disk
 * @returns OK on success, SYSERR on failure
 */
devcall swizzle(int diskfd, struct freeblock *freeblk)
{
    struct freeblock *free2 = freeblk->fr_next; // Temporarily store next pointer

    // Modify the next pointer for swizzling: converting memory pointer to disk block number
    if (freeblk->fr_next == NULL) {
        freeblk->fr_next = 0; // NULL pointers are swizzled to 0
    } else {
        freeblk->fr_next = free2->fr_blocknum; // Use block number for swizzling
    }

    // Write the modified freeblock back to disk
    seek(diskfd, freeblk->fr_blocknum); // Seek to the correct block
    if (SYSERR == write(diskfd, freeblk, sizeof(struct freeblock))) {
        freeblk->fr_next = free2; // Restore original next pointer on failure
        return SYSERR;
    }

    freeblk->fr_next = free2; // Restore original next pointer after successful write

    return OK;
}

/**
 * Swizzles and writes the superblock to disk.
 * 
 * @param diskfd The file descriptor for the disk device
 * @param psuper Pointer to the superblock to be swizzled and written to disk
 * @returns OK on success, SYSERR on failure
 */
devcall swizzleSuperBlock(int diskfd, struct superblock *psuper)
{
    // Store original next pointers
    struct freeblock *swizzle = psuper->sb_freelst;
    struct dirblock *swizzle2 = psuper->sb_dirlst;

    // Swizzle pointers to disk block numbers
    psuper->sb_freelst = swizzle->fr_blocknum;
    psuper->sb_dirlst = swizzle2->db_blocknum;

    // Write the superblock to disk
    seek(diskfd, psuper->sb_blocknum);
    if (SYSERR == write(diskfd, psuper, sizeof(struct superblock))) {
        // Restore original pointers if write fails
        psuper->sb_freelst = swizzle;
        psuper->sb_dirlst = swizzle2;
        return SYSERR;
    }

    // Restore original pointers after successful write
    psuper->sb_freelst = swizzle;
    psuper->sb_dirlst = swizzle2;

    return OK;
}

/**
 * Frees a block and adds it back to the filesystem's free block list.
 * Ensures mutual exclusion via superblock locks and writes changes to disk.
 * 
 * @param psuper Pointer to the filesystem's superblock
 * @param block The block number to be freed
 * @returns OK on success, SYSERR on failure
 */
devcall sbFreeBlock(struct superblock *psuper, int block)
{
    // Basic error checking for input parameters
    if (NULL == psuper) return SYSERR;
    struct dentry *phw = psuper->sb_disk;
    if (NULL == phw) return SYSERR;
    if ((block <= 0) || (block > DISKBLOCKTOTAL)) return SYSERR;

    int diskfd = phw - devtab; // Calculate disk file descriptor
    wait(psuper->sb_freelock); // Lock the free list for exclusive access

    struct freeblock *freeblk = psuper->sb_freelst;

    if (NULL == freeblk) { // If free list is empty, allocate new free block node
        freeblk = malloc(sizeof(struct freeblock));
        if (NULL == freeblk) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
        // Initialize new free block node
        freeblk->fr_blocknum = block;
        freeblk->fr_count = 0;
        freeblk->fr_next = NULL;
        psuper->sb_freelst = freeblk;

        // Write changes to disk
        if (SYSERR == swizzleSuperBlock(diskfd, psuper) || SYSERR == swizzle(diskfd, freeblk)) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }

        signal(psuper->sb_freelock);
        return OK;
    }

    // Traverse to the end of the free list
    while (freeblk->fr_next != NULL) {
        freeblk = freeblk->fr_next;
    }

    if (freeblk->fr_count >= FREEBLOCKMAX || ((freeblk->fr_count == 0) && (psuper->sb_freelst == freeblk))) {
        struct freeblock *collector = malloc(sizeof(struct freeblock));
        if (NULL == collector) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }
        // Initialize new collector node
        freeblk->fr_next = collector;
        collector->fr_blocknum = block;
        collector->fr_count = 0;
        collector->fr_next = NULL;

        // Swizzle and write new collector to disk
        if (SYSERR == swizzle(diskfd, collector)) {
            signal(psuper->sb_freelock);
            return SYSERR;
        }

        signal(psuper->sb_freelock);
        return OK;
    }

    // Add block to current free block node
    freeblk->fr_free[freeblk->fr_count++] = block;
    if (SYSERR == swizzle(diskfd, freeblk)) {
        signal(psuper->sb_freelock);
        return SYSERR;
    }

    signal(psuper->sb_freelock);
    return OK;
}

