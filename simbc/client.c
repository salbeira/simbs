#include "client.h"
#include "client_screen.h"

int running = 1;
int g_socket;
struct sockaddr_in g_server_addr;

extern int g_max_rows;
extern int g_max_cols;

void cancel(int signal)
{
	if(SIGINT == signal)
	{
		endwin();
		printf("This is the end ... my only friend the end ...\n");
		exit(EXIT_SUCCESS);
	}
}

void resized(int signal)
{
	if(SIGWINCH == signal)
	{
		int max_rows;
		int max_cols;
		getmaxyx(stdscr, max_rows, max_cols);
		if(g_max_rows != max_rows || g_max_cols != max_cols)
			redefine_screen(max_rows, max_cols);
		screen_refresh();
	}
}

void udp_send(char *message, unsigned int length)
{
	sendto(g_socket, message, length, 0, (struct sockaddr *) &g_server_addr, sizeof(g_server_addr));
}

void udp_receive(char *buffer, unsigned int length)
{
	struct sockaddr_in from_addr;
	int from_len = sizeof(from_addr);
	if(recvfrom(g_socket, buffer, length, 0, (struct sockaddr*) &from_addr, &from_len) == -1) exit(EXIT_FAILURE);
}

void init_connection()
{
	struct hostent *host = gethostbyname("salbeira.de");
	if(host != NULL)
	{
		g_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if(g_socket < 0) exit(EXIT_FAILURE);
		memset(&g_server_addr, 0, sizeof(g_server_addr));
		g_server_addr.sin_family = AF_INET;
		g_server_addr.sin_port = htons(2424);
		g_server_addr.sin_addr.s_addr = *((long *)host->h_addr_list[0]);
	}	
}

void init_signalhandler()
{
	struct sigaction action;
	action.sa_handler = &cancel;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	if(sigaction(SIGINT, &action, NULL)) exit(EXIT_FAILURE);
	
	struct sigaction winch;
	winch.sa_handler = &resized;
	winch.sa_flags = 0;
	sigemptyset(&winch.sa_mask);
	if(sigaction(SIGWINCH, &winch, NULL)) exit(EXIT_FAILURE);	
}

void* client_receiver(void * params)
{
	while(running)
	{
		struct line *buf = (struct line*) malloc(sizeof(struct line));
		udp_receive((char *)buf, sizeof(struct line));
		screen_out_add(buf);
		screen_refresh();
	}
	return NULL;
}

void* client_input(void* params)
{
	while(running)
	{
		if(screen_in_input())
		{
			udp_send(g_input_buffer, MSG_SIZE);
			memset(g_input_buffer, 0, MSG_SIZE);
		}
		screen_refresh();	
	}
	return NULL;
}

int main(int argc, char **argv)
{
	init_ncurses();
	init_connection();
	init_signalhandler();
	pthread_t rec, in;
	pthread_create(&rec, NULL, &client_receiver, NULL);
	pthread_create(&in, NULL, &client_input, NULL);
	pthread_join(rec, NULL);
	pthread_join(in, NULL);
	endwin();
}
