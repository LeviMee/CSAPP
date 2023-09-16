/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Whiplash",
    /* First member's full name */
    "Levi Lee",
    /* First member's email address */
    "wli675@gatech.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* define custmizde macros and constants*/
#define WSIZE 4
#define DSIZE 8
#define ALIGNMENT 8 // single word (4) or double word (8) alignment
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y)) ? (x) : (y) // must use brackets for priority
#define PACK(size, alloc) ((size) | (alloc)) // set up the header
#define GET(p) (*(unsigned long long*)(p)) // read the fields from p
#define PUT(p, val) (*(unsigned long long*)(p) = ((unsigned long long)val)) // write the fields from p
#define GET_SIZE(p) (GET(p) & ~0x7) // get the size of the block
#define GET_ALLOC(p) (GET(p) & 0x1) // test whether it's free

/* for allocated block */
#define HDRP(bp) ((char*)(bp) - 3 * DSIZE) // get the adress of header from block pointer
#define HDRP_ALLOC(bp) ((char*)(bp) - DSIZE)
#define PREDP(bp) ((char*)(bp) - 2 * DSIZE)
#define SUCCP(bp) ((char*)(bp) - 1 * DSIZE)
#define NEXTPREV(prev) ((char*)(*((char*)(prev) + 1 * DSIZE)))
#define PREVSUCC(prev) ((char*)(*(char*)(prev)))

#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7) // rounds up to the nearest multiple of ALIGNMENT
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* static private variables */
static char* heap_eflist_p_header;
static char* eflist_dummyHead;
static char* eflist_dummyTail;

/* static private functions */
static void* extend_heap(size_t size);
static void* coalesce(void* bp);
static void place(size_t asize, void* bp);
static void* find_fit(size_t asize, char choice);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_eflist_p_header = mem_sbrk(4 * DSIZE)) == (void*) -1)
    {
        return -1;
    }
    eflist_dummyHead = heap_eflist_p_header;
    eflist_dummyTail = eflist_dummyHead + 2 * DSIZE;

    PUT(eflist_dummyHead, NULL);
    PUT(eflist_dummyHead + DSIZE, eflist_dummyTail);
    PUT(eflist_dummyTail, eflist_dummyHead + DSIZE);
    PUT(eflist_dummyTail + DSIZE, NULL);

    if (extend_heap(CHUNKSIZE) == NULL)
    {
        return -1;
    }

    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    /* default*/
    // int newsize = ALIGN(size + SIZE_T_SIZE);
    // void *p = mem_sbrk(newsize);
    // if (p == (void *)-1)
	// return NULL;
    // else {
    //     *(size_t *)p = size;
    //     return (void *)((char *)p + SIZE_T_SIZE);
    // }

    /* first fit */
    if (!size)
    {
        return NULL;
    }
    char* bp;
    size_t asize = ALIGN(size + 3 * DSIZE);

    if ((bp = find_fit(asize, 'f')) != NULL)
    {
        place(asize, bp);
        return bp - 2 * DSIZE;  // remove the overhead of pred and succ
    }

    size_t extendSize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendSize)) == NULL)
    {
        return NULL;
    }
    place(asize, bp);
    return bp;
}

static void place(size_t asize, void* bp) // address order
{
    size_t size = GET_SIZE(HDRP(bp));
    char* prev_succ = PREVSUCC(PREDP(bp));
    char* next_prev = NEXTPREV(PREDP(bp));
    if (size - asize >= 4 * DSIZE)
    {
        PUT(HDRP(bp), PACK(asize, 1));

        char* next_bp = (char*)bp + asize;
        PUT(HDRP(next_bp), PACK(size - asize, 0));
        PUT(prev_succ, PREDP(next_bp));
        PUT(PREDP(next_bp), prev_succ);
        PUT(SUCCP(next_bp), next_prev);
        PUT(next_prev, SUCCP(next_bp));
    }
    else
    {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(prev_succ, next_prev);
        PUT(next_prev, prev_succ);
    }
}

