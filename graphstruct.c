/*
* File: graphstruct.c
* This file contains implementations of graph structure methods, which hold information
* of makefile 
*/

#include "mymake.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utime.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

node *head = NULL;
usermacro *head_mac = NULL;
path *head_path = NULL;
node *infe_node = NULL;

/*
* processPath(): extract paths and store path to head_path structure
*/
path *processPath(char *my_path) {
    char *prev;
    char *cur;
    path *cur_path = head_path;
    path **ptr_ptr = &head_path;
    prev = my_path;
    cur = my_path;
    int exit = 0;

    // extract path from my_path
    while (1) {
        if (*cur == ':' || *cur == '\0') {
            if (*cur == '\0')
                exit = 1;
            *cur = '\0';
            cur_path = malloc(sizeof(path));
            if (cur_path == NULL) {
                fprintf(stderr, "Out of memory when using proccessPath \n");
                return NULL;
            }
            *ptr_ptr = cur_path;
            // set field in new macro 
            cur_path->str = prev;
            cur_path->next = NULL;

            // next
            ptr_ptr = &(cur_path->next);
            cur_path = cur_path->next;
            prev = ++cur;
        } else {
            cur++;
        }

        if (exit)
            break;
    }

    return head_path;

}

/*
*  usermacro *addMacro(char *var_name, char *str): too add new macro to the structure
*/
usermacro *addMacro(char *var_name, char *str) {
    usermacro **ptr_ptr = &head_mac;
    usermacro *ptr = head_mac;

    while (ptr != NULL) {
        if (!strcmp(ptr->name, var_name)) {
            free(ptr->string);
            ptr->string = strdup(str);
            return ptr;
        }
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    if (ptr == NULL) {
        ptr = malloc(sizeof(usermacro));
        if (ptr == NULL) {
            fprintf(stderr, "Out of memory when using addMacro");
            return NULL;
        }
    }

    //set field in new macro
    ptr->name = strdup(var_name);
    ptr->string = strdup(str);
    if (ptr->name == NULL || ptr->string == NULL) {
        fprintf(stderr, "Out of memory when using addMacro");
        return NULL;
    }
    ptr->next = NULL;
    *ptr_ptr = ptr;
    return ptr;
}

/*
*  node addNode(char *name) add a new node to bottom of a linked list and returns a pointer to 
*  the new node. 
*/

node *addNode(char *name) {
    node **ptr_ptr = &head;
    node *ptr = head;
    
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    ptr = malloc(sizeof(node));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using addNode function\n");
        return NULL;
    }
    
    // set name of the new node to the given name
    ptr->name = strdup(name);
    if (ptr->name == NULL) {
        fprintf(stderr, "Out of memory at addNode\n");
        free(ptr);
        return NULL;
    }

    ptr->tars = NULL;
    ptr->deps = NULL;
    ptr->visited = 0;
    ptr->target = 0;
    ptr->commands = NULL;
    ptr->next = NULL;
    *ptr_ptr = ptr;
    return ptr;
}

/*
* targets *addTarget(node *cur, char *tar): add target
*/
targets *addTarget(node *cur, char *tar) {
    targets **ptr_ptr;
    targets *ptr;
    ptr_ptr = &(cur->tars);
    ptr = cur->tars;

    while(ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    ptr = malloc(sizeof(targets));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using addTarget function\n");
        return NULL;
    }

    //set cmd to command
    ptr->str = strdup(tar);
    if (ptr->str == NULL) {
        fprintf(stderr, "Out of memory at addTarget\n");
        free(ptr);
        return NULL;
    }

    *ptr_ptr = ptr;
    ptr->next = NULL;
    return ptr;
}

/*
*  command *addCommand(node * cur, char *cmd) adds new command to the list of commands and return 
*  the command 
*/

command *addCommand(node *cur, char *cmd) {
    command **ptr_ptr;
    command *ptr;
    ptr_ptr = &(cur->commands);
    ptr = cur->commands;

    while(ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    ptr = malloc(sizeof(command));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory when using addCommand function\n");
        return NULL;
    }

    //set cmd to command
    ptr->str = strdup(cmd);
    if (ptr->str == NULL) {
        fprintf(stderr, "Out of memory at addCommand\n");
        free(ptr);
        return NULL;
    }

    *ptr_ptr = ptr;
    ptr->next = NULL;
    return ptr;
}

