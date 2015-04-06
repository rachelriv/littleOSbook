#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "paging.h"
#include "string.h"
#include "kheap.h"


// defined in kheap.c
extern uint32_t placement_address;
extern heap_t *kheap;

/* bitset of frames - used or free. 
 *  each entry of the frame allocations 
 *  bitset represents a section of 32 frame 
 *  allocations (since each entry is a uint32_t 
 *  integer and each of the 32 bits of the int 
 *  represents the allocation-0=free,1=used-
 *  of a frame) */
uint32_t *frame_allocations;
#define FRAME_ALLOCATIONS_SECTION_SIZE 32
#define USED_FRAME_ALLOCATIONS_SECTION 0xFFFFFFFF
#define FREE_FRAME_ALLOCATIONS_SECTION 0x00000000

// Number of physical frames 
uint32_t num_of_frames;

// The kernel's page directory
page_directory_t *kernel_directory=0;

// The current page directory;
page_directory_t *current_directory=0;

#define FRAME(addr) (addr/PAGE_FRAME_SIZE)
#define FRAME_ALLOCATIONS_SECTION(frame) (frame/FRAME_ALLOCATIONS_SECTION_SIZE)
#define FRAME_ALLOCATIONS_OFFSET(frame) (frame%FRAME_ALLOCATIONS_SECTION_SIZE)

static void set_frame(uint32_t addr) {
  uint32_t frame = FRAME(addr);
  uint32_t frame_alloc_section = FRAME_ALLOCATIONS_SECTION(frame);
  uint32_t frame_alloc_offset = FRAME_ALLOCATIONS_OFFSET(frame);
  frame_allocations[frame_alloc_section] |= (1 << frame_alloc_offset);
}

static void clear_frame(uint32_t addr) {
  uint32_t frame = FRAME(addr);
  uint32_t frame_alloc_section = FRAME_ALLOCATIONS_SECTION(frame);
  uint32_t frame_alloc_offset = FRAME_ALLOCATIONS_OFFSET(frame);
  frame_allocations[frame_alloc_section] &= ~(1 << frame_alloc_offset);
}

static uint32_t first_free_frame() {
  uint32_t num_of_sections = num_of_frames/FRAME_ALLOCATIONS_SECTION_SIZE;
  int section, idx;
  for (section = 0; section < num_of_sections; section++){
    if (frame_allocations[section] != USED_FRAME_ALLOCATIONS_SECTION) { 
      for (idx = 0; idx < FRAME_ALLOCATIONS_SECTION_SIZE; idx++){
        if ( !(frame_allocations[section] & (0x1 << idx)) ){
          return (section*FRAME_ALLOCATIONS_SECTION_SIZE) + idx;
        }
      } 
    }
  }
  return num_of_sections*FRAME_ALLOCATIONS_SECTION_SIZE;
}

void alloc_frame(page_t *page, int is_supervisor, int is_writeable){
  if (page->frame != 0){
    // frame already allocated, return right away
    return;
  } else {
    uint32_t free_frame = first_free_frame();
    if (free_frame == (uint32_t)-1){
      printf("No free frames!");
      //TODO: error out here
    } else {
      // assign the page to the free frame
      page->present = PAGE_PRESENT; 
      page->rw = (is_writeable)?PAGE_READ_WRITE:PAGE_READ_ONLY; 
      page->us = (is_supervisor)?PAGE_SUPERVISOR:PAGE_USER;
      page->frame = free_frame;
      
      // mark newly allocated frame as used
      // in our frame allocations
      uint32_t physical_address = free_frame*PAGE_FRAME_SIZE;
      set_frame(physical_address);
    }
  }
}

void free_frame(page_t *page){
  uint32_t frame;
  if ( !(frame=page->frame) ){
    // The page didn't have an allocated 
    // frame in the first place
    return;
  } else {
    clear_frame(frame);
    page->frame = 0x0;
  }
}

void init_paging() {
  // The size of physical memory. For the moment we 
  // assume it is 16MB big.
  uint32_t mem_end_page = 0x10000000;
  
  num_of_frames = mem_end_page / 0x1000;
  int num_of_alloc_sections = num_of_frames / FRAME_ALLOCATIONS_SECTION_SIZE;
  frame_allocations = (uint32_t*)kmalloc(num_of_alloc_sections);
  memset(frame_allocations, 0, num_of_alloc_sections);

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
  for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += 0x1000) {
    get_page(i, 1, kernel_directory);
  }
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
  while (i < placement_address+0x1000) {
    // Kernel code is readable but not writeable from userspace.
    alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
    i += 0x1000;
  }

  // Now allocate those pages we mapped earlier.
  for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += 0x1000){
    alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
  }
  
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

  //Output an error message.
  printf("Page fault! ( ");
  if (! (regs.err_code & 0x1) ) { printf("present"); }
  if (regs.err_code & 0x2) { printf("read-only"); }
  if (regs.err_code & 0x4) { printf("user-mode"); }
  if (regs.err_code & 0x8) { printf("reserved"); }
  printf(") at 0x %x \n", faulting_address);
  while(1);
}
