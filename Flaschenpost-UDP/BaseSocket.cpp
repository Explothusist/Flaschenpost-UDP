
#include "BaseSocket.h"

#include <cstdio>

namespace flpt {

    BaseSocket::BaseSocket():
        m_socket{ INVALID_SOCKET },
        m_verbosity{ Verbosity::ErrorLogs }
    {

    };
    BaseSocket::~BaseSocket() {
        closesocket(m_socket);
        WSACleanup();
    };
    
    ErrorCode BaseSocket::initializeWinsock() {
        printf("Beginning Winsock Initialization...\n");
        int error_code = WSAStartup(MAKEWORD(2,2), &m_Winsock_implementation); // MAKEWORD(2,2) -> request version 2.2
        if (error_code != 0) {
            printf("Winsock Initialization Failed with Error Code: %d\n", WSAGetLastError());
            // return EXIT_FAILURE;
            return ErrorCode::WinsockInitialization;
        }
        printf("Winsock Initialization Complete!\n\n");
    };
    ErrorCode BaseSocket::createSocket() {

    };
    ErrorCode BaseSocket::prepServerAddress() {

    };
    ErrorCode BaseSocket::bindSocket() {

    };


    void BaseSocket::setVerbosity(Verbosity verbosity) {
        m_verbosity = verbosity;
    };

};