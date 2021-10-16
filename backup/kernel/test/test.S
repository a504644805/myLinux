str: db "12345",0xa
strl equ $-str

;write(fd,*src,sz)
_start:
push strl
push str
push 1

call write

add esp,12

mov eax,1
int 0x80

write:
push ebp
mov ebp,esp
mov eax,4
mov ebx,[ebp+8]
mov ecx,[ebp+12]
mov edx,[ebp+16]
int 0x80
pop ebp
ret
