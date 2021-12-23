#ifndef MY_STDINT
#define MY_STDINT
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
typedef uint32_t size_t;
#endif

#ifndef MY_GLOBAL_H
#define MY_GLOBAL_H

#define NULL 0

#define KB (1024)
#define MB (1024*1024)
//整数整除上取整
#define DIVUP(devidend,devisor) ((devidend+devisor-1)/devisor)

//IDT
#define IDT_DESC_NUM 0x81

#define I_G_D_P_0 0
#define I_G_D_P_1 1
#define I_G_D_DPL_0 0
#define I_G_D_DPL_3 3
#define I_G_D_S_0 0
#define I_G_D_S_1 1
#define I_G_D_TYPE 14

#define I_G_D_attr ((I_G_D_P_1<<7)+(I_G_D_DPL_0<<5)+(I_G_D_S_0<<4)+I_G_D_TYPE)

//selector
#define RPL_0 0
#define RPL_3 3
#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE ((0x0001<<3)+(TI_GDT<<2)+RPL_0)
#define SELECTOR_K_DATA ((0x0002<<3)+(TI_GDT<<2)+RPL_0)

#define TSS_SELECTOR ((4<<3)+(TI_GDT<<2)+RPL_0)
#define CODE3_SELECTOR ((5<<3)+(TI_GDT<<2)+RPL_3)
#define DATA3_SELECTOR ((6<<3)+(TI_GDT<<2)+RPL_3)

//GDT
#define GDT_BASE_ADDR 0xc0000900
//desc.high 4 bytes
#define DESC_G_4KB  (1<<23)
#define DESC_G_1B  0
#define DESC_D_32 (1<<22)
#define DESC_L_0 0
#define DESC_AVL_0 0
#define DESC_LIMIT2_DATA (0xf<<16)
#define DESC_LIMIT2_CODE DESC_LIMIT2_DATA
#define DESC_LIMIT2_TSS 0
#define DESC_P_1 (1<<15)
#define DESC_DPL_0 (0<<13)
#define DESC_DPL_1 (1<<13)
#define DESC_DPL_2 (2<<13)
#define DESC_DPL_3 (3<<13)
#define DESC_S_S (0<<12)
#define DESC_S_D (1<<12)
#define DESC_TYPE_DATA (2<<8)
#define DESC_TYPE_CODE (8<<8)
#define DESC_TYPE_TSS (9<<8)



//PTE/PDE
#define PAGE_DIR_TABLE_POS 0xfffff000

#define PTE_P 1
#define PTE_R 0
#define PTE_RW 2
#define PTE_U 4
#define PTE_S 0

#define PG_SIZE (4*1024)

//eflags
#define EFLAGS_MBS_L (1<<1)
#define EFLAGS_IF_1 (1<<9)
#define EFLAGS_IOPL_0 (0<<12)

#endif
