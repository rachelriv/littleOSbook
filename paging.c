#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "paging.h"
#include "isr.h"
#include "string.h"
#include "memory.h"


// allocated (and page aligned) by nasm
extern directory_t boot_page_directory;

// bitset of frames - used or free. 
uint32_t frame_allocations[32767];

#define FRAME(addr) (addr/PAGE_FRAME_SIZE)
#define FRAME_WORD(frame) (frame/PAGE_FRAME_WORD_SIZE)
#define OFFSET(frame) (frame % PAGE_FRAME_WORD_SIZE)

static void set_frame(uint32_t addr) {
  uint32_t frame = FRAME(addr);
  uint32_t frame_word = FRAME_WORD(frame);
  uint32_t bit_offset = OFFSET(frame);
  frame_allocations[frame_word] |= (1 << bit_offset);
}

static bool test_frame(uint32_t addr) {
  uint32_t frame = FRAME(addr);
  uint32_t frame_word = FRAME_WORD(frame);
  uint32_t bit_offset = OFFSET(frame);
  return  (frame_allocations[frame_word] >> bit_offset) & 1;
}

static void clear_frame(uint32_t addr) {
  uint32_t frame = FRAME(addr);
  uint32_t frame_word = FRAME_WORD(frame);
  uint32_t bit_offset = OFFSET(frame);
  frame_allocations[frame_word] &= ~(1 << bit_offset);
}

void handle_page_fault(registers_t regs) {
  uint32_t faulting_address;
  asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
  
  // Error code lets us know what happened.   
  int present = !(regs.err_code & 0x1); // Page not present
  int rw = regs.err_code & 0x2;         // Write operation?
  int us = regs.err_code & 0x4;         // Processor was in user-mode?
  int reserved = regs.err_code & 0x8;   // Overwritten CPU-reserved bits
  int id = regs.err_code & 0x10;        // Caused by an instruction fetch?
  
  // Output an error message.
  printf("Page fault! ( ");
  if (present) {printf("present ");}
  if (rw) {printf("read-only ");}
  if (us) {printf("user-mode ");}
  if (reserved) {printf("reserved ");}
  printf(") at 0x");
  printf("%x", faulting_address);
  printf("\n");
  while(1);
}


void map(directory_t *page_directory, uint32_t vaddr, uint32_t paddr) {
  uint32_t directory_offset = vaddr >> 22; // 31:22
  uint32_t table_offset = (vaddr >> 12) & 0x3ff; // 21:12

  directory_t *directory = &page_directory[directory_offset/4];
  page_t *table;
  if (!directory->present) {
    directory->present = PAGE_PRESENT;
    directory->rw = PAGE_READ_WRITE;
    directory->us = PAGE_SUPERVISOR;
    directory->ps = PAGE_SIZE_4KB;

    table = (page_t*)kmalloc_page(); // sizeof(page_t)*1024 == 0x1000
    memset(table, 0, 0x1000);

    directory->page_table = (uint32_t)table >> 12; // table base (20 high bits)
  } else {
    table = (page_t*)(directory->page_table << 12);
  }

  page_t *page = &table[table_offset];
  if (!page->present) {
    page->present = 1;
    page->rw = PAGE_READ_WRITE;
    page->us = PAGE_SUPERVISOR;
  }

  page->page_frame = paddr >> 12; // frame base (20 high bits)
  set_frame(paddr);
}

extern void load_page_directory(directory_t *directory);
extern void enable_paging();

void init_paging() {
  printf("initializing paging");

  // listen for page faults
  register_interrupt_handler(14, handle_page_fault);

  load_page_directory(&boot_page_directory);
  enable_paging();
  printf("paging enabled!");
}
