#include <stdbool.h>

#include "descriptor_tables.h"
#include "framebuffer.h"
#include "keyboard.h"
#include "multiboot.h"
#include "serial.h"
#include "string.h"
#include "paging.h"
#include "isr.h"

bool int3_happened = false;

void int3_handler(registers_t regs) {
  printf("interrupt 3 detected.");
  int3_happened = true;
}

void kmain(multiboot_info_t *info) {
  fb_clear();
  printf("No. 5 is alive! \n");

  char greeting[] = "No. 5 is alive! \n";
  serial_write(greeting, strlen(greeting));

  printf("multiboot header flags: %x \n", info->flags);

  init_descriptor_tables();

  printf("Generating random interrupts...\n");
  register_interrupt_handler(3, int3_handler);
  asm volatile ("int $0x3");
  asm volatile ("int $0x4");
  if (!int3_happened) printf("interrupts improperly configured, no point in continuing.");

  init_keyboard();
  init_paging();

  uint32_t *ptr = (uint32_t*)0xA0000000;
  uint32_t do_page_fault = *ptr;
  return;
}
