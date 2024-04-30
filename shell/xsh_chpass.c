/**
 * @file     xsh_chpass.c
 * @provides xsh_chpass
 *
 */
/* Embedded XINU, Copyright (C) 2024.  All rights reserved. */

#include <xinu.h>

/**
 * Shell command (chpass) changes an existing user password.
 * @param args array of arguments
 * @return OK for success, SYSERR for errors.
 */

/**
 * TODO:
 * This function creates a new password for an existing user.
 * You may break this task down into any number of new helper
 * functions within this file, and also may rely on helper functions
 * that already exist, such as getusername(), hasspassword(), and
 * passwdFileWrite().
 *
 * Steps:
 * 1) If no user name was provided to the shell command, superusr should
 *    be prompted for one.  Normal users are not prompted, because we
 *    default to changing their own password.
 * 2) Search for the user name in the usertab.
 * 3) If the current user is not superusr, prompt for the previous password.
 *    Prompt text = "Enter previous password for user %s: ".
 * 4) If the hash of the previous password matched what is on record,
 *    prompt for new password.
 *    Prompt test = "Enter new password for user %s: ".
 * 5) Place the new password hash into the user entry, and commit to disk.
 * 6) Printf "Successfully changed password for user ID %d\n" with user ID.
 *
 * Errors to watch for:
 * 1) There is not already a user logged in.
 *    Error text = "Must login first\n".
 * 2) The logged in userid is not SUPERUID, but is trying to change someone
 *    else's password.
 *    Error text = "ERROR: Only superusr can change other passwords!\n".
 * 3) The given user name cannot be found in the existing user table.
 *    Error text = "User name %s not found.\n".   
 * 4) The password change failed.  (i.e., passwords didn't match.)
 *    Error text = "Password for user %s does not match!\n".
 */

char* promptForPassword(const char* prompt, int maxlen) {
    char *password = (char *)malloc(maxlen + 1);
    if (!password) return NULL;  // Check malloc succeeded

    fprintf(CONSOLE, "%s", prompt);
    int i = 0, ch;
    while (i < maxlen && (ch = getc(CONSOLE)) != '\n' && ch != EOF) {
        password[i++] = ch;
    }
    password[i] = '\0';
    return password;
}

command xsh_chpass(int nargs, char *args[]) {
    // Step 1: Check login status
    if (userid == SYSERR) {
        fprintf(stderr, "Must login first\n");
        return SYSERR;
    }

    char *username;
    if (nargs < 2) {
        // For superusr, prompt for the username if not provided
        if (userid == SUPERUID) {
            username = promptForPassword("Enter username to change password: ", MAXUSERLEN);
        } else {
            // Normal users can only change their own password
            username = usertab[userid].username;
        }
    } else {
        username = args[1];
    }

    // Step 2: Search for the user in usertab
    int uid = searchname(username);
    if (uid == SYSERR) {
        fprintf(stderr, "User name %s not found.\n", username);
        return SYSERR;
    }

    // Step 3: If not superusr, verify old password
    if (userid != SUPERUID) {
        printf("Verify your previous password: \n"); 
        if(hashpassword(usertab[uid].salt) == usertab[uid].passhash){
            printf("Enter Your New Password: \n"); 
            usertab[uid].passhash = hashpassword(usertab[uid].salt); 
        }else{
            fprintf(stderr, "Password for user %s does not match!\n", username);
            return SYSERR;
        }
    }

    if (passwdFileWrite() == SYSERR) {
        fprintf(stderr, "Failed to update the password file.\n");
        return SYSERR;
    }

    // Step 6: Confirm the password change
    printf("Successfully changed password for user ID %d\n", uid);
    return OK;
}
