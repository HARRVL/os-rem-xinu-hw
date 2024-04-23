/**
 * @file     xsh_test.c
 * @provides xsh_test
 *
 */
/* Embedded XINU, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>

/**
 * Shell command (test) is testing hook.
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
// command xsh_test(int nargs, char *args[])
// {
//    TODO: Test your O/S.
//    printf("This is where you should put some testing code.\n");
//     return OK;
// }

command xsh_test(int nargs, char *args[])
{
    // Create a file
    int fd = fileCreate("testfile");
    if (fd == SYSERR)
    {
        printf("Test failed: file creation failed.\n");
        return SYSERR;
    }
    else
    {
        printf("File created successfully.\n");
    }

    // Write data to the file
    char data = 'X';
    if (filePutChar(fd, data) == SYSERR)
    {
        printf("Test failed: writing to file failed.\n");
        return SYSERR;
    }
    else
    {
        printf("Data written to file successfully.\n");
    }

    // Close the file
    if (fileClose(fd) == SYSERR)
    {
        printf("Test failed: file closing failed.\n");
        return SYSERR;
    }
    else
    {
        printf("File closed successfully.\n");
    }

    // Delete the file
    if (fileDelete(fd) == SYSERR)
    {
        printf("Test failed: file deletion failed.\n");
        return SYSERR;
    }
    else
    {
        printf("File deleted successfully.\n");
    }

    // Print free list

    // wait(supertab->sb_freelock); // Synchronize access to the free list

    // printf("\nFree block list visualization:\n");
    // struct freeblock *fb = supertab->sb_freelst;
    // if (fb == NULL) {
    //     printf("No free blocks.\n");
    // } else {
    //     while (fb != NULL) {
    //         printf("Free Block Node at Disk Block #%d: ", fb->fr_blocknum);
    //         printf("Free Blocks Count: %d\n", fb->fr_count);
    //         for (int i = 0; i < fb->fr_count; i++) {
    //             printf("%d ", fb->fr_free[i]);
    //         }
    //         printf("\n");
    //         fb = fb->fr_next; // Move to the next node
    //     }
    // }

    xsh_diskdat(0,NULL);

    // signal(supertab->sb_freelock); // Release the lock

    return OK;

}
