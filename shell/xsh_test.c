#include <xinu.h>

/**
 * Shell command (test) is testing hook.
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_test(int nargs, char *args[])
{
    // Example to allocate and free blocks in a loop
    printf("Starting block allocation and deallocation test...\n");
    for (int i = 0; i < 5; i++) { // Test with 5 iterations
        int block = sbGetBlock(supertab); // Simulate getting a block
        if (block == SYSERR) {
            printf("Failed to allocate a block on iteration %d.\n", i);
            continue;
        }
        printf("Allocated block number %d.\n", block);

        if (sbFreeBlock(supertab, block) == SYSERR) {
            printf("Failed to free block %d on iteration %d.\n", block, i);
        } else {
            printf("Successfully freed block number %d.\n", block);
        }
    }

    // Display the final state of the free list
    printf("\nFinal state of the disk's free list:\n");
    struct freeblock *fb = supertab->sb_freelst;
    while (fb != NULL) {
        printf("Blk %3d, cnt %3d = ", fb->fr_blocknum, fb->fr_count);
        for (int i = 0; i < fb->fr_count; i++) {
            printf("[%03d]", fb->fr_free[i]);
        }
        printf("\n");
        fb = fb->fr_next;
    }
    return OK;
}


