#ifndef MACRO_H
#define MACRO_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define VERSION "1.0.0"

#ifndef MAX
	#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
	#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef enum{
	false = 0,
	true
}bool;

typedef void* any_t;
typedef bool (*Comparator)(any_t, any_t);

/**
 * @brief Comparator
 * @param x Lefthand operand.
 * @param y Righthand operand.
 * @return true if x < y.
 *        false if x >= y.
 */
bool cmp_lt(any_t x, any_t y);

// swap two elements
void swap(any_t* x, any_t* y);

// end of line
#define CRLF "\r\n"

#endif
