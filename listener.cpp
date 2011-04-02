#include <iostream>
#include "node.h"
#include "introducer.h"
#include "socket.h"

#define INTROPORT 4325
using namespace std;

int main()
{
    //tests for in between:
    if(!inBetween(4,31,8))
        printf("4,29,8 wrong\n");
    if(inBetween(31,4,8))
        printf("31,4,8 wrong\n");
    if(!inBetween(5,2,7))
        printf("5,2,7 wrong\n");
    if(inBetween(28,29,25))
        printf("28,29,25 wrong\n");

	cout<<"listener started\n";
    if(fork() == 0)
    {
        Introducer * introducer = new Introducer(0,INTROPORT,5);
        (*introducer).addNewNode(18,INTROPORT+1);  //test,normally this is called from handler
        sleep(1);
        (*introducer).addNewNode(10,INTROPORT+2);
        sleep(100); //hack
    }    
    sleep(1);
    int lsock = new_socket();
    connect(lsock,INTROPORT);
    char c;
    while(cin>>c)
	if(c=='X')
		break;
}

