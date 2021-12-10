CFLAGS=-fno-builtin -Wall -W -Wstrict-prototypes -Wmissing-prototypes
all:
	gcc -m32 -c main.c -o build/main.o -I include/ -I lib/
	gcc -m32 -c interrupt.c -o build/interrupt.o -I include/ -fno-stack-protector
	gcc -m32 -c memory.c -o build/memory.o -I include/ -I lib/
	gcc -m32 -c lib/debug.c -o build/debug.o -I include/
	gcc -m32 -c lib/string.c -o build/string.o -I include/

	nasm -f elf int_entry_table.S -o build/int_entry_table.o
	nasm -f elf lib/print.S -o build/print.o -I include/
	
	ld -melf_i386 -Ttext 0xc0001500 -e main build/main.o build/print.o build/int_entry_table.o build/interrupt.o build/debug.o build/string.o build/memory.o -o build/a.out
write:
	dd if=build/a.out of=../../a.img bs=512 count=200 seek=9 conv=notrunc

