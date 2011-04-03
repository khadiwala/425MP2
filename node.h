#ifndef _NODE_H_
#define _NODE_H_

#include <string>
#include <map>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include "socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

enum instance { NODE, INTRODUCER, DEAD };
struct finger { volatile int nodeID; volatile int socket;};

//////////////////////////////////////////////////
// Creates a thread to listen to a connecting socket
// and take apporpriate action
/////////////////////////////////////////////////
void * spawnNewReciever(void * NodeClass);


/////////////////////////////////////////////////
//  this is used in another thread to accept connections
//  and spawn additional threads
//////////////////////////////////////////////////
void * acceptConnections(void * NodeClass);

class Node
{
//protected:
public:
    int m;      ///> m for chord system
	int nodeID; ///> relative node id to the system
	int portNumber; ///> listening port number
	int listeningSocket; 

    map<int, char *> fileMap;

	 
	/// set to node or introducer
	volatile instance instanceof;

	///////////////////////////////////////////////////
	// return the closest known finger the fileID
	// or null if the fileID should belong to this node
        ///////////////////////////////////////////////////
	char * findID(int fileID, char * message);
//public	
	sem_t * classLock;
	sem_t * strtokLock;
    sem_t * addNodeLock;



	//the finger table
	vector<finger*> fingerTable;

	Node(int nodeID, int portNumber, int m);
	~Node();

	vector<pthread_t*> connectingThreads;
	instance getInstance();
	int getListeningSock();
	
	///These should be called before changing class data
	void grabLock(sem_t * lock);
	void postLock(sem_t * lock);
	
	///////////////////////////////////////////////////////
	// Figures out if this.fingerTable needs to be changed,
	// Updates node.fingerTable if necessary
	// Calls this.fingerTable.get(0).addNode(node)
	// base case:  this.instanceof = instance.instructor
	//////////////////////////////////////////////////////
	virtual bool addNode(int nodeID, int portNumber, char * buf);

    virtual void addNodeAdjust(int nodeID, int portNumber, char * msg);

    void recieveFile(int fileID, char * buf);
	
	/////////////////////////////////////////////////////
	// add the file to its own system 
	// or forward the message to the appropriate node
	////////////////////////////////////////////////////
	bool addFile(int fileID, char * fileName, char * ipAddress, char *buf);
	
	///////////////////////////////////////////////////
	// deletes the file from its own system 
	// or forwards the message to the appropriate node
	//////////////////////////////////////////////////
	bool delFile(int fileID, char * fileName, char * buf);	

	void getTable(char * buf);
	void quit(char * buf);
	void getFileInfo(int fileID, char * fileName, char * buf);
	/////////////////////////////////////////////////
	// Handles the command in buf after recieving message
	///////////////////////////////////////////////
	virtual void handle(char * buf);
};

struct spawnNewRecieverInfo {void * node; int newConnectedSocket;};

#endif