/*
*  edge *addEdge(node *cur, char *name): add a node to the list of edges pointing to the target
*  and return a pointer to the new edge
*/

edge *addEdge(node *cur, char *name) {
    edge **ptr_ptr;
    edge *ptr;
    node *edgeNode;

    edgeNode = head;
    while (edgeNode != NULL) {
        if (strcmp(edgeNode->name, name) == 0) {
            break;
        }
        edgeNode = edgeNode->next;
    }
    
    if (edgeNode == NULL) {
        // error Out of memory when using addNode function
        if (!(edgeNode = addNode(name))) {
            return NULL;
        }
    }
    ptr_ptr = &cur->deps;
    ptr = cur->deps;
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }
    ptr = malloc(sizeof(edge));
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory at addEdge");
        return NULL;
    }
    *ptr_ptr = ptr;
    ptr->next = NULL;
    ptr->dep = edgeNode;
    return ptr;
}

/*
* node findNode(char *name) look for a node with specific name. If not found, return NULL
*/
node *findNode(char *name) {
    node *cur;
    for (cur = head; cur != NULL; cur = cur->next) {
        if (!strcmp(name, cur->name))
            return cur;
    }
    return cur;
}

/*
*  char *findMacro(char *var_name): return the string correlated with var_name and return NULL if not found
*/
char *findMacro(char *var_name) {
    usermacro *ptr = head_mac;
    while (ptr != NULL) {
        if (!strcmp(ptr->name, var_name)) {
            return ptr->string;
        }
        ptr = ptr->next;
    }
    return NULL;
}

/*
*  node *findTarget(node *n): find node that have target field == 1 and contains the target
*/
node *findTarget(node *n) {
    node *cur;
    targets *cur_target;
    for (cur = head; cur != NULL; cur = cur->next) {
        for (cur_target = cur->tars; cur_target != NULL; cur_target = cur_target->next) {
            if (!strcmp(n->name, cur_target->str))
                return cur;
        }
    }
    return NULL;
}

/*
* char *replaceWord(const char* s, const char* oldW, const char* newW): replace oldW with newW in s
*/

char *replaceWord(const char* s, const char* oldW, const char* newW) { 
    char* result; 
    int i, cnt = 0; 
    int newWlen = strlen(newW); 
    int oldWlen = strlen(oldW); 
 
    // Counting the number of times old word 
    // occur in the string 
    for (i = 0; s[i] != '\0'; i++) { 
        if (strstr(&s[i], oldW) == &s[i]) { 
            cnt++; 
 
            // Jumping to index after the old word. 
            i += oldWlen - 1; 
        } 
    } 
 
    // Making new string of enough length 
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1); 
    i = 0; 
    while (*s) { 
        // compare the substring with the result 
        if (strstr(s, oldW) == s) { 
            strcpy(&result[i], newW); 
            i += newWlen; 
            s += oldWlen; 
        } 
        else
            result[i++] = *s++; 
    } 
 
    result[i] = '\0'; 
    return result; 
}

