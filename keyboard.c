#include <stdbool.h>

#include "keyboard.h"
#include "isr.h"
#include "io.h"
#include "string.h"
#include "framebuffer.h"
#define KBD_DATA_PORT 0x60

int capsLock = 0;
int shiftDown = 0;

static void keyboard_cb(registers_t regs);


static int isLatinLetter(unsigned char c){
    return (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z');
}

int scancodes[]  = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   17,		/* Left shift: Device Control 1 */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   17,				/* Right shift: Device Control 1 */
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
    174, /* Left Arrow */
    0,
    175, /* Right Arrow */
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
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',     /* 9 */
  '(', ')', '_', '+', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'Q', 'W', 'E', 'R',   /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
    0,                  /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',     /* 39 */
 '"', '~',   17,               /* Left shift: Device Control 1 */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',                    /* 49 */
  'M', '<', '>', '?',   17,                             /* Right shift: Device Control 1 */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    7,  /* Caps lock is a BELL in ASCII. We'll use this to toggle caps.*/
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    174,  /* Left Arrow */
    0,
    175,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

static void keyboard_cb(registers_t regs) {
  unsigned char scan_code = inb(KBD_DATA_PORT);
  unsigned char c = scancodes[scan_code];
  if(scan_code & 0x80){
      // Key was just released.
      if(scan_code == 0xAA || scan_code == 0xB6){
           shiftDown = 0;
      }
  }else{
      if(c == 17) {
          shiftDown = 1;
          return;
      } else if(c == 174) {
          // Left arrow key
          fb_back_pos();
          return;
      } else if(c == 175) {
          // Right arrow key
          fb_advance_pos();
          return;
      }
      /*
       * An ASCII Latin lowercase letter and its
       * uppercase counterpart differ in the 5th 
       * bit. 
       * Example: A = 01000001
       *          a = 01100001
       *
       * To make uppercase, we simply AND a Latin letter
       * with 0b1101111, or 0xDF. 
       * To toggle the case, we can XOR the letter with
       * 0b00100000, or 0x20, to toggle the 5th bit.
       */
      if(shiftDown) c = scancodes[scan_code + 90];
      printf("%c", c);
  }
}

void init_keyboard() {
  register_interrupt_handler(IRQ1, &keyboard_cb);
}
