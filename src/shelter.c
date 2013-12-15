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
    tl.row = 16;
    tl.col = 110;
    br.row = 50;
    br.col = 149;
    Buffer viewTwigs = {
        .tl = tl,
        .br = br
    };
    grid[1][1] = viewTwigs;

    //Populate twigs array
    loadTwigs();

    //Rendering
    getScreen();

    //Display twigs
    update_viewTwigs();

    render();
}

void update_viewTwigs() {
    Buffer buf = grid[1][1];
    bufferClear(buf);
    int i;
    int row = buf.tl.row;
    for(i = twigs_start; i < twigs_size; i++) {
        if(row > buf.br.row)
            break;
        char subject[100] = "subject: ";
        strcat(subject, twigs[i].subject);
        char message[150] = "message: ";
        strcat(message, twigs[i].message);

        char num = (char)(((int)'0') + i + 1);
        buffer[row][buf.tl.col - 4] = num;
        row = bufferPrint(row, buf, subject);
        row += 1;
        row = bufferPrint(row, buf, message);
        row = bufferPrint(row, buf, "--------------------------------------");
    }
}

void keypress() {
    char c;
    if((c = getkey()) != KEY_NOTHING) {
        if(c == QUIT)
            _quit = 1;
        else if(c == KEY_UP) {
            if(twigs_start < twigs_size)
                twigs_start += 1;
            update_viewTwigs();
        } else if(c == KEY_DOWN) {
            if(twigs_start > 0)
                twigs_start -= 1;
            update_viewTwigs();
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
