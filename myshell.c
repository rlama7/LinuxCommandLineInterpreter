/****************************************************************
 * Name        : Ratna Lama                                     *
 * Class       : CSC 415                                        *
 * Date        : 10/03/2018                                     *
 * Description :  Writing a simple bash shell program           *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>     // strtok(); strcmp()
#include <unistd.h>     // dup2(); execvp()
#include <stdlib.h>     // exit(); execvp()
#include <stdio.h>      // printf(); perror()
#include <sys/types.h>
#include <sys/wait.h>   // waitpid()
#include <sys/stat.h>
#include <fcntl.h>


// pre-processor directives
#define BUFFERSIZE 256
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)

// function prototypes
void execute_commands(char *[], int, int);      // command execution controller.
int arg_found(char *[], char *, int);           // index of arg if found in the array myargv
void bg_process(char *[], int);                 // execute command in background process
void first_redirect_output(char *[], int);      // first redirection of the standard output
void second_redirect_output(char *[], int);     // second redirection of the standard output
void redirect_input(char *[], int);             // redirection of the standard input
void single_piped_command(char *[], int);       // execution of multiple commands connected by single shell pipe
void execute(char *[], int);                    // executes a single command up to four command line arguments
void pwd();                                     // prints current working directory
void change_dir();                              // changes directory
void ls();                                      // list segments

// Begin main()
int main(int argc, char** argv) {

    // variable declaration and initialization
    char *str_line = NULL; // string line
    ssize_t bytes_read;
    size_t len = 0;
    const char delims[1] = " ";
    char *token;
    char *myargv[BUFFERSIZE];  // stores parsed substrings
    int myargc;                // count of the number of  arguments (token) stored in the array myargv

    /*
     * shell#0 implementation
     * getline() read an entire line from the stream, storing the address of
     * the buffer containing the text into *str_line. The buffer is null terminated
     * and includes the newline character, if one was found.
     */
    printf("%s", PROMPT);

    while (getline(&str_line, &len, stdin) != -1) {

        myargc = 0; // reset
        str_line[strlen(str_line)-1] = '\0';

        // Parse the line and tokenize it
        // strtok returns pointer to the first token
        token = strtok(str_line, delims);

        while (token != NULL) {
            // store array of tokens in myargv array
            myargv[myargc++] = token;
            token = strtok(NULL, delims);
        } // end while loop


        // At this stage we have an array of tokens ready for execution
        // so we match the type of command for execution based on user inputs
        if (myargc > 0) {   // there exists at least a single command to execute

            // check exit condition
            if (strcmp(myargv[0], "exit") == 0) {
                exit(-1);

            } else {
                // execute commands
                execute_commands(myargv, myargc, 0);
            } // end if-els-if

        } // end if
        printf("%s", PROMPT);

    } // end while loop
    return 0;
} // end main()

/*----------------------FUNCTION------------------------------------*/

// #1
// execute_commands function delegates command execution
// to many different sub-function based on user input types
void execute_commands(char *myargv[], int myargc, int bg) {

    // #1 condition
    if (strcmp(myargv[0], "pwd") == 0) {
        // print working directory
        pwd();

    } else if (strcmp(myargv[0], "cd") == 0) {
        // change directory
        change_dir(myargv[1]);

    } else if (arg_found(myargv, "&", myargc) >= 0) {
        // execute a command in background
        bg_process(myargv, myargc);

    // #2 condition
    } else if (arg_found(myargv, ">", myargc) >= 0) {
        // redirect the first standard output of a command to a file
        first_redirect_output(myargv, myargc);

    // #3 condition
    } else if (arg_found(myargv, ">>", myargc) >= 0) {
        // redirect the second standard output of a command to a file
        second_redirect_output(myargv, myargc);

    // #4 condition
    } else if (arg_found(myargv, "<", myargc) >= 0) {
        // redirect the standard input of a command to come from a file
        redirect_input(myargv, myargc);

    // #5 condition
    } else if(arg_found(myargv, "|", myargc) >= 0) {
        // execute multiple commands connected by a single shell pipe
        single_piped_command(myargv, myargc);

    }
    else {
        // default
        execute(myargv, bg);
    }

} // end execute_commands

