#include <string.h>

#include "parse_forward.h"
#include "csapp.h"
#include "cache.h"

int read_requests(rio_t* rio, char* buf, requestInfo_t* requestInfo_ptr)
{
    char uri[MAXLINE];
    Rio_readlineb(rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", requestInfo_ptr->method, uri, requestInfo_ptr->version);
    if (strcasecmp(requestInfo_ptr->method, "GET"))
    {
        return -1;
    }

    parse_uri(uri, requestInfo_ptr);

    do
    {
        Rio_readlineb(rio, buf, MAXLINE);
        strcpy(requestInfo_ptr->header[requestInfo_ptr->headerLineCnt++], buf);
    } while (strcmp(buf, "\r\n"));

    return 0;
}

void parse_uri(const char* uri, requestInfo_t* requestInfo_ptr)
{
    char* ptr = uri;
    while (*ptr != '/')
    {
        ++ptr;
    }
    while (*ptr == '/')
    {
        ++ptr;
    }

    char* host_ptr = requestInfo_ptr->hostName;
    char* port_ptr = requestInfo_ptr->portName;
    while (*ptr != '/')
    {
        *(host_ptr++) = *(ptr++);
        if (*ptr == ':')
        {
            ++ptr;
            while (isdigit(*ptr))
            {
                *(port_ptr++) = *(ptr++);
            }
        }
    }
    if (*requestInfo_ptr->portName == '\0')
    {
        strcpy(requestInfo_ptr->portName, "80");
    }

    char* file_ptr = requestInfo_ptr->fileName;
    while (*ptr != '\0')
    {
        *(file_ptr++) = *(ptr++);
    }

    return;
}

void forward_server(rio_t* rio, int* forwardFD, const requestInfo_t* requestInfo_ptr)
{
    char buf[MAXLINE];

    *forwardFD = Open_clientfd(requestInfo_ptr->hostName, requestInfo_ptr->portName);
    Rio_readinitb(rio, *forwardFD);

    sprintf(buf, "%s %s %s\r\n", requestInfo_ptr->method, requestInfo_ptr->fileName, proxy_version);
    Rio_writen(*forwardFD, buf, strlen(buf));

    int hdrs_exist[4] = {0, 0, 0, 0};
    for (int i = 0; i < requestInfo_ptr->headerLineCnt - 1; ++i)
    {
        char tmp_hdr[5];
        strncpy(tmp_hdr, requestInfo_ptr->header[i], 4);
        if (!strcmp(tmp_hdr, "Host"))
        {
            sprintf(buf, "%s", requestInfo_ptr->header[i]);
            hdrs_exist[0] = 1;
        }
        else if (!strcmp(tmp_hdr, "User"))
        {
            sprintf(buf, "%s\r\n", user_agent_hdr);
            hdrs_exist[1] = 1;
        }
        else if (!strcmp(tmp_hdr, "Conn"))
        {
            sprintf(buf, "%s\r\n", connection_hdr);
            hdrs_exist[2] = 1;
        }
        else if (!strcmp(tmp_hdr, "Prox"))
        {
            sprintf(buf, "%s\r\n", proxy_connection_hdr);
            hdrs_exist[3] = 1;
        }
        else
        {
            sprintf(buf, "%s", requestInfo_ptr->header[i]);
        }
        Rio_writen(*forwardFD, buf, strlen(buf));
    }
    if (!hdrs_exist[0])
    {
        sprintf(buf, "Host: %s\r\n", requestInfo_ptr->hostName);
        Rio_writen(*forwardFD, buf, strlen(buf));
    }
    if (!hdrs_exist[1])
    {
        sprintf(buf, "%s\r\n", user_agent_hdr);
        Rio_writen(*forwardFD, buf, strlen(buf));
    }
    if (!hdrs_exist[2])
    {
        sprintf(buf, "%s\r\n", connection_hdr);
        Rio_writen(*forwardFD, buf, strlen(buf));
    }
    if (!hdrs_exist[3])
    {
        sprintf(buf, "%s\r\n", proxy_connection_hdr);
        Rio_writen(*forwardFD, buf, strlen(buf));
    }
    
    sprintf(buf, "%s", "\r\n");
    Rio_writen(*forwardFD, buf, strlen(buf));

    return;
}

void forward_client(rio_t* rio, int forwardFD, int connFD, char* response_server)
{
    char buf[MAXLINE];
    int n;
    int response_size = 0;
    Rio_readinitb(rio, forwardFD);

    while ((n = Rio_readlineb(rio, buf, MAXLINE)) > 0)
    {
        response_size += n;
        if (response_size < MAX_OBJECT_SIZE + RESPONSE_HDR_SIZE)
        {
            strncat(response_server, buf, n);
        }
        Rio_writen(connFD, buf, n);
    }
    printf("%s", response_server);
    return;
}



void process(int connFD)
{
    char buf[MAXLINE];
    requestInfo_t requestInfo;
    rio_t rio;

    memset(&requestInfo, 0, sizeof(requestInfo));
    Rio_readinitb(&rio, connFD);
    
    if (read_requests(&rio, buf, &requestInfo) < 0)
    {
        return;
    }

    int cached_obj_idx;
    if ((cached_obj_idx = cached(&requestInfo)) == -1)
    {
        int forwardFD;
        char response_server[MAX_OBJECT_SIZE + RESPONSE_HDR_SIZE];
        forward_server(&rio, &forwardFD, &requestInfo);
        forward_client(&rio, forwardFD, connFD, response_server);
        if (strlen(response_server) <= MAX_OBJECT_SIZE + RESPONSE_HDR_SIZE)
        {
            write_cache(&requestInfo, response_server);
        }
        Close(forwardFD);
    }
    else
    {
        read_cache(connFD, cached_obj_idx);
    }

    
    return;
}