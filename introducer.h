#ifndef _INTRODUCER_H_
#define _INTRODUCER_H_

#include "node.h"

class Introducer : public Node
{
public:
	Introducer(int nodeID, int portNumber, int m);
	
	/**
	* Should fork a new process for the new node.	
	*/
	int addNewNode(int nodeID, int portNumber); 

    /**
    * Overides addnode parent method
    */
    bool addNode(int nodeID, int portNumber, char * buf);
private:
    char * FTstring;    //stores the resulting string of the add procedure
};




#endif
