#pragma once

#include <esp_http_server.h>

class FlashduckWebServer {
public:
    bool Start();

private:
    httpd_handle_t server_ = nullptr;
    int port_ = 0;
};
