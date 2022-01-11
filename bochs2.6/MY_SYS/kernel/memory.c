#include "list.h"
#include "print.h"
#include "lib/debug.h"
#include "include/memory.h"
#include "stdio.h"

/*
enum POOL_TYPE{
    P,V
};
struct pool{
    struct bitmap bm;
    void* s_addr;
    
    size_t man_sz;//in byte, the pool manages sz bytes
    enum POOL_TYPE type;
};

#define POOL_BASE_ADDR 0xc009a000
#define K_HEAP_BASE_ADDR 0xc0100000
*/
struct pool kpool,upool,k_vpool;
void init_pool(){
    memset((void*)0xc009a000,0,4*PG_SIZE);

    size_t memsz=1*MB+(int)(*((int*)(0xb00)));//actual memory is smaller than memsz because some are shadow area in low 1MB
    if(memsz>128*MB){
        memsz=128*MB;//we have 4 page for 3 bm.p, 1 page(4KB) can manage 128MB, tired of calculation so just pick 128MB as the max memsz we support
    }
    
    size_t kmem=(memsz-2*MB)/2;
    size_t umem=(memsz-2*MB)-kmem;

    kpool.bm.p=(char*)POOL_BASE_ADDR;
    kpool.bm.byte_len=kmem/(4*KB)/8;
    kpool.s_addr=(void*)(2*MB);
    kpool.man_sz=kmem;
    kpool.type=Physical;

    upool.bm.p=(char*)(POOL_BASE_ADDR+kpool.bm.byte_len);
    upool.bm.byte_len=umem/(4*KB)/8;
    upool.s_addr=(void*)(2*MB+kmem);
    upool.man_sz=umem;
    upool.type=Physical;

    k_vpool.bm.p=(char*)POOL_BASE_ADDR + kpool.bm.byte_len + upool.bm.byte_len;
    k_vpool.bm.byte_len=kpool.bm.byte_len;
    k_vpool.s_addr=(void*)(K_HEAP_BASE_ADDR);
    k_vpool.man_sz=0;
    k_vpool.type=Virtual;

    report_init_pool();
}

/*
enum K_U_FLAG{
    K,U
};
*/
void* malloc_page(enum K_U_FLAG flag,size_t cnt){
    void* vaddr_start=valloc(flag,cnt);
    void* vaddr=vaddr_start;
    if(vaddr==NULL){
        return NULL;
    }

    for (size_t i = 0; i < cnt; i++){
        void* paddr=palloc(flag);
        if(paddr==NULL){
            /*
                回退
            */
            ASSERT(paddr!=NULL);
            return NULL;
        }
        else{
            build_mapping(vaddr,paddr);
            vaddr+=4*KB;// no need to "/4"
        }
    }
    return vaddr_start;
}

/*
alloc cnt continuous pages from vpool.
*/
void* valloc(enum K_U_FLAG WHICH_VPOOL,size_t cnt){
    if(WHICH_VPOOL==K){
        int bit_idx=scan_bm(&k_vpool.bm,cnt);
        if(bit_idx==-1){
            return NULL;
        }
        else{
            for(size_t i=0;i<cnt;i++){
                set_bit_bm(&k_vpool.bm,bit_idx+i);
            }
        }
        return k_vpool.s_addr+bit_idx*4*KB;
    }
    else{
        /*
            alloc from u_vpool
        */
        struct task_struct* cur=get_cur_running();
        ASSERT(cur->u_vpool.bm.p);
        int bit_idx=scan_bm(&(cur->u_vpool.bm),cnt);
        if(bit_idx==-1){
            return NULL;
        }
        else{
            for(size_t i=0;i<cnt;i++){
                set_bit_bm(&(cur->u_vpool.bm),bit_idx+i);
            }
        }
        return (cur->u_vpool).s_addr+bit_idx*4*KB;
    }
}

/*
alloc 1 page from ppool
*/
void* palloc(enum K_U_FLAG WHICH_PPOOL){
    struct bitmap* bm=(WHICH_PPOOL==K)?(&kpool.bm):(&upool.bm);
    void* s_addr=(WHICH_PPOOL==K)?(kpool.s_addr):(upool.s_addr);
    int bit_start=scan_bm(bm,1);
    if(bit_start==-1){
        return NULL;
    }
    else{
        set_bit_bm(bm,bit_start);
        return s_addr+bit_start*4*KB;
    }
}

