/* $begin sbufc */
#include "sbuf.h"

#include "csapp.h"

/* Create an empty, bounded, shared FIFO buffer with n slots */
/* $begin sbuf_init */
void sbuf_init(sbuf_t *sp, int n)
{
    sp->size = 0;
    sp->buf = Calloc(n, sizeof(int));   
    sp->n = n;                  /* Buffer holds max of n items */
    sp->front = sp->rear = 0;   /* Empty buffer iff front == rear */
    Sem_init(&sp->mutex, 0, 1); /* Binary semaphore for locking */
    Sem_init(&sp->slots, 0, n); /* Initially, buf has n empty slots */
    Sem_init(&sp->items, 0, 0); /* Initially, buf has zero data items */
}
/* $end sbuf_init */
/* Clean up buffer sp */
/* $begin sbuf_deinit */
void sbuf_deinit(sbuf_t *sp) { Free(sp->buf); }
/* $end sbuf_deinit */

/* Insert item onto the rear of shared buffer sp */
/* $begin sbuf_insert */
void sbuf_insert(sbuf_t *sp, int item)
{
    P(&sp->slots);                          /* Wait for available slot */
    P(&sp->mutex);                          /* Lock the buffer */
    sp->buf[(++sp->rear) % (sp->n)] = item; /* Insert the item */
    ++sp->size;
    V(&sp->mutex); /* Unlock the buffer */
    V(&sp->items); /* Announce available item */
}
/* $end sbuf_insert */

/* Remove and return the first item from buffer sp */
/* $begin sbuf_remove */
int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items);                           /* Wait for available item */
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);  // 不可取消
    P(&sp->mutex);                           /* Lock the buffer */
    item = sp->buf[(++sp->front) % (sp->n)]; /* Remove the item */
    --sp->size;

    V(&sp->mutex); /* Unlock the buffer */
    V(&sp->slots); /* Announce available slot */
    return item;
}
/* $end sbuf_remove */

/* Empty buf? */
int sbuf_empty(sbuf_t *sp)
{
    int e;
    P(&sp->mutex); /* Lock the buffer */
    e = sp->size == 0;
    V(&sp->mutex); /* Lock the buffer */
    return e;
}

/* Full buf? */
int sbuf_full(sbuf_t *sp)
{
    int f;
    P(&sp->mutex); /* Lock the buffer */
    f = sp->size == sp->n;
    V(&sp->mutex); /* Lock the buffer */
    return f;
}

/* $end sbufc */
