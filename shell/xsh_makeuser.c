/**
 * @file     xsh_makeuser.c
 * @provides xsh_makeuser
 *
 */
/* Embedded XINU, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>

/**
 * Shell command (makeuser) makes a new user account.
 * @param args array of arguments
 * @return OK for success, SYSERR for errors.
 */
command xsh_makeuser(int nargs, char *args[])
{
/**
 * TODO:
 * This function creates a new entry in the global table of users,
 * with a valid password hash and salt, and updates the passwd file
 * on disk.  You may break this task down into any number of helper
 * functions within this file, and also may rely on helper functions
 * that already exist, such as getusername(), hasspassword(), and
 * passwdFileWrite().
 *
 * Steps:
 * 1) Find a free usertab entry for a new user, and set it USERUSED.
 * 2) If the shell did not provide a user name, prompt for one.
 * 3) Prompt for a new password, and calculate the hash.
 * 4) Initialize the fields of the new user entry.
 * 5) Commit the changes to the passwd file on disk.
 * 6) Printf "Successfully created user ID %d\n" with the new user ID.
 *
 * Errors to watch for:
 * 1) There is not already a user logged in.
 *    Error text = "Must login first\n".
 * 2) The logged in userid is not already SUPERUID.
 *    Error text = "ERROR: Only superusr can make new users!\n".
 * 3) There are no more unused slots in usertab.
 *    Error text = "ERROR: No more users available in usertab!\n".
 */

int findFreeUserSlot(void) {
    for (int i = 0; i < MAXUSERS; i++) {
        if (usertab[i].state == USERFREE) {
            return i;
        }
    }
    return SYSERR;
}

/**
 * Shell command (makeuser) makes a new user account.
 * @param args array of arguments
 * @return OK for success, SYSERR for errors.
 */
command xsh_makeuser(int nargs, char *args[])
{
    if (userid != SUPERUID) {
        fprintf(stderr, "ERROR: Only superusr can make new users!\n");
        return SYSERR;
    }

    if (nargs != 3) {
        fprintf(stderr, "Usage: makeuser <username> <password>\n");
        return SYSERR;
    }

    char* username = args[1];
    char* password = args[2];

    if (strlen(username) >= MAXUSERLEN || strlen(password) >= MAXPASSLEN) {
        fprintf(stderr, "ERROR: Username or password length is out of bounds.\n");
        return SYSERR;
    }

    int newUserSlot = findFreeUserSlot();
    if (newUserSlot == SYSERR) {
        fprintf(stderr, "ERROR: No more users available in usertab!\n");
        return SYSERR;
    }

    // Initialize the new user slot
    usertab[newUserSlot].state = USERUSED;
    strncpy(usertab[newUserSlot].username, username, MAXUSERLEN - 1);
    usertab[newUserSlot].username[MAXUSERLEN - 1] = '\0';  // Ensure null termination
    usertab[newUserSlot].salt = rand(); // Assume rand() gives a sufficiently random salt
    usertab[newUserSlot].passhash = xinuhash(password, strlen(password), usertab[newUserSlot].salt);

    // Commit changes to the passwd file on disk
    if (passwdFileWrite() == SYSERR) {
        fprintf(stderr, "Failed to update the passwd file.\n");
        return SYSERR;
    }

    printf("Successfully created user ID %d\n", newUserSlot);
    return OK;
}
