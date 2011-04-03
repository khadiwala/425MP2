#include "node.h"
#include "introducer.h"

#define DEBUGLOCK if(false)

Node::Node(int nodeID, int portNumber, int m)
{
	printf("Constructing node %i \n", nodeID);
    	this->m = m;
	this->nodeID = nodeID;
	this->portNumber = portNumber;	
	instanceof = NODE;
	classLock = new sem_t;
	strtokLock = new sem_t;
    addNodeLock = new sem_t;
	sem_init(classLock, 0, 1);
	sem_init(strtokLock, 0, 1);
	sem_init(addNodeLock, 0, 1);

	/// set up socket
	listeningSocket = new_socket();
	bind(listeningSocket, portNumber);
	listen(listeningSocket);

	//spawn an acceptor process
	pthread_t * acceptor = new pthread_t;
	connectingThreads.push_back(acceptor);
	pthread_create(acceptor, NULL, 
			acceptConnections, this);

    //initialize finger table
    int i;
    for(i = 0; i < m; i++)
    {
        finger * f = new finger();
        f->nodeID = 0;
        f->socket = -1;
        fingerTable.push_back(f);
    }
}
Node::~Node()
{
    
        printf("NODE:%d IS DEAD--------\n",this->nodeID);
    if(instanceof == DEAD)
    {
    	instanceof = DEAD;
    	close(listeningSocket);
    	pthread_t * oldThread;
    	sem_destroy(classLock);
    	sem_destroy(strtokLock);
    	while(!connectingThreads.empty())
    	{
    		oldThread = connectingThreads[connectingThreads.size()-1];
    		connectingThreads.pop_back();
    		pthread_cancel(*oldThread);
    		delete oldThread;
    	}
    	finger * oldFinger;
    	while(!fingerTable.empty())
    	{
    		oldFinger = fingerTable[fingerTable.size()-1];
    		fingerTable.pop_back();
    		delete oldFinger;
    	}
    }
}


int Node::hashFileMapKey(int fileID, char * fileName)
{
	return 0;
}

char * Node::findID(int fileID, char * message)
{
	bool doWork = false;
	int socketToMessage;
	//if successor node holds the token
	if(inBetween(fileID, nodeID, fingerTable[0]->nodeID))
	{
		doWork == true;
		socketToMessage = fingerTable[0]->socket;
	}
	else
	{
		//finds the closest node ID we know about that comes before fileID
		int i;
		for(i = 0; i < fingerTable.size() - 1; i++)
		{
			if(fileID == fingerTable[i]->nodeID)
			{
				doWork = true;
				break;
			}
			if(inBetween(fileID, fingerTable[i]->nodeID, fingerTable[i + 1]->nodeID))
				break;
		}
		socketToMessage = fingerTable[i]->socket;
	}
	if(doWork)
	{
		strcpy(message, "doWork");
		message[6] = ',';		
	}
	s_send(socketToMessage, message);
	return NULL;
}

bool Node::addNode(int nodeID, int portNumber,char * buf)
{
    char tmp[256];
    strcpy(tmp,buf);    

    //build a vector out of the buf
    vector<finger *> newNodeFT;
    grabLock(strtokLock);
    char * pch = strtok(tmp,",");
    pch = strtok(NULL,",");  //skip 'a'
    pch = strtok(NULL,",");  //skip new node id
    pch = strtok(NULL,",");  //skip new node port#
    while(pch!=NULL){
        finger* temp = new finger();
        temp->nodeID = atoi(pch);
        pch = strtok(NULL,",");
        temp->socket = atoi(pch); //this is actually the port number
        newNodeFT.push_back(temp);
        pch = strtok(NULL,",");
    }
    postLock(strtokLock);
    //check each entry from string, see if I/THIS is a better fit for it
    for(int i = 0; i < m; i++)
    {
        if(inBetween(this->nodeID,(nodeID + (1<<i)) % (1<<m),newNodeFT[i]->nodeID))
        {
            newNodeFT[i]->nodeID = this->nodeID;
            newNodeFT[i]->socket = this->portNumber;
        }
    }

    //construct a string to send to sucuessor
    strcpy(tmp,"a,");
    strcat(tmp,itoa(nodeID));
    strcat(tmp,",");
    strcat(tmp,itoa(portNumber));
    for(int i = 0; i < m; i++)
    {
        strcat(tmp,",");
        strcat(tmp,itoa(newNodeFT[i]->nodeID));
        strcat(tmp,",");
        strcat(tmp,itoa(newNodeFT[i]->socket));
    }
    printf("%i sending %i message %s\n", this->nodeID, fingerTable[0]->nodeID, tmp);
    s_send(fingerTable[0]->socket,tmp);

    //TODO: delete vector
    newNodeFT.clear();    
    return true;
}

