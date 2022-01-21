#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "lock.h"

#define KBD_READ_REG 0x60

//                 make_code
#define KBD_EXTEND 0xe0
#define KBD_CTRL 0x1d
#define KBD_ALT 0x38
#define KBD_CAPSLOCK 0x3A
#define KBD_SHIFT 0x2A

//              ascii
#define KBD_ESC 0x1b 
#define KBD_INVI 0

void init_kbd();
void kbd_intr_handler();

#endif