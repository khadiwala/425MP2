#include "introducer.h"

Introducer::Introducer(int nodeId, int portNumber,int m) : Node(nodeId, portNumber, m)
{
	instanceof = INTRODUCER;
}

int Introducer::addNewNode(int nodeID, int portNumber)
{
    vector<finger*> newft;
    int i;
    bool isFirst = false;
    if(fingerTable[0]->socket == -1){ //this is the first new node
        isFirst = true;
        for(i = 0; i < m; i++){       //all of it's fingers must point to 0
            finger* f = new finger();
            f->nodeID = 0;
            f->socket = -1;
            newft.push_back(f);
        }
    }
    else{                               //begin initialization ring walk
        char buf[20];
        strcpy(buf,"a,");
        strcpy(buf,itoa(nodeID));
        strcpy(buf,",");
        strcpy(buf,itoa(portNumber));
        s_send(fingerTable[0]->socket,buf); //sends "a,newnodeid,newnodeportnumber" to ft[0]
        grabClassLock();                    //lock until recieving the message
        grabClassLock();                    //fork once finger table message is complete
        postClassLock();
    }

    //fork new node
    if(fork() == 0){
        printf("child\n");
        Node * newNode = new Node(nodeID,portNumber,this->m);
        if(isFirst){
            for(i = 0; i < m; i++){
                delete newNode->fingerTable[i];
                newNode->fingerTable[i] = newft[i];
                //printf("fing[i] = %d\n",newNode->fingerTable[i]->nodeID);
                newNode->fingerTable[i]->socket = new_socket();
                connect(newNode->fingerTable[i]->socket,this->portNumber);   // connect to introducer
            }
        }
        else{
            vector<int> FTdata;
            char * pch = strtok(FTstring,",");
            pch = strtok(NULL,",");  //skip 'a'
            pch = strtok(NULL,",");  //skip new node id
            pch = strtok(NULL,",");  //skip new node port#
            while(pch!=NULL){
                FTdata.push_back(atoi(pch));
                strtok(NULL,",");
            }
            //construct finger table from message
            for(i = 0; i < m; i++)
            {
                newNode->fingerTable[i]->nodeID = FTdata[2 * i];
                newNode->fingerTable[i]->socket = new_socket();
                connect(newNode->fingerTable[i]->socket,FTdata[2*i + 1]);
            }
        }     
        while((*newNode).getInstance() != DEAD)
            sleep(1);
    }
    else {
        //shouldn't need those fingers any more
        for(i = 0; i < m; i++)
            delete newft[i];
    }
    //printf("sending to finger[0] = %d\n",fingerTable[0]->nodeID);
    //s_send(fingerTable[0]->socket,"test message from introducer to new node 10");
	return 0;
} 

bool Introducer::addNode(int nodeID, int portNumber, char * buf){
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
    FTstring = buf;
    postClassLock();
}


