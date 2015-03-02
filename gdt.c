#include "gdt.h"

static struct gdt_entry gdt_entries[NUM_GDT_ENTRIES];

void
init_gdt(void) {
	struct gdt g =	{ 0x0, (short) -1U };

	load_gdt_entry(0, 0, 0); // NULL descriptor
	load_gdt_entry(1, PL0, CODE_RX_TYPE);
	load_gdt_entry(2, PL0, DATA_RW_TYPE);
	load_gdt(g);
}

void
load_gdt_entry(unsigned idx, unsigned char pl, unsigned char type)
{
	gdt_entries[idx].limit1 = (SEGMENT_LIMIT & 0xFFFF);
        gdt_entries[idx].limit2 = 0;

	gdt_entries[idx].base1 = (SEGMENT_BASE & 0xFFFF);
	gdt_entries[idx].base2 = (SEGMENT_BASE >> 16) & 0xFF;
	gdt_entries[idx].base3 = (SEGMENT_BASE >> 24) & 0xFF;

	gdt_entries[idx].dpl =
		(0x01 << 7) |          // set present to true
		((pl & 0x03) << 5) |   // set privilege level (0 to 3)
		(0x01 << 4) |          // descriptor type (0 for system, 1 for code/data)
		(type & 0x0F);         // type
}
