#include "thread.h"
#include "csapp.h"

void* thread(void* vargp)
{
    int connFD = *((int*)vargp);
    Pthread_detach(Pthread_self());
    Free(vargp);
    process(connFD);
    Close(connFD);
    return NULL;
}