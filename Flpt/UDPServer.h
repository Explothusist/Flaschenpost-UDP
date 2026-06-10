
#ifndef FLASCHENPOST_SERVER_
#define FLASCHENPOST_SERVER_

#include "flaschenpost_utils.h"
#include "BaseSocket.h"
#include <string>


namespace flpt {

    class UDPServer : private BaseSocket {
        public:
            UDPServer(Protocol protocol, std::string port, int buffer_length);
            ~UDPServer();

            ErrorCode launchServer();

        private:
            int m_buffer_length;

    };

};

#endif