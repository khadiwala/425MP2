#include <iostream>
#include "node.h"
#include "introducer.h"
#include "socket.h"

#define INTROPORT 4325
using namespace std;

int main()
{
	cout<<"listener started\n";
    if(fork() == 0)
    {
        Introducer * introducer = new Introducer(0,INTROPORT,5);
        (*introducer).addNewNode(10,INTROPORT+1);  //test,normally this is called from handler
        sleep(100); //hack
    }    
    int lsock = new_socket();
    connect(lsock,INTROPORT);
    s_send(lsock,"test message to introducer from listener");
    sleep(5);
	return 0;
}

