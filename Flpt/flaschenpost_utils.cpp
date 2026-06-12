
#include "flaschenpost_utils.h"

#include <stdio.h>
#include <stdarg.h>


namespace flpt {
    
    const char* k_IPv4BroadcastAll = "255.255.255.255";
    const char* k_IPv6MulticastAll = "ff02::1";

    Verbosity g_FlaschenpostVerbosity = Verbosity::ErrorLogs;
    std::string g_FlaschenpostLastError = "No Error";

    bool g_WinsockInitialized = false;
    int g_WinsockObjectCount = 0;
    WSADATA g_WinsockImplementation;

    int g_FlaschenpostNetworkRetries = 20;
    int g_FlaschenpostNetworkTimeoutMS = 200;
    bool g_FlaschenpostNetworkHasTimeout = true;


    void setFlptVerbosity(Verbosity verbosity) {
        g_FlaschenpostVerbosity = verbosity;
    };
    std::string getLastFlptError() {
        return g_FlaschenpostLastError;
    };
    void logMessage(const char* format, ...) {
        if (g_FlaschenpostVerbosity >= Verbosity::AllLogs) {
            va_list argument_pointer;
            fprintf(stdout, "FLPT INFO: ");
            va_start(argument_pointer, format);
            vfprintf(stdout, format, argument_pointer);
            va_end(argument_pointer);
            fprintf(stdout, "\n");
            fflush(stdout);
        }
    };
    void logError(const char* format, ...) {
        va_list argument_pointer;
        va_start(argument_pointer, format);
        va_list argument_pointer_copy;
        va_copy(argument_pointer_copy, argument_pointer); // Copy because va_list can only be consumed once
        int length = vsnprintf(nullptr, 0, format, argument_pointer); // Test length in order to resize buffer
        va_end(argument_pointer);
        if (length >= 0) {
            std::string buffer(length+1, '\0');
            vsnprintf(buffer.data(), buffer.size(), format, argument_pointer_copy); // Print to buffer
            g_FlaschenpostLastError = buffer.c_str();
        }
        va_end(argument_pointer_copy);
        if (g_FlaschenpostVerbosity >= Verbosity::ErrorLogs) {
            fprintf(stderr, "FLPT ERROR: %s\n", g_FlaschenpostLastError.c_str());
            fflush(stderr);
        }
    };

    
    std::string getErrorCodeDescription(ErrorCode error) {
        switch (error) {
            case ErrorCode::AllClear:
                return "All Clear: No Error Occured";
            case ErrorCode::GeneralError:
                return "General Error: An Unspecified Exception has Occured";
            case ErrorCode::WinsockInitialization:
                return "Initialization of Winsock 2.2 has Failed with an Error";
            case ErrorCode::SocketCreationFailed:
                return "Socket Creation Failed";
            case ErrorCode::SocketAllowDualStackError:
                return "Socket Disabling 'IPv6 Only' Failed";
            case ErrorCode::SocketAllowBroadcastError:
                return "Socket Options 'Allow Broadcast' Enable Failed";
            case ErrorCode::ServerAddressGetInfoError:
                return "Preparing Server Address Data: getaddrinfo() Failed";
            case ErrorCode::ServerAddressNoValidIP:
                return "Preparing Server Address Data: getaddrinfo() Returned No Valid IP";
            case ErrorCode::AttemptingToBindClient:
                return "Attempting to Bind Client Socket";
            case ErrorCode::SocketBindFailed:
                return "Binding Socket Failed with an Error";
            case ErrorCode::WinsockTermination:
                return "Termination of Winsock 2.2 has Failed with an Error";
            case ErrorCode::CreateSocketBeforeInitialize:
                return "Socket Must be Initialized Before it is Created";
            case ErrorCode::PrepAddressBeforeCreateSocket:
                return "Socket Must be Created Before Address Prepped";
            case ErrorCode::BindSocketBeforePrepAddress:
                return "Socket Must be Address Prepped Before being Bound";
            case ErrorCode::AttemptingToServerLoopClient:
                return "Attempting to Start Server Loop On Client";
            case ErrorCode::ServerLoopBeforeBound:
                return "Socket Must be Bound Before Starting Listening";
            case ErrorCode::ServerReceiveBytesFailed:
                return "Server Receive Bytes Failed with Error";
            case ErrorCode::ServerSendBytesFailed:
                return "Server Send Bytes Failed with Error";
            case ErrorCode::SocketSetRecvTimeoutError:
                return "Socket Options 'Recv Timeout' Enable Failed with Error";
            
            default:
                return "Unknown Error";
                break;
        }
    };
    

    ErrorCode initializeGlobalWinsock() {
        if (!g_WinsockInitialized) {
            logMessage("Beginning Winsock Initialization...");

            int error_code = WSAStartup(MAKEWORD(2,2), &g_WinsockImplementation); // MAKEWORD(2,2) -> request version 2.2
            if (error_code != 0) {
                logError("Winsock Initialization Failed with Error Code: %d", WSAGetLastError());
                return ErrorCode::WinsockInitialization;
            }

            g_WinsockInitialized = true;
            logMessage("Winsock Initialization Complete!");
        }else {
            logMessage("Winsock Initialization Already Completed");
        }
        g_WinsockObjectCount += 1;
        return ErrorCode::AllClear;
    };
    ErrorCode terminateGlobalWinsock() {
        g_WinsockObjectCount -= 1;
        if (g_WinsockObjectCount == 0) {
            logMessage("Terminating Winsock...");
            int error_code = WSACleanup();
            if (error_code == SOCKET_ERROR) {
                logError("Winsock Termination Failed with Error Code: %d", WSAGetLastError());
                return ErrorCode::WinsockTermination;
            }
        }
        return ErrorCode::AllClear;
    };

    void setFlptNetworkRetries(int retries) {
        g_FlaschenpostNetworkRetries = retries;
    };
    void setFlptNetworkTimeout(bool has_timeout) {
        g_FlaschenpostNetworkHasTimeout = has_timeout;
    };
    void setFlptNetworkTimeout(bool has_timeout, int timeout_ms) {
        g_FlaschenpostNetworkHasTimeout = has_timeout;
        g_FlaschenpostNetworkTimeoutMS = timeout_ms;
    };

};