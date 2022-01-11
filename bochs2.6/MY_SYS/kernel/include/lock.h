#ifndef LOCK_H
#define LOCK_H
#include "thread.h"
#include "interrupt.h"
#include "print.h"
#include "list.h"

struct semaphore{
    int val;
    struct list_head wait_task_list;
};

struct lock{
    struct semaphore sem;
    struct task_struct* holder;
    int nr;
};

void lock(struct lock* loc);
void unlock(struct lock* loc);

void P(struct semaphore* semp);
void V(struct semaphore* semp);
void block(enum TASK_STATUS status);
void wakeup(struct task_struct* p);

void init_lock(struct lock* loc);
void init_sem(struct semaphore* semp,int init_val);

#endif