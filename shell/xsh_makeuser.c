/**
 * @file     xsh_makeuser.c
 * @provides xsh_makeuser
 *
 * Creates a new user account by adding an entry to the global user table
 * and updating the password file on disk.
 */
#include <xinu.h>

/**
 * Finds the first free slot in the global user table.
 * @return Index of the first free slot or SYSERR if no slots are available.
 */
int findFreeUserSlot(void) {
    for (int i = 0; i < MAXUSERS; i++) {
        if (usertab[i].state == USERFREE) {
            return i;
        }
    }
    return SYSERR;  // No free slot found
}

/**
 * Shell command to create a new user.
 * @param nargs Number of arguments passed.
 * @param args Array of arguments.
 * @return OK on success, SYSERR on failure.
 */
command xsh_makeuser(int nargs, char *args[]) {
    // Check if the current user is the superuser
    if (userid != SUPERUID) {
        printf("ERROR: Only superusr can make new users!\n");
        return SYSERR;
    }

    // Check for correct number of arguments
    if (nargs != 3) {
        fprintf(stderr, "Usage: makeuser <username> <password>\n");
        return SYSERR;
    }

    char* username = args[1];
    char* password = args[2];

    // Validate username and password lengths
    if (strlen(username) >= MAXUSERLEN || strlen(password) >= MAXPASSLEN) {
        printf("ERROR: Username or password length is out of bounds.\n");
        return SYSERR;
    }

    // Find a free slot in the user table
    int newUserSlot = findFreeUserSlot();
    if (newUserSlot == SYSERR) {
        printf("ERROR: No more users available in usertab.\n");
        return SYSERR;
    }

    // Initialize the new user slot
    usertab[newUserSlot].state = USERUSED;
    strncpy(usertab[newUserSlot].username, username, MAXUSERLEN);
    usertab[newUserSlot].username[MAXUSERLEN - 1] = '\0'; // Ensure null termination
    usertab[newUserSlot].salt = rand();  // Generate a random salt for security

    // Hash the password
    ulong hash = xinuhash(password, strlen(password), usertab[newUserSlot].salt);
    usertab[newUserSlot].passhash = hash;

    // Write the updated user table to the passwd file
    if (passwdFileWrite() == SYSERR) {
        fprintf(stderr, "Failed to update the passwd file.\n");
        return SYSERR;
    }

    printf("Successfully created user ID %d: %s\n", newUserSlot, username);
    return OK;
}

