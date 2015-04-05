#ifndef __PAGING_H__
#define __PAGING_H__

#include <stdint.h>

struct directory {
  /* Format of a 32-bit page directory entry that references a 
   * page table
   * (Intel Manual pg. 104)
   * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8..11 | 12 .. 31 |
   * | P |R/W|U/S|PWT|PCD| A |IGN| PS|  IGN  | ADDRESS  |
   *  P: Present. Must be 1 to reference a page table
   *  R/W: Read/Write. (See Intel Manual Section 4.6)
   *  U/S: User/Supervisor. If 0, user mode accesses 
   *       are not allowed to the region controlled by
   *       this entry.
   *  PWT: Page-level write-through (See Intel Manual 4.9)
   *  PCD: Page-level cache disable (See Intel Manual 4.9)
   *  A: Accessed
   *  IGN: Ignored
   *  ADDRESS: Physical address of 4KB aligned page table
               referenced by this entry
   */
  uint8_t present:1; 
  uint8_t rw:1;     
  uint8_t us:1; 
  uint8_t pwt:1; 
  uint8_t pcd:1;   
  uint8_t a:1;     
  uint8_t ignored:1;
  uint8_t ps:1;     
  uint8_t ignored2:4; 
  uint32_t page_table:20;
} __attribute__((packed));
typedef struct directory directory_t;

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
  uint32_t page_frame:20;
} __attribute__((packed));
typedef struct page page_t;

void init_paging();

#endif
