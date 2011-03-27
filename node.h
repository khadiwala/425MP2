#ifndef _NODE_H_
#define _NODE_H_

#include <string>
#include <map>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include "socket.h"

using namespace std;

enum instance { NODE, INTRODUCER, DEAD };
struct finger { int nodeID; int socket;};

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
protected:
    int m;      ///> m for chord system
	int nodeID; ///> relative node id to the system
	int portNumber; ///> listening port number
	int listeningSocket; 
		
	// makes the inner workings of the class atomic
	// I'm not sure if this happens automatically already
	sem_t * classLock;
	
	/// maps the I.P. with key based on hashFileMapKey 
	map<int, string> fileMap;

	//the finger table
	vector<finger*> fingerTable;
	 
	/// set to node or introducer
	instance instanceof;

	////////////////////////////////////////////////////
	// Returns a unique hash kep based on the fileID 
	// and fileName to be used for the fileMap
	////////////////////////////////////////////////////
 	int hashFileMapKey(int fileID, string fileName);

	///////////////////////////////////////////////////
	// return the closest known finger the fileID
	// or null if the fileID should belong to this node
        ///////////////////////////////////////////////////
	finger * findFileLocation(int fileID);
	
public:

	Node(int nodeID, int portNumber, int m);
	~Node();

	vector<pthread_t*> connectingThreads;
	instance getInstance();
	int getListeningSock();
	
	///These should be called before changing class data
	void grabClassLock();
	void postClassLock();
	
	///////////////////////////////////////////////////////
	// Figures out if this.fingerTable needs to be changed,
	// Updates node.fingerTable if necessary
	// Calls this.fingerTable.get(0).addNode(node)
	// base case:  this.instanceof = instance.instructor
	//////////////////////////////////////////////////////
	bool addNode(int nodeID, int portNumber);
	
	/////////////////////////////////////////////////////
	// add the file to its own system 
	// or forward the message to the appropriate node
	////////////////////////////////////////////////////
	bool addFile(int fileID, string fileName, string ipAddress);
	
	///////////////////////////////////////////////////
	// deletes the file from its own system 
	// or forwards the message to the appropriate node
	//////////////////////////////////////////////////
	bool delFile(int fileID, string fileName);	

	/////////////////////////////////////////////////
	// Handles the command in buf after recieving message
	///////////////////////////////////////////////
	void handle(char * buf);
};

struct spawnNewRecieverInfo {Node * node; int newConnectedSocket;};

#endif
