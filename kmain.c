#include "serial.h"
#include "framebuffer.h"
#include "string.h"
#include "multiboot.h"
#include "descriptor_tables.h"
#include "keyboard.h"

void kmain(multiboot_info_t *info) {
  fb_clear();
  printf("Hello!\n");

  init_descriptor_tables();

  printf("Generating software interrupts 3 and 4...\n");
  asm volatile ("int $0x3");
  asm volatile ("int $0x4");

  init_keyboard();
  return;
}
