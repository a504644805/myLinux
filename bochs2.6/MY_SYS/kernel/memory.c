#include "include/memory.h"
/*
struct bitmap{
    char* p;
    size_t byte_len;
};
*/
void set_bit_bm(struct bitmap* bm,int bit_idx){
    int byte_idx=bit_idx/8;
    (bm->p)[byte_idx]|=(1<<(bit_idx%8));
}
void clear_bit_bm(struct bitmap* bm,int bit_idx){
    int byte_idx=bit_idx/8;
    (bm->p)[byte_idx]&=~(1<<(bit_idx%8));
}
int get_bit_bm(const struct bitmap* bm,int bit_idx){
    int byte_idx=bit_idx/8;
    if( (bm->p)[byte_idx] & (1<<(bit_idx%8)) ){
        return 1;
    }
    else{
        return 0;
    }
}
int scan_bm(const struct bitmap* bm,size_t cnt){
    size_t bit_cnt=0;
    int bit_pointer=0;
    //按位进行一遍搜索.
    //Note: get_bit_bm is not efficient so scan_bm is not efficient, but its idea is simple
    for(;bit_cnt<cnt && bit_pointer<((bm->byte_len)*8); bit_pointer++){
        if(get_bit_bm(bm,bit_pointer)==1){
            bit_cnt=0;
        }
        else{
            bit_cnt++;
        }
    }
    
    if(bit_cnt==cnt){
        return bit_pointer-cnt;
    }
    else{
        return -1;
    }
}


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
        return NULL;
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


void report_init_pool(){
    put_str("Here is the init_pool result:\n");
    
    put_str("kpool.bm.p==");put_int((int)kpool.bm.p);put_str(",used ");put_int(kpool.bm.byte_len);put_str(" bytes and manages area from ");put_int((int)kpool.s_addr);put_str(" to ");put_int((int)(kpool.s_addr+kpool.man_sz));put_str("\n");
    
    put_str("upool.bm.p==");put_int((int)upool.bm.p);put_str(",used ");put_int(upool.bm.byte_len);put_str(" bytes and manages area from ");put_int((int)upool.s_addr);put_str(" to ");put_int((int)(upool.s_addr+upool.man_sz));put_str("\n");

    put_str("k_vpool.bm.p==");put_int((int)k_vpool.bm.p);put_str(",used ");put_int(k_vpool.bm.byte_len);put_str(" bytes and manages area start from ");put_int((int)k_vpool.s_addr);put_str("\n");
}

