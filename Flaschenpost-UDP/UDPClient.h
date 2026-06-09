
#ifndef FLASCHENPOST_CLIENT_
#define FLASCHENPOST_CLIENT_

#include "BaseSocket.h"


namespace flpt {

    class UDPClient : private BaseSocket {
        public:
            UDPClient(Protocol protocol, std::string port, int buffer_length);
            ~UDPClient();

            ErrorCode broadcastLocateServer();
            ErrorCode connectServer(std::string ip_address);

        private:
            int m_buffer_length;
    };

};

#endif