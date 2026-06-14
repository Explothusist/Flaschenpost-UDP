CXX = g++

CXX_FLAGS = -Wall -std=c++23 -DFLPT_BUILD_DLL -O2

FLASCHENPOST_INCLUDE = UDPServer.o UDPClient.o BaseSocket.o flaschenpost_utils.o
HELPER_INCLUDE = $(FLASCHENPOST_INCLUDE)

WINSOCK_INCLUDE = -lws2_32
LIB_INCLUDE = $(WINSOCK_INCLUDE)

Flaschenpost.dll: $(HELPER_INCLUDE)
	$(CXX) $(CXX_FLAGS) -shared -o Flaschenpost.dll $(HELPER_INCLUDE) $(LIB_INCLUDE) -Wl,--out-implib,libFlaschenpost.a

UDPServer.o: BaseSocket.o flaschenpost_utils.o   UDPServer.cpp UDPServer.h
	$(CXX) $(CXX_FLAGS) -c UDPServer.cpp -o UDPServer.o

UDPClient.o: BaseSocket.o flaschenpost_utils.o   UDPClient.cpp UDPClient.h
	$(CXX) $(CXX_FLAGS) -c UDPClient.cpp -o UDPClient.o

BaseSocket.o: flaschenpost_utils.o   BaseSocket.cpp BaseSocket.h
	$(CXX) $(CXX_FLAGS) -c BaseSocket.cpp -o BaseSocket.o

flaschenpost_utils.o:   flaschenpost_utils.cpp flaschenpost_utils.h
	$(CXX) $(CXX_FLAGS) -c flaschenpost_utils.cpp -o flaschenpost_utils.o

clean:
	del /Q *.o *.dll *.a