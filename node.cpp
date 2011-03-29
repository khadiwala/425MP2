#include "node.h"



Node::Node(int nodeID, int portNumber, int m)
{
    this->m = m;
	this->nodeID = nodeID;
	this->portNumber = portNumber;	
	instanceof = NODE;
	classLock = new sem_t;
	sem_init(classLock, 0, 1);
	
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
        f->nodeID = 1<<m;
        f->socket = -1;
        fingerTable.push_back(f);
    }
}
Node::~Node()
{
	instanceof = DEAD;
	close(listeningSocket);
	pthread_t * oldThread;
	sem_destroy(classLock);
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


int Node::hashFileMapKey(int fileID, string fileName)
{
	return 0;
}

finger * Node::findFileLocation(int fileID)
{
	return NULL;
}

bool Node::addNode(int nodeID, int portNumber,char * buf)
{
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

    if(instanceof == INTRODUCER)
        printf("this is the wrong addnode function\n");

    char tmp[256];
    strcpy(tmp,buf);    
    vector<finger *> newNodeFT;
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
    
    //alter temp finger table based on this->nodeID
    int thisNodeID = this->nodeID;
    if(this->nodeID < nodeID)
        thisNodeID += 1<<m; 
    for(int i = 0; i < m; i++)
    {
        if(thisNodeID >= nodeID + 1<<i && thisNodeID < newNodeFT[i]->nodeID)
        {    
            newNodeFT[i]->nodeID = thisNodeID;
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
    if(instanceof == NODE)
    {
        s_send(fingerTable[0]->socket,tmp);
    }

    //TODO: delete vector
    newNodeFT.clear();    
    return true;
}

bool Node::addFile(int fileID, string fileName, string ipAddress)
{
	return false;
}

bool Node::delFile(int fileID, string fileName)
{
	return false;
}	
void  Node::grabClassLock()
{
	while(sem_wait(classLock) != -0)
		cout<<"waiting on the class lock failed, trying again\n";
}
void Node::postClassLock()
{
	while(sem_post(classLock) != 0)
		cout<<"posting to the class lock failed, trying again\n";
}

void Node::handle(char * buf)
{
    printf("%s\n",buf);
    char tmp[256];
    strcpy(tmp,buf);    
    char * pch = strtok(tmp,",");//pch = a
    int nn = atoi(strtok(NULL,","));
    int nnpn = atoi(strtok(NULL,","));
    addNode(nn,nnpn,buf);
	return;
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
	Node node =*(Node *)nodeClass;
	spawnNewRecieverInfo info;
	info.node = &node;
	///start accepting connections
	pthread_t * newThread;
	int connectingSocket;
	while(node.getInstance() != DEAD)
	{
		connectingSocket = accept(node.getListeningSock());
		if(connectingSocket != -1)
		{
			///Creates new thread to listen on this connection
			newThread = new pthread_t;
			node.grabClassLock();///>will be released in thread
			info.newConnectedSocket = connectingSocket;
			node.connectingThreads.push_back(newThread);
			pthread_create(newThread, NULL, 
						spawnNewReciever, &info);
		}
	}
	return NULL;	
}

void * spawnNewReciever(void * information)
{
	spawnNewRecieverInfo info =*((spawnNewRecieverInfo*)information);
	Node node =*(info.node);
	int connectedSocket =info.newConnectedSocket;
	node.postClassLock();
	char * buf = new char[256];
	
	while(s_recv(connectedSocket, buf, 256))	
	{
		node.handle(buf);
	}
	delete buf;
	
	return NULL;
}