void Node::addNodeAdjust(int nodeID, int portNumber, char * msg)
{
    printf("got an adjust message node:%d\n",this->nodeID);
    grabLock(classLock);
    for(int i = 0; i < m; i++)
    {
        //printf("index:%d,fing[i]-id: %d,fing[i]sock: %d\n",i,fingerTable[i]->nodeID,fingerTable[i]->socket,(this->nodeID + (1<<i)) % (1<<m));
        if(inBetween(nodeID,(this->nodeID + (1<<i)) % (1<<m),fingerTable[i]->nodeID))
        {
            close(fingerTable[i]->socket);
            fingerTable[i]->nodeID = nodeID;
            fingerTable[i]->socket = new_socket();
            connect(fingerTable[i]->socket,portNumber);
        }
    }
    postLock(classLock);
    printf("forwarding message to node:%d, corresponding socket: %d\n",fingerTable[0]->nodeID,fingerTable[0]->socket);
    s_send(fingerTable[0]->socket,msg);
}

bool Node::addFile(int fileID, char * fileName, char * ipAddress)
{
	printf("% adding file %i - %s with ip %s\n", nodeID, fileID, fileName, ipAddress);
	return false;
}

bool Node::delFile(int fileID, char * fileName)
{
	return false;
}	
void  Node::grabLock(sem_t * lock)
{
	DEBUGLOCK printf("%i waiting for lock %p\n", nodeID, lock);
	while(sem_wait(lock) != -0)
		cout<<"waiting on the lock failed, trying again\n";
}
void Node::postLock(sem_t * lock)
{
	DEBUGLOCK printf("%i posting lock %p\n", nodeID, lock);
	while(sem_post(lock) != 0)
		cout<<"posting to the lock failed, trying again\n";
}
void Node::getTable()
{}
void Node::quit(char * msg)
{
    instanceOf = DEAD;
    s_send(fingerTable[0]->socket,msg);
}

void Node::getFileInfo(int fileID, char * fileName)
{   

    if(fileMap.count(fileID) > 0)
    {
        if(fileMap[fileID].count(fileName) > 0)
    } 
    ipAdress = filemap[fileID][fileName];
}
void Node::handle(char * buf)
{ 
    printf("%i handling %s\n",nodeID, buf);
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
	    	addFile(fileID, fileName, ipAddress);
	    }
	    else if(strcmp(instruction, "delFile") == 0)
	    {
	    	delFile(fileID, fileName);	
	    }
	    else if(strcmp(instruction, "getTabel") == 0)
	    {
	    	getTable();
	    }
	    else if(strcmp(instruction, "quit") == 0)
	    {
	    	quit();
	    }
	    else if(strcmp(instruction, "findFile") == 0)
	    {
	    	getFileInfo(fileID, fileName);
	    }
    }
}
int Node::getListeningSock()
{
	return listeningSocket;
}
instance Node::getInstance()
{
	return instanceof;
}


void * acceptConnections(void * nodeClass)
{
	Node * node = (Node *)nodeClass;
	spawnNewRecieverInfo info;
	info.node = node;
	///start accepting connections
	pthread_t * newThread;
	int connectingSocket;
	while(node->getInstance() != DEAD)
	{
		connectingSocket = accept(node->getListeningSock());
		if(connectingSocket != -1)
		{
			///Creates new thread to listen on this connection
			newThread = new pthread_t;
			node->grabLock(node->classLock);///>will be released in thread
			info.newConnectedSocket = connectingSocket;
			node->connectingThreads.push_back(newThread);
			pthread_create(newThread, NULL, 
						spawnNewReciever, &info);
		}
	}
	return NULL;	
}

void * spawnNewReciever(void * information)
{
	spawnNewRecieverInfo info =*((spawnNewRecieverInfo*)information);
	Node * node = (Node*)info.node;
    int connectedSocket =info.newConnectedSocket;
	node->postLock(node->classLock);
    char * c = new char[2];
	char * buf = new char[256];
	int i = 0;
	while(s_recv(connectedSocket, c, 1))	
	{
        c[1] = 0;
        if(strcmp(c,".") == 0)
        {
    	    node->handle(buf);
            strcpy(buf,"");
        }
        else
            strcat(buf,c);
	}
	delete buf;
    node = NULL;
	return NULL;
}


//stuff to be added to updateTable or addNodeFinal etc
    //TODO:This needs to happen in a different function (updateTable or something)
    /*//make nessacary adjustments in this finger table
    for(int i = 0; i < m; i++)
    {
        if(fingerTable[i]->nodeID > nodeID && 
        nodeID >= ((this->nodeID + 1<<i) % (1<<m)))
        {
            delete fingerTable[i];
            finger * f = new finger();
            f->nodeID = nodeID;
            f->socket = new_socket();
            connect(f->socket,portNumber);       
            fingerTable[i] = f;
        }
    }*/
    
    //build temp finger table


//old logic - probably works, safekeeping
    ////alter temp finger table based on this->nodeID
    //int thisNodeID = this->nodeID;
    //if(this->nodeID < nodeID)
    //    thisNodeID += 1<<m; 
    //for(int i = 0; i < m; i++)
    //{
        //printf("%d,%d\n",thisNodeID, nodeID+(1<<i));
        //if(thisNodeID >= nodeID + (1<<i) && thisNodeID > newNodeFT[i]->nodeID)
        //{    
        //    newNodeFT[i]->nodeID = thisNodeID;
        //    newNodeFT[i]->socket = this->portNumber;
        //}
