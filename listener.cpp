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

int hashFileName(char * token,int m)
{
    SHA1Context sha;
    SHA1Reset(&sha);
    SHA1Input(&sha,(unsigned char*) token, strlen(token));
    if(!SHA1Result(&sha))
    	fprintf(stderr, "could not hash\n");
    return sha.Message_Digest[4]%(1<<m);
}

//returns true if you should keep parsing
bool parseSend(char * line,int sockfd,bool isFirst,int m)
{
    bool ret = false;
    char * token = strtok(line," \n");
    if(strcmp(token,"ADD_FILE") == 0)
    {
        token = strtok(NULL," \n"); //token = filename
	    int fileID = hashFileName(token,m);
        char commandmsg[256];
        strcpy(commandmsg,"findID,");   //findID,<fileID>,addFile,<filename>,<ip adress>
        strcat(commandmsg,itoa(fileID));
        strcat(commandmsg,",addFile,");
        strcat(commandmsg,token);
        strcat(commandmsg,",");
        token = strtok(NULL," \n");         //token = ip address
        strcat(commandmsg,token);
        printf("%s\n",commandmsg);
        ret = true;
	    s_send(sockfd, commandmsg);
    }
    else if(strcmp(token,"ADD_NODE") == 0)
    {
        char commandmsg[256];
        while((token = strtok(NULL," \n")) != NULL)
        {
            //sleep(1);
            strcpy(commandmsg,"addnew,");
            strcat(commandmsg,token);
            strcat(commandmsg,",");
            strcat(commandmsg,itoa(INTROPORT + portNumOffset));
            portNumOffset++;
            printf("%s\n",commandmsg);
            s_send(sockfd, commandmsg);
        }
        ret = true;
    }
    else if(strcmp(token,"GET_TABLE") == 0)
    {
        token = strtok(NULL," \n"); //token = filename
        int fileID = hashFileName(token,m);
        char commandmsg[256];
        strcpy(commandmsg,"findID,");   //findID,<fileID>,getTable,<filename>
        strcat(commandmsg,itoa(fileID));
        strcat(commandmsg,",getTable,");
        strcat(commandmsg,token); 
        printf("%s\n",commandmsg);
        s_send(sockfd,commandmsg);
        ret = true;
    }
    else if(strcmp(token,"SLEEP") == 0)
    {
        token = strtok(NULL," \n");
        sleep(atoi(token));
        ret = true;
    }
    else if(strcmp(token,"DEL_FILE") == 0)
    {
        token = strtok(NULL," \n"); //token = filename
	    int fileID = hashFileName(token,m);
        char commandmsg[256];
        strcpy(commandmsg,"findID,");   //findID,<fileID>,delFile,<filename>
        strcat(commandmsg,itoa(fileID));
        strcat(commandmsg,",delFile,");
        strcat(commandmsg,token);
        printf("%s\n",commandmsg);
        s_send(sockfd,commandmsg);
        ret = true;
    }
    else if(strcmp(token,"FIND_FILE") == 0)
    {
        token = strtok(NULL," \n"); //token = filename
	    int fileID = hashFileName(token,m);
        char commandmsg[256];
        strcpy(commandmsg,"findID,");   //findID,<fileID>,findFile,<filename>
        strcat(commandmsg,itoa(fileID));
        strcat(commandmsg,",findFile,");
        strcat(commandmsg,token);
        printf("%s\n",commandmsg);
        s_send(sockfd,commandmsg);
        ret = true;
    }
    else if(strcmp(token,"QUIT") == 0)
    {
        printf("Quit message not implemented\n");
        exit(0);
    }
    else if(isFirst)
    {
        FILE * f = fopen(token,"r");
        char command[256];
        if(f == NULL)
        {
            printf("Not a valid command or file name, try again  >> ");
            fgets(command,sizeof(command),stdin);
            ret = parseSend(command,sockfd,true,m);
        }
        else
        {
            while(fgets(command,sizeof(command),f) != NULL)
            {
                printf("read command:%s",command);
                parseSend(command,sockfd,false,m);
            }
            ret = false;
        }
    }
    else
    {
        printf("Invalid command %s\n",token);
        ret = true;
    }
    return ret;
}


int main(int argc, char* argv[])
{
    if (argc != 2){
        cout<<"Invalid arguments"<<endl;
        exit(1);
    }
    int m = atoi(argv[1]);
    if(fork() == 0)
    {
        Introducer * introducer = new Introducer(0,INTROPORT,m);
        sleep(1000); //hack
    }    
    
    sleep(2); //allow introducer to start
    //Listener functions here
	cout<<"listener started with " << m << " nodes"<<endl;
    int lsock = new_socket();
    portNumOffset = 1;

    char command[256];
    connect(lsock,INTROPORT);
    printf("input command or file name >> ");
    fgets(command,sizeof(command),stdin);
    bool isFirst = true;
    while(parseSend(command,lsock,isFirst,m))
    {    
        isFirst = false;
        printf("Input next command >> ");                 
        fgets(command,sizeof(command),stdin);
    }
    sleep(1000);
    return 0;
}
