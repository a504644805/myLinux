#include "list.h"
#include "stdio.h"
#include "lib/print.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "include/global.h"
#include "include/interrupt.h"
#include "include/memory.h"
#include "include/thread.h"
#include "include/lock.h"
#include "include/syscall.h"
#include "ata.h"
#include "fs.h"
#include "fork_exec_wait_exit.h"
#include "elf.h"
//-1, 0, child_pid
extern void exit_intr();
extern struct list_head ready_list;
extern struct list_head all_task_list;
extern struct file sys_open_file[MAX_SYSTEM_OPEN_FILE];
int sys_fork(){
    struct task_struct* child_t_s=(struct task_struct*)malloc_page(K,1);
    ASSERT(child_t_s!=NULL);
    struct task_struct* parent_t_s=get_cur_running();
    ASSERT(parent_t_s->pd!=NULL);

    //child_t_s's esp and 寄存器环境(借助kernel stack的intr_s来恢复环境)
    /*
      | ABI
      | eip
      v intr_s
    */
    void* p_intr_s=(void*)child_t_s+PG_SIZE-sizeof(struct intr_s);
    void* p_eip=p_intr_s-1*4;
    child_t_s->esp=p_intr_s-5*4;
    memcpy(p_intr_s,(((void*)parent_t_s)+PG_SIZE-sizeof(struct intr_s)),sizeof(struct intr_s));
    *(uint32_t*)p_eip=(uint32_t)exit_intr;
    ((struct intr_s*)p_intr_s)->eax=0;

    //
    child_t_s->status=READY;
    list_add_tail(&(child_t_s->tag_s),&ready_list);
    list_add_tail(&(child_t_s->tag_all),&all_task_list);
    child_t_s->prio=parent_t_s->prio;
    child_t_s->ticks=parent_t_s->ticks;
    child_t_s->elapsed_ticks=0;

    //pd,uvpool,cluster
    void* k_buf=malloc_page(K,1);
    child_t_s->pd=prepare_pd();//768-1022,1023
    for (size_t i = 0; i < (parent_t_s->u_vpool.bm.byte_len)*8; i++){
        if(get_bit_bm(&(parent_t_s->u_vpool.bm),i)==1){
            void* vaddr=parent_t_s->u_vpool.s_addr+i*PG_SIZE;
            void* paddr=palloc(U);
            memcpy(k_buf,vaddr,PG_SIZE);
            asm volatile("movl %0,%%cr3"::"a"(((pde_t*)child_t_s->pd)[1023]));
            build_mapping(vaddr,paddr);
            memcpy(vaddr,k_buf,PG_SIZE);
            asm volatile("movl %0,%%cr3"::"a"(((pde_t*)parent_t_s->pd)[1023]));
        }
        else{
            continue;
        }
    }
    prepare_u_vpool(&(child_t_s->u_vpool));//bm.p=malloc_page(K, )
    memcpy((void*)child_t_s->u_vpool.bm.p,(void*)parent_t_s->u_vpool.bm.p,parent_t_s->u_vpool.bm.byte_len);
    memcpy((void*)child_t_s->u_arena_cluster,(void*)parent_t_s->u_arena_cluster,sizeof(struct arena_cluster)*7);
    for (size_t i = 0; i < CLUSTER_CNT; i++){//把cluster对应双向链表中的内核地址改正确
        if(list_empty(&(parent_t_s->u_arena_cluster[i].lhead))){
            INIT_LIST_HEAD(&(child_t_s->u_arena_cluster[i].lhead));
        }
        else{
            asm volatile("movl %0,%%cr3"::"a"(((pde_t*)child_t_s->pd)[1023]));
            (child_t_s->u_arena_cluster[i].lhead).prev->next=&(child_t_s->u_arena_cluster[i].lhead);
            (child_t_s->u_arena_cluster[i].lhead).next->prev=&(child_t_s->u_arena_cluster[i].lhead);
            asm volatile("movl %0,%%cr3"::"a"(((pde_t*)parent_t_s->pd)[1023]));
        }
    }
    
    //fs
    for (size_t i = 0; i < MAX_PROCESS_OPEN_FILE; i++){
        uint32_t sys_ofile_idx=(child_t_s->process_open_file)[i]=(parent_t_s->process_open_file)[i];
        if(sys_ofile_idx!=-1){
            sys_open_file[sys_ofile_idx].i_p->cnt++;
        }
    }
    child_t_s->cwd_ino=parent_t_s->cwd_ino;

    //
    child_t_s->pid=allocate_pid();
    child_t_s->ppid=parent_t_s->pid;
    child_t_s->stack_overflow_chk=parent_t_s->stack_overflow_chk;

    free_page(k_buf,1);
    return child_t_s->pid;
}

