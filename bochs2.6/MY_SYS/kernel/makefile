#C #include文件查找流程: 使用双引号表示首先在当前的源文件目录中查找，若未找到才到包含目录中去查找
CFLAGS=-fno-builtin -Wall -W -Wstrict-prototypes -Wmissing-prototypes
KERNEL_PATH=/home/xy/myLinux/bochs2.6/MY_SYS/kernel
INCLUDE_FLAG=-I $(KERNEL_PATH)/include -I $(KERNEL_PATH)/lib -I $(KERNEL_PATH)/dev -I $(KERNEL_PATH)/fs
all:
	gcc -m32 -c main.c -o build/main.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c interrupt.c -o build/interrupt.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c memory.c -o build/memory.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c lib/debug.c -o build/debug.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c lib/string.c -o build/string.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c thread.c -o build/thread.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c lib/list.c -o build/list.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c lock.c -o build/lock.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c dev/keyboard.c -o build/keyboard.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c syscall.c -o build/syscall.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c lib/stdio.c -o build/stdio.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c dev/ata.c -o build/ata.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c lib/bitmap.c -o build/bitmap.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c fs/fs.c -o build/fs.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c fork_exec_wait_exit.c -o build/fork_exec_wait_exit.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c shell.c -o build/shell.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c lib/circular_queue.c -o build/circular_queue.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector
	gcc -m32 -c lib/pipe.c -o build/pipe.o $(INCLUDE_FLAG) -fno-builtin -fno-stack-protector

	nasm -f elf int_handler.S -o build/int_handler.o
	nasm -f elf lib/print.S -o build/print.o -I include/
	nasm -f elf switch_to.S -o build/switch_to.o

	ld -melf_i386 -Ttext 0xc0001500 -e main build/main.o build/print.o build/int_handler.o build/interrupt.o build/debug.o build/string.o build/memory.o build/thread.o build/switch_to.o build/list.o build/lock.o build/keyboard.o build/syscall.o build/stdio.o build/ata.o build/bitmap.o build/fs.o build/fork_exec_wait_exit.o build/shell.o build/circular_queue.o build/pipe.o -o build/a.out

	objdump -s -j .text -d build/a.out  > /home/xy/t
write:
	dd if=build/a.out of=../../a.img bs=512 count=200 seek=9 conv=notrunc

user:
	gcc -m32 -c user_prog/main.c -o user_prog/main.o -I $(KERNEL_PATH)/user_prog -fno-builtin -fno-stack-protector
	gcc -m32 -c user_prog/minicrt.c -o user_prog/minicrt.o -I $(KERNEL_PATH)/user_prog -fno-builtin -fno-stack-protector
	ld -melf_i386 -e _start user_prog/main.o user_prog/minicrt.o -o user_prog/user_prog
	dd if=user_prog/user_prog of=../../a.img bs=512 count=100 seek=300 conv=notrunc
	
