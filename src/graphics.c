#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shelter.h"
#include "graphics.h"
#include "../lib/xterm_control.h"

//Buffers for rendering
extern char buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
char *colorBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
char *styleBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

//Twig vars
Twig *twigs;
int twigs_size;
int twigs_start;

//Sections and focuses
extern Buffer grid[2][2];
extern Buffer statusBar;
Point focus;
Point focus_editTwig;
Point focus_addTwig;

//Init
void init_screen() {
    //Initializing points
    Point tl = {
        .row = 4,
        .col = 65
    };
    Point br = {
        .row = 7,
        .col = 88
    };

    //Status bar
    Buffer bar = {
        .tl = tl,
        .br = br
    };
    statusBar = bar;

    //Section for editing twigs
    Point hp = {
        .row = 12,
        .col = 48
    };
    //Create and initialize heading for sections
    Heading heading = {
        .start = hp,
        .text = "EDIT A TWIG",
        .color = XT_CH_MAGENTA,
        .flag_bold = 0
    };
    tl.row = 15;
    tl.col = 10;
    br.row = 30;
    br.col = 100;
    Buffer editTwig = {
        .heading = heading,
        .tl = tl,
        .br = br
    };
    grid[0][0] = editTwig;

    //Section for searching twigs
    tl.row = 14;
    tl.col = 107;
    br.row = 14;
    br.col = 149;
    Buffer searchTwigs = {
        .heading = {
            .text = '\0'
        },
        .tl = tl,
        .br = br
    };
    grid[0][1] = searchTwigs;

    //Section for adding twigs
    hp.row = 31;
    hp.col = 46;
    heading.start = hp;
    heading.text = "ADD A NEW TWIG";
    tl.row = 34;
    tl.col = 10;
    br.row = 50;
    br.col = 100;
    Buffer addTwig = {
        .heading = heading,
        .tl = tl,
        .br = br
    };
    grid[1][0] = addTwig;

    //Section for viewing twigs
    hp.row = 12;
    hp.col = 123;
    heading.start = hp;
    heading.text = "VIEW TWIGS";
    heading.flag_bold = 1;
    tl.row = 16;
    tl.col = 110;
    br.row = 50;
    br.col = 149;
    Buffer viewTwigs = {
        .heading = heading,
        .tl = tl,
        .br = br
    };
    grid[1][1] = viewTwigs;

    //Rendering
    loadScreen();
    fillBuffers();
    render_twigs();
    render_headings();
    render();
}

//Initialize the color and style buffers with default values
void fillBuffers() {
    int i, j, k;
    for(i = 0; i < SCREEN_HEIGHT; i++) {
        for(j = 0; j < SCREEN_WIDTH; j++) {
            colorBuffer[i][j] = XT_CH_NORMAL;
            styleBuffer[i][j] = XT_BG_DEFAULT;
        }
    }
}

//Render all of the headings
void render_headings() {
    int i, j;
    for(i = 0; i < 2; i++) {
        for(j = 0; j < 2; j++) {
            Heading heading = grid[i][j].heading;

            //If the headings exists
            if(heading.text) {
                char *str = heading.text;
                int row = heading.start.row;
                int col = heading.start.col;

                while(*str) {
                    styleBuffer[row][col] = XT_BG_BLACK;
                    //Highlight if the section is selected
                    if(heading.flag_bold)
                        styleBuffer[row][col] = XT_BG_WHITE;
                    colorBuffer[row][col] = XT_CH_MAGENTA;

                    buffer[row][col++] = *str++;
                }
            } 
        }
    }
}

