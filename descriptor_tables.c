// descriptor_tables.c - Initializes the GDT and IDT, and defines the 
// default ISR and IRQ handler.
// Based on code from Bran's kernel development tutorials.
#include "descriptor_tables.h"

// Lets us access our ASM functions from our C code.
extern void gdt_flush(uint32_t);

// For internal use only
static void init_gdt();
static void gdt_set_gate(int32_t, uint32_t, uint32_t, uint8_t, uint8_t);

gdt_entry_t gdt_entries[5];
gdt_ptr_t   gdt_ptr;

// Initializes GDT and IDT.
void init_descriptor_tables(){
    init_gdt();
    //TODO: initialize IDT
}

static void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment 
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

    gdt_flush((uint32_t)&gdt_ptr);
}


/* Set the value of one GDT entry.
 * | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * | P |  DPL  |DT |    Type       |
 * P - Segment is present? (1 = Yes)
 * DPL - Which Ring (0 to 3)
 * DT - Descriptor Type
 * Type - Which type?
 * NOTE: each GDT entry is 8 bytes
*/
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran){
    
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}
