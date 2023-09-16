#include "cache.h"
#include "csapp.h"
#include "parse_forward.h"

extern cache_t proxyCache;
extern sem_t mutex_readcnt;
extern sem_t w;
extern volatile int read_cnt;

int cached(requestInfo_t* requestInfo_ptr)
{
    int cached_obj_idx = -1;

    P(&mutex_readcnt);
    ++read_cnt;
    if (read_cnt == 1)
    {
        P(&w);
    }
    V(&mutex_readcnt);

    for (int i = 0; i < CACHED_OBJ_MAX_CNT; ++i)
    {
        proxyCache.timeStamp[i][1] = clock();
    }
    
    for (int i = 0; i < CACHED_OBJ_MAX_CNT; ++i)
    {
        if (proxyCache.valid[i])
        {
            if (!strcmp(proxyCache.cached_obj_fileName[i], requestInfo_ptr->fileName))
            {
                cached_obj_idx = i;
                break;
            }
        }
    }

    P(&mutex_readcnt);
    --read_cnt;
    if (read_cnt == 0)
    {
        V(&w);
    }
    V(&mutex_readcnt);

    return cached_obj_idx;
}

void write_cache(requestInfo_t* requestInfo_ptr, const char* response_server)
{
    P(&w);

    int evict = 1;
    for (int i = 0; i < CACHED_OBJ_MAX_CNT; ++i)
    {
        if (!proxyCache.valid[i])
        {
            evict = 0;
            strcpy(proxyCache.cached_obj[i], response_server);
            strcpy(proxyCache.cached_obj_fileName[i], requestInfo_ptr->fileName);
            proxyCache.timeStamp[i][1] = clock();
            proxyCache.timeStamp[i][0] = proxyCache.timeStamp[i][1];
            proxyCache.valid[i] = 1;
            break;
        }
    }

    if (evict)
    {
        int evict_idx = 0;
        long max_interval = 0;
        for (int i = 0; i < CACHED_OBJ_MAX_CNT; ++i)
        {
            long tmp_interval = proxyCache.timeStamp[i][1] - proxyCache.timeStamp[i][0];
            if (tmp_interval > max_interval)
            {
                evict_idx = i;
                max_interval = tmp_interval;
            }
        }
        strcpy(proxyCache.cached_obj[evict_idx], response_server);
        strcpy(proxyCache.cached_obj_fileName[evict_idx], requestInfo_ptr->fileName);
        proxyCache.timeStamp[evict_idx][1] = clock();
        proxyCache.timeStamp[evict_idx][0] = proxyCache.timeStamp[evict_idx][1];
        proxyCache.valid[evict_idx] = 1;
    }

    V(&w);
    return;
}

void read_cache(int connFD, int cached_obj_idx)
{
    P(&mutex_readcnt);
    ++read_cnt;
    if (read_cnt == 1)
    {
        P(&w);
    }
    V(&mutex_readcnt);

    proxyCache.timeStamp[cached_obj_idx][1] = clock();
    proxyCache.timeStamp[cached_obj_idx][0] = proxyCache.timeStamp[cached_obj_idx][1];
    Rio_writen(connFD, proxyCache.cached_obj[cached_obj_idx], \
    strlen(proxyCache.cached_obj[cached_obj_idx]));

    P(&mutex_readcnt);
    --read_cnt;
    if (read_cnt == 0)
    {
        V(&w);
    }
    V(&mutex_readcnt);
    return;
}