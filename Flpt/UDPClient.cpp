
#include "UDPClient.h"

namespace flpt {

    UDPClient::UDPClient(Protocol protocol, std::string port):
        BaseSocket(protocol, port, false)
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
            error = clientStartBroadcasting();
            if (error != ErrorCode::AllClear) {
                return error;
            }
            while (isBroadcastLoopRunning()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            if (getSocketState() == SocketState::AddressPrepped) {
                // This means that it found the server
                logMessage("Starting Client to Server Connection");
                clientStartSending();
            }else {
                logError("Server could not be located by the broadcasting client");
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
        error = clientStartSending();
        if (error != ErrorCode::AllClear) {
            return error;
        }
        return ErrorCode::AllClear;
    };

    
    bool UDPClient::isConnectedToServer() {
        return isClientLoopRunning();
    };
    bool UDPClient::isBroadcastLoopRunning() {
        return isClientBroadcastRunning();
    };


};