#include <stdio.h>
#include "csapp.h"
#include "parse_forward.h"
#include "thread.h"
#include "cache.h"

void SIGPIPE_handler(int sig);

// global variable
cache_t proxyCache;
sem_t mutex_readcnt;
sem_t w;
volatile int read_cnt = 0;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int listenFD;
    int* connFD_ptr;
    char hostName[MAXLINE];
    char hostPort[MAXLINE];
    socklen_t clientLen;
    struct sockaddr_storage clientAddr;
    pthread_t tid;

    Signal(SIGPIPE, SIGPIPE_handler);
    
    memset(&proxyCache, 0, sizeof(cache_t));
    Sem_init(&mutex_readcnt, 0, 1);
    Sem_init(&w, 0, 1);

    listenFD = Open_listenfd(argv[1]);
    while (1)
    {
        clientLen = sizeof(clientAddr);
        connFD_ptr = Malloc(sizeof(int));
        *connFD_ptr = Accept(listenFD, (SA*)&clientAddr, &clientLen);
        Getnameinfo((SA*)&clientAddr, clientLen, hostName, MAXLINE, hostPort, MAXLINE, 0);
        Pthread_create(&tid, NULL, thread, connFD_ptr);
    }

    return 0;
}

void SIGPIPE_handler(int sig)
{
    if (errno == EPIPE)
    {
        errno = 0;
    }

    return;
}