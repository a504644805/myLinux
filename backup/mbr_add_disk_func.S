%include "boot.inc"

SECTION MBR vstart=0x7c00
;cs is 0 now, use ax inialize regs(sregs don't have circuit to be initialized by immidiate
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov fs,ax
	mov sp,0x7c00
	mov ax,0xB800
	mov gs,ax
	
;clear
	mov ax,0x600
	mov bx,0x700
	mov cx,0
	mov dx,0x184f
	int 0x10

	
	mov byte [gs:0x00],'1'
	mov byte [gs:0x01],0x17
	mov byte [gs:0x02],' '
        mov byte [gs:0x03],0x17
        mov byte [gs:0x04],'M'
        mov byte [gs:0x05],0x17
        mov byte [gs:0x06],'B'
        mov byte [gs:0x07],0x17	
        mov byte [gs:0x08],'R'
        mov byte [gs:0x09],0x17

	mov eax,LOADER_START_SECTOR
	mov cx,0x1
	mov bx,LOADER_BASE_ADDR
	call rd_disk_func

	jmp 0:0x900

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
	mov [bx],ax
	add bx,2 
	loop read_circulation
	
	ret	

	
	times 510-($-$$) db 0
	db 0x55,0xaa
		


