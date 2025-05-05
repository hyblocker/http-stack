#include "http.hpp"

#if WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

namespace http::stack {

    // whether the network subsystem has been initialised already
    bool __subsystemIsInitialised();

    Url::Url(const std::string& url)
        : href(url)
    {
        constexpr size_t k_invalidSize = 0xFFFFFFFFFFFFFFFF;

        // Split by url parts
        std::string_view href_view(url);

        size_t protocolIdx = k_invalidSize; // index of colon with http: or https: etc
        size_t originIdx = k_invalidSize;   // first char of origin (domain)
        size_t pathnameIdx = k_invalidSize; // first / of pathname if valid
        size_t portIdx = k_invalidSize;     // first : of port if valid

        // simple lexer to make sure a url is decoded properly
        // index of works mostly but if a malformed string is inputted the app will crash and thats not good lol
        
        enum class UrlParseState {
            Start,
            ProtocolColon,
            ProtocolSlash1,
            ProtocolSlash2,
            Origin,
            Port,
            Pathname,
            End,
            Count
        };

        UrlParseState currentUrlParseState = UrlParseState::Start;
        for (size_t i = 0; i < url.size(); i++) {
            switch (url[i]) {
            case ':':
                if (currentUrlParseState == UrlParseState::Start) {
                    currentUrlParseState = UrlParseState::ProtocolColon;
                }
                else if (currentUrlParseState == UrlParseState::Origin) {
                    currentUrlParseState = UrlParseState::Port;
                    portIdx = i;
                }
                break;
            case '/':
                if (currentUrlParseState == UrlParseState::ProtocolColon) {
                    currentUrlParseState = UrlParseState::ProtocolSlash1;
                    protocolIdx = i;
                }
                else if (currentUrlParseState == UrlParseState::ProtocolSlash1) {
                    currentUrlParseState = UrlParseState::ProtocolSlash2;
                }
                else if (currentUrlParseState == UrlParseState::Origin || currentUrlParseState == UrlParseState::Port) {
                    currentUrlParseState = UrlParseState::Pathname;
                    pathnameIdx = i;
                }
                break;
            default:
                if (currentUrlParseState == UrlParseState::ProtocolSlash2) {
                    currentUrlParseState = UrlParseState::Origin;
                    originIdx = i;
                }
                break;
            }
        }

        if (pathnameIdx == k_invalidSize) {
            pathnameIdx = href_view.size();
        }
        size_t portIdxRead = portIdx == k_invalidSize ? pathnameIdx : portIdx + 1;
        std::string szPort = url.substr(portIdxRead, pathnameIdx - portIdxRead);
        if (portIdx == k_invalidSize) {
            if (pathnameIdx == k_invalidSize) {
                portIdx = href_view.size();
            } else {
                portIdx = pathnameIdx;
            }
        }
        protocol = href_view.substr(0, protocolIdx);
        origin = href_view.substr(originIdx, portIdx - originIdx);
        pathname = href_view.substr(pathnameIdx);

        // determine port basd on protocol
        if (protocol == "http:") {
            port = k_PORT_HTTP;
        }
        else if (protocol == "https:") {
            port = k_PORT_HTTPS;
        }

        // if a port was specified use it
        if (szPort.size() > 0) {
            errno = 0; // Reset errno before the call
            char* endptr = nullptr;
            long result = strtol(szPort.c_str(), &endptr, 10);

            if (endptr != szPort.data() &&
                static_cast<std::size_t>(endptr - szPort.data()) == szPort.size() &&
                errno != ERANGE &&
                result >= std::numeric_limits<int>::min() &&
                result <= std::numeric_limits<int>::max()) {
                // port is valid, override the default one
                port = result;
            }
        }

        if (pathname.size() == 0) {
            pathname = "/";
        }
    }
    Url::~Url() {}

    HttpRequest::HttpRequest(Url url, RequestType eRequestType)
        : url(url), requestType(eRequestType)
    {

    }

    HttpClient::HttpClient() {
        m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    
    HttpClient::~HttpClient() {
        int err = closesocket(m_hSocket);
        // @TODO: Error handling
    }

    HttpResponse HttpClient::MakeRequest(HttpRequest request, uint32_t timeoutMs, HttpStackError* pError) {

        if (!__subsystemIsInitialised()) {
            if (pError) {
                *pError = HttpStackError::ErrorNotInitialised;
            }
            return {};
        }

        HttpResponse resp = {};

        int err = 0;
#if WIN32
        err = setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
#else
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeoutMs;
        err = setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
        // @TODO: Error handling, check return results of setsockopt

        return resp;
    }

    std::string HttpClient::ResolveDomainToIp(const std::string& szOrigin) {
        struct addrinfo hints = {};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        char szIp4str[INET6_ADDRSTRLEN] = {};
        size_t dwIp4strSize = INET6_ADDRSTRLEN;

        struct addrinfo* result = nullptr, * rp = nullptr;
        int status = getaddrinfo(szOrigin.c_str(), NULL, &hints, &result);
        if (status == 0) {
            for (rp = result; rp != NULL; rp = rp->ai_next) {
                void* addr = nullptr;

                if (rp->ai_family == AF_INET) {
                    struct sockaddr_in* ipv4 = (struct sockaddr_in*)rp->ai_addr;
                    addr = &(ipv4->sin_addr);
                }
                else {
                    struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)rp->ai_addr;
                    addr = &(ipv6->sin6_addr);
                }

                if (inet_ntop(rp->ai_family, addr, szIp4str, dwIp4strSize) != NULL) {
                    if (result) {
                        freeaddrinfo(result);
                        result = nullptr;
                    }
                }
            }

            if (result) {
                freeaddrinfo(result);
                result = nullptr;
            }
        }

        return std::string(szIp4str);
    }
}