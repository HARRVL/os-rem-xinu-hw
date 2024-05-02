
/**
 * @file     xsh_switchuser.c
 * @provides xsh_switchuser
 *
 */
/* Embedded XINU, Copyright (C) 2009.  All rights reserved. */

#include <xinu.h>
#include <string.h>

/**
 * Shell command (delUSer) is deletinguser hook.
 * @param args array of arguments
 * @return OK for success, SYSERR for syntax error
 */
command xsh_deluser(int nargs, char *args[])
{
    int id;
    char buffer[MAXUSERLEN];
    

    if (userid != SUPERUID)
    {
        printf("Must be super user to peform this action! \n");
        return SYSERR;
    }
    

    if(nargs <1){
        printf("Provide a username to delete");
        return SYSERR;
    }
    if(nargs == 1){
        getusername(buffer,MAXUSERLEN);
       
    }else{
        if(strlen(args[1]) > MAXUSERLEN){
            printf("Username Too long"); 
            return SYSERR;
        }
        //strncopy(buffer,args[1],MAXUSERLEN) ;
        buffer[MAXUSERLEN -1] = '\0';
          
    }
        

    id = searchname(buffer); 
    
    if(userid == id){
        printf("You cannot delete this user while you are signed in as them");
        return SYSERR;
    }
    if(id = SUPERUID){
        printf("Cannot delete SuperUser");
        return SYSERR; 
    }
    if((id <SUPERUID) || (id > MAXUSERS)){
        printf("User does not exist!!!!");
        return SYSERR; 
    }

    

    usertab[id].state = USERFREE; 
     bzero(usertab[id].username,0);
    usertab[id].passhash = 0; 
    printf("Successfully deleted user %s",buffer);
    passwdFileWrite();
    return OK;
}
