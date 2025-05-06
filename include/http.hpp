#ifndef HTTP_INC
#define HTTP_INC

#include <inttypes.h>
#include <string>
#include <vector>
#include <unordered_map>
#if WIN32
#include <winsock2.h>
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

    enum class IpVersion {
        IpV4,
        IpV6,
    };
    
    enum class RequestType {
        REQUEST_GET,
        REQUEST_HEAD,
        REQUEST_POST,
        REQUEST_PUT,
        REQUEST_DELETE,
        REQUEST_CONNECT,
        REQUEST_OPTIONS,
        REQUEST_TRACE,
        REQUEST_PATCH
    };

    constexpr uint16_t k_PORT_HTTP = 80;
    constexpr uint16_t k_PORT_HTTPS = 443;

    // a URL object
    // mirrors a javascript URL object pretty much
    struct Url {
    public:
        Url(const std::string& url);
        ~Url();

        std::string origin; // domain
        std::string protocol; // http: or https: usually
        uint16_t port = k_PORT_HTTP;
        std::string pathname;
        std::string href; // actual url, everything else is a view into it
    };

    class HttpRequest {
    public:
        HttpRequest(Url url, RequestType eRequestType);
        Url url;
        RequestType requestType;

        std::unordered_map<std::string, std::string> headers = {
            // Default headers
            { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:128.0) Gecko/20100101 Firefox/128.0" }
        };
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
        std::string ResolveDomainToIp(const std::string& szOrigin, int& dwFamily);
        HttpStackError SetTimeout(const uint32_t dwTimeoutMs = 30000);
    private:
        NativeSocket m_hSocket = 0;
        IpVersion m_eIpVersion = IpVersion::IpV4;
        uint32_t m_dwTimeout = 0;
    };

    // OS-specific subsystem stuff
    HttpStackError init();
    HttpStackError shutdown();

}

#endif // HTTP_INC