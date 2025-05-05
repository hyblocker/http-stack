#include <stdio.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "http.hpp"

int main() {

    http::stack::HttpStackError error = http::stack::init();

    http::stack::HttpClient client; // container for a native connection (socket)
    http::stack::HttpRequest req = { { "https://google.com:6969/test?penis" }, http::stack::RequestType::REQUEST_GET };
    http::stack::HttpResponse resp = client.MakeRequest(req);

    SOCKET netSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    const char* k_hostname = "google.com";

    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char szIp4str[INET6_ADDRSTRLEN] = {};
    size_t dwIp4strSize = INET6_ADDRSTRLEN;

    struct addrinfo* result = nullptr, *rp = nullptr;
    int status = getaddrinfo(k_hostname, NULL, &hints, &result);
    if (status == 0) {
        for (rp = result; rp != NULL; rp = rp->ai_next) {
            void* addr = nullptr;

            if (rp->ai_family == AF_INET) {
                struct sockaddr_in* ipv4 = (struct sockaddr_in*)rp->ai_addr;
                addr = &(ipv4->sin_addr);
            } else {
                struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)rp->ai_addr;
                addr = &(ipv6->sin6_addr);
            }

            if (inet_ntop(rp->ai_family, addr, szIp4str, dwIp4strSize) != NULL) {
                printf("resolved %s to %s!\n", k_hostname, szIp4str);
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

    struct sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("142.250.180.174"); // google.com
    serverAddress.sin_port = htons(80);

    int err = connect(netSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // 4K buffer
    size_t dwBufferSize = 4096;
    char* dataBuffer = (char*) malloc(dwBufferSize);
    memset(dataBuffer, 0, dwBufferSize);

    const char* httpGetRequest = "GET / HTTP/1.1\r\n"
        "Host: www.google.com\r\n"
        "\r\n";

    err = send(netSocket, httpGetRequest, strlen(httpGetRequest), 0);

    err = recv(netSocket, dataBuffer, dwBufferSize, 0);

    err = closesocket(netSocket);

    printf("%s\n", dataBuffer);

    error = http::stack::shutdown();
    return 0;
}