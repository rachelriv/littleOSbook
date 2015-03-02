#include "gdt.h"

static struct ivt_entry ivt_entries[NUM_IVT_ENTRIES];

void init_ivt(void){
    struct ivt t = {&ivt_entries, 256};
}

// We fix the size of the gate to be 32 bits.
// We will assume the handler is in memory, set the access
// to 0.
void load_ivt_entries(unsigned idx, unsigned long offset, unsigned short seg_selector, unsigned char flags){
    ivt_entries[idx].offsetlow = offset & 0xFFFF;
    ivt_entries[idx].offsethigh = (offset & 0xFFFF0000) >> 16;
    ivt_entries[idx].segmentselector = seg_selector & 0xFFFF;
    ivt_flags[idx].flags = flags;
}

void interrupt_handler(struct cpu_state cpu, struct stack_state stack, unsigned int interrupt){
    
}
