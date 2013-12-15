#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shelter.h"
#include "graphics.h"

extern char buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

void render() {
    int row;
    for(row = 0; row < SCREEN_HEIGHT; row++) {
        printf("%s", buffer[row]);
    }
}


void getScreen() {
    FILE *fp = fopen(LAYOUT_FILE, "r");
    int row = 0;
    for(row = 0; row < SCREEN_HEIGHT; row++) {
        if(!fgets(buffer[row], SCREEN_WIDTH, fp))
            break;
    }
    fclose(fp);
}

//Print within left and right boundaries
int bufferPrint(int row, Buffer buf, char *str) {
    int lb = buf.tl.col,
        rb = buf.br.col;
    int col = lb;
    while(*str) {
        buffer[row][col++] = *str++;
        if(col >= rb) {
            col = lb;
            row++;
        }
    }
    return row + 1;
}

void bufferClear(Buffer buf) {
    int col = buf.tl.col,
        row = buf.tl.row;
    while(row <= buf.br.row) {
        buffer[row][col++] = ' ';
        if(col >= buf.br.col){
            col = buf.tl.col;
            buffer[row][buf.tl.col - 4] = ' ';
            row++;
        }
    }
}
