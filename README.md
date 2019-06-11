# LinuxCommandLineInterpreter

```
Implementation of a Shell or Linux Command Line Interpreter based on Nutt, Kernel Projects for Linux.
```

## Build Instructions

```
gcc myshell.c -o myshell
```

## Run Instructions

```
 ./myshell (Windows OS)
```

## Description:

```
The goal of the Shell or Linux Command Line Interpreter is to implement our own Shell or Command Line Interpreter.
The shell gets user input into a buffer, then it is parsed and tokenized.
Based on the token, the shell will fork off processes to execute user commands.
The shell implementation supports following instructions:

#1. Execute a single command with upto four command line arguments
For example:
myShell>> ls -l
myShell >> cat myfile
myShell >> ls-al /usr/src/linux

#2. Execute a command in background.
For example:
myShell >>ls -l &
myShell >>ls-al /usr/src/linux

#3. Redirect the standard output of a command to a file.
For example:
myShell >>ls -l > outfile
myShell >>ls -l >> outfile
myShell >>ls -l /usr/src/linux > outfile2
myShell >>ls -l /usr/src/linux >> outfile2

#4. Redirect the standard input of a command to come from a file.
For example:
myShell >> grep disk < outfile
myShell >>grep linux < outfile2

#5. Execute multiple commands connected by a single shell pipe.
For example:
myShell >>ls -al /usr/src/linux | grep linux

#6. Execute cd and pwd execute_commands.
myShell >> cd <file_path_name>
myShell >> pwd

```

## Implementation

```
1. First, I implemented shell#0. The goal was to declare and initialize variable and
   prompt the user for input, read the input and echo back to the user within do-while loop.
   The program exited only when user press Ctrl + D.

2. Second, I implemented shell#1. I extended shell#0 and added further feature to parse and tokenize
   the user input and store it into a myargv array along with a count of the number of strings in an
   int myargc variable. Also the exit feature was added so the user can simply enter 'exit' to terminate
   the shell.

3. Third, I built upon shell#1 to create a child process to execute the commands. I passed in myarv/        myargc as arguments to execvp() function and then wait for the child process to complete execution.
```
