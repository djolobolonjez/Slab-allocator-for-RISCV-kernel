#ifndef _PRINTING_HPP_
#define _PRINTING_HPP_

#include "syscall_c.h"

#define LOCK() while(copy_and_swap(lockPrint, 0, 1))
#define UNLOCK() while(copy_and_swap(lockPrint, 1, 0))

typedef unsigned long uint64;

extern "C" uint64 copy_and_swap(uint64 &lock, uint64 expected, uint64 desired);

void printString(char const *string);

char* getString(char *buf, int max);

int stringToInt(const char *s);

void printInt(int xx, int base=10, int sgn=0);

#endif // _PRINTING_HPP_

