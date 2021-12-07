#include "global.h"
#include "io.h"
#include "include/interrupt.h"
void put_str(char* p);
void put_int(int i);

void init_8259A(){
	//Master's ICW1~4
	outb(0x11,0x20);
	outb(32,0x21);
	outb(0x04,0x21);
	outb(0x01,0x21);
	//Slave's ICW1~4
	outb(0x11,0xa0);
	outb(40,0xa1);
	outb(0x02,0xa1);
	outb(0x01,0xa1);
	//OCW1 mask all except IRQ0
	outb(0xfe,0x21);
	outb(0xff,0xa1);
}

typedef struct{
	uint16_t offset_low_16_bits;
	uint16_t selector;
	uint8_t duse;
	uint8_t attr;
	uint16_t offset_high_16_bits;
}i_g_d_struct;

static i_g_d_struct idt[IDT_DESC_NUM];
extern void* int_entry_table[IDT_DESC_NUM];
void init_idt(){
	for(int i=0;i<IDT_DESC_NUM;i++){
		idt[i].offset_low_16_bits=((uint32_t)int_entry_table[i]&0x0000ffff);
		idt[i].selector=SELECTOR_K_CODE;
		idt[i].duse=0;
		idt[i].attr=I_G_D_attr;
		idt[i].offset_high_16_bits=(((uint32_t)int_entry_table[i]&0xffff0000)>>16);
	}

}

//中断服务程序具体内容
void general_interrupt_handler(int interruptNo){
	put_str("interrupt0x");
	put_int(interruptNo);
	put_str(" occured!\n");
}
//中断服务程序具体内容入口地址数组
void* int_content_entry_array[IDT_DESC_NUM];
void init_int_content_entry_array(){
	for(int i=0;i<IDT_DESC_NUM;i++){
		int_content_entry_array[i]=general_interrupt_handler;
	}
}

#define INPUT_FREQUENCY 1193180
#define OUTPUT_FREQUENCY 100
void set_clock_frequency(){
	outb(0x34,0x43);
	outb((uint8_t)(INPUT_FREQUENCY/OUTPUT_FREQUENCY),0x40);
	outb((uint8_t)((INPUT_FREQUENCY/OUTPUT_FREQUENCY)>>8),0x40);

}

void init_interrupt(){
	init_8259A();
	init_idt();
	init_int_content_entry_array();
	uint64_t t=(((uint64_t)((uint32_t)idt))<<16)+(uint16_t)(IDT_DESC_NUM*8-1);
	//lidt 16 32
	asm volatile ("lidt %0"::"m"(t));

	set_clock_frequency();

}

//---------------------------------------
/*
 * continuation: compile and test if the code is right 
 */
INTR_STATUS get_intr_status(){
    int status;
    asm volatile("pushfl;popfl %0":"=a"(status));
    if(status & Mask_IF==0){
        return OFF;
    }
    else{
        return ON;
    }
}

INTR_STATUS set_intr_status(INTR_STATUS s){
    if(s==ON){
        return enable_intr();
    }
    else{
        return disable_intr();
    }
}

INTR_STATUS enable_intr(){
    INTR_STATUS old_status=get_intr_status();
    asm volatile("sti");
    return old_status;
}

INTR_STATUS disable_intr(){
    INTR_STATUS old_status=get_intr_status();
    asm volatile("cli");
    return old_status;
}