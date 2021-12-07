CFLAGS=-fno-builtin -Wall -W -Wstrict-prototypes -Wmissing-prototypes
all:
	gcc -m32 -c main.c -o build/main.o
	gcc -m32 -c interrupt.c -o build/interrupt.o -I include/ -fno-stack-protector
	nasm -f elf int_entry_table.S -o build/int_entry_table.o
	nasm -f elf lib/print.S -o build/print.o -I include/
	ld -melf_i386 -Ttext 0xc0001500 -e main build/main.o build/print.o build/int_entry_table.o build/interrupt.o -o build/a.out
	dd if=build/a.out of=../../a.img bs=512 count=200 seek=9 conv=notrunc

