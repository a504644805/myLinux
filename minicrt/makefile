#\
main:main.o entry.o\
	ld -g -static -e entry -o main main.o entry.o\
	mv main.o objfile\
	mv entry.o objfile\
main.o:main.c\
	gcc -g -fno-builtin -nostdlib -fno-stack-protector -c main.c\
entry.o:entry.c\
	gcc -g -fno-builtin -nostdlib -fno-stack-protector -c entry.c
all:
	gcc -g -fno-builtin -nostdlib -fno-stack-protector -c main.c entry.c heapman.c ioman.c str.c
	ld -g -static -e entry -o main main.o entry.o heapman.o ioman.o str.o
	mv main.o objfile
	mv entry.o objfile
	mv heapman.o objfile
	mv ioman.o objfile
	mv str.o objfile
