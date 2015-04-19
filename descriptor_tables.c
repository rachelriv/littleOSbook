// Based (loosely) on code from Bran's kernel development tutorials.

#include <stdbool.h>
#include <cpuid.h>

#include "descriptor_tables.h"
#include "string.h"
#include "io.h"

// Internal use only
extern void gdt_flush(uint32_t);
extern void idt_flush(idt_ptr_t*);
extern void tss_flush();

static gdt_entry_t construct_null_entry();
static gdt_entry_t construct_entry(gdt_access_t access);
static void init_gdt();
static void init_idt();
static void idt_set_gate(
    uint8_t idx,
    void(*base),
    uint16_t selector,
    idt_flags_t flags);
static void PIC_remap(uint8_t offset1, uint8_t offset2);
static void write_tss(s32int, u16int, u32int);

gdt_entry_t gdt_entries[6];
gdt_ptr_t   gdt_ptr;

idt_entry_t idt_entries[256]; // 256 possible interrupt numbers
idt_ptr_t   idt_ptr;

tss_entry_t tss_entry;

// Initializes GDT and IDT.
void init_descriptor_tables(){
    init_gdt();
    init_idt();
}

static void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_entry_t null_segment = construct_null_entry();
    gdt_entry_t kernel_mode_code_segment = construct_entry(
        (struct gdt_access){
            .type = GDT_CODE_TYPE_EXEC_READ,
            .dt   = GDT_CODE_AND_DATA_DESCRIPTOR,
            .dpl  = GDT_RING0,
            .p    = GDT_SEGMENT_PRESENT
        }
    );
    gdt_entry_t kernel_mode_data_segment = construct_entry(
        (struct gdt_access){
            .type = GDT_DATA_TYPE_READ_WRITE,
            .dt   = GDT_CODE_AND_DATA_DESCRIPTOR,
            .dpl  = GDT_RING0,
            .p    = GDT_SEGMENT_PRESENT
        }
    );
    gdt_entry_t user_mode_code_segment = construct_entry(
        (struct gdt_access){
            .type = GDT_CODE_TYPE_EXEC_READ,
            .dt   = GDT_CODE_AND_DATA_DESCRIPTOR,
            .dpl  = GDT_RING3,
            .p    = GDT_SEGMENT_PRESENT
        }
    );
    gdt_entry_t user_mode_data_segment = construct_entry(
        (struct gdt_access){
            .type = GDT_DATA_TYPE_READ_WRITE,
            .dt   = GDT_CODE_AND_DATA_DESCRIPTOR,
            .dpl  = GDT_RING3,
            .p    = GDT_SEGMENT_PRESENT
        }
    );

    gdt_entry_t task_state_segment = construct_tss_entry(
        (struct gdt_access){
            .type = GDT_CODE_TYPE_EXEC_ONLY_ACCESSED,
            .dt   = GDT_SYSTEM_DESCRIPTOR,
            .dpl  = GDT_RING3,
            .p    = GDT_SEGMENT_PRESENT
        }
    );

    gdt_entries[0] = null_segment;
    gdt_entries[1] = kernel_mode_code_segment;
    gdt_entries[2] = kernel_mode_data_segment;
    gdt_entries[3] = user_mode_code_segment;
    gdt_entries[4] = user_mode_data_segment;
    gdt_entries[5] = task_state_segment;
    
    write_tss(0x10, 0x0);

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

