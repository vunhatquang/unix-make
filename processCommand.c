/*
* File: processCommand.c
* This file contains methods to execute commands in different form
*/

#include "mymake.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>

/*
*  char *get_pipe_cmd(char *command, int num): get the command number num and return that command
*/
char *get_pipe_cmd(char *command, int num) {
    int count = 0;
    char *ptr, *ret_val, *endptr;
    char temp = 'p'; 

    
    if (num == 0) {
        ptr = command;
        while (*ptr != '|') {
            ptr++;
        }
        
        *(ptr - 1) = '\0';
        ret_val = strdup(command);
        if (ret_val == NULL) {
            fprintf(stderr, "Out of memory at get pipecmd\n");
            exit(1);
        }
        *(ptr - 1) = ' ';
        return ret_val;
    } else {
        ptr = command;
        while (1) {
            if (*ptr == '|' && count == num - 1)
                break;
            if (*ptr == '|')
                count++;
            ptr++;
        }
        ptr++;
        for (endptr = ptr + 1; *endptr != '|' && *endptr != '\0'; endptr++);
        temp = *endptr;
        *endptr = '\0';
        if (*(endptr - 1) == ' ')
            *(endptr - 1) = '\0';
        ret_val = strdup(ptr + 1);
        if (ret_val == NULL) {
            fprintf(stderr, "Out of memory at get pipecmd\n");
            exit(1);
        }
        if (*(endptr - 1) == '\0')
            *(endptr - 1) = ' ';
        *endptr = temp;
        return ret_val;
    }
}

/*
* char *findPath(char *file_name): look through the paths to see where file_name is in
*/
char *findPath(char *file_name) {
    // absolute path
    if (*file_name == '/')
        return strdup(file_name);
    char *new_str;
    struct stat fileStat;
    path *cur_path;
    cur_path = head_path;

    while (cur_path != NULL) {
        if((new_str = malloc(strlen(file_name)+strlen(cur_path->str)+strlen("/")+1)) != NULL){
            new_str[0] = '\0';   // ensures the memory is an empty string
            strcat(new_str,cur_path->str);
            strcat(new_str,"/");
            strcat(new_str,file_name);
        } else {
            fprintf(stderr,"malloc failed at findPath!\n");
            return NULL;
        }
        if (stat(new_str, &fileStat) != -1) {
            if (S_ISREG(fileStat.st_mode) && (fileStat.st_mode & S_IXUSR)) {
                return new_str;
            } else {
                printf("File '%s' exists but is not executable.\n", new_str);
                free(new_str);
                exit(EXIT_FAILURE);
            }
        }
        free(new_str);
        cur_path = cur_path->next;
    }

    return NULL;
}

/*
*   int count_arg(char *command): given a command, this function will count the number of arguments the command have
*/

int count_arg(char *command) {
    int ret_val = 0;
    char *ptr;
    ptr = command;
    while (*ptr != '\0') {
        // check for string
        if (*ptr == '"') {
            ptr++;
            while (*ptr != '"') {
                ptr++;
            } 
        }
        // check for space
        if (*ptr == ' ') {
            ret_val++;
            for (; *ptr == ' '; ptr++);
        } else {
            ptr++;
        }

        if (*ptr == '\0')
            ret_val++;
    }
    return ret_val;
}

/*
*  void extract_arg(char *command, int count, char *ret_val[]): extract argument and store it to the given array of string to use
*  for execv
*/

void extract_arg(char *command, int count, char *ret_val[]) {
    char temp = 's';
    char *ptr, *endptr;
    int i = 0;

    ptr = command;
    while (*ptr != '\0') {
        if (*ptr == ' ')
            ptr++;
        else if (*ptr == '"') {
            endptr = ++ptr;
            for (;*endptr != '"'; endptr++);
            temp = *endptr;
            *endptr = '\0';
            ret_val[i++] = strdup(ptr);
            *endptr = temp;
            ptr = ++endptr;
        } else {
            endptr = ptr + 1;
            for (;*endptr != ' ' && *endptr != '\0'; endptr++);
            temp = *endptr;
            *endptr = '\0';
            ret_val[i++] = strdup(ptr);
            *endptr = temp;
            ptr = ++endptr;
        }
    }
    
    ret_val[count] = NULL;
}

