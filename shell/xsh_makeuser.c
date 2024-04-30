/**
 * @file     xsh_makeuser.c
 * @provides xsh_makeuser
 *
 * Creates a new user account by adding an entry to the global user table
 * and updating the password file on disk.
 */
#include <xinu.h>

/**
 * Searches for the first free slot in the global user table.
 * @return Index of the first free slot or SYSERR if no slots are available.
 */
int findFreeUserSlot(void) {
    for (int i = 0; i < MAXUSERS; i++) {
        if (usertab[i].state == USERFREE) {
            printf("Debug: Found free user slot at index %d\n", i);
            return i;
        }
    }
    printf("Debug: No free user slot available\n");
    return SYSERR;
}

/**
 * Shell command to create a new user.
 * @param nargs Number of arguments passed.
 * @param args Array of arguments.
 * @return OK on success, SYSERR on failure.
 */
command xsh_makeuser(int nargs, char *args[]) {
    printf("Debug: Entered xsh_makeuser function\n");

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

    printf("Debug: Username = '%s', Password = '%s'\n", username, password);

    if (strlen(username) >= MAXUSERLEN || strlen(password) >= MAXPASSLEN) {
        fprintf(stderr, "ERROR: Username or password length is out of bounds.\n");
        return SYSERR;
    }

    int newUserSlot = findFreeUserSlot();
    if (newUserSlot == SYSERR) {
        return SYSERR;
    }

    // Initialize the new user slot
    usertab[newUserSlot].state = USERUSED;
    strncpy(usertab[newUserSlot].username, username, MAXUSERLEN);
    usertab[newUserSlot].username[MAXUSERLEN - 1] = '\0'; // Ensure null termination
    usertab[newUserSlot].salt = rand(); // Generate a random salt

    // Hash the password
    ulong hash = xinuhash(password, strlen(password), usertab[newUserSlot].salt);
    printf("Debug: Generated hash = 0x%08X for user %s\n", hash, username);

    usertab[newUserSlot].passhash = hash;

    // Commit changes to the passwd file on disk
    if (passwdFileWrite() == SYSERR) {
        fprintf(stderr, "Failed to update the passwd file.\n");
        return SYSERR;
    }

    printf("Successfully created user ID %d: %s\n", newUserSlot, username);
    printf("Debug: Exiting xsh_makeuser function\n");
    return OK;
}



