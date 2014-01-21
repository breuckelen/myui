#pragma once
#define CHUNK_SIZE 50

struct KeyValue {
    char * key;
    char * value;
};
typedef struct KeyValue KeyValue;

//parsing
char * readInput(FILE *fp);
int numPairs(char *data);
KeyValue * parseInput(char *data);

//processes
int runCommand(int argc, char *argv[]);

//commands
int numTwigs();
void loadTwigs();
void addTwig(char *subject, char *message);
void editTwig(int index, char *subject, char *message);
void deleteTwig(int index);