//Render the twigs
void render_twigs() {
    Buffer buf = grid[1][1];
    bufferClear(buf);
    int i,
        row = buf.tl.row,
        col = buf.tl.col;
    //Iterate over the twigs, starting with selected twig
    for(i = twigs_start; i < twigs_size; i++) {
        //Break if the row is past the screen limit
        if(row > buf.br.row)
            break;

        char subject[100] = "subject: ",
             message[150] = "message: ";

        strcat(subject, twigs[i].subject);
        strcat(message, twigs[i].message);

        //Turn the twig number into a char and render in the sidebar
        char num = (char)(((int)'0') + (i + 1) % 10);
        buffer[row][col - 4] = num;
        char *fg = XT_CH_NORMAL;
        char *bg = XT_BG_BLACK;
        if(i == twigs_start)
            fg = XT_CH_CYAN;

        //Print everything onto the screen
        row = bufferPrintStr(row, col, buf, subject, fg, bg);
        row = bufferPrintStr(++row, col, buf, message, fg, bg);
        row = bufferPrintStr(++row, col, buf, twigs[i].date, fg, bg);
        row = bufferPrintStr(row, col, buf, "--------------------------------------", fg, bg);
    }
}

//Render the editing section
void render_edit(char c) {

}

//REnder the adding section
void render_add(char c) {
    char *fg = XT_CH_RED,
         *bg = XT_BG_BLACK;
    focus_addTwig.row = bufferPrintChar(focus_addTwig.row, &(focus_addTwig.col), grid[1][0], c, fg, bg);

    int col = focus_addTwig.col;
    //Render '|' delimiting the cursor
    bufferPrintChar(focus_addTwig.row, &(focus_addTwig.col), grid[1][0], '|', fg, bg);
    focus_addTwig.col = col;
}

//Render the buffer with corresponding colors and styles
void render() {
    int row, col;
    for(row = 0; row < SCREEN_HEIGHT; row++) {
        for(col = 0; col < SCREEN_WIDTH; col++) {
            xt_par2(XT_SET_ROW_COL_POS, row+1, col+1);
            xt_par0(colorBuffer[row][col]);
            xt_par0(styleBuffer[row][col]);
            printf("%c", buffer[row][col]);
        }
    }
}

//Load the screen from the layout file
void loadScreen() {
    FILE *fp = fopen(LAYOUT_FILE, "r");
    int row = 0;
    for(row = 0; row < SCREEN_HEIGHT; row++) {
        if(!fgets(buffer[row], SCREEN_WIDTH, fp))
            break;
        printf("%s", buffer[row]);
    }
    fclose(fp);
}

//Print within left and right boundaries
int bufferPrintStr(int row, int col, Buffer buf, char *str, char *fg, char *bg) {
    int lb = buf.tl.col,
        rb = buf.br.col;
    //Stop printing (esp. for twigs) if the row is past the bottom right
    if(row >= buf.br.row)
        return row + 1;
    while(*str) {
        //Wrap text
        if(col >= rb) {
            col = lb;
            row++;
        }

        //Assign appropriate char and colors
        colorBuffer[row][col] = fg;
        styleBuffer[row][col] = bg;
        buffer[row][col++] = *str++;
    }
    return row + 1;
}

//Print character rather than string within boundaries
int bufferPrintChar(int row, int *col, Buffer buf, char c, char *fg, char *bg) {
    int lb = buf.tl.col,
        rb = buf.br.col;

    if(c == DEL) {
        if(*col == lb){
            buffer[row][*col] = ' ';
            *col = rb;
            row -= 1;
        }
        buffer[row][*col] = ' ';
        buffer[row][--(*col)] = ' ';
        return row;
    }

    if(*col >= rb) {
        *col = lb;
        row += 1;
    }

    colorBuffer[row][*col] = fg;
    styleBuffer[row][*col] = bg;
    buffer[row][(*col)++] = c;
    return row;
}

//Clear a buffer
void bufferClear(Buffer buf) {
    int col = buf.tl.col,
        row = buf.tl.row;
    while(row <= buf.br.row) {
        if(col >= buf.br.col){
            col = buf.tl.col;
            if(buf.heading.text == "VIEW TWIGS")
                buffer[row][buf.tl.col - 4] = ' ';
            row++;
        }

        buffer[row][col++] = ' ';
    }
}
