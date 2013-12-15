#pragma once
#define SCREEN_HEIGHT 51
#define SCREEN_WIDTH 151
#define LAYOUT_FILE "assets/layout.temp"

//Structs
struct Point {
    int row;
    int col;
};
typedef struct Point Point;

struct Heading {
    Point start;
    char *text;
    char *color;
    int flag_bold;
};
typedef struct Heading Heading;

struct Buffer {
    Heading heading;
    Point tl;
    Point br;
};
typedef struct Buffer Buffer;

void init_screen();
void fillBuffers();
void render();
void render_headings();
void render_twigs();
void render_screen();
void getScreen();
int bufferPrint(int row, Buffer buf, char *str, char *fg, char *bg);
void bufferClear(Buffer buf);
