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
    char *fifo_write = "/tmp/mystore_server.dat";
    char fifo_read[40];
    char send_message[100], read_message[500];
    int fd_write, fd_read, n_read;

    char *message = (char *)calloc(1, sizeof(char));
    message[0] = ' ';
    int i;
    for(i = 0; i < argc; i++) asprintf(&message, "%s %s", message, argv[i]);

    // create and open the client's own FIFO for reading
    sprintf(fifo_read, "/tmp/mystore_client_%d.dat", getpid());
    if (mkfifo(fifo_read, 0666) != 0) {
            perror("client mkfifo failed, returns: ");
            return -1;
    }
    
    // open the server's FIFO for writing
    if ((fd_write = open(fifo_write, O_WRONLY)) < 0) {
            perror("Cannot open FIFO to server: ");
            return -1;
    }
    // compose and send write message to server's FIFO
    sprintf(send_message, "%s %s", fifo_read, message);
    free(message);
    write(fd_write, send_message, strlen(send_message));
    close(fd_write);
    
    // open the client's FIFO for reading
    if ((fd_read = open(fifo_read, O_RDONLY)) < 0) {
            perror("Cannot open FIFO to read from server: ");
            return -1;
    }
    
    // read server's reply in client's FIFO
    n_read = read(fd_read, read_message, 500);
    if (n_read >= 0) read_message[n_read] = '\0';
    
    // close and delete client's FIFO
    char *data;
    asprintf(&data, read_message);
    dump = parseInput(data);

    close(fd_read);
    unlink(fifo_read);

    return 0;
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
    if (strchr(subject, ' ') || strchr(message, ' ')) {
        asprintf(&new_subject, "\"%s\"", subject);
        asprintf(&new_message, "\"%s\"", message);
    } else {
        asprintf(&new_subject, "\"%s\"", subject);
        asprintf(&new_message, "\"%s\"", message);
    }
    char *cmd[3] = {"add", new_subject, new_message};
    runCommand(3, cmd);
}

//Edit twig in the database (command)
void editTwig(int index, char *subject, char *message) {
    char num[5];
    sprintf(num, "%d", index + 1);
    char *new_subject, *new_message;
    if (strchr(subject, ' ') || strchr(message, ' ')) {
        asprintf(&new_subject, "\"%s\"", subject);
        asprintf(&new_message, "\"%s\"", message);
    } else {
        new_subject = subject;
        new_message = message;
    }
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
