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

;权力交接: 载入内核并建立内核映像，jmp
;1.调用rd_disk函数
or eax,eax
or ecx,ecx
or ebx,ebx
mov eax,KERNEL_BIN_SECTOR_ADDR
mov cx,KERNEL_BIN_SECTOR_COUNT
mov ebx,KERNEL_BIN_BASE_ADDR
call rd_disk_func
;2.e_phnum,e_phsize,e_phoffset  44 42 28
;p_type,***p_vaddr,p_filesz,p_offset***  0 8 16 4
xor ecx,ecx
xor edx,edx
mov cx,[KERNEL_BIN_BASE_ADDR+44]
mov ebx,[KERNEL_BIN_BASE_ADDR+28]
add ebx,KERNEL_BIN_BASE_ADDR ;ebx:当前段的ph所在位置
mov esp,0xc0000900
mov dx,[KERNEL_BIN_BASE_ADDR+42]

each_segment:
mov eax,[ebx+0];
cmp eax,0
je loop_end 

push dword [ebx+16]
mov eax,[ebx+4]
add eax,KERNEL_BIN_BASE_ADDR
push eax
push dword [ebx+8]
call mem_cpy
add esp,12

loop_end:
add ebx,edx

loop each_segment
	
;3.jmp
mov esp,0xc009f000
jmp 0xc0001500

;---------------------------------------
mem_cpy:;dst,src,sz
mov ax,SELECTOR_DATA
mov es,ax
cld
push ebx;ebx as ebp
mov ebx,esp
push ecx
mov ecx,[ebx+16]
mov esi,[ebx+12]
mov edi,[ebx+8]
rep movsb

pop ecx
pop ebx
ret

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


;---------------------------------------
rd_disk_func:
        ;pass parameter
        ;eax LOADER_START_SECTOR
        ;cx
        ;bx
        mov esi,eax
        mov di,cx

        ;sector_count
        mov al,cl
        mov dx,0x1f2
        out dx,al

        ;LBA
        mov eax,esi
        mov cl,8

        mov dx,0x1f3
        out dx,al

        shr eax,cl
        mov dx,0x1f4
        out dx,al

        shr eax,cl
        mov dx,0x1f5
        out dx,al

        ;device
        shr eax,cl
        and al,0x0f
        or al,0xe0

        mov dx,0x1f6
        out dx,al

        ;commond
        mov al,0x20
        mov dx,0x1f7
        out dx,al

        ;status
        not_ready:
        nop
        mov dx,0x1f7
        in al,dx

        and al,0x08
        cmp al,0x08
        jnz not_ready

        ;read
        ;(cx*512B)/2B=cx*256(loop time)

        mov ax,256
        mov dx,di
        mul dx
        mov cx,ax

        mov dx,0x1f0
        read_circulation:
        in ax,dx
        mov [ebx],ax
        add ebx,2

        loop read_circulation

        ret