void sys_execv(const char *path, char *const argv[]){
    struct task_struct* t_s=get_cur_running();
    int fd=sys_open(path,O_RDONLY);
    ASSERT(fd!=-1);
    Elf32_Ehdr ehdr;
    sys_read(fd,&ehdr,sizeof(Elf32_Ehdr));
    ASSERT(memcmp((void*)&ehdr,"\177ELF\1\1\1",7)==0);
    ASSERT(ehdr.e_phentsize==sizeof(Elf32_Phdr));
    uint32_t page_cnt_alloc_for_ph_table=DIVUP((ehdr.e_phentsize*ehdr.e_phnum),PG_SIZE);
    Elf32_Phdr* phdr=(Elf32_Phdr*)malloc_page(K,page_cnt_alloc_for_ph_table);//malloc from kernel(我们接下来要在用户空间按elf写段，若存于用户空间可能会破坏该内容)
    ASSERT(phdr!=NULL);
    sys_read(fd,(void*)phdr,ehdr.e_phentsize*ehdr.e_phnum);
    prepare_u_arena_cluster(t_s->u_arena_cluster);
    void* buf=malloc_page(K,1);

    for (size_t i = 0; i < ehdr.e_phnum; i++){
        if(phdr[i].p_type==PT_LOAD){
            for (size_t j = 0; j < DIVUP(phdr[i].p_filesz,PG_SIZE); j++){
                sys_lseek(fd,phdr[i].p_offset+j*PG_SIZE,SEEK_SET);
                sys_read(fd,buf,PG_SIZE);
                uint32_t u_vpool_idx=((void*)phdr[i].p_vaddr-t_s->u_vpool.s_addr)/PG_SIZE;
                ASSERT(u_vpool_idx>=0);
                if(get_bit_bm(&(t_s->u_vpool.bm),u_vpool_idx)){
                    memcpy((void*)(phdr[i].p_vaddr),buf,PG_SIZE);
                }
                else{
                    malloc_page_with_vaddr(U,1,(void*)(phdr[i].p_vaddr));//won't free it till exit
                    memcpy((void*)(phdr[i].p_vaddr),buf,PG_SIZE);
                }
            }
        }
        else{
        }
    }

    struct intr_s* intr_s=(struct intr_s*)((void*)t_s+PG_SIZE-sizeof(struct intr_s));
    intr_s->eip=ehdr.e_entry;
    intr_s->esp3;

    /*
    ^   args: ls0-l0-h0
    |   argv: |  |  |  0
    |   argc: 3
    */
    uint32_t args_size=0;//将要复制到用户栈的args的长度
    uint32_t argc=0;
    for (size_t i = 0; argv[i]!=0; i++){
        argc++;
        args_size+=strlen(argv[i])+1;//+1:strlen未将ls0的0计入
    }
    ASSERT(argc>=1);
    void* const start_addr_of_args=(void*)((0xc0000000-args_size)&(0xffffff00));//&(0xffffff00):4字节对齐以保证栈的正常运作
    memcpy(start_addr_of_args,(void*)(argv[0]),args_size);

    uint32_t argv_array_size=4*(argc+1);//+1:argv以0结尾
    void* const start_addr_of_argv=start_addr_of_args-argv_array_size;
    char* new_argv[argc+1];//argv指向内核空间中的args，new_argv指向用户空间中的args
    for (size_t i = 0; i < argc; i++){
        new_argv[i]=argv[i]-(argv[0]-(char*)start_addr_of_args);
    }
    new_argv[argc]=0;
    memcpy(start_addr_of_argv,(void*)new_argv,argv_array_size);

    void* const start_addr_of_argc=start_addr_of_argv-4;
    memcpy(start_addr_of_argc,&argc,4);
    
    intr_s->esp3=(uint32_t)start_addr_of_argc;
    free_page(buf,1);
    free_page((void*)phdr,page_cnt_alloc_for_ph_table);
    sys_close(fd);
}

