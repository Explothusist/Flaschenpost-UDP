
#include <WinSock2.h>
#include <cstdio>
#include <string>
#include <WS2tcpip.h>
#include <chrono>

#include "Flpt/flashenpost.h"

// #pragma comment(lib, "ws2_32.lib") // Tells compiler to link against dependency (alternative)

/*
    Test with ncat (part of nmap) in cmd using the following command:
    ncat -4 -vv -u 127.0.0.1 8888

    -4 -> force IPv4 (instead of IPv6)
    -vv -> 'very verbose'
    -u -> force UDP (instead of TCP)
    127.0.0.1 -> This computer (like localhost)
    8888 -> port number 
*/

constexpr int kServerBufferLength = 512;
// constexpr int kServerPort = 8888;
constexpr const char* kServerPort = "8888";

/*
    IPv4Only        - Works
    IPv6Only        - Works
    Both_PreferIPv4 - Works
    Both_PreferIPv6 - Works
*/

int main() {
    printf("Program Alive!\n");

    flpt::setFlptVerbosity(flpt::Verbosity::AllLogs);
    flpt::setFlptBufferLength(kServerBufferLength);

    flpt::UDPServer m_server(flpt::Protocol::Both_PreferIPv6, kServerPort);

    m_server.launchServer();

    while (m_server.isServerRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // SOCKET m_socket;
    // // sockaddr_in m_server_address; // IP Address and Port
    // // sockaddr_in m_client_address;
    // // sockaddr_in6 m_server_address; // IP Address and Port
    // addrinfo* m_server_info = nullptr;
    // sockaddr_storage m_server_address; // Permanent copy of important part of m_server_info
    // sockaddr_storage m_client_address; // IPv4/IPv6 independent
    // socklen_t m_server_address_length;
    // int m_address_struct_length;
    // int m_num_bytes_received;
    // char m_incoming_data_buffer[kServerBufferLength];
    // WSADATA m_Winsock_implementation;

    // int m_error_code; // Temporary store for return codes
    // char m_host_data[NI_MAXHOST]; // Temporary stores for IPv4/IPv6 client address info
    // char m_service_data[NI_MAXSERV];

    // // Winsock Initialization
    // printf("Beginning Winsock Initialization...\n");
    // m_error_code = WSAStartup(MAKEWORD(2,2), &m_Winsock_implementation); // MAKEWORD(2,2) -> request version 2.2
    // if (m_error_code != 0) {
    //     printf("Winsock Initialization Failed with Error Code: %d\n", WSAGetLastError());
    //     // exit(EXIT_FAILURE); // exit == return from main, EXIT_FAILURE == 1
    //     return EXIT_FAILURE;
    // }
    // printf("Winsock Initialization Complete!\n\n");

    // // Create a Socket
    // printf("Creating Socket...\n");
    // // m_socket = socket(AF_INET, SOCK_DGRAM, 0); // AF_INET = IPv4, SOCK_DGRAM = UDP, 0 = use default protocol (AF_INET+SOCK_DGRAM+0 = UDP)
    // m_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP); // AF_INET6 = IPv6, IPPROTO_UDP = explicitly UDP
    // if (m_socket == INVALID_SOCKET) {
    //     printf("Socket Creation Failed with Error Code: %d\n", WSAGetLastError());
    //     // exit(EXIT_FAILURE); // exit == return from main, EXIT_FAILURE == 1
    //     return EXIT_FAILURE;
    // }
    // DWORD accept_ipv6_only = 0; // setsockopt to set the option to limit to only IPv6 to off (allow IPv4 as well)
    // m_error_code = setsockopt(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&accept_ipv6_only, sizeof(accept_ipv6_only));
    // if (m_error_code == SOCKET_ERROR) {
    //     printf("Socket Options 'IPv6 Only' Disable Failed with Error Code: %d\n", WSAGetLastError());
    //     return EXIT_FAILURE;
    // }
    // printf("Socket Creation Complete!\n\n");

    // // Prepare Server Address Data
    // printf("Preparing Server Address Data...\n");
    // // memset(&m_server_address, 0, sizeof(m_server_address));
    // // // m_server_address.sin_family = AF_INET; // (socket input family) Listen on IPv4
    // // // m_server_address.sin_addr.s_addr = INADDR_ANY; // (socket input address) Listen to all IP Addresses
    // // // m_server_address.sin_port = htons(kServerPort); // (socket input port) Host TO Network Short -> Network is Big Endian, Windows Little Endian
    // // m_server_address.sin6_family = AF_INET6; // (socket input family) Listen on IPv6
    // // m_server_address.sin6_addr = in6addr_any; // (socket input address) Listen to all IP Addresses on both IPv4 and IPv6
    // // m_server_address.sin6_port = htons(kServerPort); // (socket input port) Host TO Network Short -> Network is Big Endian, Windows Little Endian
    // struct addrinfo hints;
    // memset(&hints, 0, sizeof(hints));
    // hints.ai_family = AF_INET6;
    // hints.ai_socktype = SOCK_DGRAM;
    // hints.ai_protocol = IPPROTO_UDP;
    // hints.ai_flags = AI_PASSIVE; // Say "I'm a server"
    // m_error_code = getaddrinfo(nullptr, kServerPort, &hints, &m_server_info); // Addr = nullptr and AI_PASSIVE mean bind to all
    // if (m_error_code != 0) {
    //     printf("Preparing Server Address Data: getaddrinfo() Failed: Error Code: %s\n", gai_strerror(m_error_code));
    //     return EXIT_FAILURE;
    // }
    // // Finally, set up the data
    // memcpy(&m_server_address, m_server_info->ai_addr, m_server_info->ai_addrlen);
    // m_server_address_length = m_server_info->ai_addrlen;
    // freeaddrinfo(m_server_info);
    // printf("Server Address Data Prepared!\n\n");

    // // Bind Socket
    // printf("Binding Socket...\n");
    // m_error_code = bind(m_socket, (struct sockaddr*)&m_server_address, m_server_address_length);
    // // m_error_code = bind(m_socket, m_server_info->ai_addr, (int)m_server_info->ai_addrlen);
    // if (m_error_code == SOCKET_ERROR) {
    //     printf("Binding Socket Failed with Error Code: %d\n", WSAGetLastError());
    //     // exit(EXIT_FAILURE); // exit == return from main, EXIT_FAILURE == 1
    //     return EXIT_FAILURE;
    // }
    // printf("Binding Socket Complete!\n\n");


    // // Main Listening Loop
    // bool m_quit_loop = false;
    // while (!m_quit_loop) {
    //     printf("Waiting for more data...\n");
    //     // fflush(stdout); // Forces the text to be written to the screen now (shouldn't be needed if it ends with \n?)

    //     m_address_struct_length = sizeof(m_client_address);
    //     memset(m_incoming_data_buffer, '\0', kServerBufferLength); // Prevent old data from interfering with the new

    //     // Grab some data if it exists
    //     // Note: This is blocking!
    //     m_num_bytes_received = recvfrom(m_socket, m_incoming_data_buffer, kServerBufferLength, 0, (struct sockaddr*)&m_client_address, &m_address_struct_length);
    //     if (m_num_bytes_received == SOCKET_ERROR) {
    //         printf("Receive Bytes Failed with Error Code: %d\n", WSAGetLastError());
    //         // exit(EXIT_FAILURE); // exit == return from main, EXIT_FAILURE == 1
    //         return EXIT_FAILURE;
    //     }

    //     // Print details of data and client
    //     // inet_ntoa -> IPv4 address to string in standard dotted format (8.8.8.8)
    //     // ntohs -> Network TO Host Short, Network is Big Endian, Windows is Little Endian
    //     // printf("Received packet from %s:%d\n", inet_ntoa(m_client_address.sin_addr), ntohs(m_client_address.sin_port));
    //     getnameinfo((sockaddr*)&m_client_address, m_address_struct_length, m_host_data, NI_MAXHOST, m_service_data, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    //     printf("Received packet from %s:%s\n", m_host_data, m_service_data);
    //     printf("Data: '%.*s', length: %i\n\n", m_num_bytes_received, m_incoming_data_buffer, m_num_bytes_received); // Print with length

    //     // if (std::string(m_incoming_data_buffer) == "quit") {
    //     // memcmp avoids allocating a std::string
    //     if (m_num_bytes_received == 5 && memcmp(m_incoming_data_buffer, "quit", 4) == 0) {
    //         m_quit_loop = true;
    //     }

    //     // Send a reply to the client who just messaged us
    //     m_error_code = sendto(m_socket, m_incoming_data_buffer, m_num_bytes_received, 0, (struct sockaddr*)&m_client_address, m_address_struct_length);
    //     if (m_error_code == SOCKET_ERROR) {
    //         printf("Send Bytes Failed with Error Code: %d\n", WSAGetLastError());
    //         // exit(EXIT_FAILURE); // exit == return from main, EXIT_FAILURE == 1
    //         return EXIT_FAILURE;
    //     }
    // }


    // // Cleanup Environment
    // printf("Destroying Environment...\n");
    // // freeaddrinfo(m_server_info);
    // closesocket(m_socket);
    // WSACleanup();
    // printf("Exiting...\n");

    // return 0;
};