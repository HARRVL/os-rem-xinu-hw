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
            return i;
        }
    }
    return SYSERR;
}

/**
 * Shell command to create a new user.
 * @param nargs Number of arguments passed.
 * @param args Array of arguments.
 * @return OK on success, SYSERR on failure.
 */
command xsh_makeuser(int nargs, char *args[]) {
    

    if( userid < SUPERUID){
        return SYSERR; 
        printf("Must login first"); 
    }
    if (userid != SUPERUID) {
        printf("ERROR: Only superusr can make new users!\n");
        return SYSERR;
    }

    if (nargs != 3) {
        fprintf(stderr, "Usage: makeuser <username> <password>\n");
        return SYSERR;
    }
    
    char* username = args[1];
    char* password = args[2];

    if (strlen(username) >= MAXUSERLEN || strlen(password) >= MAXPASSLEN) {
        printf("ERROR: Username or password length is out of bounds.\n");
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
    usertab[newUserSlot].salt = SALT; // Generate a random salt

    // Hash the password
    ulong hash = hashpassword(usertab[newUserSlot].salt);

    usertab[newUserSlot].passhash = hash;

    // Commit changes to the passwd file on disk
    if (passwdFileWrite() == SYSERR) {
        fprintf(stderr, "Failed to update the passwd file.\n");
        return SYSERR;
    }

    printf("Successfully created user ID %d: %s\n", newUserSlot, username);
    return OK;
}
