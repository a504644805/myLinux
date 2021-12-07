#ifndef MY_STDINT
#define MY_STDINT
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
#endif

#ifndef MY_GLOBAL_H
#define MY_GLOBAL_H

//IDT
#define IDT_DESC_NUM 33


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

#endif
