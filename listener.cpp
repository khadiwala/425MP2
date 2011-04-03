#include <iostream>
#include "node.h"
#include "introducer.h"
#include "socket.h"
#include <fstream>
#include <string>
#include <sstream>
#include "mp2_sha1-c/sha1.h"
#define INTROPORT 4325
using namespace std;
int m;
void parseSend(string line,int * i,int sockfd){
    //Options are ADD_LINE, ADD_FILE, ADD_NODE, GET_TABLE, SLEEP, DEL_FILE, FIND_FILE
    stringstream ss (stringstream::in | stringstream::out);
    string tosend;
    ss << line;
    string token;
    ss >> token;
    
    //Token is now the first word in the file

	cout << "Token is "<<token<<endl;
    if (!token.compare("ADD_FILE")){
	ss >> token;
	SHA1Context sha;
	SHA1Reset(&sha);
	//unsigned char * tmp = new unsigned char[token.size() + 1];
	//strcpy(tmp, token.c_str());
	SHA1Input(&sha, (const unsigned char*)token.c_str(), token.size());
	if(!SHA1Result(&sha))
		fprintf(stderr, "could not hash\n");
	int fileID = sha.Message_Digest[4]%((int)(1<<m));
	cout<<"fileID for "<<token<<" is : "<<fileID<<endl;
        cout<<"trying to add file"<<endl;
        tosend.append("findID,"); //findID,<fileID>,addFile,<fileName>,<ip>
	tosend.append(itoa(fileID));
	tosend.append(",addFile,");
	tosend.append(token);
	tosend.append(",");
	ss >> token;
	tosend.append(token);
	//char * tmp = new char[tosend.size() + 1];
	//strcpy(tmp, tosend.c_str());
	s_send(sockfd, (char *)(tosend.c_str()));
	//delete tmp;
	
     }

	else if (!token.compare("ADD_NODE")){
            cout<<"trying to add node"<<endl;
            //tosend.append("addnew,");
            while( ss >> token){
                //tosend.append(token);
		sleep(1);
                //s_send(token
                char tmp [tosend.size() + 1];
                strcpy(tmp,"addnew,");
                strcat(tmp,token.c_str());
                strcat(tmp,",");
                strcat(tmp,itoa(INTROPORT+ *i));        
		//strcat(tmp,",");
		printf("tmp --- %s\n", tmp);
                s_send(sockfd,tmp);
                (*i)++;
            }
        }
	else if (!token.compare("GET_TABLE")){
            cout<<"trying to get table"<<endl;
            tosend.append("gt;");
            while( ss >> token){
                tosend.append(token);
                tosend.append(";");
            }
        }

	else if (!token.compare("SLEEP")){
            cout<<"trying to sleep"<<endl;
            tosend.append("sl;");
            while( ss >> token){
                tosend.append(token);
                tosend.append(";");
            }
        }

	else if (!token.compare("DEL_FILE")){
            cout<<"trying to del file"<<endl;
            tosend.append("df;");
            while( ss >> token){
                tosend.append(token);
                tosend.append(";");
            }
        }
	else if (!token.compare("FIND_FILE")){
            cout<<"trying to find file"<<endl;
            tosend.append("ff;");
            while( ss >> token){
                tosend.append(token);
                tosend.append(";");
            }
        }
	else{
		cout<<"ruh roh"<<endl;
	}
	
	
	tosend = tosend.substr(0,tosend.length() -1);
    cout <<"sending "<< tosend<<endl;



}

int main(int argc, char* argv[])
{
    if (argc != 2){
        cout<<"Invalid arguments"<<endl;
        exit(1);
    }
    int m = atoi(argv[1]);
	cout<<"listener started with " << m << " nodes"<<endl;

    cout << "would you like to use a [F]ile or [T]erminal to input commands?" << endl;
    string mode = "";
    cin >> mode;
    if(fork() == 0)
    {
        Introducer * introducer = new Introducer(0,INTROPORT,m);
        //(*introducer).addNewNode(18,INTROPORT+1);  //test,normally this is called from handler
        //sleep(1);
        //(*introducer).addNewNode(10,INTROPORT+2);
        sleep(100); //hack
    }    
    
    //Listener functions here
    int lsock = new_socket();
    //ifstream listenerFile; 
    FILE * f;
    string line;

    if(!mode.compare("T") || !mode.compare("t")){
	int i = 1;
	char command[256];
	connect(lsock,INTROPORT);
        while(mode.compare("exit")){
	    
            cout << "type command or \"exit\" to quit" << endl;
	    //cin.getline(command,256);
	    scanf("%s", command);
	    cout<<"terminal sent " << command<<endl;
            parseSend(command,&i,lsock);
        }
	
    }
    else if (!mode.compare("F") || !mode.compare("f")){
	   cout << "Enter Filename" <<endl;
	   //cin >> mode;
	   cin >> mode;
	   cout<<mode;
	   f = fopen((char *)mode.c_str(), "r");
	   char buf[256];
	   int i = 1;
	   connect(lsock,INTROPORT);
	   while(fgets(buf, 256, f) != NULL){
		//printf("%s \n", buf);
		line = buf;
		parseSend(line,&i,lsock);
	   }
	   
	   //listenerFile.open(buf);
	   ////f = fopen(mode.c_str(),"r");
	   //char* buf[256];
	   // if (listenerFile.is_open()){//f = NULL){
	   //     int i = 1;
	   //     connect(lsock,INTROPORT);
	   //     while ( listenerFile.//fread(buf,256,256,f) != 0 ){
	   //         //getline(f,line);
	   //         line = buf;
	   //         parseSend(line,&i,lsock);
	   //         sleep(2);
	   //         //cout<<"ListenerFile read "<<line<<endl;
	   //     }
	   //     //f.close();
	   //     fclose(f);
	   //     sleep(100);
	   // }
    //cout << "Failed to open File" << endl;
    sleep(100);
	
    
    return 0; 
    }
}


