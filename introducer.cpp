#include "introducer.h"

Introducer::Introducer(int nodeId, int portNumber,int m) : Node(nodeId, portNumber, m)
{
	printf("constructing introducer\n");
	instanceof = INTRODUCER;
}

void Introducer::handle(char * buf)
{
    printf("introducer handling %s\n",buf);
    char tmp[256];
    strcpy(tmp,buf);
    grabLock(strtokLock);    
    char * pch = strtok(tmp,",");
    if(strcmp(pch, "a") == 0) //add node
    {
	cout<<nodeID<<" got add node command\n";
    	int nn = atoi(strtok(NULL,","));
   	int nnpn = atoi(strtok(NULL,","));
	postLock(strtokLock);
        addNode(nn,nnpn,buf);
	return;
    }
    else if(strcmp(pch,"aadjust") == 0)
    {
        int nn = atoi(strtok(NULL,","));
        int nnpn = atoi(strtok(NULL,","));
        postLock(strtokLock);
        addNodeAdjust(nn,nnpn,buf);
    }
    else if(strcmp(pch, "findID") == 0)
    {
	    int fileID = atoi(strtok(NULL, ",")); // should be the file id
	    postLock(strtokLock);
	    findID(fileID, buf);	
	    return;
    }
    else if(strcmp(pch, "doWork") == 0)
    {
	    int fileID = atoi(strtok(NULL, ",")); // should be the file id
	    char * instruction = strtok(NULL, ","); //should be the instruction
	    char * fileName = strtok(NULL, ","); //could be fileName or NULL
	    char * ipAddress = strtok(NULL, ","); //could be ip or NULL
	    postLock(strtokLock);
	    if(strcmp(instruction, "addFile") == 0)
	    {
		cout<<"intro adding file\n";
	    	addFile(fileID, fileName, ipAddress, buf);
		cout<<"done adding file\n";
	    }
	    else if(strcmp(instruction, "delFile") == 0)
	    {
	    	delFile(fileID, fileName, buf);	
	    }
	    else if(strcmp(instruction, "getTabel") == 0)
	    {
	    	getTable(buf);
	    }
	    else if(strcmp(instruction, "quit") == 0)
	    {
	    	quit(buf);
	    }
	    else if(strcmp(instruction, "findFile") == 0)
	    {
	    	getFileInfo(fileID, fileName, buf);
	    }
	    else if(strcmp(instruction, "AddedFile") == 0)
	    {
		cout<<"introducer knows that a file has been added\n";
	    }
	    else if(strcmp(instruction, "DeltFile") == 0)
	    {
   		cout<<"introducer knows that a file has been deleted\n";
	    }
	    else if(strcmp(instruction, "GotTable") == 0)
	    {
 		cout<<"Introducer knows that a table has been found\n";
	    }
	    else if(strcmp(instruction, "GotFile") == 0)
	    {
		cout<<"Introducer know that a file has been got\n";	
	    }
    }
    else if(strcmp(pch,"addnew") == 0)
    {
        int nn = atoi(strtok(NULL,","));
        int nnpn = atoi(strtok(NULL,","));
        postLock(strtokLock);
        addNewNode(nn,nnpn, buf);
    }
}
int Introducer::addNewNode(int nodeID, int portNumber, char * buf)
{
    grabLock(addNodeLock);  //released when finished
    printf("adding node %d \n", nodeID);
    int i;
    if(fingerTable[0]->socket == -1){ //this is the first new node
        addNode(nodeID,portNumber,NULL);
    }
    else{                               //begin initialization ring walk
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

    //fork new node
    if(fork() == 0)
    {
        printf("forking a new node: %d!\n",nodeID);
        Node * newNode = new Node(nodeID,portNumber,m);
        //if this is the first new node added...
        if(fingerTable[0]->socket == -1)
        {
            printf("first new node\n");
            newNode->grabLock(classLock);
            for(int i = 0; i < m; i++){
                if(nodeID + 1<<i > 1<<m)
                {
                    newNode->fingerTable[i]->nodeID = newNode->nodeID;
                }
                else
                {
                    newNode->fingerTable[i]->socket = new_socket();
                    connect(newNode->fingerTable[i]->socket,this->portNumber);   // connect to introducer
                }
            }
            newNode->postLock(classLock);
        }
        else //this is not the first new node added
        {
            printf("new nodes ft recieved: %s \n",buf);
            vector<int> FTdata;
            grabLock(strtokLock);
            char * pch = strtok(buf,",");
            pch = strtok(NULL,",");  //skip 'a'
            pch = strtok(NULL,",");  //skip new node id
            pch = strtok(NULL,",");  //skip new node port#
            while(pch!=NULL)
            {
                FTdata.push_back(atoi(pch));
                pch = strtok(NULL,",");
            }
            postLock(strtokLock);
            //construct finger table from message
            newNode->grabLock(classLock);
            for(int i = 0; i < m; i++)
            {
                newNode->fingerTable[i]->nodeID = FTdata[2 * i];
                newNode->fingerTable[i]->socket = new_socket();
                connect(newNode->fingerTable[i]->socket,FTdata[2*i + 1]);
                printf("ft %d: port:%d, nodeid:%d\n",i,FTdata[2*i + 1],newNode->fingerTable[i]->nodeID);
            }
            newNode->postLock(classLock);
        }     

        //spin
        while((*newNode).getInstance() != DEAD)
            sleep(1);
        delete newNode;
    }

    else //parent
    {        
        sleep(2);//TODO: somehow wait until new node has been constructed in fork

        //adjust introducer table (must be done after fork)
        grabLock(classLock);
        for(int i = 0; i < m; i++)
        {
            if(inBetween(nodeID,(this->nodeID + 1<<i),fingerTable[i]->nodeID))
            {
                //printf("introducer ft:%d is changing \n",i);
                close(fingerTable[i]->socket);
                fingerTable[i]->nodeID = nodeID;
                fingerTable[i]->socket = new_socket();
                connect(fingerTable[i]->socket,portNumber);
            }
        }
        postLock(classLock);

        //send a message for adjustment walk
        //printf("introducer sending an initial adjustment message\n");
        char message[256];
        strcpy(message,"aadjust,");
        strcat(message,itoa(nodeID));
        strcat(message,",");
        strcat(message,itoa(portNumber));
        s_send(fingerTable[0]->socket, message);
    }

    return true;
}

void Introducer::addNodeAdjust(int nodeID, int portNumber, char * msg){
    printf("introducer got m\n");
    postLock(addNodeLock);
    grabLock(classLock);
    char * succloc;
    if((succloc = strstr(msg,",succ")) != NULL)
    {
        map<int,char*>::iterator it;
        for(it = fileMap.begin(); it != fileMap.end(); it++)
        {
            if(inBetween(nodeID,it->first,this->nodeID))
            {
                int predSocket = new_socket();
                connect(predSocket,portNumber);
                char buf[256];
                strcpy(buf,"recieve,");
                strcat(buf,itoa(it->first));
                strcat(buf,",");
                strcat(buf,it->second);
                //printf("node:%d sending %s \n",this->nodeID,buf);
                s_send(predSocket,buf);
                close(predSocket);
            }
        }
        *succloc = 0;
    }        
    postLock(classLock);
}