/*
* char *replaceMacro(char *cmd): this function will replace macro by its correspond string  
*/
char *replaceMacro(char *cmd) {
    char *temp, *ptr, *endptr, *name, *format, *rm_temp;
    char temp_char = ' ';
    usermacro *cur_macro;

    ptr = cmd;
    temp = cmd;
    while (*ptr != '\0') {
        if (*ptr == '$') {
            if (*(++ptr) == '(') {
                endptr = ptr++;
                while (*endptr != ')') {
                    endptr++;
                }

                // set name
                *endptr = '\0';
                name = strdup(ptr);
                if (name == NULL) {
                    fprintf(stderr, "Out of memory at replaceMacro");
                    return NULL;
                }
                *endptr = ')';

                // set format
                endptr++;
                temp_char = *endptr;
                *endptr = '\0';
                ptr -= 2;
                format = strdup(ptr);
                if (format == NULL) {
                    fprintf(stderr, "Out of memory at replaceMacro");
                    free(name);
                    return NULL;
                }
                *endptr = temp_char;
                ptr = endptr--;
            } else {
                endptr = ptr;
                // assume that macro end with space or new line
                while (*endptr != ' ' && *endptr != '\0') {
                    endptr++;
                }

                // set name
                temp_char = *endptr;
                *endptr = '\0';
                name = strdup(ptr);
                if (name == NULL) {
                    fprintf(stderr, "Out of memory at replaceMacro");
                    return NULL;
                }
                *endptr = temp_char;

                // set format
                ptr--;
                temp_char = *endptr;
                *endptr = '\0';
                format = strdup(ptr);
                if (format == NULL) {
                    fprintf(stderr, "Out of memory at replaceMacro");
                    free(name);
                    return NULL;
                }
                *endptr = temp_char;
                ptr = endptr--;
            }
            for (cur_macro = head_mac; cur_macro != NULL; cur_macro = cur_macro->next) {
                if (strcmp(cur_macro->name, name) == 0) {
                    rm_temp = temp;
                    temp = replaceWord(temp, format, cur_macro->string);
                    free(format);
                    free(name);
                    if (strcmp(rm_temp, cmd) != 0)
                        free(rm_temp);
                    break;
                }
            }
        }
        ptr++;
    }
    return temp;
}

/*
* char *replaceSymbol(node *n, char *cmd, int type) : replace $@ for target without suffix and $< for the source
* need to free this char after use
*/
char *replaceSymbol(char *name, char *cmd, int type, char *suffix) {
    char *str_ptr, *sec_ptr, *source, *rm_ptr;
    char * new_str, *new_cmd;
    if (type == 1) {
        for (str_ptr = suffix; *str_ptr != '\0'; str_ptr++) {
            if (*str_ptr == '.') {
                sec_ptr = str_ptr;
            }
        }
        if((new_str = malloc(strlen(sec_ptr)+strlen(name)+1)) != NULL){
            new_str[0] = '\0';   // ensures the memory is an empty string
            strcat(new_str,name);
            strcat(new_str,sec_ptr);
        } else {
            fprintf(stderr,"malloc failed at replace symbol!\n");
            return NULL;
        }
        new_cmd = replaceWord(cmd, "$<", new_str);
        rm_ptr = new_cmd;
        new_cmd = replaceWord(new_cmd, "$@", name);
        free(rm_ptr);
        free(new_str);
        return new_cmd;
    } else {
        for (str_ptr = suffix; *str_ptr != '\0'; str_ptr++) {
            if (*str_ptr == '.') {
                sec_ptr = str_ptr;
            }
        }
        // get suffix of source and store it to target variable
        *sec_ptr = '\0';
        source = strdup(suffix);
        *sec_ptr = '.';

        // create source file name
        if((new_str = malloc(strlen(source)+strlen(name)+1)) != NULL){
            new_str[0] = '\0';   // ensures the memory is an empty string
            strcat(new_str,name);
            strcat(new_str,source);
        } else {
            fprintf(stderr,"malloc failed at replace symbol!\n");
            return NULL;
        }
        new_cmd = replaceWord(cmd, "$<", new_str);
        free(new_str);
        free(source);
        return new_cmd;
    }

    return NULL;
}

