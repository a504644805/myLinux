#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "global.h"
#include "io.h"
#include "print.h"
#include "thread.h"
#include "keyboard.h"

#define Mask_IF 0x200
enum INTR_STATUS{
    ON,OFF
};
typedef enum INTR_STATUS INTR_STATUS;

void init_interrupt();

INTR_STATUS get_intr_status();
//return the old status
INTR_STATUS set_intr_status(INTR_STATUS s);
INTR_STATUS enable_intr();
INTR_STATUS disable_intr();

#endif