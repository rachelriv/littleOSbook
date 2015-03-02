#ifndef _IVT_H
#define _IVT_H

#define SYS_PRIV 0x8E00

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

void init_ivt(void);
void load_ivt_entries(unsigned idx, unsigned long offset, unsigned short seg_selector, unsigned char flags);

#endif /* _IVT_H */
