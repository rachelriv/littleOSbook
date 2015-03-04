#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>
#include <stdint.h>


size_t strlen(const char *buf);
void *memmove(void *dst, const void *src, size_t len);
#endif /* _STRING_H_ */
