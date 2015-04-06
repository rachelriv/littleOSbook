#ifndef __PAGING_H__
#define __PAGING_H__

#include <stdint.h>
#include "isr.h"

#define FRAME_SIZE              4096

#define PAGE_NOT_PRESENT        0
#define PAGE_PRESENT            1 
#define PAGE_READ_ONLY          0
#define PAGE_READ_WRITE         1
#define PAGE_USER               0
#define PAGE_SUPERVISOR         0
#define PAGE_SIZE_4KB           0
#define PAGE_SIZE_4MB           1


struct page {
  /* Format of a 32-bit page table entry that maps a 4 KB page
   * (Intel Manual pg. 104)
   * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9..11 | 12 .. 31 |
   * | P |R/W|U/S|PWT|PCD| A | D |PAT| G | IGN   | ADDRESS  |
   *  P: Present. Must be 1 to map a 4-KB page
   *  R/W: Read/Write. (See Intel Manual 4.6)
   *  U/S: User/supervisor. If 0, user-mode accesses are not
           allowed to the 4 KB page referenced by this entry
   *  PWT: Page-level write-through (See Intel Manual 4.9)
   *  PCD: Page-level cache-disable (See Intel Manual 4.9)
   *  A: Accessed. Indicates whether the software has accessed
   *     the 4 KB page referenced by this entity.
   *  G: Global. Determines whether translation is global
   *     (See Intel Manual 4.10)
   *  IGN: Ignored
   *  ADDRESS: Physical address of 4KB page referenced by 
   *           this entry.
   */ 
  uint8_t present:1;     
  uint8_t rw:1;          
  uint8_t us:1;          
  uint8_t pwt:1;
  uint8_t pcd:1; 
  uint8_t a:1;     
  uint8_t d:1; 
  uint8_t pat:1;    
  uint8_t g:1;
  uint8_t ignored2:3; 
  uint32_t frame:20;
} __attribute__((packed));
typedef struct page page_t;

typedef struct page_table{
  page_t pages[1024];
}page_table_t;

typedef struct page_directory{
  //Array of pointers to page tables.
  page_table_t *tables[1024];
  
  /*
   * Array of pointers to the pagetables above, but gives their *physical*
   * location, for loading into the CR3 register.
   */
  uint32_t tablesPhysical[1024];

  /*
   * Physical address of tablesPhysical. This comes into play 
   * when we get our kernel heap allocated and this directory
   * may be in a different location in virtual memory.
   */
  uint32_t physicalAddr;
} page_directory_t;

/* Sets up the environment, page directories etc and
 * enables paging.
 */
void init_paging();

/* Causes the specified page directory to be loaded 
 * into the CR3 register, where the MMU expects it.
 * Enables paging and flushes the page-directory cache.
 */ 
void enable_paging(page_directory_t *page);

/* Retrieves a pointer to the page required.
 * If make == 1, if the page-table in which this page should
 * reside isn't created, create it!
 */
page_t *get_page(uint32_t address, int make, page_directory_t *dir);

/*
 * Handler for page faults.
 */
void page_fault(registers_t regs);
#endif
