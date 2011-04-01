#include "introducer.h"

Introducer::Introducer(int nodeId, int portNumber,int m) : Node(nodeId, portNumber, m)
{
	instanceof = INTRODUCER;
}

void Introducer::handle(char * buf)
{
    //in the future this will be an if
    printf("introducer got final ft string: %s\n",buf);
    char tmp[256];
    strcpy(tmp,buf);    
    char * pch = strtok(tmp,",");//pch = a
    int nn = atoi(strtok(NULL,","));
    int nnpn = atoi(strtok(NULL,","));
    addNode(nn,nnpn,buf);
	return;
}
int Introducer::addNewNode(int nodeID, int portNumber)
{
    printf("adding node %d \n", nodeID);
    int i;
    if(fingerTable[0]->socket == -1){ //this is the first new node
        addNode(nodeID,portNumber,NULL);
    }
    else{                               //begin initialization ring walk
        char buf[255];
        strcpy(buf,"a,");
        strcat(buf,itoa(nodeID));
        strcat(buf,",");
        strcat(buf,itoa(portNumber));
        for(i = 0; i < m; i++){
            strcat(buf,",");
            strcat(buf,itoa(this->nodeID));
            strcat(buf,",");
            strcat(buf,itoa(this->portNumber));
        }
        //this message starts initialization ring walk (creates finger table)
        s_send(fingerTable[0]->socket,buf); //sends "a,newnodeid,newnodeportnumber,(0,introportn#)*7" to ft[0]
    }
	return 0;
} 

bool Introducer::addNode(int nodeID, int portNumber, char * buf){

    vector<finger*> newft;
    if(fingerTable[0]->socket == -1){ //this is the first new node
        for(int i = 0; i < m; i++){   //all of its fingers point to 0
            finger* f = new finger();
            f->nodeID = 0;
            f->socket = -1;
            newft.push_back(f);
        }
    }

    //fork new node
    if(fork() == 0)
    {
        printf("forking a new node!\n");
        Node * newNode = new Node(nodeID,portNumber,m);
        //if this is the first new node added...
        if(fingerTable[0]->socket == -1)
        {
            printf("first new node\n");
            for(int i = 0; i < m; i++){
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
        else //this is not the first new node added
        {
            printf("new nodes ft recieved: %s \n",buf);
            vector<int> FTdata;
            char * pch = strtok(buf,",");
            pch = strtok(NULL,",");  //skip 'a'
            pch = strtok(NULL,",");  //skip new node id
            pch = strtok(NULL,",");  //skip new node port#
            while(pch!=NULL)
            {
                FTdata.push_back(atoi(pch));
                pch = strtok(NULL,",");
            }
            //construct finger table from message
            for(int i = 0; i < m; i++)
            {
                newNode->fingerTable[i]->nodeID = FTdata[2 * i];
                newNode->fingerTable[i]->socket = new_socket();
                connect(newNode->fingerTable[i]->socket,FTdata[2*i + 1]);
                printf("ft %d: %d\n",i,newNode->fingerTable[i]->nodeID);
            }
        }     

        //spin
        while((*newNode).getInstance() != DEAD)
            sleep(1);
    }

    else {  //parent
        //shouldn't need those fingers any more
        for(int i = 0; i < m; i++)
            delete newft[i];

        //adjust introducer table (must be done after fork)
        for(int i = 0; i < m; i++)
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
    return true;
}


