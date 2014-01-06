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

//init
void init_screen();
void fillBuffers();

//rendering
void render_headings();
void render_twigs();
void render_edit(char c);
void render_add(char c);
void render_search(char c);
void render();

//printing utils
int bufferPrintStr(int row, int col, Buffer buf, char *str, char *fg, char *bg);
int bufferPrintChar(int row, int *col, Buffer buf, char c, char *fg, char *bg);

//clearing
void bufferClear(Buffer buf);

//loading
void loadScreen(char *filename);
