#include "framebuffer.h"
#include "string.h"

void kmain() {
  fb_clear();
  int i;
  for (i = 0; i < 100; i++){
    fb_write("abcdefghijklmnopqrstuvwxyz\n", 27);
  }
  fb_write("\n", 1);
  return;
}
