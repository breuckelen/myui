#define CHUNK_SIZE 50

struct KeyValue {
    char * key;
    char * value;
};

typedef struct KeyValue KeyValue;


char * readInput(FILE *fp);
int numPairs(char *data);
KeyValue * parseInput(char *data);
void printReverse(KeyValue *dict, int len);
int main(int argc, char **argv);
