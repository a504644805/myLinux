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
;mov eax,SELECTOR_CODE
;mov cs,eax ;cs貌似不能直接mov赋值
mov eax,SELECTOR_DATA
mov ds,eax
mov ss,eax
mov eax,SELECTOR_VIDEO
mov gs,eax

;开启分页
call setup_pdt_pt
;lpdt
mov eax,PAGE_DIR_TABLE_POS
mov cr3,eax
;cr0.PG
mov eax,cr0
or eax,0x80000000
mov cr0,eax

;gdtr.addr+=0xc0000000, VIDEO_S.baseAddr+=0xc0000000
add dword [gdt_ptr+2],0xc0000000
lgdt [gdt_ptr]
or dword [VIDEO_DESC+4],0xc0000000


mov ax,SELECTOR_VIDEO
mov gs,ax
mov byte [gs:160],'V'

jmp $

;--------------------------------------
;PDT,PT
setup_pdt_pt:
;逐字节清0
mov ecx, 4096
mov esi, 0
clear_pdt:
mov byte [PAGE_DIR_TABLE_POS+esi],0
inc esi
loop clear_pdt

;更新PDT的第0,768,和1023项
mov eax,PAGE_DIR_TABLE_POS
or eax,PTE_US_U|PTE_RW_RW|PTE_P_1
add eax,0x1000;4kB
mov ebp,PAGE_DIR_TABLE_POS
mov [ebp+0x00],eax 
mov [ebp+768*4],eax
sub eax,0x1000
mov [ebp+1023*4],eax

;pdt的769-1022项
mov esi,769
mov ecx,254
mov eax,PAGE_DIR_TABLE_POS
add eax,0x2000
or eax,PTE_US_U|PTE_RW_RW|PTE_P_1
update_pdte:
mov [PAGE_DIR_TABLE_POS+esi*4],eax
inc esi
add eax,0x1000
loop update_pdte

;更新PDT[768]对应的PT的前256项使其对应总线地址空间的低端1MB
mov eax,0
or eax,PTE_US_U|PTE_RW_RW|PTE_P_1
mov esi,0
mov ecx,256
mov ebx,PAGE_DIR_TABLE_POS+0x1000
update_pte:
mov [ebx+esi*4],eax
add eax,0x1000
inc esi
loop update_pte

ret
