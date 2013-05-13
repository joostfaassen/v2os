; Default display driver
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, init

; Set 80x25 color text mode
        mov eax, [SysData.RMStruc]
        mov [eax + RMRegs.ax], word 3
        mov al, 10h
        call [SysRealMode.Interrupt]

%endif
%ifidn KernelSection, DWORDvariables

DisplayVars:
.Cursor         dd 0b8000h
.NextLine       dd 0b8000h + 160

DisplayCalltable dd Display_Write, Display_GetBuffer, Display_Flush

%endif
%ifidn KernelSection, implementation

Display_Write:
; in: ecx = data size
;     esi = data address
; out: ecx = size of not written data
;      cf set on error

        push eax
        push ebx
        push edx
        push esi
        push edi
        push ebp
        mov edi, [DisplayVars.Cursor]
        mov ebp, [DisplayVars.NextLine]
        jmp short .start

.loop   cmp eax, byte 20h
        jb short .miss
        mov ah, 7
.out    stosw
        cmp edi, ebp
        jnb short .LF

.start  sub ecx, byte 4
        jb short .end
        lodsd
        cmp eax, 2028h          ; LS
        sete bl
        cmp eax, 2029h          ; PS
        sete bh
        cmp eax, byte 7eh
        ja short .miss
        cmp eax, byte 0ah       ; LF
        sete dl
        cmp eax, byte 0ch       ; FF
        sete dh
        or bx, dx
        jnz short .LF
        cmp eax, byte 0dh       ; CR
        jne short .loop

.CR     cmp ecx, byte 4
        jb short .LF
        cmp dword [esi], byte 0ah
        sete al
        lea esi, [esi + eax * 4]
        sub ecx, eax

.LF     mov edi, ebp
        cmp ebp, 0b8000h + 160 * 25
        jae short .scroll
        add ebp, 160
        jmp short .start

.miss   mov eax, '?' + 800h
        jmp short .out

.end    add ecx, byte 4
        mov [DisplayVars.Cursor], edi
        mov [DisplayVars.NextLine], ebp
        mov dx, 3d4h
        mov al, 0fh
        out dx, al
        inc edx
        mov eax, edi
        shr eax, 1
        out dx, al
        dec edx
        mov al, 0eh
        out dx, al
        inc edx
        shr eax, 8
        and al, 3fh
        out dx, al
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ebx
        pop eax
        clc
        ret

.scroll mov eax, esi
        mov edx, ecx
        mov esi, 0b8000h + 160
        mov edi, 0b8000h
        mov ecx, 160 * 24
        rep movsd
        mov esi, eax
        mov eax, 07200720h
        mov ecx, 40
        rep stosd
        mov edi, 0b8000h + 160 * 24
        mov ecx, edx
        jmp near .start
        
Display_GetBuffer:
; in: ecx = min. buffer size
;     edx = instance
;     esi = suggested buffer
; out: ecx = buffer size
;      edi = buffer address
;      cf set on error

Display_Flush:
; in: ecx = data size to flush
;     edx = instance
; out: cf set on error

	stc
	ret

%endif
