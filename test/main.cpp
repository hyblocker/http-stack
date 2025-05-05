#include <stdio.h>

#include <windows.h>

#include "http.hpp"

int main() {

    http::stack::HttpStackError error = http::stack::init();

    http::stack::HttpClient client; // container for a native connection (socket)
    http::stack::HttpRequest req = {};
    http::stack::HttpResponse resp = client.MakeRequest(req);

    SOCKET netSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("142.250.180.174"); // google.com
    serverAddress.sin_port = htons(80);

    err = connect(netSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

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