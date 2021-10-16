void put_char(char c);
void put_str(char* p);
void put_int(int i);

void init_interrupt();

int main(void){
	put_str("Hi, I am kernel\n");
	init_interrupt();//1.init_8259A
			 //2.init_idt
			 //3.lidt	
	asm volatile("sti");
	while(1){
	}
	return 0;
}
