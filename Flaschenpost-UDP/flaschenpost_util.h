
#ifndef FLASCHENPOST_UTILS_
#define FLASCHENPOST_UTILS_

#include <string>


namespace flpt {

    enum class Verbosity {
        NoLogs = 0,
        ErrorLogs = 1,
        AllLogs = 2
    };

    enum class ErrorCode {
        AllClear = 0,
        GeneralError = 1,
        WinsockInitialization = 2
    };

    std::string getErrorCodeDescription(ErrorCode error) {
        switch (error) {
            case ErrorCode::AllClear:
                return "All Clear: No Error Occured";
            case ErrorCode::GeneralError:
                return "General Error: An Unspecified Exception has Occured";
            case ErrorCode::WinsockInitialization:
                return "Winsock Initialization: Initialization of Winsock 2.2 has Failed with an Error";
            
            default:
                break;
        }
    };

};

#endif