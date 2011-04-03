all: Nodes Utilities Listener Run 

Nodes: node.o introducer.o		

Utilities : socket.o hashlib.o
	
Listener : socket.o
	g++ -c listener.cpp

node.o : node.cpp node.h socket.h
	g++ -c node.cpp

introducer.o: node.o introducer.cpp introducer.h
	g++ -c introducer.cpp

socket.o : socket.cpp socket.h
	g++ -c socket.cpp 

hashlib.o : mp2_sha1-c/sha1.c mp2_sha1-c/sha1.h
	g++ -c mp2_sha1-c/sha1.c

Run : Nodes Utilities Listener
	g++ -o runchord listener.o node.o introducer.o socket.o sha1.o -lpthread -lm
clean : 
	rm  runchord node.o socket.o listener.o introducer.o sha1.o
# DO NOT DELETE
