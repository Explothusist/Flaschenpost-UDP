
#ifndef FLASCHENPOST_CLIENT_
#define FLASCHENPOST_CLIENT_

#include "BaseSocket.h"


namespace flpt {

    class UDPClient : private BaseSocket {
        public:
            UDPClient();
            ~UDPClient();


        private:
    };

};

#endif