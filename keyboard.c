#include <stdbool.h>

#include "keyboard.h"
#include "isr.h"
#include "io.h"
#include "string.h"

#define KBD_DATA_PORT 0x60
static void keyboard_cb(registers_t regs);
int scancodes[] = { 
  0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*'
 };

static void keyboard_cb(registers_t regs) {
  unsigned char scan_code = inb(KBD_DATA_PORT);
  int c = scancodes[scan_code];   
  printf("%c", c);
}

void init_keyboard() {
  register_interrupt_handler(IRQ1, &keyboard_cb);
}
