FLASCHENPOST_INCLUDE = Flpt\UDPServer.o Flpt\UDPClient.o Flpt\BaseSocket.o Flpt\flaschenpost_utils.o
HELPER_INCLUDE = $(FLASCHENPOST_INCLUDE)

WINSOCK_INCLUDE = -lws2_32
LIB_INCLUDE = $(WINSOCK_INCLUDE)

server.exe: server.cpp $(HELPER_INCLUDE)
	g++ -Wall -o server.exe server.cpp $(HELPER_INCLUDE) $(LIB_INCLUDE)

client.exe: client.cpp $(HELPER_INCLUDE)
	g++ -Wall -o client.exe client.cpp $(HELPER_INCLUDE) $(LIB_INCLUDE)

Flpt\UDPServer.o: Flpt\BaseSocket.o Flpt\flaschenpost_utils.o   Flpt\UDPServer.cpp Flpt\UDPServer.h
	g++ -Wall -c Flpt\UDPServer.cpp -o Flpt\UDPServer.o

Flpt\UDPClient.o: Flpt\BaseSocket.o Flpt\flaschenpost_utils.o   Flpt\UDPClient.cpp Flpt\UDPClient.h
	g++ -Wall -c Flpt\UDPClient.cpp -o Flpt\UDPClient.o

Flpt\BaseSocket.o: Flpt\flaschenpost_utils.o   Flpt\BaseSocket.cpp Flpt\BaseSocket.h
	g++ -Wall -c Flpt\BaseSocket.cpp -o Flpt\BaseSocket.o

Flpt\flaschenpost_utils.o:   Flpt\flaschenpost_utils.cpp Flpt\flaschenpost_utils.h
	g++ -Wall -c Flpt\flaschenpost_utils.cpp -o Flpt\flaschenpost_utils.o
