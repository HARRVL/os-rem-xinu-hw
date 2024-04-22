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

    return OK;
}
