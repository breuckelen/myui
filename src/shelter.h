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

//Update functions
void update_viewTwigs();

//Event detection
void keypress();

//Main loop
int main();