//由于wait和exit涉及到的miniOS细节较多，因此代码注释也会较多。见谅:)
#define INIT_PID 2
extern struct list_head all_task_list,ready_list;
extern struct pool kpool;
void sys_exit(int status){
    struct task_struct* const t_s=get_cur_running();
    struct task_struct* cur_t_s;
    list_for_each_entry(cur_t_s,&all_task_list,tag_all){
        if(cur_t_s->ppid==t_s->pid){
            cur_t_s->ppid=INIT_PID;
        }
        else{
            continue;
        }
    }

    //释放进程的用户地址空间
    for (size_t i = 0; i < (t_s->u_vpool.bm.byte_len)*8; i++){
        if(get_bit_bm(&(t_s->u_vpool.bm),i)==1){
            void* vaddr=t_s->u_vpool.s_addr+i*PG_SIZE;
            free_page(vaddr,1);
        }
        else{
            continue;
        }
    }
    //由于free_page不会将已空二级页表释放，因此还需专门释放下
    for (size_t i = 0; i < 768; i++){
        void* p_pde=get_pde_addr((void*)(0+i*4*MB));
        uint32_t pde=*((uint32_t*)p_pde);
        if(pde&PTE_P){
            //由于malloc_page新建二级页表时只是借助palloc从kpool里分配出空间，而不涉及k_vpool的分配，因此释放时只需要处理下kpool.bm
            uint32_t pte_paddr=pde&0xfffff000;//pde中存放的是pte的物理地址
            clear_bit_bm(&(kpool.bm),((void*)pte_paddr-(kpool.s_addr))/PG_SIZE);
        }
        else{
            continue;
        }
    }
    free_page(t_s->u_vpool.bm.p,DIVUP(t_s->u_vpool.bm.byte_len,PG_SIZE));

    for (size_t i = 3; i < MAX_PROCESS_OPEN_FILE; i++){
        if(t_s->process_open_file[i]!=-1){
            sys_close(i);
        }
        else{
            continue;
        }
    }
    
    t_s->exit_status=status;

    ASSERT(cur_t_s->ppid!=-1);
    list_for_each_entry(cur_t_s,&all_task_list,tag_all){
        if(cur_t_s->pid==t_s->ppid){
            if(cur_t_s->status==WAITING){
                wakeup(cur_t_s);
            }
            else{
            }
            break;//只有一个父亲，找到第一个即可退出循环
        }
        else{
            continue;
        }
    }

    /*
    block完成的就是置一下status然后调度下一进程，其不负责tag_s的处理。
    exit函数block之前无需对tag_s做处理：
    首先，在miniOS中tag_s只可能在rdy_list或某个semaphore的list中。而此时进程正在运行，其不可能在这两个队列中，因此无需对tag_s做处理。*/
    ASSERT(list_find(&ready_list,&(t_s->tag_s))==NULL);
    block(HANGING);
    ASSERT(1==2);//shouldn't be here
}


/*
If wstatus is not NULL, wait() store  status  information in  the int to which it points.  
on success, returns the process ID of the terminated child; on error, -1 is returned.*/
/*
功能: wait掉一个孩子
看有没有孩子,若没则rt-1
		   若有且处于HANGING则拿出exit_status,释放pcb及pcb里的剩余资源，rt pid
		   若有且都不处于HANGING则block(WAITING)
*/
int sys_wait(int *wstatus){
    struct task_struct* const t_s=get_cur_running();
    struct task_struct* cur_t_s;
    while(1){
        boolean child_exist_flag=0;
        list_for_each_entry(cur_t_s,&all_task_list,tag_all){
            if(cur_t_s->ppid==t_s->pid){
                child_exist_flag=1;
                if(cur_t_s->status==HANGING){
                    *wstatus=cur_t_s->exit_status;

                    ASSERT(list_find(&ready_list,&(cur_t_s->tag_s))==NULL);
                    ASSERT(list_find(&all_task_list,&(cur_t_s->tag_all)));
                    __list_del(cur_t_s->tag_all.prev,cur_t_s->tag_all.next);
                    free_page(cur_t_s->pd,1);
                    uint32_t child_pid=cur_t_s->pid;
                    free_page((void*)cur_t_s,1);

                    return child_pid;
                }
                else{
                    continue;
                }
            }
            else{
                continue;
            }
        }
        if(child_exist_flag==0){
            return -1;
        }
        else{
            block(WAITING);
        }
    }
}