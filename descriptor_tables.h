// http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html/

#ifndef __DESCRIPTOR_TABLES_H__
#define __DESCRIPTOR_TABLES_H__

#include <inttypes.h>


// ---------------------------
// Global Descriptor Tables
// ---------------------------


/** This structure contains the value of one GDT entry.
 *  Each entry is 64 bits.
 *  | 0 ... 15 | 16 ... 31 | 32 ... 39 | 40...47 | 48...55 | 56...63 |
 *  |limit_low | base_low  |base_middle| access  | gran    |base_high|
 *  limit_low: the lower 16 bits of the limit
 *  base_low: the lower 16 bits of the base
 *  base_middle: the next 8 bits of the base
 *  access:
 *       | 0 | 1..2 | 3 |   4..7   |
 *       | P | DPL  | DT|   Type   |
 *           P: segment present? (1=yes)
 *           DPL: which ring (0 to 3)
 *           DT: descriptor type
 *           Type: Which type?
 *  gran:
 *       | 0 | 1 | 2 | 3 | 4..7    |
 *       | G | D | 0 | A | Seg Len |
 *           G: granularity (0 = 1byte, 1 = 4kbyte)
 *           D: Operand Size (0 = 16bit, 1 = 32-bit)
 *           0: Always 0
 *           A: Available for System
 *  More info: http://www.osdever.net/bkerndev/Docs/gdt.htm
 */
struct gdt_entry_struct {
   uint16_t limit_low;
   uint16_t base_low;
   uint8_t  base_middle;
   uint8_t  access;
   uint8_t  granularity;
   uint8_t  base_high;
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

/* To tell the processor where to find our GDT */
struct gdt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));
typedef struct gdt_ptr gdt_ptr_t;

void init_descriptor_tables();





// ---------------------------
// Interrupt Descriptor Tables
// ---------------------------

/** This structure contains the access flags of an IDT entry.
 *  | 0 | 1 | 2 ... 7 |
 *  | P |DPL|  UNUSED |
 *  P: segment is present (1 = Yes)
 *  DPL: which ring
 *  UNUSED: always 01110
 */
struct idt_flags {
  uint8_t unused:4;
  uint8_t zero:1;
  uint8_t dpl:2;
  uint8_t p:1;
} __attribute__((packed));
typedef struct idt_flags idt_flags_t;


/** This structure contains the value of one IDT entry. (similar to GDT entries)
 *  Each entry is 64 bits.
 *  | 0 ... 15 |  16 ... 31     | 32 ... 39 | 40...47 | 48 ... 63 |
 *  | base_low |segment_selector| reserved  | flags   | base_high |
 *  base_low: the lower 16 bits of the base
 *  segment_selector: Segment Selector for destination code segment
 *  reserved: always 0
 *  flags: (defined above)
 *  base_high: higher 16 bits of the base
 */
struct idt_entry {
  uint16_t base_low;
  uint16_t segment_selector;
  uint8_t reserved;
  idt_flags_t flags;
  uint16_t base_high;
} __attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct idt_ptr {
  uint16_t limit;
  idt_entry_t *base;
} __attribute__((packed));
typedef struct idt_ptr idt_ptr_t;


extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
#endif
