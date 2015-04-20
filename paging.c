#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "paging.h"
#include "string.h"
#include "kheap.h"
#include "error.h"

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

#define FRAME(addr) (addr/FRAME_SIZE)
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

void alloc_frame(page_t *page, int is_supervisor, int is_writeable) {
  if (page->frame != 0) {
    // frame already allocated, return right away
    return;
  } else {
    uint32_t free_frame = first_free_frame();
    if (free_frame == (uint32_t)-1) {
      ERROR("No free frames!");
    } else {
      // assign the free frame to the page
      page->present = PAGE_PRESENT; 
      page->rw = (is_writeable)?PAGE_READ_WRITE:PAGE_READ_ONLY; 
      page->us = (is_supervisor)?PAGE_SUPERVISOR:PAGE_USER;
      page->frame = free_frame;
      
      // mark newly allocated frame as used
      // in our frame allocations
      uint32_t physical_address = free_frame*FRAME_SIZE;
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
  // Some necessary set up
  set_up_frame_allocations();
  set_up_page_directory();
  
  map_heap_pages();
  identity_map();
  allocate_heap_pages();
  
  register_interrupt_handler(14, page_fault);
  enable_paging(kernel_directory);
  kheap = create_heap(KHEAP_START, KHEAP_START+KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);
  current_directory = clone_directory(kernel_directory);
  switch_page_directory(current_directory);
}

void allocate_heap_pages() {
  uint32_t i;
  for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += FRAME_SIZE){
    alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
  }
}

void map_heap_pages() {
  uint32_t i;
  for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += FRAME_SIZE) {
    get_page(i, 1, kernel_directory);
  }
}

void set_up_frame_allocations() {
  num_of_frames = SIZE_OF_PHYSICAL_MEMORY / FRAME_SIZE;
  frame_allocations = (uint32_t*)kmalloc(num_of_frames/FRAME_ALLOCATIONS_SECTION_SIZE);
  memset(frame_allocations, 0, num_of_frames/FRAME_ALLOCATIONS_SECTION_SIZE);
}

void set_up_page_directory() {
  uint32_t phys;
  kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
  memset(kernel_directory, 0, sizeof(page_directory_t));
  kernel_directory->physicalAddress = (uint32_t)kernel_directory->page_tables_physical;
}

void identity_map() {
  uint32_t i;
  for (i = 0; i < placement_address+FRAME_SIZE; i+=FRAME_SIZE) { 
    alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
  }  
}

void enable_paging(page_directory_t *dir) {
  current_directory = dir;
  asm volatile("mov %0, %%cr3":: "r"(dir->physicalAddress));
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
  if (dir->page_tables_virtual[table_idx]) { // If this table is already assigned
    return &dir->page_tables_virtual[table_idx]->pages[address%1024];
  } else if(make) {
    uint32_t tmp;
    dir->page_tables_virtual[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
    memset(dir->page_tables_virtual[table_idx], 0, 0x1000);
    dir->page_tables_physical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
    return &dir->page_tables_virtual[table_idx]->pages[address%1024];
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
  ERROR("Page fault");
}


static page_table_t * clone_table(page_table_t *src, uint32_t *physAddress) {
    //Make a brand new, blank, empty, page-aligned table. 
    page_table_t * table = (page_table_t*)kmalloc_ap(sizeof(page_table_t), physAddress);
    memset(table, 0, sizeof(page_directory_t));

    int i;
    for(i = 0; i < 1024; i++) {
        if(!src->page_tables_virtual[i].frame){
            continue;
        }
        // Create a new frame and copy all flags. All of them.
        alloc_frame(&table->page_tables_virtual[i], 0, 0);
        if(src->page_tables_virtual[i].present) table->page_tables_virtual[i].present = 1;
        if(src->page_tables_virtual[i].rw) table->page_tables_virtual[i].rw = 1;
        if(src->page_tables_virtual[i].us) table->page_tables_virtual[i].us = 1;
        if(src->page_tables_virtual[i].pwt) table->page_tables_virtual[i].pwt = 1;
        if(src->page_tables_virtual[i].pcd) table->page_tables_virtual[i].pcd = 1;
        if(src->page_tables_virtual[i].a) table->page_tables_virtual[i].a = 1;
        if(src->page_tables_virtual[i].d) table->page_tables_virtual[i].d = 1;
        if(src->page_tables_virtual[i].pat) table->page_tables_virtual[i].pat = 1;
        if(src->page_tables_virtual[i].g) table->page_tables_virtual[i].g = 1;
        // This will physically copy the data.
        copy_page_physical(src->page_tables_virtual[i].frame*0x1000, table->page_tables_virtual[i].frame*0x1000);
    }
    return table;
}


page_directory_t * clone_directory(page_directory_t *src) {
    uint32_t phys;
    // We create a new page directory and get its physical address.
    page_directory_t * dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    // We blank out the directory's content.
    memset(dir, 0, sizeof(page_directory_t));

    // Get the offset of tables
    uint32_t offset = (uint32_t)dir->page_tables_physical - (uint32_t)dir;
    //
    dir->page_tables_physical = phys + offset;

    int i;
    for(i = 0; i < 1024; i++ ) {
        if(!scr->tables[i]) {
            continue;      
        }

        if (kernel_directory->tables[i] == src->tables[i]) {
            // The page table is in the kernel directory, so we link.
            dir->page_tables_virtual[i] = src->page_tables_virtual[i];
            dir->page_tables_physical[i] = src->page_physical_tables[i];
        } else {
            // The page table is not in the kernel directory, we we
            // will copy the page table.
            uint32_t phys;
            dir->page_tables_virtual[i] = clone_table(src->tables[i], &phys);
            dir->page_tables_physical[i] = phys | 0x07; // Present, RW, User-Mode
        }
    }

    return dir;
}