void build_mapping(void* vaddr,void* paddr){//vaddr和paddr是4KB对齐的空闲块始址
    uint32_t* pde_addr=get_pde_addr(vaddr);//通过将pde_addr作为逻辑地址进行访存(即*pde_addr)可以访问到对应的pde
    uint32_t* pte_addr=get_pte_addr(vaddr);
    if(!(*pde_addr & PTE_P)){
        //pde.P==0
        void* pt_paddr=palloc(K);
        ASSERT(pt_paddr!=NULL);
        *pde_addr=(unsigned int)pt_paddr|PTE_U|PTE_RW|PTE_P;
        memset((void*)((unsigned int)pte_addr&0xfffff000),0,PG_SIZE);//clear the new pt
    }
    else{
    }
    *pte_addr=(unsigned int)paddr|PTE_U|PTE_RW|PTE_P;
}

void* get_pde_addr(void* _vaddr){
    unsigned int vaddr=(unsigned int)_vaddr;
    return (void*)((1023<<22)|(1023<<12)|((vaddr>>22)*4));
}

void* get_pte_addr(void* _vaddr){
    unsigned int vaddr=(unsigned int)_vaddr;
    return (void*)((1023<<22)|((vaddr>>22)<<12)|(((vaddr<<10)>>22)*4));
}

void* get_phy_addr(void* _vaddr){
    void* pte_addr=get_pte_addr(_vaddr);
    unsigned int pte_content=*(unsigned int*)pte_addr;
    return (void*)((pte_content&0xfffff000) + (((unsigned int)_vaddr)&0x00000fff));
}

void report_init_pool(){
    put_str("Here is the init_pool result:\n");
    
    put_str("kpool.bm.p==");put_int((int)kpool.bm.p);put_str(",used ");put_int(kpool.bm.byte_len);put_str(" bytes and manages area from ");put_int((int)kpool.s_addr);put_str(" to ");put_int((int)(kpool.s_addr+kpool.man_sz));put_str("\n");
    
    put_str("upool.bm.p==");put_int((int)upool.bm.p);put_str(",used ");put_int(upool.bm.byte_len);put_str(" bytes and manages area from ");put_int((int)upool.s_addr);put_str(" to ");put_int((int)(upool.s_addr+upool.man_sz));put_str("\n");

    put_str("k_vpool.bm.p==");put_int((int)k_vpool.bm.p);put_str(",used ");put_int(k_vpool.bm.byte_len);put_str(" bytes and manages area start from ");put_int((int)k_vpool.s_addr);put_str("\n");
}

//---------------free_page-------------------
void free_page(void* vaddr,size_t cnt){
    struct task_struct* cur=get_cur_running();
    ASSERT(((uint32_t)vaddr>=0x8048000&&(uint32_t)vaddr<0xc0000000&&cur->pd) || ((uint32_t)vaddr>=0xc0100000&&cur->pd==NULL));
    ASSERT(((uint32_t)vaddr&0x00000fff)==0);

    enum K_U_FLAG k_u_flag=((uint32_t)vaddr>=0xc0100000)?K:U;
    struct pool* ppool=(k_u_flag==K)?(&kpool):(&upool);
    struct pool* vpool=(k_u_flag==K)?(&k_vpool):(&(cur->u_vpool));
    void* pte_addr;
    void* phy_addr;
    //把映射一个个解决掉
    for(int i=0;i<cnt;i++){
        pte_addr=get_pte_addr(vaddr);
        phy_addr=get_phy_addr(vaddr);
        clear_bit_bm(&(ppool->bm),(phy_addr-(ppool->s_addr))/PG_SIZE);//palloc
        clear_bit_bm(&(vpool->bm),(vaddr-(vpool->s_addr))/PG_SIZE);//valloc
        *(uint32_t*)pte_addr&=~PTE_P;
        asm volatile("invlpg %0"::"m"(vaddr));

        vaddr+=PG_SIZE;
    }
}

