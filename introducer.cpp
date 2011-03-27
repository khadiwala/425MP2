#include "introducer.h"

Introducer::Introducer(int nodeId, int portNumber,int m) : Node(nodeId, portNumber, m)
{
	instanceof = INTRODUCER;
}

int Introducer::addNewNode(int nodeID, int portNumber)
{
    //fork new node
    if(fork() == 0){
        Node * newNode = new Node(nodeID,portNumber,this->m);
        //need to make newNodes finger table
        while((*newNode).getInstance() != DEAD) //hack, TODO: a function that detaches all the threads
            sleep(1);
    }

    //adjust introducer finger table
    int i;
    for(i = 0; i < m; i++)
    {
        if(fingerTable[i]->nodeID > nodeID && nodeID >= 1<<i)
        {
            delete fingerTable[i];
            finger * f = new finger();
            f->nodeID = nodeID;
            f->socket = new_socket();
            connect(f->socket,portNumber);
            fingerTable[i] = f;
        }
    }
    printf("sending to finger[0] = %d\n",fingerTable[0]->nodeID);
    s_send(fingerTable[0]->socket,"test message from introducer to new node 10");
    
    //TODO:send a message initiate add_node procedure in finger[0]

	return 0;
} 


