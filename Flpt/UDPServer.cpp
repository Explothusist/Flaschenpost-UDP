
#include "UDPServer.h"

namespace flpt {

    UDPServer::UDPServer(Protocol protocol, std::string port, int buffer_length):
        BaseSocket(protocol, port, true),
        m_buffer_length{ buffer_length }
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
        return ErrorCode::AllClear;
    };


};