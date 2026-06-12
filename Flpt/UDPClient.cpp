
#include "UDPClient.h"

namespace flpt {

    UDPClient::UDPClient(Protocol protocol, std::string port, int buffer_length):
        BaseSocket(protocol, port, buffer_length, false)
    {

    };
    UDPClient::~UDPClient() {

    };


    ErrorCode UDPClient::broadcastLocateServer() {
        ErrorCode error = initializeWinsock();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        error = createSocket();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        if (prefersIPv4()) {
            error = prepBroadcastAddress();
            if (error != ErrorCode::AllClear) {
                return error;
            }
        }else {
            error = prepMulticastAddress();
            if (error != ErrorCode::AllClear) {
                return error;
            }
        }
        return ErrorCode::AllClear;
    };
    ErrorCode UDPClient::connectServer(std::string ip_address) {
        ErrorCode error = initializeWinsock();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        error = createSocket();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        setTargetIPAddress(ip_address);
        error = prepServerAddress();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        return ErrorCode::AllClear;
    };


};