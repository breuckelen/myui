#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../lib/keyboard.h"
#include "../lib/xterm_control.h"
#include "shelter.h"
#include "utils.h"
#include "graphics.h"

char buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
Twig *twigs;
int twigs_size;
Map layout;
Buffer statusBar;
Point focus;
int _quit = 0;

void init() {
    //Initializing twig array
    twigs = (Twig *)malloc(20 * sizeof(Twig));
    twigs_size = numTwigs();

    //Grid for layout
    Buffer grid[2][2];

    //Initializing points
    Point tl = {
        .row = 5,
        .col = 65
    };
    Point br = {
        .row = 8,
        .col = 88
    };

    //Status bar
    Buffer bar = {
        .tl = tl,
        .br = br
    };
    statusBar = bar;

    //Point of focus
    Point pof = {
        .row = 1,
        .col = 1
    };
    focus = pof;

    //Section for editing twigs
    tl.row = 15;
    tl.col = 1;
    br.row = 31;
    br.col = 103;
    Buffer editTwig = {
        .tl = tl,
        .br = br
    };
    grid[0][0] = editTwig;

    //Section for searching twigs
    tl.row = 15;
    tl.col = 107;
    br.row = 15;
    br.col = 149;
    Buffer searchTwigs = {
        .tl = tl,
        .br = br
    };
    grid[0][1] = searchTwigs;

    //Section for adding twigs
    tl.row = 34;
    tl.col = 1;
    br.row = 50;
    br.col = 103;
    Buffer addTwig = {
        .tl = tl,
        .br = br
    };
    grid[1][0] = addTwig;

    //Section for viewing twigs
    tl.row = 17;
    tl.col = 110;
    br.row = 50;
    br.col = 149;
    Buffer viewTwigs = {
        .tl = tl,
        .br = br
    };
    grid[1][1] = viewTwigs;

    //Create the map and assign to the global variable 'layout'
    Map map = {
        .grid = grid
    };
    layout = map;

    //Populate twigs array
    loadTwigs();

    //Rendering
    getScreen();
    render();
}

void keypress() {
    char c;
    if((c = getkey()) != KEY_NOTHING) {
        if(c == QUIT)
            _quit = 1;
        render();
    }
}

//update functions (for the screen)

int main() {
    init();
    while(!_quit) {
        keypress();
    }
}
