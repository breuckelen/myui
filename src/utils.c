#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "shelter.h"

//Twig vars
Twig *twigs;
int twigs_size;
KeyValue *dump;

//Read text from file input stream
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

//Get the number of key val pairs in a string
int numPairs(char *data) {
    int count = 0;
    while(*data) {
        if(*data++ == '|') {
            count++;
        }
    }
    return count / 2;
}

//Parse a string and return a key value pair array
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

//Run a command from mystore
int runCommand(int argc, char **argv) {
    int pid, status;
    int mypipe[2];
     
     //create the pipe
    if (pipe(mypipe) == -1) {
        perror("Error creating the pipe\n");
        return 1;
    }
               
     //fork
    pid = fork();
    if(pid == -1) {
        perror("Error forking\n");
        exit(1);
    } else if(pid == 0) {
        char ** newargv = (char **)calloc(argc + 1, sizeof(char *));
        int i;
         
        close(mypipe[0]);
        dup2(mypipe[1], STDOUT_FILENO);
         
        for(i = 0; i < argc; i++) {
            newargv[i + 1] = argv[i];
        }
        newargv[0] = "./build/mystore";
        execvp(newargv[0], newargv);
         
        close(mypipe[1]);
        exit(0);
    } else {
        FILE *fp;
        char *buffer;
         
        close(mypipe[1]);
        wait(&status);
         
        if ((fp = fdopen(mypipe[0], "r")) == NULL) {
            perror("Error opening file descriptor\n");
        }
        buffer = readInput(fp);
        dump = parseInput(buffer);
        close(mypipe[0]);
    }
}

//Get the number of twigs (command)
int numTwigs() {
    char *cmd[2] = {"stat", NULL};
    runCommand(2, cmd);
    return atoi(dump[3].value);
}

//Load the twigs into an array (command)
void loadTwigs() {
    int i;
    for(i = 0; i < twigs_size; i++) {
        char num[5];
        sprintf(num, "%d", i + 1);
        char *cmd[3] = {"display", num, NULL};
        runCommand(3, cmd);
        Twig twig = {
            .date = dump[2].value,
            .subject = dump[3].value,
            .message = dump[4].value
        };
        twigs[i] = twig;
    }
}

//Add twig to the database (command)
void addTwig(char *subject, char *message) {
    Twig twig = {
        .subject = subject,
        .message = message
    };
    twigs[twigs_size++] = twig;
    char *cmd[4] = {"add", subject, message, NULL};
    runCommand(4, cmd);
}

//Edit twig in the database (command)
void editTwig(int index, char *subject, char *message) {
    char num[5];
    sprintf(num, "%d", index + 1);
    char *cmd[5] = {"edit", num, subject, message, NULL};
    runCommand(5, cmd);
}

//Delete a twig (command)
void deleteTwig(int index) {
    char num[5];
    sprintf(num, "%d", index + 1);
    char *cmd[3] = {"delete", num, NULL};
    runCommand(3, cmd);
}
