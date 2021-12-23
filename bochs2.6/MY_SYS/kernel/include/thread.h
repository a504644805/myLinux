#ifndef THREAD_H
#define THREAD_H
#include "debug.h"
#include "memory.h"
#include "global.h"
#include "interrupt.h"
#include "list.h"
#include "string.h"

#define MAIN_THREAD_TASK_STRUCT 0xc009e000

void make_main_thread();
void* start_thread(void (*f)(void*), void* f_arg, int prio);
void time_intr_handler();

struct task_struct* schedule();
struct task_struct* get_cur_running();
void start_kthread(void (*f)(void*),void* f_arg);

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

enum TASK_STATUS{ RUNNING,READY,WAITING,HANGING,BLOCKED };
struct task_struct{
    void* esp;

    uint32_t pid;

    enum TASK_STATUS status;
    list_node tag_s;//挂载在和status相关的队列上
    list_node tag_all;
    int prio;
    int ticks;//==prio in our policy
    int elapsed_ticks;

    void* pd;
    struct pool u_vpool;
    struct arena_cluster u_arena_cluster[7];

   int stack_overflow_chk;
};

//--------------after we introduce 用户态----------------------
typedef unsigned int pte_t;
typedef unsigned int pde_t;

void* create_process(void (*f)(), int prio);

void* prepare_pd();
void prepare_u_vpool(struct pool* p);
void prepare_u_arena_cluster(struct arena_cluster* u_arena_cluster);
void prepare_intr_s(struct intr_s* p,void(*f)());
#define USTACK_PAGE_SIZE 1
void start_uprocess();

struct TSS{
    uint32_t unused;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t _unused[32];
};
void update_gdt();//在引入用户态后，gdt需添加tss,code3,data3


//-------------------------------------------------------------
uint32_t allocate_pid();

#endif