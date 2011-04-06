#include <iostream>
#include "node.h"
#include "introducer.h"
#include "socket.h"
#include <fstream>
#include <string>
#include <sstream>
#include "mp2_sha1-c/sha1.h"
#include <math.h>

#define INTROPORT 4325
using namespace std;
//global variables
int portNumOffset;
int connectedSocket;
void recieveMessage()
{
	char message[256];
	char character[2];
	message[0] = 0;
	character[0] = 0;
	cout<<"Listener waiting for response\n";
        do
        {
		strcat(message,character);
		cout<<"Getting a character from socket "<<connectedSocket<<"\n";
		s_recv(connectedSocket, character, 1);
		printf("%c",character[0]);
               	character[1] = 0;
	}
	while(strcmp(character,"+") != 0);
	printf("/n%s/n", message);
	cout<<"Listener got response\n";
}
char * lowerToUpper(char * string, int size)
{
	int lowerToUpper = 'A' - 'a';
	for(int i = 0; i < size + 1; i++)
		if(string[i] >= 'a' && string[i] <= 'z')
			string[i] += lowerToUpper;
	return string;
}
int hashFileName(char * token,int m)
{
    SHA1Context sha;
    SHA1Reset(&sha);
    SHA1Input(&sha,(unsigned char*) token, strlen(token));
    if(!SHA1Result(&sha))
    	fprintf(stderr, "could not hash\n");
    return sha.Message_Digest[4]%(1<<m);
}
void helpMenu()
{
	cout<<"ADD_NODE <nodeID>\n";
	cout<<"ADD_FILE <filename> <contents>\n";
	cout<<"DEL_FILE <filename>\n";
	cout<<"FIND_FILE <filename>\n";
	cout<<"GET_TABLE <nodeID>\n";
	cout<<"SLEEP <# seconds>\n";
	cout<<"QUIT\n";
	cout<<"<file name with valid commands>\n";
}
//returns true if you should keep parsing
bool parseSend(char * line, int sockfd, int m)
{
    lowerToUpper(line, sizeof(line));
    bool ret = false;
    char * token = strtok(line," \n");
    if(strcmp(token,"ADD_FILE") == 0)
    {
        char * filename = strtok(NULL," \n"); //token = filename
        char * ipAddress = strtok(NULL," \n");//token = ip address
	if(filename == NULL || ipAddress == NULL)
		{cout<<"invalid input, try ADD_FILE <filename> <ipaddress>\n";return 0;}
	int fileID = hashFileName(filename,m);
        char commandmsg[256];
        strcpy(commandmsg,"findID,");   //findID,<fileID>,addFile,<filename>,<ip adress>
        strcat(commandmsg,itoa(fileID));
        strcat(commandmsg,",addFile,");
        strcat(commandmsg,filename);
        strcat(commandmsg,",");
        strcat(commandmsg,ipAddress);
        printf("%s\n",commandmsg);
        ret = true;
	s_send(sockfd, commandmsg);
    }
    else if(strcmp(token,"ADD_NODE") == 0)
    {
        char commandmsg[256];
	ret = false;
	//handles multiple node joins
        while((token = strtok(NULL," \n")) != NULL)
        {
	    ret = true;
            //sleep(1);
            strcpy(commandmsg,"addnew,");
            strcat(commandmsg,token);
            strcat(commandmsg,",");
            strcat(commandmsg,itoa(INTROPORT + portNumOffset));
            portNumOffset++;
            printf("%s\n",commandmsg);
            s_send(sockfd, commandmsg);
        }
    }
    else if(strcmp(token,"GET_TABLE") == 0)
    {
        char * nodeID = strtok(NULL," \n"); //token = filename
	if(nodeID == NULL) 
		{cout<<"Invalid input, try GET_TABLE <filename>\n"; return 0;}
        char commandmsg[256];
        strcpy(commandmsg,"findID,");   //findID,<nodeID>,getTable
        strcat(commandmsg,nodeID);
        strcat(commandmsg,",getTable");
        printf("%s\n",commandmsg);
        s_send(sockfd,commandmsg);
        ret = true;
    }
    else if(strcmp(token,"SLEEP") == 0)
    {
        token = strtok(NULL," \n");
	if(token == NULL) {cout<<"Invalid input try SLEEP <# seconds>\n"; return 0;}
        sleep(atoi(token));
        ret = false;
    }
    else if(strcmp(token,"DEL_FILE") == 0)
    {
        char * filename = strtok(NULL," \n"); //token = filename
	if(filename == NULL)
	{  cout<<"Invalid input, try DEL_FILE <filename>\n"; return 0; }
	int fileID = hashFileName(filename,m);
        char commandmsg[256];
        strcpy(commandmsg,"findID,");   //findID,<fileID>,delFile,<filename>
        strcat(commandmsg,itoa(fileID));
        strcat(commandmsg,",delFile,");
        strcat(commandmsg,filename);
        printf("%s\n",commandmsg);
        s_send(sockfd,commandmsg);
        ret = true;
    }
    else if(strcmp(token,"FIND_FILE") == 0)
    {
        char * filename = strtok(NULL," \n"); //token = filename
	if(filename == NULL) 
	{ cout<<"invalid input, try FIND_FILE <filename>\n"; return 0;}
    	int fileID = hashFileName(filename,m);
        char commandmsg[256];
        strcpy(commandmsg,"findID,");   //findID,<fileID>,findFile,<filename>
        strcat(commandmsg,itoa(fileID));
        strcat(commandmsg,",findFile,");
        strcat(commandmsg,filename);
        printf("%s\n",commandmsg);
        s_send(sockfd,commandmsg);
        ret = true;
    }
    else if(strcmp(token,"QUIT") == 0)
    {
        s_send(sockfd, "quit");
	exit(0);
    }
    else if(strcmp(token, "HELP") == 0)
    {
	helpMenu();
	ret = false;
    }
    else // try to open a file based on the input
    {
        FILE * f = fopen(token,"r");
        char command[256];
        if(f == NULL)
        {
            printf("Not a valid command or file name, try again  >> ");
        }
        else
        {
	    int numberOfCommandsSent = 0;
            while(fgets(command,sizeof(command),f) != NULL)
	    {
               if(parseSend(command,sockfd,m))
			numberOfCommandsSent++;
	    }
	    for(int i = 0; i < numberOfCommandsSent; i++)
		recieveMessage();
        }
	ret = false;
    }
    return ret;
}


