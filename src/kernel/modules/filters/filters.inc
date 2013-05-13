; Text encoding filters
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, calltable

dd Filters_32to8
dd Filters_8to32

%endif
%ifidn KernelSection, DWORDvariables

Filters_range:
        dd 80h, 800h, 10000h, 200000h, 4000000h, 80000000h

%endif
%ifidn KernelSection, variables

Filters_32to8mask:
        db 0c0h, 0e0h, 0f0h, 0f8h, 0fch

Filters_8to32count:
        times 32 db 1
        times 16 db 2
        times 8 db 3
        times 4 db 4
        db 5, 5

Filters_8to32mask:
        times 32 db 1fh
        times 16 db 0fh
        times 8 db 7
        times 4 db 3
        db 1, 1

%endif
%ifidn KernelSection, implementation

Filters_32to8:
; in: ecx = size of the source buffer
;     esi = source buffer
;     edi = destination buffer
;     ebp = size of the destination buffer
; out: ecx = size of processed (source) data
;      ebp = size of resulted (destination) data
;      cf set on error

        or ebp, ebp
        jz .nop
        cmp ecx, 4
        jb .nop
        pusha
        xor edx, edx
        dec ecx
.loop   lodsd
        cmp eax, 7fh
        ja .multi
        stosb
.next   sub ecx, 4
        dec ebp
        ja .loop
        inc ecx
        sub [esp + 8], ebp
        sub [esp + 24], ecx
        popa
.nop    clc
        ret

.multi  cmp eax, 0d800h
        setnb dl
        cmp eax, 0e000h
        sbb dl, 0
        jz .error
        or eax, eax
        js .error
        mov dl, 0
.loop2  inc dl
        cmp eax, [Filters_range + edx * 4]
        jnb .loop2
        sub ebp, edx
        jna .error2
        mov dh, dl
        mov ebx, eax
        add esi, edx
        std
.loop3  and al, 0bfh
        or al, 80h
        stosb
        shr ebx, 6
        mov al, bl
        dec dh
        jnz .loop3
        or al, [Filters_32to8mask + (edx - 1) * 4]
        stosb
        cld
        add esi, edx
        inc esi
        jmp short .next

.error2 add ebp, edx
.error  inc ecx
        sub esi, 4
        sub [esp + 8], ebp
        sub [esp + 24], ecx
        popa
        stc
        ret

Filters_8to32:
; in: ecx = size of the source buffer
;     esi = source buffer
;     edi = destination buffer
;     ebp = size of the destination buffer
; out: ecx = size of processed (source) data
;      ebp = size of resulted (destination) data
;      cf set on error

        jecxz .exit2
        push eax
        push ebx
        push edx
        xor edx, edx
.loop   sub ebp, 4
        jb .exit
        xor eax, eax
        lodsb
        bt eax, 7
        jc .multi
.next   stosd
        loop .loop
.exit   add ebp, 4
        pop edx
        pop ebx
        pop eax
.exit2  clc
        ret

.multi  cmp al, 0c0h
        jb .error
        cmp al, 0feh
        jnb .error
        mov dl, byte [Filters_8to32count + eax - 0c0h]
        sub ecx, edx
        jna .error2
        and al, byte [Filters_8to32mask + eax - 0c0h]
        xor ebx, ebx
        push esi
        push edx
.loop2  or ebx, eax
        shl ebx, 6
        lodsb
        btr eax, 7
        jnc .error3
        bt eax, 6
        jc .error3
        dec dl
        jnz .loop2
        mov edx, [esp]
        or eax, ebx
        cmp eax, [Filters_range + (edx - 1) * 4]
        jb .error3
        cmp eax, 0d800h
        jb .ok
        cmp eax, 0e000h
        jb .error3
.ok     add esp, 8
        jmp short .next

.error3 mov esi, [esp + 4]
        add esp, 8
.error2 add ecx, edx
.error  add ebp, 4
        pop edx
        pop ebx
        pop eax
        stc
        ret

%endif
%ifidn KernelSection, data

%endif
