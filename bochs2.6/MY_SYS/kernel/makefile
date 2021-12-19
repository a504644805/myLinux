#C #include文件查找流程: 使用双引号表示首先在当前的源文件目录中查找，若未找到才到包含目录中去查找
CFLAGS=-fno-builtin -Wall -W -Wstrict-prototypes -Wmissing-prototypes
KERNEL_PATH=/home/xy/myLinux/bochs2.6/MY_SYS/kernel
INCLUDE_FLAG=-I $(KERNEL_PATH)/include -I $(KERNEL_PATH)/lib -I $(KERNEL_PATH)/dev
all:
	gcc -m32 -c main.c -o build/main.o $(INCLUDE_FLAG)
	gcc -m32 -c interrupt.c -o build/interrupt.o $(INCLUDE_FLAG) -fno-stack-protector
	gcc -m32 -c memory.c -o build/memory.o $(INCLUDE_FLAG)
	gcc -m32 -c lib/debug.c -o build/debug.o $(INCLUDE_FLAG)
	gcc -m32 -c lib/string.c -o build/string.o $(INCLUDE_FLAG)
	gcc -m32 -c thread.c -o build/thread.o $(INCLUDE_FLAG) -fno-stack-protector
	gcc -m32 -c lib/list.c -o build/list.o $(INCLUDE_FLAG)
	gcc -m32 -c lock.c -o build/lock.o $(INCLUDE_FLAG)
	gcc -m32 -c dev/keyboard.c -o build/keyboard.o $(INCLUDE_FLAG)

	nasm -f elf int_entry_table.S -o build/int_entry_table.o
	nasm -f elf lib/print.S -o build/print.o -I include/
	nasm -f elf switch_to.S -o build/switch_to.o

	ld -melf_i386 -Ttext 0xc0001500 -e main build/main.o build/print.o build/int_entry_table.o build/interrupt.o build/debug.o build/string.o build/memory.o build/thread.o build/switch_to.o build/list.o build/lock.o build/keyboard.o -o build/a.out
write:
	dd if=build/a.out of=../../a.img bs=512 count=200 seek=9 conv=notrunc

