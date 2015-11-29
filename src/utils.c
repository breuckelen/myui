#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
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
int runCommand(int argc, char *argv[]) {
    char send_buffer[MAX_BUF], receive_buffer[MAX_BUF];
    int portno;

    char *message = (char *)calloc(1, sizeof(char));
    message[0] = ' ';
    int i;
    for(i = 0; i < argc; i++) asprintf(&message, "%s %s", message, argv[i]);
    asprintf(&message, "%s", message);
    char *new_argv[] = {message};

    portno = 51000;
    gather_message(send_buffer, new_argv, 1, MAX_BUF);
    receive_buffer[0] = '\0';

    if (send_to_server("localhost", portno, send_buffer, receive_buffer, MAX_BUF) < 0) {
        printf("Client: ERROR in send_to_server\n");
        return -1;
    }

    char *data;
    asprintf(&data, receive_buffer);
    dump = parseInput(data);

    return 0;
}

// Gather message
void gather_message(char *buffer, char **args, int nargs, int max_buf) {
    int i;
    int n = 0;
    char *sout = buffer;
    char *sin, c;

    for (i = 0; i < nargs; ++i) {
        sin = args[i];
        while ((c = *sin++) && n < max_buf - 1) {
            if (c != '\\') {
                *sout++ = c;
                ++n;
            }
            else {
                c = *sin;
                if (c == '\\' || c == 'n' || c == 'r' || c == 't') {
                    if (c == '\\') *sout++ = '\\';
                    else if (c == 'n') *sout++ = '\n';
                    else if (c == 't') *sout++ = '\t';
                    else if (c == 'r') *sout++ = '\r';
                    sin++;
                    n++;
                }
                else {
                    *sout++ = '\\';
                    *sout++ = c;
                    ++sin;
                    n += 2;
                }
            }
        }
    }
    *sout = '\0';
}

//Get the number of twigs (command)
int numTwigs() {
    char *cmd[1] = {"stat"};
    runCommand(1, cmd);
    return atoi(dump[3].value);
}

//Load the twigs into an array (command)
void loadTwigs() {
    int i;
    for(i = 0; i < twigs_size; i++) {
        char num[5];
        sprintf(num, "%d", i + 1);
        char *cmd[2] = {"display", num};
        runCommand(2, cmd);
        Twig twig = {
            .index = atoi(dump[1].value) - 1,
            .date = dump[2].value,
            .subject = dump[3].value,
            .message = dump[4].value
        };
        twigs[i] = twig;
    }
}

//Add twig to the database (command)
void addTwig(char *subject, char *message) {
    char *new_subject, *new_message;
    asprintf(&new_subject, "\"%s\"", subject);
    asprintf(&new_message, "\"%s\"", message);
    char *cmd[3] = {"add", new_subject, new_message};
    runCommand(3, cmd);
}

//Edit twig in the database (command)
void editTwig(int index, char *subject, char *message) {
    char num[5];
    char *new_subject, *new_message;
    sprintf(num, "%d", index + 1);
    asprintf(&new_subject, "\"%s\"", subject);
    asprintf(&new_message, "\"%s\"", message);
    char *cmd[4] = {"edit", num, new_subject, new_message};
    runCommand(4, cmd);
}

//Delete a twig (command)
void deleteTwig(int index) {
    char num[5];
    sprintf(num, "%d", index + 1);
    char *cmd[2] = {"delete", num};
    runCommand(2, cmd);
}
