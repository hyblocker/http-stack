#include "http.hpp"

#if 0
#include <sys/socket.h>

namespace http::stack {

    bool g_networkSubsystemInitialised = false;

    HttpStackError init() {
        if (g_networkSubsystemInitialised) {
            return HttpStackError::ErrorAlreadyInitialised;
        }

        WSADATA wsaData = {};
        int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (err != 0) {
            return HttpStackError::ErrorFailInitialise;
        }

        // verify we have winsock 2.2 support
        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
            WSACleanup();
            return HttpStackError::ErrorNetworkSubsystemIncompatible;
        }

        g_networkSubsystemInitialised = true;
        return HttpStackError::OK;
    }

    HttpStackError shutdown() {
        if (!g_networkSubsystemInitialised) {
            return HttpStackError::ErrorNotInitialised;
        }

        WSACleanup();

        g_networkSubsystemInitialised = false;
        return HttpStackError::OK;
    }

    // wrapper for cross platform layers, to be used by platform agnostic paths
    bool __subsystemIsInitialised() {
        return g_networkSubsystemInitialised;
    }

}

#endif