CXX = g++

CXX_FLAGS = -Wall -std=c++23

FLASCHENPOST_INCLUDE = Flpt\UDPServer.o Flpt\UDPClient.o Flpt\BaseSocket.o Flpt\flaschenpost_utils.o
HELPER_INCLUDE = $(FLASCHENPOST_INCLUDE)

WINSOCK_INCLUDE = -lws2_32
LIB_INCLUDE = $(WINSOCK_INCLUDE)

server.exe: server.cpp $(HELPER_INCLUDE)
	$(CXX) $(CXX_FLAGS) -Wall -o server.exe server.cpp $(HELPER_INCLUDE) $(LIB_INCLUDE)

client.exe: client.cpp $(HELPER_INCLUDE)
	$(CXX) $(CXX_FLAGS) -Wall -o client.exe client.cpp $(HELPER_INCLUDE) $(LIB_INCLUDE)

Flpt\UDPServer.o: Flpt\BaseSocket.o Flpt\flaschenpost_utils.o   Flpt\UDPServer.cpp Flpt\UDPServer.h
	$(CXX) $(CXX_FLAGS) -Wall -c Flpt\UDPServer.cpp -o Flpt\UDPServer.o

Flpt\UDPClient.o: Flpt\BaseSocket.o Flpt\flaschenpost_utils.o   Flpt\UDPClient.cpp Flpt\UDPClient.h
	$(CXX) $(CXX_FLAGS) -Wall -c Flpt\UDPClient.cpp -o Flpt\UDPClient.o

Flpt\BaseSocket.o: Flpt\flaschenpost_utils.o   Flpt\BaseSocket.cpp Flpt\BaseSocket.h
	$(CXX) $(CXX_FLAGS) -Wall -c Flpt\BaseSocket.cpp -o Flpt\BaseSocket.o

Flpt\flaschenpost_utils.o:   Flpt\flaschenpost_utils.cpp Flpt\flaschenpost_utils.h
	$(CXX) $(CXX_FLAGS) -Wall -c Flpt\flaschenpost_utils.cpp -o Flpt\flaschenpost_utils.o
