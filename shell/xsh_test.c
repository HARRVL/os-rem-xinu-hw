#include <xinu.h>

/**
 * Shell command (test) is testing hook.
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_test(int nargs, char *args[])
{
    int numTests = 5; // Number of tests to perform
    int block;
    kprintf("Starting block allocation and deallocation test...\n");

    for (int i = 0; i < numTests; i++) {
        block = sbGetBlock(supertab); // Attempt to allocate a block
        if (block == SYSERR) {
            kprintf("Failed to allocate block on iteration %d.\n", i);
            continue; // Skip this iteration if block allocation failed
        }

        kprintf("Allocated block number %d.\n", block);

        if (sbFreeBlock(supertab, block) == SYSERR) {
            kprintf("Failed to free block %d on iteration %d.\n", block, i);
        } else {
            kprintf("Successfully freed block number %d.\n", block);
        }
    }

    kprintf("Final state of the disk's free list:\n");
    xsh_diskstat(0, NULL); 

    return OK;
}

