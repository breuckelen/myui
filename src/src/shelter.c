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
extern Point focus_editTwig;
extern Point focus_addTwig;

//Flags
int flag_edit_subject;
int flag_add_subject;

//Buffers for saving typing
char edit_subject[30];
char add_subject[30];
char edit_message[140];
char add_message[140];

//Whether to quit or not
int _quit = 0;

void init() {
    //Initializing twig array
    twigs = (Twig *)malloc(20 * sizeof(Twig));
    twigs_size = numTwigs();
    twigs_start = 0;
    loadTwigs();

    //Point of focus for screen
    Point pof = {
        .row = 1,
        .col = 1
    };
    focus = pof;

    //Point of focus for editing
    pof.row = 15;
    pof.col = 10;
    focus_editTwig = pof;

    //pof.row = 21;
    //pof.col = 7;
    //focus_editTwig_message = pof;

    //Point of focus for adding
    pof.row = 34;
    pof.col = 10;
    focus_addTwig = pof;

    //pof.row = 40;
    //pof.col = 7;
    //focus_addTwig_message = pof;

    //Flags
    flag_edit_subject = 1;
    flag_add_subject = 1;

    //Graphics init
    init_screen();
}

void keypress() {
    char c;
    if((c = getkey()) != KEY_NOTHING) {
        if(c == QUIT)
            _quit = 1;

        if(c == 'r') {
            render();
        } else if(focus.row == 1 && focus.col == 1) {
            if(c == 'j') {
                if(twigs_start < twigs_size)
                    twigs_start += 1;
                render_twigs();
            } else if(c == 'k') {
                if(twigs_start > 0)
                    twigs_start -= 1;
                render_twigs();
            } else if(c == DEL) {
                if(twigs_size > 0) {
                    deleteTwig(twigs_start);
                    twigs_size = numTwigs();
                    loadTwigs();
                    render_twigs();
                }
            } else if(c == KEY_ENTER) {
                //Display the twig in the edit bar
            }
        } else if(focus.row == 0 && focus.col == 0) {
            //Do the same thing as below
        } else if(focus.row == 1 && focus.col == 0) {
            if(c >= ' ' && c <= '~') {
                if(flag_add_subject && strlen(add_subject) < sizeof(add_subject)) {
                    add_subject[strlen(add_subject) + 1] = '\0';
                    add_subject[strlen(add_subject)] = c;
                    render_add(c);
                } else if(!flag_add_subject && strlen(add_message) < sizeof(add_message)){
                    add_message[strlen(add_message) + 1] = '\0';
                    add_message[strlen(add_message)] = c;
                    render_add(c);
                }
            } else if(c == KEY_ENTER) {
                if(flag_add_subject) {
                    focus_addTwig.row = 39;
                    focus_addTwig.col = 10;
                    flag_add_subject = 0;
                } else {
                    addTwig(add_subject, add_message);
                    add_subject[0] = '\0';
                    add_message[0] = '\0';
                    bufferClear(grid[1][0]);

                    flag_add_subject = 1;
                    focus_addTwig.row = 34;
                    focus_addTwig.col = 10;

                    twigs_size = numTwigs();
                    loadTwigs();
                    render_twigs();
                }
            } else if(c == DEL) {
                if(flag_add_subject && strlen(add_subject) > 0) {
                    add_subject[strlen(add_subject) - 1] = '\0';
                    render_add(c);
                } else if(!flag_add_subject && strlen(add_message) > 0) {
                    add_message[strlen(add_message) - 1] = '\0';
                    render_add(c);
                }
            }
        }

        if(c == KEY_UP) {
            if(focus.row > 0) {
                grid[focus.row--][focus.col].heading.flag_bold = 0;
                grid[focus.row][focus.col].heading.flag_bold = 1;
                render_headings();
            }
        } else if(c == KEY_DOWN) {
            if(focus.row < 1) {
                grid[focus.row++][focus.col].heading.flag_bold = 0;
                grid[focus.row][focus.col].heading.flag_bold = 1;
                render_headings();
            }
        } else if(c == KEY_LEFT) {
            if(focus.col > 0) {
                grid[focus.row][focus.col--].heading.flag_bold = 0;
                grid[focus.row][focus.col].heading.flag_bold = 1;
                render_headings();
            }
        } else if(c == KEY_RIGHT) {
            if(focus.col < 1) {
                grid[focus.row][focus.col++].heading.flag_bold = 0;
                grid[focus.row][focus.col].heading.flag_bold = 1;
                render_headings();
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
    getkey_terminate();
}