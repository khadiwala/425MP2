#ifndef __SOCKET_H_
#define __SOCKET_H_


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <iostream>

#define BACKLOG 10

using namespace std;

int new_socket();
void bind(int sfd, int port);
void listen(int sfd);
int accept(int sfd);
void connect(int sfd, int port);
bool s_recv(int sfd, char * buf, int sizeOfBuf);
void s_send(int sfd, char * buf);
char * itoa(int integer);

#endif
