
#ifndef FLASCHENPOST_UTILS_
#define FLASCHENPOST_UTILS_

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>


namespace flpt {

    const char* k_IPv4BroadcastAll = "255.255.255.255";
    const char* k_IPv6MulticastAll = "ff02::1";

    enum class Verbosity {
        NoLogs = 0,
        ErrorLogs = 1,
        AllLogs = 2
    };

    Verbosity g_FlaschenpostVerbosity = Verbosity::ErrorLogs;
    std::string g_FlaschenpostLastError = "No Error";

    void setFlptVerbosity(Verbosity verbosity);
    std::string getLastFlptError();
    void logMessage(const char* format, ...);
    void logError(const char* format, ...);


    enum class Protocol {
        IPv4Only = 0,
        IPv6Only = 1,
        Both_PreferIPv4 = 2,
        Both_PreferIPv6 = 3
    };

    enum class ErrorCode {
        AllClear = 0,
        GeneralError = 1,
        WinsockInitialization = 2,
        SocketCreationFailed = 3,
        SocketAllowDualStackError = 4,
        SocketAllowBroadcastError = 5,
        ServerAddressGetInfoError = 6,
        ServerAddressNoValidIP = 7,
        AttemptingToBindClient = 8,
        SocketBindFailed = 9
    };

    std::string getErrorCodeDescription(ErrorCode error);


    bool g_WinsockInitialized = false;
    WSADATA g_WinsockImplementation;

    ErrorCode initializeGlobalWinsock();

};

#endif