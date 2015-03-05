#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>
#include <stdint.h>

char *itoa(int val, char *buf, int radix);
size_t strlen(const char *buf);
void *memmove(void *dst, const void *src, size_t len);
void *memset(void *s, int c, size_t n);
#endif /* _STRING_H_ */

