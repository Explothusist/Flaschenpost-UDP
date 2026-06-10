
#ifndef FLASCHENPOST_BASESOCKET_
#define FLASCHENPOST_BASESOCKET_

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <stop_token>
#include "flaschenpost_utils.h"

namespace flpt {

    class BaseSocket {
        public:
            BaseSocket(Protocol protocol, std::string port, bool is_server);
            BaseSocket(Protocol protocol, std::string ip_address, std::string port, bool is_server);
            ~BaseSocket();

            ErrorCode initializeWinsock();
            ErrorCode createSocket();
            ErrorCode prepServerAddress();
            ErrorCode prepBroadcastAddress(); // IPv4
            ErrorCode prepMulticastAddress(); // IPv6
            ErrorCode bindSocket();
            ErrorCode serverStartListening();

            void setTargetIPAddress(std::string ip_address);

            bool prefersIPv4();
            bool usesIPv4();
            bool prefersIPv6();
            bool usesIPv6();
            bool usesIPv4AndIPv6();

            // Multithreading Functions
            void ServerListeningLoop(std::stop_token stop_token);
            void ClientToServerLoop(std::stop_token stop_token);
            void ClientBroadcastLoop(std::stop_token stop_token);
            void ClientMulticastLoop(std::stop_token stop_token);

        private:
            Protocol m_protocol;
            std::string m_ip_address;
            std::string m_port;
            bool m_is_server;

            SOCKET m_socket;
            sockaddr_storage m_server_address;
            socklen_t m_server_address_length;
            WSADATA m_Winsock_implementation;

            SocketState m_state;
            std::jthread m_network_loop;
    };

};

#endif