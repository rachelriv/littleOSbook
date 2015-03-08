#include "serial.h"
#include "framebuffer.h"
#include "string.h"
#include "multiboot.h"
#include "descriptor_tables.h"
#include "keyboard.h"

void kmain(multiboot_info_t *info) {
  fb_clear();
  printf("No. 5 is alive!\n");

  char greeting[] = "No. 5 is alive!\n";
  serial_write(greeting, strlen(greeting));


  init_descriptor_tables();

  printf("Generating random interrupts...\n");
  asm volatile ("int $0x3");
  asm volatile ("int $0x4");

   init_keyboard();
  return;
}
