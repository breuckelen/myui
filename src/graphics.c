#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
