#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "framebuffer.h"

//TODO: implement printf

char *itoa(int val, char *buf, int radix) {
  unsigned int i = 0;
  unsigned int start = i;
  if (val < 0 && radix == 10) {
    buf[i++] = '-';
    start = i;
  }

  if (radix == 16) {
    buf[i++] = '0';
    buf[i++] = 'x';
    start = i;
  }

  int x = val;
  do {
    int a = x % radix;
    if (a < 10) buf[i++] = a + '0';
    else buf[i++] = a + 'a' - 10;
  } while (x /= radix);

  char *s = buf+start;
  char *e = s+(i-3);

  while(s < e) {
    char t = *s;
    *s = *e;
    *e = t;
    s++; e--;
  }

  buf[i] = 0;
  return buf;
}
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

// Terribly naive implementation of memset to get things compiling
// See http://www.xs-labs.com/en/blog/2013/08/06/optimising-memset/
// for more details
void *memset(void *s, int c, size_t n) {
  char *mem = (char*)s;
  size_t i;
  for (i=0; i<n; i++) {
    mem[i] = (uint8_t)c;
  }
  return s;
}
