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


#define CIRCULAR_QUEUE_MAXSIZE 100

struct circular_queue{ 
    char buf[CIRCULAR_QUEUE_MAXSIZE];
    int head;
    int tail;
    struct lock lock;
    struct semaphore full;
};
void init_cq(struct circular_queue* q);
char cq_take_one_elem(struct circular_queue* q);
void cq_put_one_elem(struct circular_queue* q,char c);
int cq_is_empty(struct circular_queue* q);
int cq_is_full(struct circular_queue* q);

#endif