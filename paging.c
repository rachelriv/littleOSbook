#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "paging.h"
#include "string.h"
#include "kheap.h"


// defined in kheap.c
extern uint32_t placement_address;
extern heap_t *kheap;

// bitset of frames - used or free. 
uint32_t *frame_allocations;
uint32_t num_of_frames;

// The kernel's page directory
page_directory_t *kernel_directory=0;

// The current page directory;
page_directory_t *current_directory=0;

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

static uint32_t first_frame() {
  uint32_t num_of_words = FRAME_WORD(num_of_frames);
  uint32_t word, bit;
  for (word = 0; word < num_of_words; word++){
  if (frame_allocations[word] != 0xFFFFFFFF) { //nothing free, exit early
  for (bit = 0; bit < PAGE_FRAME_WORD_SIZE; bit++){
  uint32_t bitToTest = 0x1 << bit;
  if ( !(frame_allocations[word]&bitToTest) ){
    return (word*PAGE_FRAME_WORD_SIZE) + bit;
  }
  } 
  }
  }
  return 0xFFFFFFFF;
}

void alloc_frame(page_t *page, int is_supervisor, int is_writeable){
  if (page->frame != 0){
  return; // Frame was already allocated, return straight away.
  } else {
  uint32_t idx = first_frame();
  if (idx == (uint32_t)-1){
  printf("No free frames!");
  }
  set_frame(idx*0x1000);  // this frame is now ours!
  page->present = PAGE_PRESENT; // Mark it as present.
  page->rw = (is_writeable)?PAGE_READ_WRITE:PAGE_READ_ONLY; 
  page->us = (is_supervisor)?PAGE_SUPERVISOR:PAGE_USER;
  page->frame = idx;
  }
}

void free_frame(page_t *page){
  uint32_t frame;
  if ( !(frame=page->frame) ){
  return; // The given page didn't actually have an allocated frame!
  } else {
  clear_frame(frame); //frame is now free again.
  page->frame = 0x0; // Page now doesn't have a frame.
  }
}
void init_paging() {
  // The size of physical memory. For the moment we 
  // assume it is 16MB big.
  uint32_t mem_end_page = 0x10000000;
  
  num_of_frames = mem_end_page / 0x1000;
  frame_allocations = (uint32_t*)kmalloc(FRAME_WORD(num_of_frames));
  memset(frame_allocations, 0, FRAME_WORD(num_of_frames));

  // Let's make a page directory.
  kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
  memset(kernel_directory, 0, sizeof(page_directory_t));
  current_directory = kernel_directory;

  // Map some pages in the kernel heap area.
  // Here we call get_page but not alloc_frame. This causes page_table_t's 
  // to be created where necessary. We can't allocate frames yet because they
  // they need to be identity mapped first below, and yet we can't increase
  // placement_address between identity mapping and enabling the heap!
  int i = 0;
  for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += 0x1000)
  get_page(i, 1, kernel_directory);
  // We need to identity map (phys addr = virt addr) from
  // 0x0 to the end of used memory, so we can access this
  // transparently, as if paging wasn't enabled.
  // NOTE that we use a while loop here deliberately.
  // inside the loop body we actually change placement_address
  // by calling kmalloc(). A while loop causes this to be
  // computed on-the-fly rather than once at the start.
  // Allocate a lil' bit extra so the kernel heap can be
  // initialised properly.
  i = 0;
  while (i < placement_address+0x1000)
  {
    // Kernel code is readable but not writeable from userspace.
    alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
    i += 0x1000;
  }

  // Now allocate those pages we mapped earlier.
  for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += 0x1000)
    alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
  
  // Before we enable paging, we must register our page fault handler
  register_interrupt_handler(14, page_fault);

  // Now, enable paging!
  switch_page_directory(kernel_directory);

  // Initialise the kernel heap.
  kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);
}

void switch_page_directory(page_directory_t *dir){
  current_directory = dir;
  asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
  uint32_t cr0; 
  asm volatile("mov %%cr0, %0": "=r"(cr0));
  cr0 |= 0x80000000; // Enable paging!
  asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir){
  // Turn the address into an index
  address /= 0x1000;
  // Find the page table containing the address
  uint32_t table_idx = address / 1024;
  if (dir->tables[table_idx]) { // If this table is already assigned
  return &dir->tables[table_idx]->pages[address%1024];
  } else if(make) {
  uint32_t tmp;
  dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
  memset(dir->tables[table_idx], 0, 0x1000);
  dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
  return &dir->tables[table_idx]->pages[address%1024];
  } else {
  return 0;
  }
}
 
void page_fault(registers_t regs){
  // A page fault has occurred.
  // The faulting address is stored in the CR2 register.
  uint32_t faulting_address;
  asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

  // The error code gives us details of what happened.
  int present   = !(regs.err_code & 0x1); // Page not present
  int rw = regs.err_code & 0x2;     // Write operation?
  int us = regs.err_code & 0x4;     // Processor was in user-mode?
  int reserved = regs.err_code & 0x8;   // Overwritten CPU-reserved bits?
  int id = regs.err_code & 0x10;    // Caused by an instruction fetch?

  //Output an error message.
  printf("Page fault! ( ");
  if (present) { printf("present"); }
  if (rw) { printf("read-only"); }
  if (us) { printf("user-mode"); }
  if (reserved) { printf("reserved"); }
  printf(") at 0x %x \n", faulting_address);
  while(1);
}
