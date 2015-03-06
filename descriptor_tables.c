// Based on code from Bran's kernel development tutorials.

#include "descriptor_tables.h"
#include "string.h"

// Internal use only
extern void gdt_flush(uint32_t);
extern void idt_flush(idt_ptr_t*);

static void init_gdt();
static void init_idt();

static void gdt_set_entry(
    int32_t idx,
    uint32_t base,
    uint32_t limit,
    gdt_access_t access,
    gdt_gran_t gran);

static void idt_set_entry(
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

    gdt_set_entry(
        0,
        0,
        0,
        (struct gdt_access){
          .p = 0,
          .dpl = 0x0,
          .dt = 0,
          .type = 0
        },
        (struct gdt_granularity){
          .g = 1,
          .d = 1,
          .zero = 0,
          .a = 0,
          .seglen = 0xf
        });        // Null segment
    gdt_set_entry(
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

    gdt_set_entry(
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

    gdt_set_entry(
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

gdt_set_entry(
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

/* Construct a GDT entry. */
static void gdt_set_entry(uint32_t base, uint32_t limit, gdt_access_t access, gdt_gran_t gran){
    (struct gdt_entry_struct){
        limit_low = (limit & 0xFFFF);
    }
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
//    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity = gran;
    gdt_entries[num].access      = access;
}

/* Set the value of one GDT entry.
 * Each entry is 64-bits wide (8 bytes)
 */
static void gdt_set_entry(int32_t num, uint32_t base, uint32_t limit, gdt_access_t access, gdt_gran_t gran){

    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
//    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity = gran;
    gdt_entries[num].access      = access;
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
    idt_set_entry(0, isr0, 0x08, flags);
    idt_set_entry(1, isr1, 0x08, flags);
    idt_set_entry(2, isr2, 0x08, flags);
    idt_set_entry(3, isr3, 0x08, flags);
    idt_set_entry(4, isr4, 0x08, flags);
    idt_set_entry(5, isr5, 0x08, flags);
    idt_set_entry(6, isr6, 0x08, flags);
    idt_set_entry(7, isr7, 0x08, flags);
    idt_set_entry(8, isr8, 0x08, flags);
    idt_set_entry(9, isr9, 0x08, flags);
    idt_set_entry(10, isr10, 0x08, flags);
    idt_set_entry(11, isr11, 0x08, flags);
    idt_set_entry(12, isr12, 0x08, flags);
    idt_set_entry(13, isr13, 0x08, flags);
    idt_set_entry(14, isr14, 0x08, flags);
    idt_set_entry(15, isr15, 0x08, flags);
    idt_set_entry(16, isr16, 0x08, flags);
    idt_set_entry(17, isr17, 0x08, flags);
    idt_set_entry(18, isr18, 0x08, flags);
    idt_set_entry(19, isr19, 0x08, flags);
    idt_set_entry(20, isr20, 0x08, flags);
    idt_set_entry(21, isr21, 0x08, flags);
    idt_set_entry(22, isr22, 0x08, flags);
    idt_set_entry(23, isr23, 0x08, flags);
    idt_set_entry(24, isr24, 0x08, flags);
    idt_set_entry(25, isr25, 0x08, flags);
    idt_set_entry(26, isr26, 0x08, flags);
    idt_set_entry(27, isr27, 0x08, flags);
    idt_set_entry(28, isr28, 0x08, flags);
    idt_set_entry(29, isr29, 0x08, flags);
    idt_set_entry(30, isr30, 0x08, flags);
    idt_set_entry(31, isr31, 0x08, flags);

    idt_flush(&idt_ptr);
}


static void idt_set_entry(
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
