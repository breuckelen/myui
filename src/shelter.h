#pragma once
#define TRUE 1
#define FALSE 0
#define QUIT KEY_F1

struct Twig {
    char * subject;
    char * message;
};
typedef struct Twig Twig;

//Initialization
void init();

//Event detection
void keypress();

//Main loop
int main();
