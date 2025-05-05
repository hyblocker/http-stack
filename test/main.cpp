#include <stdio.h>
#include "http.hpp"

int main() {

    http::stack::HttpStackError error = http::stack::init();

    http::stack::HttpClient client; // container for a native connection (socket)
    http::stack::HttpRequest req = { { "http://www.google.com" }, http::stack::RequestType::REQUEST_GET };
    http::stack::HttpResponse resp = client.MakeRequest(req);

    error = http::stack::shutdown();
    return 0;
}