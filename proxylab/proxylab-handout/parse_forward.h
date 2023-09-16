#include "csapp.h"

#ifndef PARSE_FORWARD_H
#define PARSE_FORWARD_H
#define METHOD_MAX_LEN 128
#define HOST_MAX_LEN 1024
#define FILENAME_MAX_LEN 1024
#define PORTNAME_MAX_LEN 64
#define VERSION_MAX_LEN 64
#define HEADER_MAX_LINE 64
#define HEADER_MAX_LEN 256

static const char* user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3";
static const char* proxy_version = "HTTP/1.0";
static const char* connection_hdr = "Connection: close";
static const char* proxy_connection_hdr = "Proxy-Connection: close";

typedef struct requestInfo
{
    char method[METHOD_MAX_LEN];
    char hostName[HOST_MAX_LEN];
    char fileName[FILENAME_MAX_LEN];
    char portName[PORTNAME_MAX_LEN];
    char version[VERSION_MAX_LEN];
    char header[HEADER_MAX_LINE][HEADER_MAX_LEN];
    //char message[MAXLINE];
    int headerLineCnt;
} requestInfo_t;

int read_requests(rio_t *rio, char* buf, requestInfo_t* requestInfo_ptr);
void parse_uri(const char* uri, requestInfo_t* requestInfo_ptr);
void forward_server(rio_t* rio, int* forwardFD, const requestInfo_t* requestInfo_ptr);
void forward_client(rio_t* rio, int forwardFD, int connFD, char* response_server);

void process(int connFD);

#endif