/*
*  node *findInference(node *n): return new node that come out from inference rules
*/
node *findInference(node *n) {
    node **ptr_ptr = &(infe_node);
    node *ptr = infe_node;

    node *cur, *new_node;
    char *str_ptr, *sec_ptr, *temp_str;
    char *type = NULL;
    char *new_cmd;
    int count = 0;
    command *cmd;

    // find place to add new inference node
    while (ptr != NULL) {
        ptr_ptr = &(ptr->next);
        ptr = ptr->next;
    }

    // check type of node n
    for (str_ptr = n->name; *str_ptr != '.' && *str_ptr != '\0'; str_ptr++);
    // for (str_ptr = end_ptr; str_ptr >= n->name || *str_ptr != '.'; str_ptr--);]
    if (*str_ptr != '\0') {
        type = str_ptr++;
    }
    for (cur = head; cur != NULL; cur = cur->next) {

        // find inference rule
        str_ptr = cur->name;
        if (*str_ptr != '.')
            continue;

        // count to see which form it is .s1 or .s1.s2
        for (str_ptr = cur->name; *str_ptr != '\0'; str_ptr++) {
            if (*str_ptr == '.') {
                sec_ptr = str_ptr;
                count++;
            }
        }

        if (count == 1) {
            // .s1
            if (type != NULL)
                continue;
            new_node = malloc(sizeof(node));
            if (new_node == NULL) {
                fprintf(stderr, "Out of memory when using findInference function\n");
                return NULL;
            }
            new_node->name = strdup(n->name);
            if (new_node->name == NULL) {
                fprintf(stderr, "Out of memory at findInference\n");
                free(new_node);  
                return NULL;
            }

            new_node->tars = NULL;
            new_node->deps = NULL;
            new_node->visited = 0;
            new_node->target = 0;
            new_node->commands = NULL;
            new_node->next = NULL;
            addTarget(new_node, n->name);
            for (cmd = cur->commands; cmd != NULL; cmd = cmd->next) {
                new_cmd = replaceSymbol(n->name, cmd->str, count, cur->name);
                addCommand(new_node, new_cmd);
                free(new_cmd);
            }
            *ptr_ptr = new_node;
            return new_node;

        } else {
            // count == 2 which is .s1.s2
            if (type == NULL || strcmp(sec_ptr++, type) != 0)
                continue;

            *type = '\0';
            temp_str = strdup(n->name);
            *type = '.';

            new_node = malloc(sizeof(node));
            if (new_node == NULL) {
                fprintf(stderr, "Out of memory when using findInference function\n");
                return NULL;
            }
            new_node->name = strdup(n->name);
            if (new_node->name == NULL) {
                fprintf(stderr, "Out of memory at findInference\n");
                free(new_node);  
                return NULL;
            }

            new_node->tars = NULL;
            new_node->deps = NULL;
            new_node->visited = 0;
            new_node->target = 0;
            new_node->commands = NULL;
            new_node->next = NULL;
            addTarget(new_node, n->name);
            for (cmd = cur->commands; cmd != NULL; cmd = cmd->next) {
                new_cmd = replaceSymbol(temp_str, cmd->str, count, cur->name);
                addCommand(new_node, new_cmd);
                free(new_cmd);
            }
            *ptr_ptr = new_node;
            free(temp_str);
            return new_node;
        }
        
    }
    return NULL;
}

/*
*  void postOrder(node *n): run postorder of target from n
*/
void postOrder(node *n) {
    char *ready_cmd;
    edge *dep;
    command *cmd;
    struct stat statbuf1;
    struct stat statbuf2;
    targets *cur_tar;
    node *temp;

    n->visited = 1;
    if (n->target == 0) {
        temp = findTarget(n);
        if (temp == NULL) {
            temp = findInference(n);
            if (temp == NULL) {
                return;
            }
        }

        n = temp;
        if (n->visited > 0)
            return;
        
        n->visited = 1;
    }
    for (dep = n->deps; dep != NULL; dep = dep->next) {
        if (dep->dep->visited == 1) {
            // circular detect
        } else {
            postOrder(dep->dep);
        }
    }

    // check timestamp if the file already exist
    for (cur_tar = n->tars; cur_tar != NULL; cur_tar = cur_tar->next) {
        if (stat(cur_tar->str, &statbuf1) != -1) {
            for (dep = n->deps; dep != NULL; dep = dep->next) {
                if (stat(dep->dep->name, &statbuf2) != -1) {
                    // if the modify time of file 1 is newer than the modify time of file 2, out of this
                    if (difftime(statbuf1.st_mtime, statbuf2.st_mtime) > 0) {
                        return;
                    }
                }
            }
        }
    }

    
    //printf("%s\n", n->name);
    for (cmd = n->commands; cmd != NULL; cmd = cmd->next) {
        ready_cmd = replaceMacro(cmd->str);
        if (d_flag == 1) {
            printf("%s\n", ready_cmd);
            fflush(stdin);
        }
        
        if (process_command(ready_cmd) != 0 && k_flag == 0) {
            return;
        }
        if (ready_cmd != cmd->str)
            free(ready_cmd);
    }
    n->visited = 2;
}

