// descriptor_tables.c - Initializes the GDT and IDT, and defines the
// default ISR and IRQ handler.
// Based on code from Bran's kernel development tutorials.
#include "descriptor_tables.h"
#include "string.h"
// Internal use only
extern void gdt_flush(uint32_t);
extern void idt_flush(idt_ptr_t*);
static gdt_entry_t construct_entry();
static void init_gdt();
static void init_idt();
static void gdt_set_gate(int32_t idx, uint32_t base, uint32_t limit, gdt_access_t access, gdt_gran_t gran);
static void idt_set_gate(
  uint8_t idx,
  void(*base),
  uint16_t selector,
  idt_flags_t flags);
gdt_entry_t gdt_entries[5];
gdt_ptr_t   gdt_ptr;

idt_entry_t idt_entries[256]; // 256 possible interrupt numbers
idt_ptr_t   idt_ptr;


// Initializes GDT and IDT.
void init_descriptor_tables(){
    init_gdt();
    init_idt();
}

static void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;
    
   // gdt_entry_t null_segment = construct_entry();
   // gdt_entries[0] = null_segment;
    gdt_entry_t kernel_mode_code_segment = construct_entry(
    	(struct gdt_access){
    		.p 
    )
    gdt_set_gate(
        1,
        0, 
        0xFFFFFFFF, 
        (struct gdt_access){
            .p = 1,
            .dpl = 0x0,
            .dt = 1,
            .type = 0xa
        },
        (struct gdt_granularity){
          .g = 1,
          .d = 1,
          .zero = 0,
          .a = 0,
          .seglen = 0xf
        });        // Null segment

    gdt_set_gate(
        2, 
        0, 
        0xFFFFFFFF, 
        (struct gdt_access){
            .p = 1,
            .dpl = 0x0,
            .dt = 1,
            .type = 0x2
        },
        (struct gdt_granularity){
          .g = 1,
          .d = 1,
          .zero = 0,
          .a = 0,
          .seglen = 0xf
        });        // Null segment
 
    gdt_set_gate(
        3, 
        0, 
        0xFFFFFFFF, 
        (struct gdt_access){
            .p = 1,
            .dpl = 0xf,
            .dt = 1,
            .type = 0xa
        },
        (struct gdt_granularity){
          .g = 1,
          .d = 1,
          .zero = 0,
          .a = 0,
          .seglen = 0xf
        });        // Null segment
    
gdt_set_gate(
        4, 
        0, 
        0xFFFFFFFF, 
        (struct gdt_access){
            .p = 1,
            .dpl = 0xf,
            .dt = 1,
            .type = 0x2
        },
        (struct gdt_granularity){
          .g = 1,
          .d = 1,
          .zero = 0,
          .a = 0,
          .seglen = 0xf
        });        // Null segment

    gdt_flush((uint32_t)&gdt_ptr);
}


/* Set the value of one GDT entry.
 * Each entry is 64-bits wide (8 bytes)
*/
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, gdt_access_t access, gdt_gran_t gran){

    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
//    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity = gran;
    gdt_entries[num].access      = access;
}

/* The only thing that changes between the non-null GDT
 * segment descriptors is the access. Given the access,
 * this returns a GDT entry.
 */
static gdt_entry_t construct_entry(gdt_access_t access) {
    gdt_entry_t entry = (struct gdt_entry_struct){
        .base_low  = GDT_BASE & 0xFFFF,
	.base_middle = (GDT_BASE >> 16) & 0xFF,
        .base_high = (GDT_BASE >> 24) & 0xFF,
	.limit_low = (GDT_LIMIT & 0xFFFF),
        .access = access,
        .granularity = (struct gdt_granularity){
          .g = GDT_4KBYTE_GRANULARITY,
          .d = GDT_32BIT_OPERAND_SIZE,
          .zero = 0,
          .seglen = GDT_SEGMENT_LENGTH
        }
    };
    return entry;
}
static void init_idt() {
  idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
  idt_ptr.base = idt_entries;

  memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);
  idt_flags_t flags = {
    .reserved = IDT_FLAG_RESERVED,         // always 01110
    .dpl = 0,               //ring 0
    .p = 1                 //idt segment present
  };
  idt_set_gate(0, isr0, 0x08, flags);
  idt_set_gate(1, isr1, 0x08, flags);
  idt_set_gate(2, isr2, 0x08, flags);
  idt_set_gate(3, isr3, 0x08, flags);
  idt_set_gate(4, isr4, 0x08, flags);
  idt_set_gate(5, isr5, 0x08, flags);
  idt_set_gate(6, isr6, 0x08, flags);
  idt_set_gate(7, isr7, 0x08, flags);
  idt_set_gate(8, isr8, 0x08, flags);
  idt_set_gate(9, isr9, 0x08, flags);
  idt_set_gate(10, isr10, 0x08, flags);
  idt_set_gate(11, isr11, 0x08, flags);
  idt_set_gate(12, isr12, 0x08, flags);
  idt_set_gate(13, isr13, 0x08, flags);
  idt_set_gate(14, isr14, 0x08, flags);
  idt_set_gate(15, isr15, 0x08, flags);
  idt_set_gate(16, isr16, 0x08, flags);
  idt_set_gate(17, isr17, 0x08, flags);
  idt_set_gate(18, isr18, 0x08, flags);
  idt_set_gate(19, isr19, 0x08, flags);
  idt_set_gate(20, isr20, 0x08, flags);
  idt_set_gate(21, isr21, 0x08, flags);
  idt_set_gate(22, isr22, 0x08, flags);
  idt_set_gate(23, isr23, 0x08, flags);
  idt_set_gate(24, isr24, 0x08, flags);
  idt_set_gate(25, isr25, 0x08, flags);
  idt_set_gate(26, isr26, 0x08, flags);
  idt_set_gate(27, isr27, 0x08, flags);
  idt_set_gate(28, isr28, 0x08, flags);
  idt_set_gate(29, isr29, 0x08, flags);
  idt_set_gate(30, isr30, 0x08, flags);
  idt_set_gate(31, isr31, 0x08, flags);

  idt_flush(&idt_ptr);
}


static void idt_set_gate(
  uint8_t idx,
  void(*base),
  uint16_t selector,
  idt_flags_t flags) {

  idt_entries[idx] = (struct idt_entry){
    .base_low = (uint32_t)base & 0xffff,
    .base_high = ((uint32_t)base >> 16) & 0xffff,
    .segment_selector = selector,
    .flags = flags
  };
}
