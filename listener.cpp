#include <iostream>
#include "node.h"
#include "introducer.h"
#include "socket.h"
#include <fstream>
#include <string>
#include <sstream>

#define INTROPORT 4325
using namespace std;

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
         cout<<"trying to add file"<<endl;
         tosend.append("af;");
         while( ss >> token){
             tosend.append(token);
             tosend.append(";");
         }
     }

	else if (!token.compare("ADD_NODE")){
            cout<<"trying to add node"<<endl;
            //tosend.append("addnew,");
            while( ss >> token){
                //tosend.append(token);
                //s_send(token
                char tmp[tosend.size() + 1];
                strcpy(tmp,"addnew,");
                strcat(tmp,token.c_str());
                strcat(tmp,",");
                strcat(tmp,itoa(INTROPORT+ *i));
                strcat(tmp,",");
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

int main()
{
    int m = 5;
	cout<<"listener started\n";
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
    ifstream listenerFile;
    string line;
    listenerFile.open("ListenerFile.txt");
    
    if (listenerFile.is_open()){
        int i = 1;
        connect(lsock,INTROPORT);
        while ( listenerFile.good()){
            getline(listenerFile,line);
            parseSend(line,&i,lsock);
            sleep(2);
            //cout<<"ListenerFile read "<<line<<endl;
        }
        listenerFile.close();
        sleep(100);
    }
    cout << "Failed to open File" << endl;
    sleep(100);
	
    
    return 0;
}


