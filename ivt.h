#ifndef _IVT_H
#define _IVT_H

#define SYS_PRIV 0x8E00

struct cpu_state {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int esp;
} __attribute__((packed));

struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));

struct ivt
{
    unsigned int    address;
    unsigned short  size;
} __attribute__((packed));

struct ivt_entry
{
        unsigned short offsethigh;
        unsigned short offsetlow;
        unsigned char  flags;
        unsigned short segmentselector;    
} __attribute__((packed));

void interrupt_handler(struct cpu_state cpu, struct stack_state stack, unsigned int interrupt);
void init_ivt(void);
void load_ivt_entries(unsigned idx, unsigned long offset, unsigned short seg_selector, unsigned char flags);

#endif /* _IVT_H */