/*
*   int simple_command(char *command): this function will execute a simple command
*/
int simple_command(char *command) {
    int count = count_arg(command);
    char *argv[count + 1];
    extract_arg(command, count, argv);
    char *ptr, *name;
    char temp;
    int i;
    pid_t pid;

    for (ptr = command; *ptr != ' ' && *ptr != '\0'; ptr++);
    temp = *ptr;
    *ptr = '\0';
    name = strdup(command);
    if (name == NULL) {
        fprintf(stderr, "Out of memory at simple command\n");
        exit(EXIT_FAILURE);
    }
    
    *ptr = temp;
    argv[0] = findPath(name);

    // fork here
    pid = fork();
    if (pid == -1) {
        // Fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    }

    

    if (pid == 0) {
        // execute the command
        if(execv(argv[0], &argv[0]) == -1) {
			fprintf(stderr, "Command execution failed. at simple_command\n");
            exit(EXIT_FAILURE);
		} 
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        int status;

        // Wait for the child to terminate and get its exit status
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {

            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                fprintf(stderr, "Child process failed with exit status: %d\n", exit_status);
                exit(exit_status);
            }
        } else {
            fprintf(stderr, "Child process did not terminate normally.\n");
            exit(1);
        }
    }

    for (i = 0; i < count; i++) {
        free(argv[i]);
    }
    free(name);
    return 0;
}

/*
*  int multiple_command(command): This function will excute multiple command sequentially in order
*/
int multiple_command(char *command) {
    char *cmd_ptr, *end_ptr, *space_ptr, *name, *argm;
    char temp = 's';
    // fork here
    pid_t pid = fork();
    if (pid == -1) {
        // Fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        cmd_ptr = command;
        while (1) {
            for (end_ptr = cmd_ptr; *end_ptr != ';' && *end_ptr != '\0'; end_ptr++);
            for (space_ptr = cmd_ptr; !isspace(*space_ptr) && *space_ptr != '\0'; space_ptr++);
            
            temp = *space_ptr;
            *space_ptr = '\0';
            name = strdup(cmd_ptr); // TODO free name
            if (name == NULL) {
                fprintf(stderr, "Out of memory at multiple command\n");
                exit(EXIT_FAILURE);
            }
            *space_ptr = temp;
            temp = *end_ptr;
            *end_ptr = '\0';
            if (strcmp(name, "cd") == 0) {
                free(name);
                for (;isspace(*space_ptr); space_ptr++);
                argm = strdup(space_ptr);
                if (argm == NULL) {
                    fprintf(stderr, "Out of memory at multiple command\n");
                    exit(EXIT_FAILURE);
                }
                // change directory
                if (chdir(argm) < 0) {
                    free(argm);
                    perror("chdir");
                    exit(EXIT_FAILURE);
                }
                free(argm);
            } else {
                free(name);
                argm = strdup(cmd_ptr);
                if (argm == NULL) {
                    free(argm);
                    fprintf(stderr, "Out of memory at multiple command\n");
                    exit(EXIT_FAILURE);
                }
                simple_command(argm);
                free(argm);
            }

            *end_ptr = temp;
            
            if (*end_ptr == '\0')
                break;
            for (cmd_ptr = ++end_ptr; isspace(*cmd_ptr); cmd_ptr++);
        }

        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        int status;

        // Wait for the child to terminate and get its exit status
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {

            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                fprintf(stderr, "Child process failed with exit status: %d\n", exit_status);
                return exit_status;
            }
        } else {
            fprintf(stderr, "Child process did not terminate normally.\n");
            return 1;
        }
    }

    return 0;
}