int main(int argc, char* argv[])
{
    if (argc != 2 || (argv[1][0] < '5' && !(argv[1][0] == '1' && argv[1][1] == '0')) || argv[1][0] > '9'){
        cout<<"first argument should be the number of nodes 5<=m<=10"<<endl;
        exit(1);
    }
    int m = atoi(argv[1]);
    if(fork() == 0)
    {
        Introducer * introducer = new Introducer(0,INTROPORT,m);
        while(introducer->instanceof != DEAD)
		sleep(5);
	//sleep(1000); //hack
    }    
    
    //Listener functions here
    cout<<"listener started with " << m << " nodes"<<endl;
    
    //set up socket to listen on from introducer
    int listeningSocket = new_socket();
    bind(listeningSocket, INTROPORT-1);
    listen(listeningSocket);
    connectedSocket = accept(listeningSocket);
    
    //set up socket to send things to introducer
    int lsock = new_socket();
    connect(lsock,INTROPORT);
    
    //start getting commands from user
    portNumOffset = 1;
    char command[256];
    cout<<"type help to display a help menu\n";
    printf("input command or file name >> ");
    fgets(command,sizeof(command),stdin);
    while(1)
    {
	//if a command sent to introducer, wait for a response
 	if(parseSend(command,lsock, m))
	{
		recieveMessage();
	}       
        printf("Input next command >> ");                 
        fgets(command,sizeof(command),stdin);
    }
    return 0;
}
