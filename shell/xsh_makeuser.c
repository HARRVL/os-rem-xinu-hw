/**
 * @file     xsh_makeuser.c
 * @provides xsh_makeuser
 *
 * Shell command to create a new user in the system.
 */
#include <xinu.h>

/**
 * Prompts for a string input with a custom message, ensuring the input does not exceed the maximum length.
 * @param prompt The message to display to the user.
 * @param buffer The buffer to store the input string.
 * @param buflen The maximum length of the buffer.
 */
void promptForInput(const char *prompt, char *buffer, int buflen) {
    printf("%s", prompt);
    fgets(buffer, buflen, CONSOLE);
    buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character.
}

/**
 * Main function to create a new user.
 * @param nargs Number of arguments passed.
 * @param args Array of arguments.
 * @return OK for success, SYSERR for error.
 */
command xsh_makeuser(int nargs, char *args[]) {
    if (userid != SUPERUID) {
        fprintf(stderr, "ERROR: Only superusr can make new users!\n");
        return SYSERR;
    }

    char username[MAXUSERLEN];
    char password[MAXPASSLEN];

    // Prompt for username
    promptForInput("Enter new username: ", username, sizeof(username));

    // Prompt for password
    promptForInput("Enter new password: ", password, sizeof(password));

    // Check for user slot availability
    int newUserSlot = findFreeUserSlot();
    if (newUserSlot == SYSERR) {
        fprintf(stderr, "ERROR: No more users available in usertab.\n");
        return SYSERR;
    }

    // Initialize the new user slot
    usertab[newUserSlot].state = USERUSED;
    strncpy(usertab[newUserSlot].username, username, MAXUSERLEN);
    usertab[newUserSlot].username[MAXUSERLEN - 1] = '\0'; // Ensure null termination
    usertab[newUserSlot].salt = rand(); // Generate a random salt
    usertab[newUserSlot].passhash = xinuhash(password, strlen(password), usertab[newUserSlot].salt);

    // Commit the new user to persistent storage
    if (passwdFileWrite() == SYSERR) {
        fprintf(stderr, "Failed to update the passwd file.\n");
        return SYSERR;
    }

    printf("Successfully created user ID %d: %s\n", newUserSlot, username);
    return OK;
}


