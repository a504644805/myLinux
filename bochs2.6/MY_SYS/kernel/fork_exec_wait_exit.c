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