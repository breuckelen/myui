#pragma once
#define CHUNK_SIZE 50
#define MAX_BUF	10000

struct KeyValue {
    char * key;
    char * value;
};
typedef struct KeyValue KeyValue;

// other
extern int send_to_server(char *server_name, int portno, char *send_buffer,
        char *receive_buffer, int max_buf);

//parsing
char * readInput(FILE *fp);
int numPairs(char *data);
KeyValue * parseInput(char *data);

//processes
int runCommand(int argc, char *argv[]);
void gather_message(char *buffer, char **args, int nargs, int max_buf);

//commands
int numTwigs();
void loadTwigs();
void addTwig(char *subject, char *message);
void editTwig(int index, char *subject, char *message);
void deleteTwig(int index);
