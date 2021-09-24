%include "boot.inc"

SECTION OBR vstart=LOADER_BASE_ADDR

;GDT
GDT_BASE: dd 0x00000000
	  dd 0x00000000

CODE_DESC: dd 0x0000ffff
	   dd DESC_CODE_HIGH4

DATA_DESC: dd 0x0000ffff
           dd DESC_DATA_HIGH4

VIDEO_DESC: dd 0x80000007;32KB/4KB=8,8-1
	    dd DESC_VIDEO_HIGH4

GDT_END:

times 60 dq 0

;256B: gdt_ptr,mem_size,ARDS,nr
mem_size dd 0

ards_buf times 244 db 0
ards_nr dw 0

gdt_ptr dw GDT_END-GDT_BASE-1
	dd GDT_BASE


loader_start:

;int 0x15
;move ardses to memory
mov ax,0x00
mov es,ax
mov eax,0xe820
xor ebx,ebx
mov di,ards_buf
mov ecx,20
mov edx,0x534d4150

getArds:
int 0x15
jc int_0x15_fail
inc word [ards_nr]

mov eax,0xe820
add di,cx
mov cx,20

cmp ebx,0
jne getArds

;get MaxArds as mem_size
mov cx,[ards_nr]
mov bx,ards_buf
mov ax,0
mov ds,ax
mov edx,0

getMaxArds:
mov eax,[bx+8]
add bx,20
cmp edx,eax
jge loop_getMaxArds
mov edx,eax

loop_getMaxArds:
loop getMaxArds

mov [mem_size],edx

jmp open_protection_mode

int_0x15_fail:
jmp $

open_protection_mode:
;lgdt
mov ax,0
mov gs,ax
lgdt [gs:gdt_ptr]

;A20
in al,0x92
or al,0x02
out 0x92,al

;cr0.PE
mov eax,cr0 ;eax
or eax,0x01
mov cr0,eax

;refresh pipeline
jmp SELECTOR_CODE:p_mode_start

[bits 32]
p_mode_start:
mov ax,SELECTOR_VIDEO
mov gs,ax
mov byte [gs:160],'P'

jmp $
