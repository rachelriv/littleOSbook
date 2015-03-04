#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "framebuffer.h"

//TODO: implement printf

size_t strlen(const char *buf) {
  unsigned int i=0;
  while(buf[i] != 0) i++;
  return i;
}


// http://man7.org/linux/man-pages/man3/memmove.3.html
void *memmove(void *dst, const void *src, size_t len) {
    char *dp = (char*)dst;
    char *sp = (char*)src;

    if (dp < sp) {
    // overlap guarantee
        while(len-- > 0){
            *dp++ = *sp++;
        }
    } else {
        dp += len;
        sp += len;
        while (len-- > 0){
            *--dp = *--sp;
        }
    }
    return dst;
}
