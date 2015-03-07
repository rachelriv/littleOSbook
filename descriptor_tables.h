// http://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html/

#ifndef __DESCRIPTOR_TABLES_H__
#define __DESCRIPTOR_TABLES_H__

#include <inttypes.h>


//Reserved bits in IDT/GDT entries
#define IDT_FLAG_RESERVED                         0x0E


//GDT fields
#define BASE                                      0x00000000
#define LIMIT                                     0xFFFFFFFF

    //GDT granularity fields 
    #define 1BYTE_GRANULARITY                     0
    #define 4KBYTE_GRANULARITY                    1

    #define 16BIT_OPERAND_SIZE                    0
    #define 32BIT_OPERAND_SIZE                    1

    #define SEGMENT_LENGTH                        0xF

    //GDT access fields
#define SEGMENT_NOT_PRESENT                       0
#define SEGMENT_PRESENT                           1

        //3.4.5.1 of Intel 64 and IA-32 Architectures Developer's Manual
#define DATA_TYPE_READ_ONLY                       0x0
#define DATA_TYPE_READ_ONLY_ACCESSED              0x1
#define DATA_TYPE_READ_WRITE                      0x2
#define DATA_TYPE_READ_WRITE_ACCESSED             0x3
#define DATA_TYPE_READ_ONLY_EXPAND_DOWN           0x4
#define DATA_TYPE_READ_ONLY_EXPAND_DOWN_ACCESSED  0x5
#define DATA_TYPE_READ_WRITE_EXPAND_DOWN          0x6
#define DATA_TYPE_READ_WRITE_EXPAND_DOWN_ACCESSED 0x7

#define CODE_TYPE_EXEC_ONLY                       0x8
#define CODE_TYPE_EXEC_ONLY_ACCESSED              0x9
#define CODE_TYPE_EXEC_READ                       0xA
#define CODE_TYPE_EXEC_READ_ACCESSED              0xB
#define CODE_TYPE_EXEC_CONFORMING                 0xC
#define CODE_TYPE_EXEC_CONFORMING_ACCESSED        0xD
#define CODE_TYPE_EXEC_READ_CONFORMING            0xE
#define CODE_TYPE_EXEC_READ_CONFORMING_ACCESSED   0xF


// ---------------------------
// Global Descriptor Tables
// ---------------------------

/**  access:
 *       | 0...3 | 4 | 5... 6 | 7 |
 *       |  Type | DT|   DPL  | P |
 *           Type: Which type?
 *           DT: descriptor type
 *           DPL: which ring (0 to 3)
 *           P: segment present? (1=yes)
 */
struct gdt_access {
    uint8_t type:4;           
    uint8_t dt:1;
    uint8_t dpl:2;
    uint8_t p:1;
} __attribute__((packed));
typedef struct gdt_access gdt_access_t;
/*
 *       | 0..3    | 4 | 5 | 6 | 7 | 
 *       | SEGLEN  |   0   | D | G | 
 *           SEGLEN: segment length
 *           0: Always 0
 *           D: Operand Size (0 = 16bit, 1 = 32-bit)
 *           G: granularity (0 = 1byte, 1 = 4kbyte)
*/
struct gdt_granularity {
    uint8_t seglen:4;
    uint8_t zero:2;
    uint8_t d:1;
    uint8_t g:1;
} __attribute__((packed));
typedef struct gdt_granularity gdt_gran_t;

/** This structure contains the value of one GDT entry.
 *  Each entry is 64 bits.
 *  | 0 ... 15 | 16 ... 31 | 32 ... 39 | 40...47 | 48...55 | 56...63 |
 *  |limit_low | base_low  |base_middle| access  | gran    |base_high|
 *  limit_low: the lower 16 bits of the limit
 *  base_low: the lower 16 bits of the base
 *  base_middle: the next 8 bits of the base
 *  access: (see above)
 *  gran:  (see above)
 */
struct gdt_entry_struct {
   uint16_t limit_low;
   uint16_t base_low;
   uint8_t  base_middle;
   gdt_access_t  access;
   gdt_gran_t  granularity;
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
 *  | 0 ... 4 | 5...6 | 7 |
 *  |RESERVED |  DPL  | P |
 *  P: segment is present (1 = Yes)
 *  DPL: which ring (0 to 3)
 *  RESERVED: always 01110 (14)
 */
struct idt_flags {
  uint8_t reserved:5;
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
