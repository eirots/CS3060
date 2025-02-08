/**
 * Filename: assn2.c
 */

 /* Promise of Originality 
I promise that this source code file has, in its entirety, been written by myself and no other person or persons. 
If at any time an exact copy of this source code is found to be used by another person in this term, I understand that both 
myself and the student that submitted the copy will receive a zero on this assignment.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


/**
 * main - The main entry point for the program.
 * @argc: The number of command-line arguments.
 * @argv: An array of strings containing the command-line arguments.
 *
 * Return: 0 on success, non-zero on failure.
 */
int main(int argc, char* argv[])
{
    pid_t p = fork();

    if(p < 0){
        perror("Failed to fork, terminating");
        exit(1);
    }else if(p > 0){
        printf("PARENT [%d]: fork succeeded, waiting for child (PID %d) to complete.\n", getpid(), p);
        
        int status;
        waitpid(p, &status, 0);

        if(WIFEXITED(status)){
            printf("PARENT [%d]: CHILD (PID %d), exited with status code %d. Parent terminating.\n", getpid(), p, WEXITSTATUS(status));
        }

    }else{
        //p == 0 
        printf("CHILD  [%d]: started. \n", getpid());

        if(argc == 1){
            printf("CHILD  [%d]: no arguments given. Terminating. \n", getpid());
            exit(0);
        }else if(argc == 2){
            printf("CHILD  [%d]: Running one argument, \'%s\' with execvp(). \n", getpid(), argv[1]);
            
            execvp(argv[1], &argv[1]);
            perror("CHILD failed to run execvp, exiting with code 1");
            exit(1);
        }else{

            printf("CHILD  [%d]: Running more than one argument with execvp(). \n", getpid());

            for (int i = 1; i < argc; i++) {
                printf("  argv[%d] = '%s'\n", i, argv[i]);
            }
            
            execvp(argv[1], &argv[1]);
            perror("CHILD failed to run execvp, exiting with code 1");
            exit(1);
        } 

    }


    /*
    * call fork() at the very start
        parent process will display a message saying that it started, and it's waiting 
        for a specific child id process to complete

        once the child process has completed, the parent process will display a messae indicating that 
        the child has completed and that the parent will now terminate 


        child process will display a message indicating that it has started. If one or more arguments 
        were provided ont he command line, the child process will perform the appropriate exec() system call to 
        load the new exe into memory with appropriate arguments if provided. 

            if no arguments were provided, then the child will display a message saying that it will term
    */








    return 0;  
}