static void* find_fit(size_t asize, char choice)
{
    char* prev = eflist_dummyHead;
    char* bp;
    switch(choice)
    {
        case 'f':
            while (prev != eflist_dummyTail)
            {
                size_t tmp_size = GET_SIZE(NEXTPREV(prev) - DSIZE);
                if (tmp_size >= asize)
                {
                    bp = NEXTPREV(prev) + 2 * DSIZE;
                    return bp;
                }
                prev = NEXTPREV(prev);
            }
            return NULL;
            break;
        case 'n':

            break;
        case 'b':

            break;
        default:
            exit(-1);
    }
    return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) // allocated or free
{
    size_t size = GET_SIZE(HDRP_ALLOC(ptr));
    char* bp = (char*)ptr + 2 * DSIZE;

    char* curr_prev = eflist_dummyHead;
    char* next_prev;
    while (curr_prev != eflist_dummyTail)
    {
        next_prev = NEXTPREV(curr_prev);
        if (bp < curr_prev && bp > next_prev)
        {
            PUT(curr_prev + DSIZE, PREDP(bp));
            PUT(PREDP(bp), curr_prev + DSIZE);
            PUT(SUCCP(bp), next_prev);
            PUT(next_prev, SUCCP(bp));
        }
        curr_prev = next_prev;
    }

    coalesce((void*)bp); // ptr not in the eflist
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    // void *oldptr = ptr;
    // void *newptr;
    // size_t copySize;
    
    // newptr = mm_malloc(size);
    // if (newptr == NULL)
    //   return NULL;
    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    // if (size < copySize)
    //   copySize = size;
    // memcpy(newptr, oldptr, copySize);
    // mm_free(oldptr);
    // return newptr;

    return ptr;
}

/*
 * mm_check - heap consistency checker
 */
int mm_check(void)
{


    return 0;
}

static void* extend_heap(size_t bytes)
{
    char* bp;
    size_t size = ALIGN(bytes);

    if ((void*)(bp = mem_sbrk(size)) == (void*)-1)
    {
        return NULL;
    }

    bp += DSIZE;
    char* eflist_dummyTail_prev_succ = PREVSUCC(eflist_dummyTail);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(PREDP(bp), eflist_dummyTail_prev_succ);
    eflist_dummyTail = HDRP(bp) + size;
    PUT(SUCCP(bp), eflist_dummyTail);

    PUT(eflist_dummyTail, SUCCP(bp));
    PUT(eflist_dummyTail + DSIZE, NULL);

    return coalesce(bp); // bp already in the eflist
}

static void* coalesce(void* bp)
{
    char* bp_prev_succ = PREVSUCC(PREDP(bp));
    char* bp_next_prev = NEXTPREV(SUCCP(bp));
    size_t size = GET_SIZE(HDRP(bp));

    int coa_prev = !(bp_prev_succ == (eflist_dummyHead + DSIZE));
    int coa_next = !(bp_next_prev== eflist_dummyTail);    
    if (coa_prev)
    {
        coa_prev = (GET_SIZE(bp_prev_succ - 2 * DSIZE) + DSIZE + bp_prev_succ == (char*)bp) ? 1 : 0;
    }
    if (coa_next)
    {   
        coa_next = (size + DSIZE +  (char*)bp == bp_next_prev) ? 1 : 0;
    }

    
    if (coa_prev && !coa_next)
    {
        size_t prev_size = GET_SIZE(bp_prev_succ - 2 * DSIZE);
        char* new_bp = bp_prev_succ + DSIZE;
        PUT(HDRP(new_bp), PACK(prev_size + size, 0));
        return (void*)new_bp;
    }
    if (!coa_prev && coa_next)
    {
        size_t next_size = GET_SIZE(bp_next_prev - DSIZE);
        PUT(HDRP(bp), PACK(size + next_size, 0));
        return bp;
    }
    if (coa_prev && coa_next)
    {
        size_t prev_size = GET_SIZE(bp_prev_succ - 2 * DSIZE);
        size_t next_size = GET_SIZE(bp_next_prev - DSIZE);
        char* new_bp = bp_prev_succ + DSIZE;
        PUT(HDRP(new_bp), PACK(prev_size + size + next_size, 0));
        return (void*)new_bp;
    }
    return bp;
}
