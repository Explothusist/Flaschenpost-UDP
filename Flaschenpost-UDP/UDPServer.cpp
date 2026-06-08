
#include "UDPServer.h"

namespace flpt {

    UDPServer::UDPServer(const char* port, int buffer_length):
        m_port{ port },
        m_buffer_length{ buffer_length },
        m_server_address{ },
        m_client_address{ },
        m_server_address_length{ 0 },
        m_Winsock_implementation{ }
    {

    };
    UDPServer::~UDPServer() {

    };


    void UDPServer::setVerbosity(Verbosity verbosity) {
        BaseSocket::setVerbosity(verbosity);
    };

};