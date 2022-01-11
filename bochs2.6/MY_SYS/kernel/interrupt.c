#include "include/interrupt.h"
#include "ata.h"
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
	//OCW1 mask all except IRQ0,IRQ1,IRQ14,IRQ2
	outb(0xf8,0x21);
	outb(0xbf,0xa1);
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
void syscall_handler();//in int_handler.S
void init_idt(){
	for(int i=0;i<IDT_DESC_NUM;i++){
		idt[i].offset_low_16_bits=((uint32_t)int_entry_table[i]&0x0000ffff);
		idt[i].selector=SELECTOR_K_CODE;
		idt[i].duse=0;
		idt[i].attr=I_G_D_attr;
		idt[i].offset_high_16_bits=(((uint32_t)int_entry_table[i]&0xffff0000)>>16);
	}
	//idt[0x80]=syscall_handler;
	idt[0x80].offset_low_16_bits=((uint32_t)syscall_handler&0x0000ffff);
	idt[0x80].selector=SELECTOR_K_CODE;
	idt[0x80].duse=0;
	idt[0x80].attr=((I_G_D_P_1<<7)+(I_G_D_DPL_3<<5)+(I_G_D_S_0<<4)+I_G_D_TYPE);
	idt[0x80].offset_high_16_bits=(((uint32_t)syscall_handler&0xffff0000)>>16);
}

//中断服务程序具体内容
void general_interrupt_handler(int interruptNo){
	asm volatile("call set_cursor"::"b"(0):"eax","edx");
	for (size_t i = 0; i < 80*4; i++){//清理出4行的空间
		put_char(' ');
	}
	put_str("**********interrupt occured**********\n");//time intr won't have such msg
	put_str("interrupt0x");put_int(interruptNo);put_str(" occured!\n");
	put_str("**********interrupt msg end**********\n");
	while(1);//there shall be no intr(except time_intr) now
}
//中断服务程序具体内容入口地址数组
void* int_content_entry_array[IDT_DESC_NUM];
void init_int_content_entry_array(){
	for(int i=0;i<IDT_DESC_NUM;i++){
		int_content_entry_array[i]=general_interrupt_handler;
	}
	int_content_entry_array[32]=time_intr_handler;
	int_content_entry_array[33]=kbd_intr_handler;
	int_content_entry_array[46]=ata0_intr_handler;
}

/*
#define INPUT_FREQUENCY 1193180
#define OUTPUT_FREQUENCY 100
*/
void set_clock_frequency(){
	outb(0x34,0x43);
	outb((uint8_t)(INPUT_FREQUENCY/OUTPUT_FREQUENCY),0x40);
	outb((uint8_t)((INPUT_FREQUENCY/OUTPUT_FREQUENCY)>>8),0x40);

}

//elapsed time after boot
//take 1.3 year to reach 0xffffffff in 100HZ time intr
uint32_t SYS_ELAPSED_TIME;
void init_interrupt(){
	SYS_ELAPSED_TIME=0;
	init_8259A();
	init_idt();
	init_int_content_entry_array();
	uint64_t t=(((uint64_t)((uint32_t)idt))<<16)+(uint16_t)(IDT_DESC_NUM*8-1);
	//lidt 16 32
	asm volatile ("lidt %0"::"m"(t));

	set_clock_frequency();

}

//---------------------------------------

INTR_STATUS get_intr_status(){
    int status;
    asm volatile("pushfl;popl %0":"=a"(status));
    if((status & Mask_IF)==0){
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