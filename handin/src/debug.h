#ifndef _DEBUG_H
#define _DEBUG_H

#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

//#define DEBUG
#ifdef DEBUG
#define DEBUGPRINT(fmt, args...) \
        do { fprintf(stderr, fmt, ##args); } while(0)
#else
#define DEBUGPRINT(fmt, args...)
#endif

#endif