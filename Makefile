WINSOCK_INCLUDE = -lws2_32
HELPER_INCLUDE = $(WINSOCK_INCLUDE)

server.exe: server.cpp
	g++ -Wall -o server.exe server.cpp $(HELPER_INCLUDE)

client.exe: client.cpp
	g++ -Wall -o client.exe client.cpp $(HELPER_INCLUDE)