
/**
 * @file     xsh_switchuser.c
 * @provides xsh_switchuser
 *
 */
/* Embedded XINU, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>

/**
 * Shell command (delUSer) is deletinguser hook.
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_deluser(int nargs, char *args[])
{
    int id;
    char buffer[MAXUSERLEN];

    if (userid < SUPERUID)
    {
        printf("Must login first\n");
        return SYSERR;
    }

    getusername(buffer,MAXUSERLEN); 
    

    
    // //Attempt authentication
    // if (nargs == 2)
    // {
    //     id = login(args[1]);
    // }
    // else
    // {
    //     id = login(NULL);
    // }
    // if (SYSERR == id)
    // {
    //     printf("Login failure.\n");
    //     return SYSERR;
    // }
    // userid = id;
    // printf("Success!\n");
    return OK;
}
