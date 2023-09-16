#include "csapp.h"
#include "parse_forward.h"

typedef void* (func)(void*);

void* thread(void* vargp);