#define CHUNK_SIZE 50

struct KeyValue {
    char * key;
    char * value;
};
typedef struct KeyValue KeyValue;

struct Twig {
    KeyValue subject;
    KeyValue message;
};
typedef struct Twig Twig;

char * readInput(FILE *fp);
int numPairs(char *data);
KeyValue * parseInput(char *data);
int runCommand(int argc, char **argv);
void loadTwigs();
void addTwig(char *subjet, char *message);
