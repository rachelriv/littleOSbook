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
// simple implementation of memmove
void *memmove(void *dst, const void *src, size_t len) {
  char *dstmem = (char*)dst;
  char *srcmem = (char*)src;
  size_t i;
  for (i=0; i<len; i++) {
    dstmem[i] = srcmem[i];
  }
  return dstmem;
}
