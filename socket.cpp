#include "socket.h"

//returns socket fd
int new_socket()
{
    addrinfo hints;
    addrinfo * sinfo;
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    sinfo = new addrinfo();
    if(getaddrinfo(NULL,"3748",&hints,&sinfo) != 0)
        perror("failed to setup struct in new_socket");
    int sockfd = socket(sinfo->ai_family, sinfo->ai_socktype,
                 sinfo->ai_protocol);

    //dont hang on port after execution
    int iSetOption = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,
              (char*)&iSetOption,sizeof(iSetOption));

    free(sinfo);
    return sockfd;
} 

//binds socket fd to a port number (string)
void bind(int sfd,int portNum)
{
    char * port = itoa(portNum);
    addrinfo * sinfo;
    addrinfo hints;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    sinfo = new addrinfo();
    if(getaddrinfo(NULL,port,&hints,&sinfo) != 0)
        perror("failed to setup struct in new_socket");
    if(bind(sfd,sinfo->ai_addr,sinfo->ai_addrlen) == -1)
        perror("failed to bind socket");
    delete port;
}

void listen(int sfd)
{
    if(listen(sfd,BACKLOG) == -1)
        perror("listen failed");
}

/* accept a connection on a listening socket
 * returns a new socket id to read and write on
 */
int accept(int sfd)
{
    int ret = accept(sfd,NULL,NULL);
    if(ret == -1)
        perror("accept failed");
    return ret;
}

/* connect socket (to a listeing socket on port port) */
void connect(int sfd, int portNum)
{
    char * port = itoa(portNum);
    addrinfo * sinfo;
    addrinfo hints;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    sinfo = new addrinfo();
    if(getaddrinfo(NULL,port,&hints,&sinfo) != 0)
        perror("failed to setup struct in new_socket");
    if(connect(sfd,sinfo->ai_addr,sinfo->ai_addrlen) == -1)
        perror("failed to connect");
    delete port;
} 

/* reads sizeOfBuf characters from socket into buffer 
 * buf must be already allocated 
 * returns false if socket is dead*/
bool s_recv(int sfd, char * buf, int sizeOfBuf)
{
    int status = recv(sfd,buf,sizeOfBuf,0);
    if(status == -1) {
        perror("error recieving from socket");
        return false;
    }
    else if(status == 0)
        return false;
    else
        return true;
    
	//for(int i = 0; i < 5; i ++ )
    //{
	//	if(recv(sfd,buf, sizeOfBuf,0) != -1)
	//		return true;
    //    else
    //        perror("error recieving from socket");
    //}
	//cout<<"socket has died\n";
	//return false;
} 

/* writes strlen(buf) characters from buf into socket*/
void s_send(int sfd, char* buf)
{
    if(send(sfd,buf,strlen(buf),0) == -1)
        perror("error writing to socket");
}

///converts the integer to its ascii representation
char * itoa(int integer)
{
	const int ZERO = 48;
	int i;
	int digits = 1;
	for(i = 10; i < INT_MAX; i=i*10)
	{
		if(i >= integer)
			break;
		digits++;
	}
	
	char * returnChar = new char[digits + 1]; 
	int modNumber = 10;
	int divideNumber = 1;
	returnChar[digits] = '\0';
	for(i = digits - 1; i != -1; i--)
	{
		returnChar[i] = ((integer % modNumber) / divideNumber) + ZERO;
		modNumber *= 10;
		divideNumber *= 10;
	}
    if(integer == 10)   //hack
        strcpy(returnChar,"10");
	return returnChar;
}

//returns true if arg 1 is in between args 2 and 3 modulo m
// false if middle = right
bool inBetween(int middle,int left, int right)
{
    if(left <= middle && middle < right)
        return true;
    if(middle <= left && middle < right && left > right)
        return true;
    if(right < left && (middle >= left || middle < right))
        return true;
    return false;
}
