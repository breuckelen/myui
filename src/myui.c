#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "myui.h"


char * readInput(FILE *fp) {
    char * rawData = (char *)calloc(CHUNK_SIZE + 1, sizeof(char));
    char * temp = (char *)calloc(CHUNK_SIZE + 1, sizeof(char));
    int count = 0;

    while(fgets(temp, CHUNK_SIZE, fp) != NULL) {
        rawData = (char *)realloc(rawData, CHUNK_SIZE * ++count * sizeof(char) + 1);
        strcat(rawData, temp);
    }

    return rawData;
}


int numPairs(char *data) {
    int count = 0;
    while(*data) {
        if(*data++ == '|') {
            count++;
        }
    }
    return count / 2;
}


KeyValue * parseInput(char *data) {
    int size = numPairs(data);
    KeyValue * dict = (KeyValue *)calloc(size + 1, sizeof(KeyValue));
    KeyValue * copy = dict;

    int isKey = 1;
    while (*data) {
        if(*data == '|') {
            if(isKey) {
                dict->key = ++data;
            } else {
                *data = '\0';
                isKey = 1;
            }
        } else if(*data == ' ') {
            if(isKey) {
                (dict++)->value = ++data;
                isKey = 0;
            }
        } else if(*data == ':') {
            if(isKey) {
                *data = '\0';
            }
        }
        data++;
    }

    return copy;
}


void printReverse(KeyValue *dict, int len) {
    int i;

    printf("{\n");
    for(i = len - 1; i >= 0; i--) {
        printf("\t%s: %s\n", dict[i].key, dict[i].value);
    }
    printf("}\n");
}


int main(int argc, char **argv) {
    int pid, status;
    int mypipe[2];

    // create the pipe
    if (pipe(mypipe) == -1) {
        perror("Error creating the pipe\n");
        return 1;
    }
          
    // fork
    pid = fork();
    if(pid == -1) {
        perror("Error forking\n");
        exit(1);
    } else if(pid == 0) {
        // while loop to continually read data, send when enter is pressed
        char ** newargv = (char **)calloc(argc + 1, sizeof(char *));
        int i;

        close(mypipe[0]);
        dup2(mypipe[1], STDOUT_FILENO);

        for(i = 1; i < argc; i++) {
            newargv[i] = argv[i];
        }
        newargv[0] = "./build/mystore";
        newargv[argc] = NULL;
        execvp(newargv[0], newargv);

        close(mypipe[1]);
        exit(0);
    } else {
        FILE * fp;
        char * buffer;
        int n;

        close(mypipe[1]);
        wait(&status);

        if ((fp = fdopen(mypipe[0], "r")) == NULL) {
            perror("Error opening file descriptor\n");
        }
        buffer = readInput(fp);
        int len = numPairs(buffer);
        printReverse(parseInput(buffer), len);

        close(mypipe[0]);
    }
    exit(0);
}
