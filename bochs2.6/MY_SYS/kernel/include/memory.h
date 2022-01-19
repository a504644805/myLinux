#ifndef MEMORY_H
#define MEMORY_H
#include "bitmap.h"
#include "print.h"
#include "string.h"
#include "global.h"
#include "list.h"

typedef int testm;

/*
struct bitmap{
    char* p;
    size_t byte_len;
};
void set_bit_bm(struct bitmap* bm,int bit_idx);
void clear_bit_bm(struct bitmap* bm,int bit_idx);
int get_bit_bm(const struct bitmap* bm,int bit_idx);
int scan_bm(const struct bitmap* bm,size_t cnt);
*/

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
void malloc_page_with_vaddr(enum K_U_FLAG flag,size_t cnt,void* vaddr);
void free_page(void* vaddr,size_t cnt);

void* valloc(enum K_U_FLAG WHICH_VPOOL,size_t cnt);
void* palloc(enum K_U_FLAG WHICH_PPOOL);
void build_mapping(void* vaddr,void* paddr);//vaddr和paddr是4KB对齐的空闲块始址

void* get_pde_addr(void* _vaddr);
void* get_pte_addr(void* _vaddr);
void* get_phy_addr(void* _vaddr);
void report_init_pool();


//----------------------------arena--------------------------
enum BIG_FLAG {BIG,NOT_BIG};
struct arena{
    struct arena_cluster* cluster;
    enum BIG_FLAG big_flag;
    size_t cnt;//struct arena加上数据部分总共占用的页数

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

void init_k_arena_cluster();
void* sys_malloc(size_t sz);
void sys_free(void* p);

struct arena* block2arena(struct block_header* block_addr);
#endif
