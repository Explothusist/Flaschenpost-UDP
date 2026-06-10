
#ifndef FLASCHENPOST_UTILS_
#define FLASCHENPOST_UTILS_

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>


namespace flpt {

    enum class Verbosity {
        NoLogs = 0,
        ErrorLogs = 1,
        AllLogs = 2
    };

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

    enum class SocketState {
        Uninitialized = 0,
        Initialized = 1,
        Created = 2,
        AddressPrepped = 3,
        Bound = 4,
        Connected = 5,
        Broadcasting = 6,
        Multicasting = 7,
        ServerListening = 8
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
        SocketBindFailed = 9,
        WinsockTermination = 10,
        CreateSocketBeforeInitialize = 11,
        PrepAddressBeforeCreateSocket = 12,
        BindSocketBeforePrepAddress = 13,
        AttemptingToServerLoopClient = 14,
        ServerLoopBeforeBound = 15
    };

    std::string getErrorCodeDescription(ErrorCode error);

    ErrorCode initializeGlobalWinsock();
    ErrorCode terminateGlobalWinsock();

    extern const char* k_IPv4BroadcastAll;
    extern const char* k_IPv6MulticastAll;

    extern Verbosity g_FlaschenpostVerbosity;
    extern std::string g_FlaschenpostLastError;

    extern bool g_WinsockInitialized;
    extern int g_WinsockObjectCount;
    extern WSADATA g_WinsockImplementation;

};

#endif