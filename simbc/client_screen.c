#include "client_screen.h"

WINDOW *g_chat_out;
WINDOW *g_chat_in;
WINDOW *g_chat_info;

int g_max_rows = 0;
int g_max_cols = 0;

struct line *lines[LINE_AMOUNT];
int mrl = 0;

void init_ncurses()
{
	initscr();
	raw();
	keypad(stdscr, true);
	noecho();
	int max_rows;
	int max_cols;
	getmaxyx(stdscr, max_rows, max_cols);
	g_chat_out = newwin(max_rows - 5, max_cols - 2, 1           , 1);
	g_chat_in  = newwin(3           , max_cols - 2, max_rows - 5, 1);
	screen_refresh();
}

void screen_refresh()
{
	if(g_chat_out == NULL || g_chat_in == NULL) return;
	erase();
	werase(g_chat_out);
	werase(g_chat_in);
	refresh();
	int o_height;
	int o_width;
	getmaxyx(g_chat_out, o_height, o_width);
	for(int i = 0; i < mrl && i < o_height - 2 ; i++)
	{
		mvwprintw(g_chat_out, o_height - i - 2, 1, "%s: %s", lines[mrl - i - 1]->name, lines[mrl - i - 1]->msg);
	}
	wborder(g_chat_out, '|','|','-','-','+','+','+','+');
	wrefresh(g_chat_out);
	mvwprintw(g_chat_in, 1,1,"%s", g_input_buffer);
	wborder(g_chat_in,  '|','|','-','-','+','+','+','+');
	wrefresh(g_chat_in);
	refresh();
}

void screen_out_add(struct line *line)
{
	if(mrl == LINE_AMOUNT)
	{
		free(lines[0]);
		memmove(&lines[0], &lines[1], sizeof(struct line *) * (mrl-1));
		lines[LINE_AMOUNT-1] = line;
	} else {
		lines[mrl] = line;
		mrl++;
	}
}

int screen_in_input()
{
	static int buf_pos = 0;
	int in;
	in = getch();
	if(in == '\n')
	{
		buf_pos = 0;
		return 1;
	}
	if(in == 127)
	{
		if(buf_pos > 0)
			g_input_buffer[--buf_pos] = 0;
		return 0;
	}
	if(in == '|')
	{
		endwin();
		exit(EXIT_SUCCESS);
		return 0;
	}
	if(32 <= in && in < 127 && buf_pos < MSG_SIZE)
	{
		g_input_buffer[buf_pos++] = in;
	}
	return 0;
}

void redefine_screen(int max_rows, int max_cols)
{
	if(g_chat_out != NULL)
		wresize(g_chat_out, max_rows - 5, max_cols - 2);
	if(g_chat_in != NULL)
	{
		wresize(g_chat_in, 3, max_cols - 2);
		mvwin(g_chat_in, max_rows - 5, 1);
	}
}
