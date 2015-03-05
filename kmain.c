#include "serial.h"
#include "framebuffer.h"
#include "string.h"
#include "multiboot.h"
#include "descriptor_tables.h"

void kmain() {
  init_descriptor_tables();

  fb_clear();
  char greeting[] = "Hello World!\n";
  fb_write_str(greeting);
  serial_write(greeting, strlen(greeting));

  char flags[100];
  fb_write_str(flags);

  asm volatile ("int $0x3");
  asm volatile ("int $0x4");
  return;
}
