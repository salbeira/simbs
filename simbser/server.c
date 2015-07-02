#include <server.h>

#define BUFFER_SIZE 128
#define PORT 2424

struct sockaddr_in g_server;
int g_connection;
struct user **g_user_list;
int g_list_size;
int g_users;

volatile int running = 1;

void handle_strgc(int sig)
{
	if(SIGINT == sig)
	{
		printf("SIGINT caught! Program cancelled by user!\n");
		close(g_connection);
		running = 0;
		exit(EXIT_SUCCESS);
	}
}

void error(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

struct user** create_user_list(int size)
{
	struct user ** list = malloc(sizeof(struct user*) * size);
	memset(list, 0, sizeof(struct user*) * size);
	return list;
}

void copy_user_list(struct user** old, struct user** new)
{
	memcpy(new, old, g_list_size * sizeof(struct user*));
}

int insert_new_user(struct user* user)
{
	int pos = 0;
	for(; pos < g_list_size; pos++)
	{
		if(g_user_list[pos] == NULL) {
			fprintf(stderr, "[DEBUG] Inserting user at position: %d\n", pos);
			g_user_list[pos] = user;
			g_users++;
			if(g_users == g_list_size){
				struct user** new = create_user_list(g_list_size*2);
				copy_user_list(g_user_list, new);
				free(g_user_list);
				g_user_list = new;
				g_list_size *= 2;
			}
			break;
		}
	}
	fprintf(stderr, "[DEBUG] Returning position: %d\n", pos);
	return pos;
}

void remove_user(int i)
{
	struct user *rem = g_user_list[i];
	char *ip = (char*) &(rem->addr->sin_addr.s_addr);
	printf("[SERVER] Removing user: %s from IP %d.%d.%d.%d\n", rem->name, (unsigned char) ip[0],(unsigned char) ip[1],(unsigned char) ip[2],(unsigned char) ip[3]);
	g_users--;
	if(g_users != 0)
	{
		g_user_list[i] = g_user_list[g_users];
		g_user_list[g_users] = NULL;
	} else {
		g_user_list[i] = NULL;
	}
	destroy_user(rem);
}

struct user* create_user()
{
	struct sockaddr_in *addr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
	struct user *new = (struct user*) malloc(sizeof(struct user));
	new->addr = addr;
	memcpy(new->name, "Nameless", sizeof("Nameless")-1);
	return new;
}

void destroy_user(struct user *user)
{
	free(user->addr);
	free(user);
}

/* Remove all users that were 2 minutes inactive */
void cleanup()
{
	printf("[DEBUG] ");
	for(int i=0; i < g_list_size; i++)
	{
		printf("%d ", g_user_list[i] == NULL ? 0 : 1);
	}
	printf("\n");
	for(int i=0; i < g_users; i++)
	{
		if(time(0) - g_user_list[i]->last_message > 120)
		{
			remove_user(i--);
		}
	}
}

void* server_listen(void* params)
{
	char buf[BUFFER_SIZE];
	int client;
	socklen_t sock_addr_len = sizeof(struct sockaddr_in);
	struct user *user, current;
	int user_pos;
	char *ip;
	//fprintf(stderr, "[DEBUG] Starting receive-loop.\n");
	while(running)
	{
		//fprintf(stderr, "[DEBUG] Reserving space for a new user.\n");
		user = create_user();
		if(recvfrom(g_connection, buf, BUFFER_SIZE / 2, 0,(struct sockaddr*) user->addr, &sock_addr_len) == -1)
			break;
		user_pos = -1;
		//fprintf(stderr, "[DEBUG] Searching for existing user.\n");
		for(int i = 0; i < g_users; i++)
		{
			if(user->addr->sin_addr.s_addr == g_user_list[i]->addr->sin_addr.s_addr && user->addr->sin_port == g_user_list[i]->addr->sin_port){
				user_pos = i;
				break;
			}
		}
		if(user_pos == -1)
		{
			//fprintf(stderr, "[DEBUG] No existing user found.\n");
			user_pos = insert_new_user(user);
			ip = (char *) &(g_user_list[user_pos]->addr->sin_addr.s_addr);
			printf("[SERVER] New connection (%d) from: %d.%d.%d.%d:%d\n",user_pos ,(unsigned char) ip[0],(unsigned char) ip[1],(unsigned char) ip[2],(unsigned char) ip[3], user->addr->sin_port);
		} else {
			//fprintf(stderr, "[DEBUG] User found.\n");
			destroy_user(user);
			user = g_user_list[user_pos];
		}
		user->last_message = time(0);
		ip = (char *) &(user->addr->sin_addr.s_addr);	
		printf("[CLIENT] Message from connection (%d) (IP: %d.%d.%d.%d): %s\n", user_pos, (unsigned char) ip[0],(unsigned char) ip[1],(unsigned char) ip[2],(unsigned char) ip[3], buf);
		if(strncmp(buf, "/n ", 3) == 0)
		{
			printf("[SERVER] Changed username from %d.%d.%d.%d from %s to %s\n", (unsigned char) ip[0], (unsigned char) ip[1], (unsigned char) ip[2], (unsigned char) ip[3], user->name, &(buf[3]));
			memcpy(user->name, &(buf[3]), 64);
		} else {
			memcpy(&(buf[64]), buf, 64);
			memcpy(buf, user->name, 64);
			for(int i = 0; i < g_users; i++)
			{
				sendto(g_connection, buf, sizeof(buf), 0, (struct sockaddr*) g_user_list[i]->addr, sock_addr_len);
			}
		}
		cleanup();
	}
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t recer;
	struct sigaction action;
	action.sa_handler = &handle_strgc;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	if(sigaction(SIGINT, &action, NULL)){
		error("sigaction");
	}
	printf("[SERVER] Requesting socket\n");
	g_connection = socket(AF_INET, SOCK_DGRAM, 0);
	if(g_connection < 0){
		error("socket");
	}
	printf("[SERVER] Preparing server information\n");
	memset(&g_server, 0, sizeof(g_server));
	g_server.sin_family = AF_INET;
	g_server.sin_port = htons(2424);
	g_server.sin_addr.s_addr = htonl( INADDR_ANY ) ;
	printf("[SERVER] Binding socket\n");
	if(bind(g_connection, (struct sockaddr*) &g_server, sizeof(g_server)) < 0)
		error("bind");
	g_users = 0;
	g_list_size = 16;
	g_user_list = create_user_list(16);
	printf("[SERVER] Creating listening thread\n");
	if(pthread_create(&recer, NULL, &server_listen, NULL))
		error("pthread_create");
	if(pthread_join(recer, NULL))
		error("pthread_join");
	printf("[SYSTEM] SERVER closed");
	return 0;
}
