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
    int i;
    //make nessacary adjustments in this finger table
    for(i = 0; i < m; i++)
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
    }

    char tmp[256];
    strcpy(tmp,buf);    
    vector<int> newNodeFT;
    char * pch = strtok(tmp,",");
    pch = strtok(NULL,",");  //skip 'a'
    pch = strtok(NULL,",");  //skip new node id
    pch = strtok(NULL,",");  //skip new node port#
    while(pch!=NULL){
        newNodeFT.push_back(atoi(pch));
        strtok(NULL,",");
    }
    int currentIndex = newNodeFT.size() / 2;
    int lastNode = newNodeFT[currentIndex * 2 - 1];
    bool done = false;
    while(!done && currentIndex < m)
    {
        int pointsTo = ((nodeID + 1<<currentIndex) % 1<<m);
        if(pointsTo < this->nodeID && pointsTo > lastNode){
            strcat(buf,",");
            strcat(buf,itoa(this->nodeID));
            strcat(buf,",");
            strcat(buf,itoa(this->portNumber));
        }
        else
            done = true;
        currentIndex++;
    }

    if(instanceof == NODE)
        s_send(fingerTable[0]->socket,buf);

    newNodeFT.clear();    
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
    printf("%s \n",buf);
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
