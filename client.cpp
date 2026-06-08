
#include <WinSock2.h>
#include <cstdio>
#include <string>
#include <WS2tcpip.h>

// constexpr const char* kServerIPAddress = "127.0.0.1"; // i.e. localhost (IPv4)
constexpr const char* kServerIPAddress = "192.168.0.20"; // KrampusMeister (IPv4)
// constexpr const char* kServerIPAddress = "fe80::5189:770e:49fb:7080%2"; // KrampusMeister
// constexpr const char* kServerIPAddress = "::1"; // localhost (IPv6)
constexpr bool kUseIPv6 = false;
constexpr int kServerBufferLength = 512;
// constexpr int kServerPort = 8888;
constexpr const char* kServerPort = "8888";


int main() {
    printf("Program Alive!\n");
    SOCKET m_socket;
    // sockaddr_in6 m_server_address; // IP Address and Port
    addrinfo* m_server_info; // IP Address and Port
    sockaddr_storage m_server_address; // Permanent copy of important part of m_server_info
    sockaddr_storage m_sender_address; // IPv4/IPv6 independent
    socklen_t m_server_address_length;
    int m_address_struct_length;
    int m_num_bytes_received;
    char m_incoming_data_buffer[kServerBufferLength];
    char m_message_to_send[kServerBufferLength];
    WSADATA m_Winsock_implementation;

    int m_error_code; // Temporary store for return codes
    char m_host_data[NI_MAXHOST]; // Temporary stores for IPv4/IPv6 client address info
    char m_service_data[NI_MAXSERV];

    // Winsock Initialization
    printf("Beginning Winsock Initialization...\n");
    m_error_code = WSAStartup(MAKEWORD(2,2), &m_Winsock_implementation); // MAKEWORD(2,2) -> request version 2.2
    if (m_error_code != 0) {
        printf("Winsock Initialization Failed with Error Code: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }
    printf("Winsock Initialization Complete!\n\n");

    // Create a Socket
    printf("Creating Socket...\n");
    m_socket = socket(kUseIPv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP); // AF_INET6 = IPv6, SOCK_DGRAM = UDP, IPPROTO_UDP = explicitly UDP
    if (m_socket == INVALID_SOCKET) {
        printf("Socket Creation Failed with Error Code: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }
    DWORD accept_ipv6_only = 0; // setsockopt to set the option to limit to only IPv6 to off (allow IPv4 as well)
    m_error_code = setsockopt(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&accept_ipv6_only, sizeof(accept_ipv6_only));
    if (m_error_code == SOCKET_ERROR) {
        printf("Socket Options 'IPv6 Only' Disable Failed with Error Code: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }
    printf("Socket Creation Complete!\n\n");

    // Prepare Server Address Data
    printf("Preparing Server Address Data...\n");
    // memset(&m_server_address, 0, sizeof(m_server_address));
    // Setup for getaddrinfo()
    struct addrinfo hints;
    // addrinfo* result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = kUseIPv6 ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    m_error_code = getaddrinfo(kServerIPAddress, kServerPort, &hints, &m_server_info);
    if (m_error_code != 0) {
        printf("Preparing Server Address Data: getaddrinfo() Failed: Error Code: %s\n", gai_strerror(m_error_code));
        return EXIT_FAILURE;
    }
    // Finally, set up the data
    memset(&m_server_address, 0, sizeof(m_server_address));
    memcpy(&m_server_address, m_server_info->ai_addr, m_server_info->ai_addrlen);
    m_server_address_length = m_server_info->ai_addrlen;
    freeaddrinfo(m_server_info);
    // memcpy(&m_server_address, result->ai_addr, sizeof(sockaddr_in6));
    // m_server_address.sin6_family = AF_INET6; // Server uses IPv6
    // m_server_address.sin6_addr = ((struct sockaddr_in6*)result->ai_addr)->sin6_addr;
    // m_server_address.sin6_port = htons(kServerPort);
    // freeaddrinfo(result);

    // Main Communication Loop
    bool m_quit_loop = false;
    while (!m_quit_loop) {
        printf("Enter message: \n");
        fgets(m_message_to_send, kServerBufferLength, stdin);

        m_error_code = sendto(m_socket, m_message_to_send, strlen(m_message_to_send), 0, (sockaddr*)&m_server_address, m_server_address_length);
        if (m_error_code == SOCKET_ERROR) {
            printf("Send Bytes Failed with Error Code: %d\n", WSAGetLastError());
            return EXIT_FAILURE;
        }

        memset(m_incoming_data_buffer, '\0', kServerBufferLength);
        m_address_struct_length = sizeof(m_sender_address);

        m_num_bytes_received = recvfrom(m_socket, m_incoming_data_buffer, kServerBufferLength, 0, (struct sockaddr*)&m_sender_address, &m_address_struct_length);
        if (m_num_bytes_received == SOCKET_ERROR) {
            printf("Receive Bytes Failed with Error Code: %d\n", WSAGetLastError());
            return EXIT_FAILURE;
        }

        getnameinfo((sockaddr*)&m_sender_address, m_address_struct_length, m_host_data, NI_MAXHOST, m_service_data, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        printf("Received packet from %s:%s\n", m_host_data, m_service_data);
        printf("Data: '%.*s', length: %i\n\n", m_num_bytes_received, m_incoming_data_buffer, m_num_bytes_received); // Print with length

        if (m_num_bytes_received == 5 && memcmp(m_incoming_data_buffer, "quit", 4) == 0) {
            m_quit_loop = true;
        }
    }


    // Cleanup Environment
    printf("Destroying Environment...\n");
    // freeaddrinfo(m_server_info);
    closesocket(m_socket);
    WSACleanup();
    printf("Exiting...\n");

    return 0;
};