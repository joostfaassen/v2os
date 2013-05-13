; Default stream implementation
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn ModuleMode, calltable

%elifidn ModuleMode, implementation

                struc iStream, iUnknown
.Buffer         resd 1
.Size           resd 1
                endstruc

Stream_Read:
; in: ecx = size
;     edx = instance
;     edi = buffer address
; out: ecx = data size
;      esi = data address
;      cf set on error

        


RStream_ReadBuffer:
; in: ecx = minimal buffer size
;     edx = instance
; out: ecx = actual buffer size
;      esi = buffer address
;      cf set on EOF

        


Stream_Write:
; in: ecx = buffer size
;     esi = buffer address
; out: ecx = size of written data
;      cf set on buffer overflow

        push eax
        push edx
        push esi
        push edi
        push ecx
        mov edi, [StreamVars.Cursor]
        mov edx, [StreamVars.NextLine]
        jmp short .start

.loop   cmp eax, 20h
        jb short .miss
        mov ah, 7
.out    stosw
        cmp edi, edx
        jnb short .LF

.start  sub ecx, byte 4
        jb short .end
        lodsd
        cmp eax, byte 7eh
        ja short .miss
        cmp eax, 0ah
        je short .LF
        cmp eax, 0dh
        jne short .loop

.CR     cmp ecx, byte 4
        jb short .LF
        cmp dword [esi], 0ah
        sete al
        shl eax, 2
        add esi, eax
        sub ecx, eax

.LF     mov edi, edx
        cmp edx, 0b8000h + 160 * 25
        jae short .scroll
        add edx, 160
        jmp short .start

.miss   mov eax, '?' + 800h
        jmp short .out

.scroll mov eax, esi
        mov edx, ecx
        mov esi, 0b8000h + 160
        mov edi, 0b8000h
        mov ecx, 160 * 24
        rep movsd
        mov esi, eax
        mov eax, 7000700h
        mov ecx, 40
        rep stosd
        mov edi, 0b8000h + 160 * 24
        mov ecx, edx
        jmp short .start

.end    add ecx, byte 4
        pop eax
        sub eax, ecx
        mov ecx, eax
        mov [StreamVars.Cursor], edi
        mov [StreamVars.NextLine], edx
        mov dx, 3d4h
        mov al, 0fh
        out dx, al
        inc dl
        mov eax, edi
        shr eax, 1
        out dx, al
        dec dl
        mov al, 0eh
        out dx, al
        shr eax, 8
        and al, 3fh
        out dx, al
        pop edi
        pop esi
        pop edx
        pop eax
        clc
        ret

Keyboard_Handler:
        push eax
        in al, 60h
        mov [0b8000h], al
        mov al, 20h
        out 20h, al
        pop eax
        iretd

%endif
