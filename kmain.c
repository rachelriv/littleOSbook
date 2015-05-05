#include <stdint.h>

#include "descriptor_tables.h"
#include "framebuffer.h"
#include "keyboard.h"
#include "multiboot.h"
#include "serial.h"
#include "string.h"
#include "paging.h"
#include "isr.h"
#include "kheap.h"


void kmain(/*multiboot_info_t *info*/) {
   fb_clear();
   
   printf("Initializing descriptor tables...\n");
   init_descriptor_tables();
   printf("Allocate memory for a variable before we initialize paging...");
   printf("(so it is allocated via placement address)\n");
   uint32_t a = kmalloc(8);
   printf("Initializing paging...\n");
   init_paging();
   printf("Allocate b and c on the heap...\n");
   uint32_t b = kmalloc(8);
   uint32_t c = kmalloc(8);
   printf("a: ");
   printf("%x", a);
   printf(", b: ");
   printf("%x", b);
   printf("\nc: ");
   printf("%x", c);
   printf("\n");
   printf("Free a and b\n");
   kfree((void *)c);
   kfree((void *)b);
   printf("Allocate d. ");
   printf("d is in the same place as b, so b must have been properly freed \n");
   uint32_t d = kmalloc(12);
   printf("d: ");
   printf("%x", d);
   printf("\n");
   printf("Initializing keyboard...");
   init_keyboard();
//   uint32_t *ptr = (uint32_t *)0xA0000000;
//   uint32_t do_page_fault = *ptr;

   return;
}