// #2
// execute() executes a single command up to four command line arguments
void execute(char *myargv[], int bg) {

    // Begin new process
    pid_t pid = fork();

    if (pid == 0) {
        // The child process creation attempt was successful
        if (bg) {
            setpgid(pid, 0);
        }
        execvp(myargv[0], myargv);

    } else if (pid > 0) {
        // waitpid() system call suspends execution of the calling process until a child
        // specified by pid argument has changed state.
        // the third argument 0 refers to wait for any child whose process ID is equal to the value of pid
        wait(NULL);

    } else {
        // pid < 0
        perror("The creation of a child process was unsuccessful...");
    } // end if-else

} // end execute()


// #3
// arg_found() function returns index of argument arg
// if found in the array myargv
int arg_found(char *myargv[], char *arg, int myargc) {
    int index = 0;

    while (myargc > index) {
        if (strcmp(myargv[index], arg) == 0) {
            return index;
        }
        index++;
    }

    // if arg is not found in the array then return -1
    return (-1);
} // end arg_found()

// 4
// bg_process() executes a command in background
void bg_process(char *myargv[], int myargc) {
    int index_of_ampersand = arg_found(myargv, "&", myargc);

    // check if '&' is at the end of the command
    if  (index_of_ampersand != myargc -1) {
        perror("'&' was NOT found at the end of the command\n");
        exit(EXIT_FAILURE);

    } else {
        // '&' is at the end of the command therefore delegate to execute_commands
        myargv[myargc - 1] = NULL;
        myargc--;
        execute_commands(myargv, myargc, 1);
    } // end if-else
} // end bg_process()


// #5
// first_redirect_output() redirect the standard output of a command to a file
void first_redirect_output(char *myargv[], int myargc) {
    int index = arg_found(myargv, ">", myargc);

    // check the correct position of ">"
    if (index != myargc -2 || myargc < 3) {
        perror("The standard output redirection error... '>' is not in proper position.\n");
        exit(EXIT_FAILURE);
    }

    char *left_side_arg[index+1];
    int i = 0;

    while (index > i) {
        left_side_arg[i] = myargv[i];
        i++;
    }

    left_side_arg[index] = NULL;
    int out = dup(STDOUT_FILENO);

    int new_fd = open(myargv[myargc-1], O_RDWR|O_CREAT|O_TRUNC, 0644);     // new file descriptor

    if (new_fd < 0) {
        perror("Error opening file...");
        exit(EXIT_FAILURE);

    } else {
        dup2(new_fd, STDOUT_FILENO);
        execute_commands(left_side_arg, index, 0);
        close(new_fd);

//        if (close(new_fd) < 0) {
//            perror("Error closing file...\n");
//            exit(EXIT_FAILURE);
//        }

        dup2(out, STDOUT_FILENO);

    }
} // end first_redirect_output()


// #6
// second_redirect_output() redirect the standard output of a command to a file
void second_redirect_output(char *myargv[], int myargc) {
    int index = arg_found(myargv, ">>", myargc);

    // check if ">>" is in correct position
    if (index != myargc - 2 || myargc < 3) {
        perror("The standard output redirection error... '>>' is not in proper position.\n");
        exit(-1);
    }

    char *left_side_arg[index+1];
    int i = 0;

    while (index > i) {
        left_side_arg[i] = myargv[i];
        i++;
    }

    left_side_arg[index] = NULL;
    int out = dup(STDOUT_FILENO);

    int new_fd = open(myargv[myargc -1], O_RDWR| O_CREAT| O_APPEND, 0644);

    if (new_fd < 0) {
        perror("Error opening file...");
        exit(-1);
    } else {
        dup2(new_fd, STDOUT_FILENO);
        execute_commands(left_side_arg, index, 0);
        close(new_fd);
        dup2(out, STDOUT_FILENO);

        //if (close(new_fd) < 0) {
        //    perror("Error closing file...\n");
        //    exit(EXIT_FAILURE);
        //}
    }
} // end second_redirect_output()


