#include "introducer.h"

Introducer::Introducer(int nodeId, int portNumber,int m) : Node(nodeId, portNumber, m)
{
	instanceof = INTRODUCER;
}

int Introducer::addNewNode(int nodeID, int portNumber)
{
    printf("adding node %d \n", nodeID);
    vector<finger*> newft;
    int i;
    bool isFirst = false;
    if(fingerTable[0]->socket == -1){ //this is the first new node
        isFirst = true;
        for(i = 0; i < m; i++){       //all of its fingers point to 0
            finger* f = new finger();
            f->nodeID = 0;
            f->socket = -1;
            newft.push_back(f);
        }
    }
    else{                               //begin initialization ring walk
        char buf[255];
        strcpy(buf,"a,");
        strcat(buf,itoa(nodeID));
        strcat(buf,",");
        strcat(buf,itoa(portNumber));
        for(i = 0; i < m; i++){
            strcat(buf,",");
            strcat(buf,itoa(this->nodeID + 1<<m));
            strcat(buf,",");
            strcat(buf,itoa(this->portNumber));
        }
        s_send(fingerTable[0]->socket,buf); //sends "a,newnodeid,newnodeportnumber,(0,introportn#)*7" to ft[0]
        grabClassLock();                    //lock until recieving the message
        grabClassLock();                    //fork once finger table message is complete
        postClassLock();
    }

    //fork new node
    if(fork() == 0){
        printf("child\n");
        Node * newNode = new Node(nodeID,portNumber,m);
        if(isFirst){
            printf("first new node\n");
            for(i = 0; i < m; i++){
                if(nodeID + 1<<i > 32)
                    newNode->fingerTable[i]->nodeID = nodeID;
                else
                {
                    delete newNode->fingerTable[i];
                    newNode->fingerTable[i] = newft[i];
                    //printf("fing[i] = %d\n",newNode->fingerTable[i]->nodeID);
                    newNode->fingerTable[i]->socket = new_socket();
                    connect(newNode->fingerTable[i]->socket,this->portNumber);   // connect to introducer
                }
            }
        }
        else{
            printf("%s \n",FTstring);
            //TODO:switch this to modulus 1<<m
            /*vector<int> FTdata;
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
            }*/
        }     
        while((*newNode).getInstance() != DEAD)
            sleep(1);
    }
    else {  //parent
        //shouldn't need those fingers any more
        for(i = 0; i < m; i++)
            delete newft[i];

        //adjust introducer table (must be done after fork)
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
    }
    //printf("sending to finger[0] = %d\n",fingerTable[0]->nodeID);
    //s_send(fingerTable[0]->socket,"test message from introducer to new node 10");
	return 0;
} 

bool Introducer::addNode(int nodeID, int portNumber, char * buf){
    printf("got that msg back\n");
    FTstring = buf;
    postClassLock();
}


