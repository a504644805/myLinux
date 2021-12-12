#ifndef THREAD_H
#define THREAD_H
#include "global.h"
#include "memory.h"
#include "interrupt.h"
#include "debug.h"
#include "list.h"

#define MAIN_THREAD_TASK_STRUCT 0xc009e000

void make_main_thread();
void* start_thread(void (*f)(void*), void* f_arg, int prio);
void time_intr_handler();

struct task_struct* schedule();
struct task_struct* get_cur_running();
void f_wrapper(void (*f)(void*),void* f_arg);

//in switch_to.S
void switch_to(struct task_struct* cur,struct task_struct* next);

/*
中断服务程序具体内容(intr_no)刚开始执行时的内核栈
push INTR_NO
. <--Here
add esp,4
*/
struct intr_s{
    uint32_t intrNo;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;//popad will omit esp
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint32_t errNo;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp3;//
    uint32_t ss3;
};

struct task_struct{
    void* esp;

    enum{
        RUNNING,READY,WAITING,HANGING
    }status;
    list_node tag_s;//挂载在和status相关的队列上
    list_node tag_all;
    int prio;
    int ticks;//==prio in our policy
    int elapsed_ticks;

    
    /*
    void* pdt;
    struct pool u_vpool;
    */

   int stack_overflow_chk;
};

#endif