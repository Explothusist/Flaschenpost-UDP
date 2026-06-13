
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
            BaseSocket(Protocol protocol, std::string port, bool is_server);
            BaseSocket(Protocol protocol, std::string ip_address, std::string port, bool is_server);
            ~BaseSocket();

            // Client and Server
            ErrorCode initializeWinsock();
            ErrorCode createSocket();
            ErrorCode prepServerAddress();

            // Client Only
            ErrorCode prepBroadcastAddress(); // IPv4
            ErrorCode prepMulticastAddress(); // IPv6
            ErrorCode clientStartSending();
            ErrorCode clientStartBroadcasting(); // IPv4
            ErrorCode clientStartMulticasting(); // IPv6

            // Server Only
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
            bool isServerLoopRunning();
            bool isClientLoopRunning();
            bool isClientBroadcastRunning();

            // Multithreading Functions - Server
            void ServerListeningLoop(std::stop_token stop_token);

            // Multithreading Functions - Client
            void ClientToServerLoop(std::stop_token stop_token);
            void ClientBroadcastLoop(std::stop_token stop_token);
            void ClientMulticastLoop(std::stop_token stop_token);

        private:
            Protocol m_protocol;
            std::string m_ip_address;
            std::string m_port;
            bool m_is_server;

            SOCKET m_socket; // SHOULD BE ATOMIC?
            sockaddr_storage m_server_address; // SHOULD BE ATOMIC?
            socklen_t m_server_address_length; // SHOULD BE ATOMIC?
            WSADATA m_Winsock_implementation; // SHOULD BE ATOMIC?

            std::jthread m_network_loop;

            // Atomics
            std::atomic<SocketState> m_state;
            std::atomic<ErrorCode> m_network_loop_error;

            // Network Loop ONLY
            sockaddr_storage m_client_address;
            sockaddr_storage m_sender_address;
            int m_buffer_length;
            std::vector<char> m_incoming_data_buffer;
            std::vector<char> m_sending_data_buffer;
            int m_num_bytes_received;
            char m_host_data[NI_MAXHOST];
            char m_service_data[NI_MAXSERV];
    };

};

#endif