/*
*  int pipe_command(char *command): This function will excute multiple piped commands
*/
int pipe_command(char *command) {
    char *ptr, *cmd, *name;
    char temp = 'p';
    int pipe_count = 0;
    int temp_id = 0;
    int pid, i, j;
    for (ptr = command; *ptr != '\0'; ptr++) {
        if (*ptr == '|' && isspace(*(ptr - 1)) && isspace(*(ptr + 1))) {
            pipe_count++;
        }
    }
    int fd[pipe_count][2];
    for (i = 0; i < pipe_count; i++) {
        if (pipe(fd[i]) == -1) {
            return 1;
        }
    }
    int pids[pipe_count + 1];

    while (temp_id < pipe_count + 1) {
        pid = fork();
        if (pid == -1) {
            // Fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // child
            // change file descriptor
            if (temp_id != 0) {
                dup2(fd[temp_id - 1][0], STDIN_FILENO);
            } 

            if (temp_id != pipe_count) {
                dup2(fd[temp_id][1], STDOUT_FILENO);
            } 

            // close dont use file
            for (i = 0; i < pipe_count; i++) {
                for (j = 0; j < 2; j++) {
                    if (i == temp_id && j == 1)
                        continue;
                    if (i == temp_id - 1 && j == 0)
                        continue;
                    close(fd[i][j]);
                }
            }
            //get cmd
            cmd = get_pipe_cmd(command, temp_id);
            int count = count_arg(cmd);
            char *argv[count + 1];
            extract_arg(cmd, count, argv);
            for (ptr = cmd; *ptr != ' ' && *ptr != '\0'; ptr++);
            temp = *ptr;
            *ptr = '\0';
            name = strdup(cmd);
            if (name == NULL) {
                fprintf(stderr, "Out of memory at pipe command\n");
                exit(EXIT_FAILURE);
            }
            *ptr = temp;
            for (; *ptr == ' ' && *ptr != '\0'; ptr++);
            argv[0] = findPath(name);

            
            
            // execute the command
            if(execv(argv[0], &argv[0]) == -1) {
                free(cmd);
                free(name);
                fprintf(stderr, "Command execution failed at pipe command\n");
                exit(EXIT_FAILURE);
            } 
            exit(EXIT_SUCCESS);
        } else {
            // parent
            pids[temp_id++] = pid;
        }
    }

    // close dont use file from parent side
    for (i = 0; i < pipe_count; i++) {
        for (j = 0; j < 2; j++) {
            close(fd[i][j]);
        }
    }

    // wait for children
    for (i = 0; i < pipe_count + 1; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)) {

            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                fprintf(stderr, "Child process failed with exit status: %d\n", exit_status);
                return exit_status;
            }
        } else {
            fprintf(stderr, "Child process did not terminate normally with temp_id %d and pid%d\n", i, pids[i]);
            return 1;
        }
    }
    return 0;
}

/*
* int redi_command(char *command)
*/
int redi_command(char *command) {
    char *ptr, *cmd, *file, *name;
    int pid;
    char dir = '>';
    char temp = '<';

    for (ptr = command; *ptr != '<' && *ptr != '>'; ptr++);
    dir = *ptr;
    ptr--;
    for (;*ptr == ' ';ptr--);
    ptr++;
    temp = *ptr;
    *ptr = '\0';
    cmd = strdup(command); 
    *ptr = temp;
    for (;*ptr != '<' && *ptr != '>'; ptr++);
    ptr++;
    for (;*ptr == ' '; ptr++);
    file = strdup(ptr); 

    pid = fork();
    if (pid == -1) {
        // Fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    } 

    if (pid == 0) {        
        // Child process

        // redirection
        if (dir == '>') 
            close(STDOUT_FILENO);
        else if (dir == '<')
            close(STDIN_FILENO);

        open (file, O_RDWR | O_CREAT | O_TRUNC, 0777);
        free(file);

        int count = count_arg(cmd);
        char *argv[count + 1];
        extract_arg(cmd, count, argv);
        for (ptr = cmd; *ptr != ' ' && *ptr != '\0'; ptr++);
        temp = *ptr;
        *ptr = '\0';
        name = strdup(cmd);
        if (name == NULL) {
            fprintf(stderr, "Out of memory at pipe command\n");
            exit(EXIT_FAILURE);
        }
        *ptr = temp;
        for (; *ptr == ' ' && *ptr != '\0'; ptr++);
        argv[0] = findPath(name);

        // execute the command
        if(execv(argv[0], &argv[0]) == -1) {
            free(cmd);
            free(name);
            fprintf(stderr, "Command execution failed at pipe command\n");
            exit(EXIT_FAILURE);
        } 
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        int status;

        // Wait for the child to terminate and get its exit status
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {

            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                fprintf(stderr, "Child process failed with exit status: %d\n", exit_status);
                return exit_status;
            }
        } else {
            fprintf(stderr, "Child process did not terminate normally.\n");
            return 1;
        }
    }
    return 0;
}

/*
*   int process_command(char *command): This function will take in input from graphstruct and pass to the right 
*   function in this file to execute the command correctly
*/
int process_command(char *command) {

    char *ptr;
    // type of command s: simple, m: multiple, p: pipe, r: I/O
    char t = 's';

    for (ptr = command; *ptr != '\0'; ptr++) {
        if (*ptr == '|' && *(ptr + 1) == ' ' && *(ptr - 1) == ' ') {
            t = 'p';
            break;
        } 
        if (*ptr == '>' || *ptr == '<') {
            t = 'r';
            break;
        }
        if (*ptr == ';') {
            t = 'm';
            break;
        }
    }


    if (t == 's') {
        return simple_command(command);
    } else if (t == 'm') {
        return multiple_command(command);
    } else if (t == 'p') {
        return pipe_command(command);
    } else {
        return redi_command(command);
    }

    return 0;
}
