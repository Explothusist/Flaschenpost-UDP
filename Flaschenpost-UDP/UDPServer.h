
#ifndef FLASCHENPOST_CLIENT_
#define FLASCHENPOST_CLIENT_

#include "flaschenpost_util.h"
#include "BaseSocket.h"


namespace flpt {

    class UDPServer : private BaseSocket {
        public:
            UDPServer(const char* port, int buffer_length);
            ~UDPServer();

            ErrorCode connect();

            void setVerbosity(Verbosity verbosity);

        private:
            const char* m_port;
            int m_buffer_length;
            sockaddr_storage m_server_address;
            sockaddr_storage m_client_address;
            socklen_t m_server_address_length;
            WSADATA m_Winsock_implementation;

    };

};

#endif