#include <stdbool.h>

#include "keyboard.h"
#include "isr.h"
#include "io.h"
#include "string.h"

#define KBD_DATA_PORT 0x60

int capsLock = 0;

static void keyboard_cb(registers_t regs);
int scancodes[]  = {
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
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    7,	/* Caps lock is a BELL in ASCII. We'll use this to toggle caps.*/
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

static void keyboard_cb(registers_t regs) {
  unsigned char scan_code = inb(KBD_DATA_PORT);
  int c = scancodes[scan_code];
  if(scan_code & 0x80){
      // TODO:
      // Key was just released.
      if(c == 7){
          capsLock = 1 - capsLock;
      }
  }else{
      printf("%c", c);
  }
}

void init_keyboard() {
  register_interrupt_handler(IRQ1, &keyboard_cb);
}