/*
*  void travGraph(char *t) does a graph traversal starting at target t
*/
void travGraph(char *t) {
    node *cur;
    
    for (cur = head; cur != NULL; cur = cur->next) {
        if (!strcmp(cur->name, t)) {
            break;
        }
    }
    if (cur == NULL || !cur->target) {
        fprintf(stderr, "Target %s not found\n", t);
        return;
    } 
    postOrder(cur);
    return;
}

/*
*   freeGraph(): frees the memory taken by malloc of the graph
*/
void freeGraph() {
    node *nptr, *nptr2;
    edge *eptr, *eptr2;
    command *cptr, *cptr2;
    usermacro *umptr, *umptr2;
    path *paptr, *paptr2;
    targets *taptr, *taptr2;

    // free node 
    nptr = head;
    while (nptr != NULL) {
        nptr2 = nptr->next;
        free(nptr->name);
        cptr = nptr->commands;
        while (cptr != NULL) {
            cptr2 = cptr->next;
            free(cptr->str);
            free(cptr);
            cptr = cptr2;
        }
        taptr = nptr->tars;
        while (taptr != NULL) {
            taptr2 = taptr->next;
            free(taptr->str);
            free(taptr);
            taptr = taptr2;
        }
        eptr = nptr->deps;
        while (eptr != NULL) {
            eptr2 = eptr->next;
            free(eptr);
            eptr = eptr2;
        }
        free(nptr);
        nptr = nptr2;
    }

    // free usermacro 
    umptr = head_mac;
    while (umptr != NULL) {
        umptr2 = umptr->next;
        free(umptr->name);
        free(umptr->string);
        free(umptr);
        umptr = umptr2;
    }

    // free path
    paptr = head_path;
    while (paptr != NULL) {
        paptr2 = paptr->next;
        free(paptr);
        paptr = paptr2;
    }

    // free infe_node
    nptr = infe_node;
    while (nptr != NULL) {
        nptr2 = nptr->next;
        free(nptr->name);
        cptr = nptr->commands;
        while (cptr != NULL) {
            cptr2 = cptr->next;
            free(cptr->str);
            free(cptr);
            cptr = cptr2;
        }
        taptr = nptr->tars;
        while (taptr != NULL) {
            taptr2 = taptr->next;
            free(taptr->str);
            free(taptr);
            taptr = taptr2;
        }
        eptr = nptr->deps;
        while (eptr != NULL) {
            eptr2 = eptr->next;
            free(eptr);
            eptr = eptr2;
        }
        free(nptr);
        nptr = nptr2;
    }

  return;
}


/*
*  void printStruct() use to print out the rules from the database 
*/

void printStruct() {
    char *str_ptr;
    node *cur;
    command *cmd;
    edge *curEdge;
    usermacro *cur_mac;
    int mac_count = 0;
    int node_count = 0;
    int command_count = 0; 
    int infe_count = 0;

    for (cur_mac = head_mac; cur_mac != NULL; cur_mac = cur_mac->next) {
        mac_count = mac_count + 1;
    }
    for (cur = head; cur != NULL; cur = cur->next) {
        if (cur->target != 1)
            continue;

        str_ptr = cur->name;
        if (*str_ptr == '.')
            infe_count = infe_count + 1;
        else
            node_count = node_count + 1;
        for (cmd = cur->commands; cmd != NULL; cmd = cmd->next)
            command_count = command_count + 1;
    }
    printf("%d macros, %d target rules, %d inf. rules, %d cmds\n", mac_count, node_count, infe_count, command_count);


    // print out the macros
    for (cur_mac = head_mac; cur_mac != NULL; cur_mac = cur_mac->next) {
        printf("%s = %s\n", cur_mac->name, cur_mac->string);
    }

    // print out the target rules and inference rules   
    
    for (cur = head; cur != NULL; cur = cur->next) {
        if (cur->target != 1) // we only print the one produce from target 
            continue;
        printf("%s:", cur->name);
        for (curEdge = cur->deps; curEdge != NULL; curEdge = curEdge->next)
            printf(" %s ", curEdge->dep->name);
        printf("\n");
        for (cmd = cur->commands; cmd != NULL; cmd = cmd->next)
            printf("\t%s\n", cmd->str);
    }


}