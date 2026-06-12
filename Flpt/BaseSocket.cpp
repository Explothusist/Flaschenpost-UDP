
#include "BaseSocket.h"

#include <cstdio>
#include <chrono>

namespace flpt {

    BaseSocket::BaseSocket(Protocol protocol, std::string port, size_t buffer_length, bool is_server):
        BaseSocket(protocol, "", port, buffer_length, is_server)
    {

    };
    BaseSocket::BaseSocket(Protocol protocol, std::string ip_address, std::string port, size_t buffer_length, bool is_server):
        m_protocol{ protocol },
        m_ip_address{ ip_address },
        m_port{ port },
        m_buffer_length{ buffer_length },
        m_is_server{ is_server },
        m_socket{ INVALID_SOCKET },
        m_server_address{ },
        m_server_address_length{ 0 },
        m_Winsock_implementation{ },
        m_network_loop{ },
        m_state{ SocketState::Uninitialized },
        m_network_loop_error{ ErrorCode::AllClear },
        m_client_address{ },
        m_incoming_data_buffer( buffer_length ),
        m_num_bytes_received{ 0 },
        m_host_data{ },
        m_service_data{ }
    {

    };
    BaseSocket::~BaseSocket() {
        m_network_loop.request_stop();
        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
        }
        if (m_network_loop.joinable()) {
            m_network_loop.join();
        }
        if (getSocketState() >= SocketState::Initialized) {
            terminateGlobalWinsock();
        }
    };
    
    ErrorCode BaseSocket::initializeWinsock() {
        if (getSocketState() >= SocketState::Initialized) {
            logMessage("Socket Initialization Already Completed");
            return ErrorCode::AllClear;
        }
        ErrorCode error_code = initializeGlobalWinsock();
        if (error_code == ErrorCode::AllClear) {
            setSocketState(SocketState::Initialized);
        }
        return error_code;
    };
    ErrorCode BaseSocket::createSocket() {
        logMessage("Creating Socket...");
        if (getSocketState() <= SocketState::Uninitialized) {
            logError("Socket Must be Initialized Before it is Created");
            return ErrorCode::CreateSocketBeforeInitialize;
        }
        if (getSocketState() >= SocketState::Created) {
            logMessage("Socket Has Already Been Created");
            return ErrorCode::AllClear;
        }

        // if (m_protocol == Protocol::IPv4Only) {
        if (!usesIPv6()) {
            m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // AF_INET = IPv4, IPPROTO_UDP = explicitly UDP
        }else {
            m_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP); // AF_INET6 = IPv6, IPPROTO_UDP = explicitly UDP
        }
        if (m_socket == INVALID_SOCKET) {
            logError("Socket Creation Failed with Error Code: %d", WSAGetLastError());
            return ErrorCode::SocketCreationFailed;
        }
        // if (m_protocol == Protocol::Both_PreferIPv4 || m_protocol == Protocol::Both_PreferIPv6) {
        if (usesIPv4AndIPv6()) {
            DWORD accept_ipv6_only = 0; // setsockopt to set the option to limit to only IPv6 to off (allow IPv4 as well)
            int error_code = setsockopt(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&accept_ipv6_only, sizeof(accept_ipv6_only));
            if (error_code == SOCKET_ERROR) {
                logError("Socket Options 'IPv6 Only' Disable Failed with Error Code: %d", WSAGetLastError());
                return ErrorCode::SocketAllowDualStackError;
            }
        }
        // if (!m_is_server && m_protocol != Protocol::IPv6Only) {
        if (!m_is_server && usesIPv4()) {
            DWORD allow_broadcast = 1; // setsockopt to set the option to limit to only IPv6 to off (allow IPv4 as well)
            int error_code = setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (char*)&allow_broadcast, sizeof(allow_broadcast));
            if (error_code == SOCKET_ERROR) {
                logError("Socket Options 'Allow Broadcast' Enable Failed with Error Code: %d", WSAGetLastError());
                return ErrorCode::SocketAllowBroadcastError;
            }
        }
        if (g_FlaschenpostNetworkHasTimeout) {
            DWORD timeout_ms = g_FlaschenpostNetworkTimeoutMS;
            int error_code = setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_ms, sizeof(timeout_ms));
            if (error_code == SOCKET_ERROR) {
                logError("Socket Options 'Recv Timeout' Enable Failed with Error Code: %d", WSAGetLastError());
                return ErrorCode::SocketSetRecvTimeoutError;
            }
        }

        logMessage("Socket Creation Complete!\n");
        setSocketState(SocketState::Created);
        return ErrorCode::AllClear;
    };
    ErrorCode BaseSocket::prepServerAddress() {
        logMessage("Preparing Server Address Data...");
        if (getSocketState() <= SocketState::Initialized) {
            logError("Socket Must be Created Before Address Prepped");
            return ErrorCode::PrepAddressBeforeCreateSocket;
        }
        if (getSocketState() >= SocketState::AddressPrepped) {
            logMessage("Socket Has Already Been Address Prepped");
            return ErrorCode::AllClear;
        }

        addrinfo hints{};
        addrinfo* server_info = nullptr;
        // if (m_protocol == Protocol::IPv4Only) {
        //     hints.ai_family = AF_INET;
        // }else if (m_protocol == Protocol::IPv6Only) {
        //     hints.ai_family = AF_INET6;
        // }else {
        //     hints.ai_family = AF_UNSPEC;
        // }
        if (usesIPv4AndIPv6()) {
            hints.ai_family = AF_UNSPEC;
        }else if (usesIPv6()) {
            hints.ai_family = AF_INET6;
        }else {
            hints.ai_family = AF_INET;
        }
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        if (m_is_server) {
            hints.ai_flags = AI_PASSIVE; // Say "I'm a server"
        }
        // std::string ip_address;
        // if (!m_is_server) {
        //     ip_address = m_ip_address;
        // } 
        int error_code = getaddrinfo(m_is_server ? nullptr : m_ip_address.c_str(), m_port.c_str(), &hints, &server_info); // Addr = nullptr and AI_PASSIVE mean bind to all
        if (error_code != 0) {
            logError("Preparing Server Address Data: getaddrinfo() Failed: Error Code: %s", gai_strerror(error_code));
            return ErrorCode::ServerAddressGetInfoError;
        }
        // Locate the most fitting IP address
        addrinfo* best = nullptr; // First of requested type fallback to first of either type
        for (addrinfo* current = server_info; current != nullptr; current = current->ai_next) {
            if (current->ai_socktype == SOCK_DGRAM && current->ai_protocol == IPPROTO_UDP) {
                // if (m_protocol == Protocol::IPv4Only || m_protocol == Protocol::Both_PreferIPv4) {
                if (!usesIPv6() || (prefersIPv4() && !m_is_server)) {
                    if (current->ai_family == AF_INET) {
                        best = current;
                        break;
                    }
                }else {
                    if (current->ai_family == AF_INET6) {
                        best = current;
                        break;
                    }
                }
                // if (m_protocol == Protocol::Both_PreferIPv4 || m_protocol == Protocol::Both_PreferIPv6) {
                if (usesIPv4AndIPv6()) {
                    if (best == nullptr) {
                        best = current;
                    }
                }
            }
        }
        if (best == nullptr) {
            freeaddrinfo(server_info);
            logError("Preparing Server Address Data: getaddrinfo() Returned No Valid IP: Error Code: %s", gai_strerror(error_code));
            return ErrorCode::ServerAddressNoValidIP;
        }
        // Finally, set up the data
        memset(&m_server_address, 0, sizeof(m_server_address));
        memcpy(&m_server_address, best->ai_addr, best->ai_addrlen);
        m_server_address_length = best->ai_addrlen;
        freeaddrinfo(server_info);

        logMessage("Server Address Data Prepared!\n");
        setSocketState(SocketState::AddressPrepped);
        return ErrorCode::AllClear;
    };
    ErrorCode BaseSocket::prepBroadcastAddress() {
        // logMessage("Preparing Server Address Data...");

        // addrinfo hints{};
        // addrinfo* server_info = nullptr;
        // if (m_protocol == Protocol::IPv4Only) {
        //     hints.ai_family = AF_INET;
        // }else if (m_protocol == Protocol::IPv6Only) {
        //     hints.ai_family = AF_INET6;
        // }else {
        //     hints.ai_family = AF_UNSPEC;
        // }
        // hints.ai_socktype = SOCK_DGRAM;
        // hints.ai_protocol = IPPROTO_UDP;
        // if (m_is_server) {
        //     hints.ai_flags = AI_PASSIVE; // Say "I'm a server"
        // }
        // std::string ip_address = nullptr;
        // if (!m_is_server) {
        //     if (m_ip_address != nullptr) {
        //         ip_address = m_ip_address;
        //     }else {
        //         if (m_protocol == Protocol::IPv4Only || m_protocol == Protocol::Both_PreferIPv4) {
        //             ip_address = k_IPv4BroadcastAll;
        //         }else {
        //             ip_address = k_IPv6MulticastAll;
        //         }
        //     }
        // } 
        // int error_code = getaddrinfo(ip_address, m_port, &hints, &server_info); // Addr = nullptr and AI_PASSIVE mean bind to all
        // if (error_code != 0) {
        //     logError("Preparing Server Address Data: getaddrinfo() Failed: Error Code: %s", gai_strerror(error_code));
        //     return ErrorCode::ServerAddressGetInfoError;
        // }
        // // Locate the most fitting IP address
        // addrinfo* best = server_info;
        // for (addrinfo* current = server_info; current != nullptr; current = current->ai_next) {
        //     if (m_protocol == Protocol::IPv4Only || m_protocol == Protocol::Both_PreferIPv4) {
        //         if (current->ai_family == AF_INET) {
        //             best = current;
        //             break;
        //         }
        //     }else {
        //         if (current->ai_family == AF_INET6) {
        //             best = current;
        //             break;
        //         }
        //     }
        // }
        // // Finally, set up the data
        // memcpy(&m_server_address, best->ai_addr, best->ai_addrlen);
        // m_server_address_length = best->ai_addrlen;
        // freeaddrinfo(server_info);

        // logMessage("Server Address Data Prepared!\n");
        return ErrorCode::AllClear;
    };
    ErrorCode BaseSocket::prepMulticastAddress() {
        // logMessage("Preparing Server Address Data...");

        // addrinfo hints{};
        // addrinfo* server_info = nullptr;
        // if (m_protocol == Protocol::IPv4Only) {
        //     hints.ai_family = AF_INET;
        // }else if (m_protocol == Protocol::IPv6Only) {
        //     hints.ai_family = AF_INET6;
        // }else {
        //     hints.ai_family = AF_UNSPEC;
        // }
        // hints.ai_socktype = SOCK_DGRAM;
        // hints.ai_protocol = IPPROTO_UDP;
        // if (m_is_server) {
        //     hints.ai_flags = AI_PASSIVE; // Say "I'm a server"
        // }
        // std::string ip_address = nullptr;
        // if (!m_is_server) {
        //     if (m_ip_address != nullptr) {
        //         ip_address = m_ip_address;
        //     }else {
        //         if (m_protocol == Protocol::IPv4Only || m_protocol == Protocol::Both_PreferIPv4) {
        //             ip_address = k_IPv4BroadcastAll;
        //         }else {
        //             ip_address = k_IPv6MulticastAll;
        //         }
        //     }
        // } 
        // int error_code = getaddrinfo(ip_address, m_port, &hints, &server_info); // Addr = nullptr and AI_PASSIVE mean bind to all
        // if (error_code != 0) {
        //     logError("Preparing Server Address Data: getaddrinfo() Failed: Error Code: %s", gai_strerror(error_code));
        //     return ErrorCode::ServerAddressGetInfoError;
        // }
        // // Locate the most fitting IP address
        // addrinfo* best = server_info;
        // for (addrinfo* current = server_info; current != nullptr; current = current->ai_next) {
        //     if (m_protocol == Protocol::IPv4Only || m_protocol == Protocol::Both_PreferIPv4) {
        //         if (current->ai_family == AF_INET) {
        //             best = current;
        //             break;
        //         }
        //     }else {
        //         if (current->ai_family == AF_INET6) {
        //             best = current;
        //             break;
        //         }
        //     }
        // }
        // // Finally, set up the data
        // memcpy(&m_server_address, best->ai_addr, best->ai_addrlen);
        // m_server_address_length = best->ai_addrlen;
        // freeaddrinfo(server_info);

        // logMessage("Server Address Data Prepared!\n");
        return ErrorCode::AllClear;
    };
    ErrorCode BaseSocket::bindSocket() {
        if (!m_is_server) {
            logError("Attempting to Bind Client Socket");
            return ErrorCode::AttemptingToBindClient;
        }
        if (getSocketState() <= SocketState::Created) {
            logError("Socket Must be Address Prepped Before being Bound");
            return ErrorCode::BindSocketBeforePrepAddress;
        }
        if (getSocketState() >= SocketState::Bound) {
            logMessage("Socket Has Already Been Bound");
            return ErrorCode::AllClear;
        }
        logMessage("Binding Socket...");

        int error_code = bind(m_socket, (struct sockaddr*)&m_server_address, m_server_address_length);
        if (error_code == SOCKET_ERROR) {
            logError("Binding Socket Failed with Error Code: %d", WSAGetLastError());
            return ErrorCode::SocketBindFailed;
        }

        logMessage("Binding Socket Complete!\n");
        setSocketState(SocketState::Bound);
        return ErrorCode::AllClear;
    };
    ErrorCode BaseSocket::serverStartListening() {
        if (!m_is_server) {
            logError("Attempting to Start Server Loop On Client");
            return ErrorCode::AttemptingToServerLoopClient;
        }
        if (getSocketState() <= SocketState::AddressPrepped) {
            logError("Socket Must be Bound Before Starting Listening");
            return ErrorCode::ServerLoopBeforeBound;
        }
        if (getSocketState() >= SocketState::ServerListening) {
            logMessage("Socket Is Already Listening");
            return ErrorCode::AllClear;
        }

        setSocketState(SocketState::ServerListening);
        // m_network_loop = std::jthread(&BaseSocket::ServerListeningLoop, this);
        m_network_loop = std::jthread([this](std::stop_token token) {
            this->ServerListeningLoop(std::move(token));
        });

        logMessage("Server Launched and Listening!\n");
        return ErrorCode::AllClear;
    };
    ErrorCode BaseSocket::serverAbortListening() {
        if (!m_is_server) {
            logError("Attempting to Abort Server Loop On Client");
            return ErrorCode::AttemptingToAbortServerLoopClient;
        }
        if (getSocketState() <= SocketState::Bound) {
            logError("Socket Must be Bound Before Starting Listening");
            return ErrorCode::ServerAbortBeforeStart;
        }

        m_network_loop.request_stop();
        if (m_network_loop.joinable() && m_network_loop.get_id() != std::this_thread::get_id()) {
            m_network_loop.join();
        }
        setSocketState(SocketState::Bound);

        logMessage("Server Aborted\n");
        return ErrorCode::AllClear;
    };


    void BaseSocket::setTargetIPAddress(std::string ip_address) {
        m_ip_address = ip_address;
        if (getSocketState() >= SocketState::AddressPrepped) {
            if (getSocketState() >= SocketState::ServerListening) {
                serverAbortListening();
            }
            setSocketState(SocketState::Created);
        }
    };

    
    void BaseSocket::ServerListeningLoop(std::stop_token stop_token) {
        bool quit = false;
        int failed_attempts = 0;
        while (!stop_token.stop_requested() && !quit) {
            logMessage("Waiting for more data...");
            // fflush(stdout); // Forces the text to be written to the screen now (shouldn't be needed if it ends with \n?)

            int m_address_struct_length = sizeof(m_client_address);
            // memset(m_incoming_data_buffer.data(), '\0', m_buffer_length); // Prevent old data from interfering with the new

            // Grab some data if it exists
            // Note: This is blocking!
            m_num_bytes_received = recvfrom(m_socket, m_incoming_data_buffer.data(), (int)m_buffer_length, 0, (struct sockaddr*)&m_client_address, &m_address_struct_length);
            if (m_num_bytes_received == SOCKET_ERROR) {
                int error_code = WSAGetLastError();

                if (error_code == WSAETIMEDOUT) {
                    continue;
                }

                logError("Server Receive Bytes Failed with Error Code: %d", error_code);
                recordNetworkLoopError(ErrorCode::ServerReceiveBytesFailed);
                if (failed_attempts < g_FlaschenpostNetworkRetries) {
                    failed_attempts += 1;
                    continue;
                }else {
                    break;
                }
            }

            // Print details of data and client
            // inet_ntoa -> IPv4 address to string in standard dotted format (8.8.8.8)
            // ntohs -> Network TO Host Short, Network is Big Endian, Windows is Little Endian
            // printf("Received packet from %s:%d\n", inet_ntoa(m_client_address.sin_addr), ntohs(m_client_address.sin_port));
            getnameinfo((sockaddr*)&m_client_address, m_address_struct_length, m_host_data, NI_MAXHOST, m_service_data, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
            logMessage("Received packet from %s:%s", m_host_data, m_service_data);
            logMessage("Data: '%.*s', length: %i\n", m_num_bytes_received, m_incoming_data_buffer.data(), m_num_bytes_received); // Print with length

            std::string_view message(m_incoming_data_buffer.data(), m_num_bytes_received);

            // if (std::string(m_incoming_data_buffer) == "quit") {
            // memcmp avoids allocating a std::string
            // if (m_num_bytes_received == 5 && memcmp(m_incoming_data_buffer.data(), "quit", 4) == 0) {
            if (message == "quit") {
                quit = true;
            }

            // Send a reply to the client who just messaged us
            int error_code = sendto(m_socket, m_incoming_data_buffer.data(), m_num_bytes_received, 0, (struct sockaddr*)&m_client_address, m_address_struct_length);
            if (error_code == SOCKET_ERROR) {
                logError("Server Send Bytes Failed with Error Code: %d", WSAGetLastError());
                recordNetworkLoopError(ErrorCode::ServerSendBytesFailed);
                if (failed_attempts < g_FlaschenpostNetworkRetries) {
                    failed_attempts += 1;
                    continue;
                }else {
                    break;
                }
            }

            failed_attempts = 0;
        }
        if (failed_attempts >= g_FlaschenpostNetworkRetries) {
            logError("Server Listening Loop Aborted Due to Failed Attempts");
        }else {
            logMessage("Server Listening Loop Ended");
        }
        if (getSocketState() >= SocketState::ServerListening) {
            setSocketState(SocketState::Bound);
        }
    };
    void BaseSocket::ClientToServerLoop(std::stop_token stop_token) {

    };
    void BaseSocket::ClientBroadcastLoop(std::stop_token stop_token) {

    };
    void BaseSocket::ClientMulticastLoop(std::stop_token stop_token) {

    };

    
    bool BaseSocket::prefersIPv4() {
        return (m_protocol == Protocol::IPv4Only || m_protocol == Protocol::Both_PreferIPv4);
    };
    bool BaseSocket::usesIPv4() {
        return !(m_protocol == Protocol::IPv6Only);
    };
    bool BaseSocket::prefersIPv6() {
        return (m_protocol == Protocol::IPv6Only || m_protocol == Protocol::Both_PreferIPv6);
    };
    bool BaseSocket::usesIPv6() {
        return !(m_protocol == Protocol::IPv4Only);
    };
    bool BaseSocket::usesIPv4AndIPv6() {
        return (m_protocol == Protocol::Both_PreferIPv4 || m_protocol == Protocol::Both_PreferIPv6);
    };


    void BaseSocket::setSocketState(SocketState state) {
        m_state.store(state);
    };
    SocketState BaseSocket::getSocketState() {
        return m_state.load();
    };
    void BaseSocket::recordNetworkLoopError(ErrorCode error) {
        m_network_loop_error.store(error);
    };
    ErrorCode BaseSocket::getNetworkLoopError() {
        return m_network_loop_error.load();
    };
    bool BaseSocket::isNetworkLoopRunning() {
        return getSocketState() >= SocketState::ServerListening;
    };

};