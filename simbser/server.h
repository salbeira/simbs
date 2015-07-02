#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

struct user {
	char name[64];
	int socket;
	struct sockaddr_in *addr;
	time_t last_message;
};

void handle_strgc(int sig);
void error(const char *msg);
struct user** create_user_list(int size);
void copy_user_list(struct user** old, struct user** new);
int insert_new_user(struct user* user);
void remove_user(int i);
struct user* create_user();
void destroy_user(struct user* user);
void cleanup();
void* server_listen(void* params);


#endif
