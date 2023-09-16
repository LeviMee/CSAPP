#include <time.h>
#include "csapp.h"
#include "parse_forward.h"

#ifndef CACHE_H
#define CACHE_H
#define CACHED_OBJ_MAX_CNT 9
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define RESPONSE_HDR_SIZE 256

typedef struct cache
{
    char cached_obj[CACHED_OBJ_MAX_CNT][MAX_OBJECT_SIZE + RESPONSE_HDR_SIZE];
    char cached_obj_fileName[CACHED_OBJ_MAX_CNT][FILENAME_MAX_LEN];
    int valid[CACHED_OBJ_MAX_CNT];
    long timeStamp[CACHED_OBJ_MAX_CNT][2];
} cache_t;

int cached(requestInfo_t* requestInfo_ptr);
void write_cache(requestInfo_t* requestInfo_ptr, const char* response_server);
void read_cache(int connFD, int cached_obj_idx);






#endif