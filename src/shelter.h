#pragma once
#define TRUE 1
#define FALSE 0
#define QUIT KEY_F1
#define DEL 8

struct Twig {
    char * date;
    char * subject;
    char * message;
};
typedef struct Twig Twig;

//Initialization
void init();

//Event detection
void keypress_viewTwigs(char c);
void keypress_editTwig(char c);
void keypress_addTwig(char c);
void keypress();

//Main loop
int main();
