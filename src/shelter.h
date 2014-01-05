#pragma once
#define TRUE 1
#define FALSE 0
#define QUIT KEY_F1
#define DEL 8

struct Twig {
    int index;
    char * date;
    char * subject;
    char * message;
};
typedef struct Twig Twig;

//Initialization
void init();

//Utils
void fillEdit();
void filterTwigs(char *match);

//Event detection
void keypress_editTwig(char c);
void keypress_searchTwigs(char c);
void keypress_addTwig(char c);
void keypress_viewTwigs(char c);
void keypress(char c);

//Main loop
int main();
