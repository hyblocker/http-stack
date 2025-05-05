#ifndef HTTP_INC
#define HTTP_INC

#include <inttypes.h>
#include <string>
#include <vector>
#include <unordered_map>
#if WIN32
#include <winsock.h>
#endif

namespace http::stack {

    enum class HttpStackError {
        // No errors were found
        OK = 0,
        // Failed to initialise
        ErrorFailInitialise,
        // The underlying networking sub-system is incompatible
        ErrorNetworkSubsystemIncompatible,
        // The network subsystem has already been initialised!
        ErrorAlreadyInitialised,
        // The network subsystem has not been initialised
        ErrorNotInitialised,
    };

    class Header {
        std::string key;
        std::string value;
    };

    class HttpRequest {

    };

    class HttpResponse {

    };

    // on windows sizeof(NativeSocket) = 8
    // on linux   sizeof(NativeSocket) = 4
#if WIN32
    typedef SOCKET NativeSocket;
#else
    typedef int NativeSocket;
#endif

    class HttpClient {
    public:
        HttpClient();
        ~HttpClient();

        HttpResponse MakeRequest(HttpRequest request, uint32_t timeoutMs = 30000, HttpStackError* pError = nullptr);
    private:
        NativeSocket m_hSocket = 0;
    };

    // OS-specific subsystem stuff
    HttpStackError init();
    HttpStackError shutdown();

}

#endif // HTTP_INC