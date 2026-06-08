
#ifndef FLASCHENPOST_BASESOCKET_
#define FLASCHENPOST_BASESOCKET_

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "flaschenpost_util.h"

namespace flpt {

    class BaseSocket {
        public:
            BaseSocket();
            ~BaseSocket();

            ErrorCode initializeWinsock();
            ErrorCode createSocket();
            ErrorCode prepServerAddress();
            ErrorCode bindSocket();

            void setVerbosity(Verbosity verbosity);


        private:
            SOCKET m_socket;

            Verbosity m_verbosity;
    };

};

#endif