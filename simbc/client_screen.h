#ifndef __CLIENT_SCR_H__
#define __CLIENT_SCR_H__

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#define NAME_SIZE 64
#define MSG_SIZE 128
#define LINE_AMOUNT 128
#define IN_BUFFER_SIZE (NAME_SIZE + MSG_SIZE)

struct line {
	char name[NAME_SIZE];
	char msg[MSG_SIZE];
};

void init_ncurses();

char g_input_buffer[MSG_SIZE];

void screen_refresh();
void screen_out_add(struct line *buf);
int screen_in_input();

void redefine_screen(int max_rows, int max_cols);

#endif
