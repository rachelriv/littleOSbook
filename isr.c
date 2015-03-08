// from http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
#include <stdint.h>
#include "isr.h"
#include "string.h"
#include "framebuffer.h"
#include "io.h"

#define PIC1            0x20    /* IO base address for master PIC */
#define PIC2            0xA0    /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)

#define PIC_EOI         0x20    /* End-of-interrupt command code */

/* This is a simple string array. It contains the message that
*  corresponds to each and every exception. We get the correct
*  message by accessing like:
*  exception_message[interrupt_number] */
unsigned char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint Exception",
    "Into Detected Overflow Exception",
    "Out of Bounds Exception",
    "Invalid Opcode Exception", 
    "No Coprocessor Exception",
    "Double Fault Exception",
    "Coprocessor Segment Overrun Exception",
    "Bad TSS Exception",
    "Segment Not Present Exception",
    "Stack Fault Exception",
    "General Protection Fault Exception",
    "Page Fault Exception",
    "Unknown Interrupt Exception",
    "Coprocessor Fault Exception",
    "Alignment Check Exception (486+)",
    "Machine Check Exception (Pentium/586+)",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/* All of our Exception handling Interrupt Service Routines will
*  point to this function. This will tell us what exception has
*  happened! We might want to halt the system.
*/
void isr_handler(registers_t regs) {
  printf(exception_messages[regs.int_no]);
  printf("\n");
  //TODO: infinite loop to halt the system?
}

isr_t interrupt_handlers[256];

void ack_irq(int int_no) {
  // Send an EOI (end of interrupt) signal to the PICs.
  // If this interrupt involved the slave.
  if (int_no >= 40)
  {
     // Send reset signal to slave.
     outb(PIC2_COMMAND, PIC_EOI);
  }
  // Send reset signal to master. (As well as slave, if necessary).
  outb(PIC1_COMMAND, PIC_EOI);
}

// This gets called from our ASM interrupt handler stub.
void irq_handler(registers_t regs)
{
  //printf("h/w interrupt: %i\n", regs.int_no);

  ack_irq(regs.int_no);

  if (interrupt_handlers[regs.int_no] != 0)
  {
     isr_t handler = interrupt_handlers[regs.int_no];
     handler(regs);
  }
}

void register_interrupt_handler(uint8_t n, isr_t handler)
{
  interrupt_handlers[n] = handler;
}
