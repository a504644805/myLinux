[bits 32]
%include "boot.inc"

str_buffer_in_put_int dq 0
db 0

global put_int
put_int:

pushad
mov ebp,esp
mov eax,[ebp+36]
mov ebx,str_buffer_in_put_int+7
mov ecx,8

;从低位到高位将各16进制转字符
loop_in_put_int:
mov edx,eax
and edx,0xf
cmp edx,9
jg cur_hex_is_a2f

;0to9
add edx,'0'
jmp cur_loop_end_in_put_int

cur_hex_is_a2f:
sub edx,10
add edx,'a'
jmp cur_loop_end_in_put_int

cur_loop_end_in_put_int:
mov byte [ebx],dl
dec ebx
shr eax,4

loop loop_in_put_int


;找到第一个非'0'字符, 并从该处开始put_str, 使形如0x00012345显示为12345
mov ebx,str_buffer_in_put_int
mov ecx,7;只比较到倒数第二个，以保证输入是0x00还是能输出一个0的
mov edx,0
loop2_in_put_int:
mov al,[ebx]
cmp al,'0'
jne loop2_end_in_put_int

inc ebx
inc edx
loop loop2_in_put_int
loop2_end_in_put_int:

;call push_str
add edx,str_buffer_in_put_int
push edx
call put_str
add esp,4

popad
ret

;-----------------------------------------
global put_str
put_str:
pushad
mov ebp,esp

mov ebx,[ebp+36];字符串在内存中的起始地址
loop_in_put_str:
xor eax,eax
mov al,[ebx]
cmp al,0
je loop_end

push eax
call put_char
add esp,4

inc ebx
jmp loop_in_put_str

loop_end:
popad
ret

;-----------------------------------------
global put_char
put_char:
mov ax,SELECTOR_VIDEO
mov gs,ax
pushad

mov ecx,[esp+36];4*8+4(eip) 近跳转
cmp ecx,0xa
je lf
cmp ecx,0xd
je lf;cr
cmp ecx,0x08
je bs

jmp show_char


lf:
call read_cursor
xor dx,dx
mov ax,bx
mov si,80
div si
sub bx,dx;cr
add bx,80;lf
call set_cursor

cmp bx,2000
jl not_call_screen_roll_in_lf
call screen_roll

not_call_screen_roll_in_lf:
jmp end


bs:
call read_cursor
sub ebx,1
shl ebx,1
mov word [gs:ebx],0x0020
shr ebx,1
call set_cursor

jmp end


show_char:
call read_cursor
cmp bx,2000
jl not_call_screen_roll_in_showChar
call screen_roll
not_call_screen_roll_in_showChar:

call read_cursor
mov ecx,[esp+36];4*8+4(eip)
shl bx,1
mov byte [gs:bx],cl
inc bx
mov byte [gs:bx],0x07
shr bx,1
inc bx
call set_cursor

jmp end


end:
popad
ret

;---------------------------------------------------------
;cls_screen
global cls_screen
cls_screen:
mov ax,SELECTOR_VIDEO
mov gs,ax
pushad

mov ecx,80*25
mov ebx,0
loop_in_cls_screen:
mov word [gs:ebx],0x0020
add ebx,2
loop loop_in_cls_screen

mov ebx,0
call set_cursor
popad
;---------------------------------------------------------

read_cursor:
;may change ax,dx
;return ebx
mov dx,0x3d4
mov al,0x0e
out dx,al

mov dx,0x3d5
in al,dx
mov ah,al

mov dx,0x3d4
mov al,0x0f
out dx,al

mov dx,0x3d5
in al,dx

xor ebx,ebx
mov bx,ax

ret

global set_cursor
set_cursor:
;input ebx as cursor pos
;may change ax,dx
mov dx,0x3d4
mov al,0x0e
out dx,al

mov dx,0x3d5
mov al,bh
out dx,al

mov dx,0x3d4
mov al,0x0f
out dx,al

mov dx,0x3d5
mov al,bl
out dx,al

ret

screen_roll:
;may change esi,edi,ecx...
;1,24 co 0,23
mov esi,0xc00b80a0
mov edi,0xc00b8000
mov ecx,960
rep movsd

;fill 24 row with blanks
mov ecx,80
mov ebx,3840
loop_in_screen_roll:
mov word [gs:ebx],0x0020
add ebx,2
loop loop_in_screen_roll

;set cursor at begin of 24 
mov ebx,1920
call set_cursor

ret;


