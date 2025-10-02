/*
*  File: processFile.c
*  This file contains method to process makefile  
*/

#include "mymake.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



/* 
*  char *trimEndandComment(char *str) removes white space in the end of given str and
*  detect and eliminate comment
*/
char *trimEndandComment(char *str) {
    int len;
    for (len = 0; len < strlen(str); len++) {
        if (str[len] == '#') {
            str[len] = '\0';
            break;
        }
    }

    len = strlen(str)-1;
    while (len >= 0 && isspace(str[len])) 
        --len;
    str[++len] = '\0';
    return str;
} 

/*
*  isMacro(char *str): given a str, this function will check whether the given string is macro or not
*/
int isMacro(char *str) {
    int len;
    int inString = 0;  // This variable is used to track whether the equal sign is in a string or not
    for (len = 0; len < strlen(str); len++) {
        if (str[len] == '"')
            inString = !inString;
        if (str[len] == '=' && !inString) {
            return 1;
        }
    }
    return 0;
}

/* int readFile(FILE *mfile)
   reads the make file and builds the dependency graph. If there is an
   error in the file it returns 0, otherwise it returns 1
*/
int readFile(FILE *mfile) {
    char *buff=NULL;
    char *ptr, *endPtr, *tptr;
    char *tempPtr, *prevPtr;
    size_t sz=0;
    node *lastNode=NULL;
    int done;

    while(getline(&buff, &sz, mfile) > 0) {
        trimEndandComment(buff);
        if (strlen(buff) == 0)
            continue;		//blank line
        ptr = buff;
        while (isspace(*ptr))
            ++ptr;
        // check to add command if it detect a tab
        if (buff[0] == '\t') {
            // line detect as command
            if (lastNode == NULL) {
                fprintf(stderr,"Error in file, command with no target\n");
                free(buff);
                return 0;
            }
            if (!addCommand(lastNode, ptr)) {
                free(buff);
                return 0;
            }
        } else if (isMacro(ptr)) {
            // line detect as macro 
            for (endPtr = ptr; *endPtr != '='; ++endPtr);
            // extract target to ptr
            
            //trim white space off end of target
            tptr = endPtr-1;
            while (isspace(*tptr)) {
                *tptr = '\0';
                --tptr;
            }
            *endPtr = '\0'; // change value at '=' to '\0' 

            // extract value to endPtr
            ++endPtr;
            while (isspace(*endPtr))
                ++endPtr;
            
            if (!addMacro(ptr, endPtr)) {
                free(buff);
                return 0;
            }
        } else { 
            // line detect as target rules
            for (endPtr = ptr; *endPtr != '\0' && *endPtr != ':'; ++endPtr) ;
            if (*endPtr != ':') {
                fprintf(stderr,"Illegal line: %s\n", buff);
                free(buff);
                return 0;
            }
        
            //check for second :
            for (tptr = endPtr + 1; *tptr != '\0'; ++tptr) {
                if (*tptr == ':') {
                    fprintf(stderr,"Illegal line: %s\n", buff);
                    free(buff);
                    return 0;
                }
            }

            *endPtr = '\0';
            //trim white space off end of target
            tptr = endPtr-1;
            while (isspace(*tptr)) {
                *tptr = '\0';
                --tptr;
            }

            if (*ptr == '\0') {	//missing target
                fprintf(stderr,"No target in rule\n");
                free(buff);
                return 0;
        }


            if ((lastNode = findNode(ptr)) == NULL) {
                if (!(lastNode = addNode(ptr))) {
                    free(buff);
                    return 0;
                }
            } else if (lastNode->target) {
                fprintf(stderr,"Repeated target: %s\n", ptr);
                free(buff);
                return 0;
            }
        
            lastNode->target = 1;

            //extract target 
            tempPtr = strdup(ptr);
            prevPtr = tempPtr;
            tptr = tempPtr;
            while (*tptr != '\0') {
                if(isspace(*tptr)) {
                    *tptr = '\0';
                    addTarget(lastNode, prevPtr);
                    *tptr = ' ';
                    while (isspace(*(++tptr)));
                    prevPtr = tptr;
                } else {
                    ++tptr;
                }
            }
            addTarget(lastNode, prevPtr);
            free(tempPtr);
            

            done = 0;
            while (!done) {
                ptr = endPtr+1;
                while (isspace(*ptr))
                    ++ptr;
                if (*ptr == '\0')
                    break;
                endPtr = ptr+1;
                while (*endPtr != '\0' && !isspace(*endPtr))
                    ++endPtr;
                if (*endPtr == '\0')
                    done = 1;
                *endPtr = '\0';
                if (!addEdge(lastNode, ptr)) {
                    free(buff);
                    return 0;
                }
            } 
        }
    }
  
    free(buff);
    return 1;
}