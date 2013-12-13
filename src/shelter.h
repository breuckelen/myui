#define TRUE 1
#define FALSE 0
#define QUIT KEY_F1

struct Point {
    int row;
    int col;
};
typedef struct Point Point;

struct Buffer {
    Point tl;
    Point br;
};
typedef struct Buffer Buffer;

struct Map {
    Buffer grid[2][2];
};
typedef struct Map Map;

void init();
void keypress();
int main();
