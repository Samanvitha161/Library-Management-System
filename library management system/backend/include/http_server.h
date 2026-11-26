#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <winsock2.h>
#include <windows.h>
#include <stdbool.h>
#include "library.h"

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 65536
#define MAX_PATH_LEN 512

typedef struct {
    char method[16];
    char path[MAX_PATH_LEN];
    char query[MAX_PATH_LEN];
    char* body;
    size_t body_len;
} HttpRequest;

// Server control functions
int http_server_start(int port, const char* web_root);
void http_server_stop(void);

#endif // HTTP_SERVER_H
