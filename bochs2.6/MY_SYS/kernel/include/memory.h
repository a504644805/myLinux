#ifndef MEMORY_H
#define MEMORY_H
#include "global.h"
#include "debug.h"
#include "print.h"
#include "string.h"

typedef int testm;

struct bitmap{
    char* p;
    size_t byte_len;
};
void set_bit_bm(struct bitmap* bm,int bit_idx);
void clear_bit_bm(struct bitmap* bm,int bit_idx);
int get_bit_bm(const struct bitmap* bm,int bit_idx);
int scan_bm(const struct bitmap* bm,size_t cnt);

enum POOL_TYPE{
    Physical,Virtual
};
struct pool{
    struct bitmap bm;
    void* s_addr;
    
    size_t man_sz;//in byte, the pool manages sz bytes
    enum POOL_TYPE type;
};

#define POOL_BASE_ADDR 0xc009a000
#define K_HEAP_BASE_ADDR 0xc0100000

enum K_U_FLAG{
    K,U
};
void init_pool();
void* malloc_page(enum K_U_FLAG flag,size_t cnt);

void* valloc(enum K_U_FLAG WHICH_VPOOL,size_t cnt);
void* palloc(enum K_U_FLAG WHICH_PPOOL);
void build_mapping(void* vaddr,void* paddr);//vaddr和paddr是4KB对齐的空闲块始址

void* get_pde_addr(void* _vaddr);
void* get_pte_addr(void* _vaddr);
void* get_phy_addr(void* _vaddr);
void report_init_pool();

#endif