//------------------arena----------------------
/*
enum BIG_FLAG {BIG,NOT_BIG};
struct arena{
    struct arena_cluster* cluster;
    enum BIG_FLAG big_flag;
    size_t cnt;

    int magicNumber;//19985757
};

struct block_header{
    list_node lnode;
};
struct arena_cluster{
    size_t block_sz;//16,32,64,128,256,512,1024
    struct list_head lhead;//双向链表

    size_t block_per_page;
};

#define CLUSTER_CNT 7
*/
struct arena_cluster k_arena_cluster[CLUSTER_CNT];
void init_k_arena_cluster(){
    size_t block_sz=16;
    for(int i=0;i<CLUSTER_CNT;i++,block_sz*=2){
        k_arena_cluster[i].block_sz=block_sz;
        INIT_LIST_HEAD(&(k_arena_cluster[i].lhead));
        k_arena_cluster[i].block_per_page=(PG_SIZE-sizeof(struct arena))/block_sz;
    }
}

void* sys_malloc(size_t sz){
    struct task_struct* cur=get_cur_running();
    enum K_U_FLAG k_u_flag=(cur->pd)?U:K;
    struct arena_cluster* arena_cluster=(k_u_flag==K)?(k_arena_cluster):(cur->u_arena_cluster);
    struct arena* a;
    if(sz>1024){
        a=malloc_page(k_u_flag,DIVUP(sz+sizeof(struct arena),PG_SIZE));
        a->cluster=NULL;
        a->big_flag=BIG;
        a->cnt=DIVUP(sz,PG_SIZE);
        a->magicNumber=19985757;
        return (void*)a+sizeof(struct arena);
    }
    else{//sz<=1024
        int idx=0;//cluster_idx
        for(;idx<CLUSTER_CNT && arena_cluster[idx].block_sz<sz;idx++);
        if(list_empty(&(arena_cluster[idx].lhead))){
            a=malloc_page(k_u_flag,1);
            //准备好arena块中的struct arena以及把各block_header链上去
            a->cluster=&(arena_cluster[idx]);
            a->big_flag=NOT_BIG;
            a->cnt=arena_cluster[idx].block_per_page;
            a->magicNumber=19985757;
            for(void* p=(void*)a+sizeof(struct arena); p+arena_cluster[idx].block_sz<=(void*)a+PG_SIZE; p+=arena_cluster[idx].block_sz){
                ASSERT(list_find(&(arena_cluster[idx].lhead),&(((struct block_header*)p)->lnode))==NULL);
                list_add_tail(&(((struct block_header*)p)->lnode),&(arena_cluster[idx].lhead));
                ASSERT(list_find(&(arena_cluster[idx].lhead),&(((struct block_header*)p)->lnode)));
            }
        }
        else{}
        ASSERT(!list_empty(&(arena_cluster[idx].lhead)));
        struct block_header* rt=container_of(list_pop(&(arena_cluster[idx].lhead)),struct block_header,lnode);
        a=block2arena(rt);
        a->cnt--;
        return (void*)rt;
    }
}

void sys_free(void* p){
    struct task_struct* cur=get_cur_running();
    struct arena* a=block2arena(p);
    uint32_t _p=(uint32_t)p;//used for judge
    struct block_header* bh=(struct block_header*)p;
    ASSERT((_p>=0x8048000&&_p<0xc0000000&&cur->pd) || (_p>=0xc0100000&&cur->pd==NULL));
    ASSERT(a->magicNumber==19985757);

    if(a->big_flag==BIG){
        free_page(a,a->cnt);
    }
    else{
        list_add_tail(&(bh->lnode),&(a->cluster->lhead));
        a->cnt++;
        if(a->cnt==a->cluster->block_per_page){
            //依次释放arena块的各block_header并清理struct arena的魔数
            for(void* p=(void*)a+sizeof(struct arena); p+a->cluster->block_sz<=(void*)a+PG_SIZE; p+=a->cluster->block_sz){
                ASSERT(list_find(&(a->cluster->lhead),&(((struct block_header*)p)->lnode)));
                __list_del(((struct block_header*)p)->lnode.prev,((struct block_header*)p)->lnode.next);
                ASSERT(list_find(&(a->cluster->lhead),&(((struct block_header*)p)->lnode))==NULL);
            }
            a->magicNumber=0;
            free_page(a,1);
        }
    }
}

struct arena* block2arena(struct block_header* block_addr){
    return (struct arena*)((uint32_t)block_addr&0xfffff000);
}