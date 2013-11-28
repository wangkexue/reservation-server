#ifndef _UTIL_H_
#define _UTIL_H_
#include "thread_pool.h"

int handle_connection(void*);
#ifdef PRIORITY_QUEUE
int get_line(int, char*, int);
int parse_int_arg(char* filename, char* arg);
#endif

#endif
