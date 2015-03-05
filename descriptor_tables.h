// inspired by http://www.osdever.net/bkerndev/Docs/printing.htm

#ifndef __DESCRIPTOR_TABLES_H__
#define __DESCRIPTOR_TABLES_H__

#include <inttypes.h>

// This structure contains the value of one GDT entry.
// We use the attribute 'packed' to tell GCC not to change
// any of the alignment in the structure.
struct gdt_entry_struct {
   uint16_t limit_low;           // The lower 16 bits of the limit.
   uint16_t base_low;            // The lower 16 bits of the base.
   uint8_t  base_middle;         // The next 8 bits of the base.
   uint8_t  access;              // Access flags, (what ring). 8 bits
   uint8_t  granularity;         // 8 bits
   uint8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

// To tell the processor where to find our GDT
struct gdt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));
typedef struct gdt_ptr gdt_ptr_t;

void init_descriptor_tables();

// ---------------------------
// Interrupt Descriptor Tables
// ---------------------------

struct idt_flags {
  uint8_t gate_type:3;  // 0 - 2, task, interrupt,
  uint8_t d:1;          // 3, gate size
  uint8_t zero:1;       // 4, value=0
  uint8_t dpl:2;        // 5 - 6, descriptor privilege level
  uint8_t p:1;          // 7, segment present
} __attribute__((packed));
typedef struct idt_flags idt_flags_t;

struct idt_entry {
  uint16_t base_low; // aka offset
  uint16_t segment_selector;
  uint8_t reserved; // always zero
  idt_flags_t flags;
  uint16_t base_high; // aka offset
} __attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct idt_ptr {
  uint16_t limit;
  idt_entry_t *base; // aka offset
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
