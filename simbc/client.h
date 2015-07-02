#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>

void cancel(int signal);

void udp_send(char *message, unsigned int length);
void udp_send_ip(char *message, unsigned int length, struct sockaddr_in *to);

void udp_receive(char *buffer, unsigned int length);
void udp_receive_ip(char *buffer, unsigned int length, struct sockaddr_in *from);

#endif
