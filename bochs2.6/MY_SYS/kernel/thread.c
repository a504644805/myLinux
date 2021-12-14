#include "include/thread.h"
/*
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

    
    //void* pdt;
    //struct pool u_vpool;
    

   int stack_overflow_chk;
};
*/

struct list_head ready_list;
struct list_head all_task_list;
//#define MAIN_THREAD_TASK_STRUCT 0xc009e000
void make_main_thread(){
    INIT_LIST_HEAD(&ready_list);
    INIT_LIST_HEAD(&all_task_list);

    struct task_struct* p=(struct task_struct*)MAIN_THREAD_TASK_STRUCT;
    p->esp;//no need to save esp because main thread is running now, esp is now somewhat below 0xc009f000
    p->status=RUNNING;
    p->tag_s;
    __list_add(&(p->tag_all),&all_task_list,&all_task_list);
    p->prio=31;
    p->ticks=p->prio;
    p->elapsed_ticks=0;
    p->stack_overflow_chk=19980211;
}

void* start_thread(void (*f)(void*), void* f_arg, int prio){
    struct task_struct* pcb=(struct task_struct*)malloc_page(K,1);
    if(pcb==NULL){
        return NULL;
    }
    pcb->esp=(void*)pcb+4*KB;
    pcb->status=READY;
    list_add_tail(&(pcb->tag_s),&ready_list);
    list_add_tail(&(pcb->tag_all),&all_task_list);
    pcb->prio=prio;
    pcb->ticks=pcb->prio;
    pcb->elapsed_ticks=0;
    pcb->stack_overflow_chk=19980211;

    struct s{
        //prepare for switch_to
        uint32_t ebp;
        uint32_t ebx;
        uint32_t edi;
        uint32_t esi;
        void* f_wrapper;
        //f_wrapper充分信任caller的工作
        uint32_t unused_4B;
        void* f;
        void* f_arg;
    };
    pcb->esp-=sizeof(struct intr_s);//never use: create_process, fork时才会有此操作，此处只是为了统一(习惯了)
    pcb->esp-=sizeof(struct s);
    struct s* p=(struct s*)(pcb->esp);
    p->f_wrapper=f_wrapper;
    p->f=f;
    p->f_arg=f_arg;

    return pcb;
}

extern uint32_t SYS_ELAPSED_TIME;
void time_intr_handler(){
    SYS_ELAPSED_TIME++;
    struct task_struct* cur=get_cur_running();
    ASSERT((cur->stack_overflow_chk)==19980211);
    if(++(cur->elapsed_ticks) == (cur->ticks)){
        struct task_struct* next=schedule();
        if(next==NULL){
            return;
        }
        else{
            cur->esp;//saved via switch_to
            cur->status=READY;
            ASSERT(list_find(&ready_list,&(cur->tag_s))==0);
            list_add_tail(&(cur->tag_s),&ready_list);
            cur->elapsed_ticks=0;

            //restore next->esp  done by switch_to
            next->status=RUNNING;
            __list_del(next->tag_s.prev,next->tag_s.next);
            next->elapsed_ticks=0;

            switch_to(cur,next);
            //                  <--back to here when cur is rescheduled
        }
    }
    else{
    }
}

struct task_struct* schedule(){
    if(list_empty(&ready_list)){
        return NULL;
    }
    else{
        return container_of(ready_list.next,struct task_struct,tag_s);
    }
}

struct task_struct* get_cur_running(){
    uint32_t esp;
    asm volatile("mov %%esp,%0":"=a"(esp));
    return (struct task_struct*)(esp&0xfffff000);
}

void f_wrapper(void (*f)(void*),void* f_arg){
    enable_intr();//
    f(f_arg);
}