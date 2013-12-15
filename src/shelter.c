#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../lib/keyboard.h"
#include "../lib/xterm_control.h"
#include "shelter.h"
#include "utils.h"
#include "graphics.h"

//Screen is rendered from this buffer
char buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

//Twigs
extern Twig *twigs;
extern int twigs_size;
int twigs_start;

//UI structs
Buffer grid[2][2];
Buffer statusBar;
Point focus;

//Whether to quit or not
int _quit = 0;

void init() {
    //Initializing twig array
    twigs = (Twig *)malloc(20 * sizeof(Twig));
    twigs_size = numTwigs();
    twigs_start = 0;

    //Point of focus
    Point pof = {
        .row = 1,
        .col = 1
    };
    focus = pof;

    //Populate twigs array
    loadTwigs();

    //Graphics init
    init_screen();
}

void keypress() {
    char c;
    if((c = getkey()) != KEY_NOTHING) {
        if(c == QUIT)
            _quit = 1;
        else if(c == 'j') {
            if(focus.row == 1 && focus.col == 1) {
                if(twigs_start < twigs_size)
                    twigs_start += 1;
            }
        } else if(c == 'k') {
            if(focus.row == 1 && focus.col == 1) {
                if(twigs_start > 0)
                    twigs_start -= 1;
            }
        } else if(c == KEY_UP) {
            if(focus.row > 0) {
                grid[focus.row--][focus.col].heading.flag_bold = 0;
                grid[focus.row][focus.col].heading.flag_bold = 1;
            }
        } else if(c == KEY_DOWN) {
            if(focus.row < 1) {
                grid[focus.row++][focus.col].heading.flag_bold = 0;
                grid[focus.row][focus.col].heading.flag_bold = 1;
            }
        } else if(c == KEY_LEFT) {
            if(focus.col > 0) {
                grid[focus.row][focus.col--].heading.flag_bold = 0;
                grid[focus.row][focus.col].heading.flag_bold = 1;
            }
        } else if(c == KEY_RIGHT) {
            if(focus.col < 1) {
                grid[focus.row][focus.col++].heading.flag_bold = 0;
                grid[focus.row][focus.col].heading.flag_bold = 1;
            }
        }
        render();
    }
}

int main() {
    init();
    while(!_quit) {
        keypress();
    }
}
