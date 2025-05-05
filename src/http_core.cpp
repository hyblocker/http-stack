#include "http.hpp"


namespace http::stack {

    // whether the network subsystem has been initialised already
    bool __subsystemIsInitialised();

    HttpClient::HttpClient() {
        m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    
    HttpClient::~HttpClient() {
        int err = closesocket(m_hSocket);
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

}