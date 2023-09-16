#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

typedef struct inputs{
  int s;
  int E;
  int b;
  char* t;
  int hits;
  int misses;
  int evictions;
} inputs_table;

typedef struct trace{
    char ins;
    long add;
    int size; // useless due to aligned memory access
} trace_table;

typedef struct line{
    int valid;
    int tag;
    int time;
} line_table;

void getTrace(const char* line, trace_table* tt_pt);
line_table* cache(inputs_table* part_A);
void desCache(line_table* cache);
void sim(trace_table* trace, line_table* cache, inputs_table* part_A, int vFlag);

int main(int argc, char *argv[])
{
    int vFlag = 0;
    char opt;
    inputs_table part_A = {0, 0, 0, NULL, 0, 0, 0};

    /* parse the command line arguments */
    while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 'v':
            vFlag = 1;
            break;
        case 's':
            part_A.s = atoi(optarg);
            break;
        case 'E':
            part_A.E = atoi(optarg);
            break;
        case 'b':
            part_A.b = atoi(optarg);
            break;
        case 't':
            part_A.t = optarg;
            break;
        default:
            printf("wrong input arguments!");
            exit(1);
        }
    }

    /* set up the cache */
    line_table* simCache = cache(&part_A);

    /* read the trace file and simulate caching */
    FILE* trace_file = fopen(part_A.t, "r");
    if (!trace_file)
    {
        printf("Cannot open trace file!");
        exit(1);
    }
    char tf_line[20];
    while (fgets(tf_line, 20, trace_file) != NULL)
    {
        if (tf_line[0] == 'I')
        {
            continue;
        }
        trace_table trace = {' ', 0, 0};
        getTrace(tf_line, &trace);
        sim(&trace, simCache, &part_A, vFlag);
    }

    /* wrap up */
    printSummary(part_A.hits, part_A.misses, part_A.evictions);
    fclose(trace_file);
    desCache(simCache);
    return 0;
}

void getTrace(const char* line, trace_table* tt_pt)
{
    int left = 1;
    tt_pt->ins = line[left];
    left += 2;
    int right = left;
    while (line[right] != ',')
    {
        ++right;
    }

    /*transform hex address to dec int*/
    int i = 1;
    for (int j = right - 1; j >= left; --j)
    {
        int digit;
        if (line[j] >= '0' && line[j] <= '9')
        {
            digit = line[j] - '0';
        }
        else
        {
            digit = line[j] - 'a' + 10;
        }
        tt_pt->add += digit * i;
        i *= 16;
    }

    tt_pt->size = line[right + 1] - '0';
}

line_table* cache(inputs_table* part_A)
{
    int S = (1 << part_A->s);
    line_table* cache = (line_table*)malloc(S * part_A->E * sizeof(line_table));
    if (!cache)
    {
        printf("Cache building failed!");
        exit(1);
    }
    for (int i = 0; i < S * part_A->E; ++i)
    {
        cache[i].valid = 0;
        cache[i].tag = 0;
        cache[i].time = 0;
    }
    return cache;
}

void desCache(line_table* cache)
{
    free(cache);
    cache = NULL;
}

void sim(trace_table* trace, line_table* cache, inputs_table* part_A, int vFlag)
{
    char ins = trace->ins;
    int S = (1 << part_A->s);
    int hit = 0;

    //int blk_idx = 0;
    int set_idx = 0;
    long tmp_tag = 0;

    int mask = 0;
    int i;
    for (i = 0; i < part_A->b; ++i)
    {
        mask |= (1 << i);
    }
    //blk_idx = mask & trace->add;
    mask = 0;
    for (; i < part_A->b + part_A->s; ++i)
    {
        mask |= (1 << i);
    }
    set_idx = (mask & trace->add) >> part_A->b;
    mask = 0;
    for (; i < 64; ++i)
    {
        mask |= (1 << i);
    }
    tmp_tag = (mask & trace->add) >> (part_A->b + part_A->s);

    for (int i = set_idx * part_A->E; i < set_idx * part_A->E + part_A->E; ++i)
    {
        if (cache[i].tag == tmp_tag && cache[i].valid)
        {
            hit = 1;
            if (ins == 'L' || ins == 'S')
            {
                ++part_A->hits;
            }
            else
            {
                part_A->hits += 2;
            }
            cache[i].time = -1;
            break;
        }
    }

    if (!hit)
    {
        ++part_A->misses;
        int evict = 1;
        int lru_idx = set_idx * part_A->E;
        for (int i = set_idx * part_A->E; i < set_idx * part_A->E + part_A->E; ++i)
        {
            if (cache[i].valid)
            {
                lru_idx = (cache[i].time > cache[lru_idx].time) ? i : lru_idx;
            }
            else
            {
                evict = 0;
                cache[i].tag = tmp_tag;
                cache[i].valid = 1;
                cache[i].time = -1;
                break;
            }
        }

        if (evict)
        {
            ++part_A->evictions;
            cache[lru_idx].tag = tmp_tag;
            cache[lru_idx].time = -1;
        }
        if (ins == 'M')
        {
            ++part_A->hits;
        }
    }

    for (int i = 0; i < S * part_A->E; ++i)
    {
        if (cache[i].valid)
        {
            ++cache[i].time;
        }
    }
}