
#include "UDPServer.h"

namespace flpt {

    UDPServer::UDPServer(Protocol protocol, std::string port):
        BaseSocket(protocol, port, true)
    {

    };
    UDPServer::~UDPServer() {

    };


    ErrorCode UDPServer::launchServer() {
        ErrorCode error = initializeWinsock();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        error = createSocket();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        error = prepServerAddress();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        error = bindSocket();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        error = serverStartListening();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        return ErrorCode::AllClear;
    };
    ErrorCode UDPServer::abortServer() {
        ErrorCode error = serverAbortListening();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        return ErrorCode::AllClear;
    };

    bool UDPServer::isServerRunning() {
        return isServerLoopRunning();
    };


};