// Initialize the task state segment struct
static void write_tss(u16int ss0, u32int esp0) {
    // Ensure the descriptor is initially zero.
   memset(&tss_entry, 0, sizeof(tss_entry));

   tss_entry.ss0  = ss0;  // Set the kernel stack segment.
   tss_entry.esp0 = esp0; // Set the kernel stack pointer.

   // Here we set the cs, ss, ds, es, fs and gs entries in the TSS. These specify what
   // segments should be loaded when the processor switches to kernel mode. Therefore
   // they are just our normal kernel code/data segments - 0x08 and 0x10 respectively,
   // but with the last two bits set, making 0x0b and 0x13. The setting of these bits
   // sets the RPL (requested privilege level) to 3, meaning that this TSS can be used
   // to switch to kernel mode from ring 3.
   tss_entry.cs   = 0x0b;
   tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

// When we change tasks, the TSS entry updates so it has the address
// of the correct kernel stack.
void set_kernel_stack(u32int stack) {
    tss_entry.esp0 = stack;
}

/** The only thing that changes between the non-null GDT
 *  segment descriptors is the access. Given the access,
 *  this returns the corresponding GDT entry.
 */
static gdt_entry_t construct_entry(gdt_access_t access) {
    gdt_entry_t entry = (struct gdt_entry_struct){
        .base_low  = GDT_BASE & 0xFFFF,
        .base_middle = (GDT_BASE >> 16) & 0xFF,
        .base_high = (GDT_BASE >> 24) & 0xFF,
        .limit_low = (GDT_LIMIT & 0xFFFF),
        .access = access,
        .granularity = (struct gdt_granularity){
            .g = GDT_GRANULARITY_4K,
            .d = GDT_OPERAND_SIZE_32,
            .zero = 0,
            .seglen = GDT_SEGMENT_LENGTH
        }
    };
    return entry;
}

/* Constructor for Task State Segment. */
static gdt_entry_t construct_tss_entry(gdt_access_t access) {
    u32int base = (u32int) &tss_entry;
    u32int limit = base + sizeof(tss_entry);

    gdt_entry_t entry = (struct gdt_entry_struct){
        .base_low  = base & 0xFFFF,
        .base_middle = (base >> 16) & 0xFF,
        .base_high = (base >> 24) & 0xFF,
        .limit_low = (limit & 0xFFFF),
        .access = access,
        .granularity = (struct gdt_granularity){
            .g = GDT_GRANULARITY_1K,
            .d = GDT_OPERAND_SIZE_16,
            .zero = 0,
            .seglen = 0
        }
    };
    return entry;
}

/* Constructs a null GDT entry. */
static gdt_entry_t construct_null_entry() {
    gdt_entry_t null_entry = (struct gdt_entry_struct){
        .base_low = 0,
        .base_middle = 0,
        .base_high = 0,
        .limit_low = 0,
        .access = (struct gdt_access){
            .p = 0,
            .dpl = 0,
            .dt = 0,
            .type = 0
        },
        .granularity = (struct gdt_granularity){
            .g = 0,
            .d = 0,
            .zero = 0,
            .seglen = 0
        }
    };
    return null_entry;
}
static inline bool are_interrupts_enabled(){
    unsigned long flags;
    asm volatile ( "pushf\n\t"
                   "pop %0"
                   : "=g"(flags) );
    return flags & (1 << 9);
}



static void init_idt() {

  idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
  idt_ptr.base = idt_entries;

  memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

  idt_flags_t flags = {
    .reserved = IDT_FLAG_RESERVED,         // always 01110
    .dpl = 0,                              //ring 0
    .p = 1                                //idt segment present
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

  // Remap the irq table
  printf("Remapping IRQs\n");
  PIC_remap(PIC1_START_INTERRUPT, PIC2_START_INTERRUPT);

  idt_set_gate(32, irq0, 0x08, flags);
  idt_set_gate(33, irq1, 0x08, flags);
  idt_set_gate(34, irq2, 0x08, flags);
  idt_set_gate(35, irq3, 0x08, flags);
  idt_set_gate(36, irq4, 0x08, flags);
  idt_set_gate(37, irq5, 0x08, flags);
  idt_set_gate(38, irq6, 0x08, flags);
  idt_set_gate(39, irq7, 0x08, flags);
  idt_set_gate(40, irq8, 0x08, flags);
  idt_set_gate(41, irq9, 0x08, flags);
  idt_set_gate(42, irq10, 0x08, flags);
  idt_set_gate(43, irq11, 0x08, flags);
  idt_set_gate(44, irq12, 0x08, flags);
  idt_set_gate(45, irq13, 0x08, flags);
  idt_set_gate(46, irq14, 0x08, flags);
  idt_set_gate(47, irq15, 0x08, flags);

  idt_flush(&idt_ptr);
  // enable hardware interrupts
  asm volatile ("sti");
  if (are_interrupts_enabled()) printf("interrupts enabled.\n");
}



static inline void io_wait() {

}

// from http://wiki.osdev.org/8259_PIC
static void PIC_remap(uint8_t offset1, uint8_t offset2) {
  unsigned char a1, a2;

  a1 = inb(PIC1_DATA);                        // save masks
  a2 = inb(PIC2_DATA);

  outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
  io_wait();
  outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
  io_wait();
  outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  io_wait();
  outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
  io_wait();

  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  outb(PIC1_DATA, a1);   // restore saved masks.
  outb(PIC2_DATA, a2);
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
