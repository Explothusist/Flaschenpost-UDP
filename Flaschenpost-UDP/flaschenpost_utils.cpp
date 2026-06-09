
#include "flaschenpost_utils.h"

#include <stdio.h>
#include <stdarg.h>


namespace flpt {


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
                return "Winsock Initialization: Initialization of Winsock 2.2 has Failed with an Error";
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
            
            default:
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
            logMessage("Winsock Initialization Complete!\n");
            return ErrorCode::AllClear;
        }else {
            logMessage("Winsock Initialization Already Completed\n");
            return ErrorCode::AllClear;
        }
    };

};