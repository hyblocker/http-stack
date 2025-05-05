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
        // Split by url parts
        std::string_view href_view(url);
        protocol = href_view.substr(0, href_view.find("://") + 1);
        
        // determine port basd on protocol
        if (protocol == "http:") {
            port = k_PORT_HTTP;
        } else if (protocol == "https:") {
            port = k_PORT_HTTPS;
        }

        // get port if defined in url
        std::string_view szPort = href_view.substr(href_view.find("://") + 1);
        szPort = szPort.substr(szPort.find(":") + 1);
        szPort = szPort.substr(0, szPort.find("/"));

        pathname = href_view.substr(href_view.find("://") + 3);
        pathname = pathname.substr(pathname.find("/"));

        origin = href_view.substr(href_view.find("://") + 3);
        origin = origin.substr(0, origin.find("/"));
        origin = origin.substr(0, origin.find(":"));
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