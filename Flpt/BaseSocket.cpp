
#include "BaseSocket.h"

#include <cstdio>

namespace flpt {

    BaseSocket::BaseSocket(Protocol protocol, std::string port, bool is_server):
        BaseSocket(protocol, "", port, is_server)
    {

    };
    BaseSocket::BaseSocket(Protocol protocol, std::string ip_address, std::string port, bool is_server):
        m_protocol{ protocol },
        m_ip_address{ ip_address },
        m_port{ port },
        m_is_server{ is_server },
        m_socket{ INVALID_SOCKET },
        m_server_address{ },
        m_server_address_length{ 0 },
        m_Winsock_implementation{ },
        m_state{ SocketState::Uninitialized },
        m_network_loop{ }
    {

    };
    BaseSocket::~BaseSocket() {
        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
        }
        if (m_state >= SocketState::Initialized) {
            terminateGlobalWinsock();
        }
    };
    
    ErrorCode BaseSocket::initializeWinsock() {
        if (m_state >= SocketState::Initialized) {
            logMessage("Socket Initialization Already Completed");
            return ErrorCode::AllClear;
        }
        ErrorCode error_code = initializeGlobalWinsock();
        if (error_code == ErrorCode::AllClear) {
            m_state = SocketState::Initialized;
        }
        return error_code;
    };
    ErrorCode BaseSocket::createSocket() {
        logMessage("Creating Socket...");
        if (m_state <= SocketState::Uninitialized) {
            logError("Socket Must be Initialized Before it is Created");
            return ErrorCode::CreateSocketBeforeInitialize;
        }
        if (m_state >= SocketState::Created) {
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

        logMessage("Socket Creation Complete!\n");
        m_state = SocketState::Created;
        return ErrorCode::AllClear;
    };
    ErrorCode BaseSocket::prepServerAddress() {
        logMessage("Preparing Server Address Data...");
        if (m_state <= SocketState::Initialized) {
            logError("Socket Must be Created Before Address Prepped");
            return ErrorCode::PrepAddressBeforeCreateSocket;
        }
        if (m_state >= SocketState::AddressPrepped) {
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
        m_state = SocketState::AddressPrepped;
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
        if (m_state <= SocketState::Created) {
            logError("Socket Must be Address Prepped Before being Bound");
            return ErrorCode::BindSocketBeforePrepAddress;
        }
        if (m_state >= SocketState::Bound) {
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
        m_state = SocketState::Bound;
        return ErrorCode::AllClear;
    };
    ErrorCode BaseSocket::serverStartListening() {
        if (!m_is_server) {
            logError("Attempting to Start Server Loop On Client");
            return ErrorCode::AttemptingToServerLoopClient;
        }
        if (m_state <= SocketState::AddressPrepped) {
            logError("Socket Must be Bound Before Starting Listening");
            return ErrorCode::ServerLoopBeforeBound;
        }
        if (m_state >= SocketState::ServerListening) {
            logMessage("Socket Is Already Listening");
            return ErrorCode::AllClear;
        }

        m_network_loop = std::jthread(&BaseSocket::ServerListeningLoop, this);

        logMessage("Binding Socket Complete!\n");
        m_state = SocketState::ServerListening;
        return ErrorCode::AllClear;
    };


    void BaseSocket::setTargetIPAddress(std::string ip_address) {
        m_ip_address = ip_address;
        if (m_state >= SocketState::AddressPrepped) {
            m_state = SocketState::Created;
        }
    };

    
    void BaseSocket::ServerListeningLoop(std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {

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


};