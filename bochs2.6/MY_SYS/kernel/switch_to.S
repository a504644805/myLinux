[bits 32]
; switch_to(cur,next)
; ^ next
; | cur
; | eip <--esp
; | esi
; | edi
; | ebx
; | ebp <--esp
global switch_to
section .text
switch_to:
    push esi
    push edi
    push ebx
    push ebp
    mov eax,[esp+20]
    mov [eax],esp ;save cur's esp
    
    ;both cur and next's stack looks like above

    mov eax,[esp+24]
    mov esp,[eax] ;restore esp to next's esp
    pop ebp
    pop ebx
    pop edi
    pop esi
    ret

