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
#include "syscall.h"
#include "task.h"

void kmain(multiboot_info_t *info, uint32_t initial_stack) {
   initial_esp = initial_stack;
   // Initialise all the ISRs and segmentation
   init_descriptor_tables();
   // Initialise the screen (by clearing it)
   fb_clear();

   uint32_t a = kmalloc(8);
   init_paging();
   printf("Hello, paging world!\n");
   uint32_t b = kmalloc(8);
   uint32_t c = kmalloc(8);
   printf("a: ");
   printf("%x", a);
   printf(", b: ");
   printf("%x", b);
   printf("\nc: ");
   printf("%x", c);

   kfree(c);
   kfree(b);
   uint32_t d = kmalloc(12);
   printf(", d: ");
   printf("%x", d);
//   uint32_t *ptr = (uint32_t *)0xA0000000;
//   uint32_t do_page_fault = *ptr;

   //initialise_syscalls();
   switch_to_user_mode();
   //syscall_fb_clear();

   return;
}
