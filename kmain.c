#include "framebuffer.h"
#include "string.h"
#include "serial.h"
#include "descriptor_tables.h"

void kmain() {
  init_descriptor_tables();

  fb_clear();
  int i;
  for (i = 0; i < 100; i++){
    fb_write("abcdefghijklmnopqrstuvwxyz\n", 27);
  }
  fb_write("\n", 1);
  serial_write("serial write", 12);
  return;
}
