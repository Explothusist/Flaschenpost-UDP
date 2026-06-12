
#ifndef FLASCHENPOST_BASESOCKET_
#define FLASCHENPOST_BASESOCKET_

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <stop_token>
#include <vector>
#include "flaschenpost_utils.h"

namespace flpt {

    class BaseSocket {
        public:
            BaseSocket(Protocol protocol, std::string port, size_t buffer_length, bool is_server);
            BaseSocket(Protocol protocol, std::string ip_address, std::string port, size_t buffer_length, bool is_server);
            ~BaseSocket();

            ErrorCode initializeWinsock();
            ErrorCode createSocket();
            ErrorCode prepServerAddress();
            ErrorCode prepBroadcastAddress(); // IPv4
            ErrorCode prepMulticastAddress(); // IPv6
            ErrorCode bindSocket();
            ErrorCode serverStartListening();
            ErrorCode serverAbortListening();

            void setTargetIPAddress(std::string ip_address);

            bool prefersIPv4();
            bool usesIPv4();
            bool prefersIPv6();
            bool usesIPv6();
            bool usesIPv4AndIPv6();

            void setSocketState(SocketState state);
            SocketState getSocketState();
            void recordNetworkLoopError(ErrorCode error);
            ErrorCode getNetworkLoopError();
            bool isNetworkLoopRunning();

            // Multithreading Functions
            void ServerListeningLoop(std::stop_token stop_token);
            void ClientToServerLoop(std::stop_token stop_token);
            void ClientBroadcastLoop(std::stop_token stop_token);
            void ClientMulticastLoop(std::stop_token stop_token);

        private:
            Protocol m_protocol;
            std::string m_ip_address;
            std::string m_port;
            size_t m_buffer_length;
            bool m_is_server;

            SOCKET m_socket;
            sockaddr_storage m_server_address;
            socklen_t m_server_address_length;
            WSADATA m_Winsock_implementation;

            std::jthread m_network_loop;

            // Atomics
            std::atomic<SocketState> m_state;
            std::atomic<ErrorCode> m_network_loop_error;

            // Network Loop ONLY
            sockaddr_storage m_client_address;
            std::vector<char> m_incoming_data_buffer;
            int m_num_bytes_received;
            char m_host_data[NI_MAXHOST];
            char m_service_data[NI_MAXSERV];
    };

};

#endif