// #7
// redirect_input() redirects the standard input of a command to come from a file
void redirect_input(char *myargv[], int myargc) {
    int index = arg_found(myargv, "<", myargc);

    // check if '<' is in proper position
    if (index != myargc -2 || myargc < 3) {
        perror("The standard input redirection error... '<' is not in proper position.\n");
        exit(-1);
    }

    char *left_side_arg[index + 1];
    int i = 0;
    while (index > i) {
        left_side_arg[i] = myargv[i];
        i++;
    }

    left_side_arg[index] = NULL;


    // open file to read
    int new_fd = open(myargv[myargc-1], O_RDONLY);
    if (new_fd < 0) {
        perror("Error opening file...");
        exit(-1);

    } else {
        dup2(new_fd, STDIN_FILENO);
        execute_commands(left_side_arg, index, 0);
        close(new_fd);

//        if (close(new_fd) < 0) {
//            perror("Error closing file...\n");
//            exit(EXIT_FAILURE);
//        }

    }

} // end redirect_input()


// #8
// execute multiple commands connected by a single shell pipe
void single_piped_command(char *myargv[], int myargc) {

    int index = 0, left_side_arg = 0;
    int k, fds[2];
    int prev = -1, fds_file_in = -1, fds_file_out = -1;
    pid_t pid;
    char *left_arg[BUFFERSIZE];


    while (myargc > index) {
        if (strcmp(myargv[index], "|") == 0 || index == myargc -1) {

            if (prev > 0) {
                k = prev + 1;

            } else {
                k = 0;
            } // end if-else
        } // enf if

        if (myargc-1 > index) {
            prev = index;

            while (index > k) {
                left_arg[left_side_arg++] = myargv[k];
                k++;
            }
            left_arg[left_side_arg] = NULL;

            // Begin new pipe
            // connect fds_file_out to the write end of the pipe
            fds_file_out = fds[1];

        } else {
            while (index >= k) {
                left_arg[left_side_arg++] = myargv[k];
                k++;
            }
            left_arg[left_side_arg] = NULL;
            fds_file_out = -1;

        } // end if-else

        // Begin new process
        pid = fork();

        if (pid == 0) {
            // new process creation was successful next execute commands
            if (fds_file_in != 0 && fds_file_in != -1) {
                dup2(fds_file_in, STDIN_FILENO);
                close(fds_file_in);

            }

            if (fds_file_out != -1) {
                dup2(fds_file_out, STDOUT_FILENO);
                close(fds_file_out);
            }

            execute_commands(left_arg, left_side_arg, 0);
            exit(0);

        } else if (pid > 0) {
            // wait for child process to complete
            waitpid(pid, NULL, 0);
            close(fds_file_in);
            close(fds_file_out);
            fds_file_in = fds[0];

        } else {
            // pid < 0
            perror("Error creating new process...");
            exit(-1);

        } // end if -else if -else

        index++;
    } // end while loop
} // end single_piped_command()




// #9
// pwd() function prints working directory
void pwd() {
    char path_name[BUFFERSIZE];

    if (getcwd(path_name, sizeof(path_name)) != NULL) {
        printf("%s\n", path_name);
    } else {
        perror("Error printing working directory...\n");
        exit(EXIT_FAILURE);
    } // end if-else

} // end pwd()


// #10
// change_dir() function changes directory
void change_dir(const char *path_name) {

    if (chdir(path_name) != 0) {
        perror("Error changing directory...\n");
        exit(EXIT_FAILURE);
    }
} // end change_dir()

// ls()
//void ls() {
//    printf("Printing ls...\n");
//
//} // end ls()
