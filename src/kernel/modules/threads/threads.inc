; Thread manager
; 09/02/2001 trem

; trem:
; Actually, this thread manager is minimal.
; You can have 8 threads, each thread have a 
; stack of 1000h at 50000h - num_thread * 1000h
; This will be changed as soon as the memory manager 
; will be ready

%ifidn KernelSection, init

        ; nothing

%endif
%ifidn KernelSection, calltable

dd Threads_Start
dd Threads_Stop
dd Threads_Resume

%endif
%ifidn KernelSection, DWORDvariables

; first, structure
struc ThreadRegs
.eflags: resd 1
.regs:   resd 8
.eip:    resd 1
.data:   resd 1
.size   endstruc

ThreadData      times 8 * ThreadRegs_size db 0
ThreadCurrent   dw      0

%endif
%ifidn KernelSection, implementation

; in:
; esi: begin of the code
; ecx: size of the stack
; out: cf if error
Threads_Start:
        ; put everything on the (current) stack
        pushad
        pushfd
        push    esi
        push    ecx

        ; found data of the current thread
        mov     eax, ThreadRegs_size
        mul     word [ThreadCurrent]
        mov     ebx, ThreadData 
        add     ebx, eax
        mov     [ebx + ThreadRegs.data], dword 1

        ; save all
        cld
        mov     ecx, 10
        mov     esi, esp
        add     esi, 8
        mov     edi, ebx
        rep     movsd

        ; found data of the next thread
        xor     eax, eax
        mov     ebx, ThreadData
.loop:  cmp     [ebx + ThreadRegs.data], dword 0
        je      .found
        inc     eax
        add     ebx, ThreadRegs_size
        cmp     eax, 8
        jne     .loop

        ; cant found a free entry
        stc
        popfd
        popad
        ret     

.found: ; set register for new thread
        pop     ecx
        pop     esi
        mov     [ebx + ThreadRegs.eip], esi
        mov     [ebx + ThreadRegs.regs + 16], esi
        mov     [ebx + ThreadRegs.regs + 12], dword 50000h
        mov     [ThreadCurrent], ax
        mov     [ebx + ThreadRegs.data], dword 1
        mov     eax, 1000h
        mul     word [ThreadCurrent]
        sub     [ebx + ThreadRegs.regs + 12], eax

        ; next one
        mov     esp, [ebx + ThreadRegs.regs + 12]
        cld
        mov     ecx, 10
        mov     esi, ebx
        sub     esp, 36
        mov     edi, esp
        rep     movsd
        
        popfd
        popad
        ret


; come back to old thread
; in: nothing
; out: nothing
Threads_Stop:
        ; delete current thread
        mov     eax, ThreadRegs_size
        mul     word [ThreadCurrent]
        mov     ebx, ThreadData 
        add     ebx, eax
        mov     [ebx + ThreadRegs.data], dword 0

        ; search prec thread
        xor     eax, eax
        mov     ax, [ThreadCurrent]
.loop:  cmp     [ebx + ThreadRegs.data], dword 1
        je      .found
        sub     ebx, ThreadRegs_size
        sub     eax, 1
        jne     .loop

        ; restore esp and the registers
.found: mov     [ThreadCurrent], ax
        mov     esp, [ebx + ThreadRegs.regs + 12]
        mov     ecx, 10
        mov     esi, ebx
        sub     esp, 36
        mov     edi, esp
        rep     movsd
        
        popfd
        popad
        ret


Threads_Resume:
        ret

; in: esi: addr of the code
; out: cf if error
Threads_Kill:
        ret

; in: nothing
; out: nothing
; should be called next
; go on next thread
Threads_Next:
        ; save current thread
        ; put everything on the (current) stack
        pushad
        pushfd

        ; found data of the current thread
        mov     eax, ThreadRegs_size
        mul     word [ThreadCurrent]
        mov     ebx, ThreadData 
        add     ebx, eax

        ; save all
        cld
        mov     ecx, 10
        mov     esi, esp
        mov     edi, ebx
        rep     movsd

        ; search next thread
        xor     eax, eax
        mov     ax, [ThreadCurrent]

.loop:  add     ebx, ThreadRegs_size
        inc     eax
        cmp     [ebx + ThreadRegs.data], dword 1
        je      .found
        cmp     eax, 8
        jne     .loop

        xor     eax, eax
        mov     ebx, ThreadData

        ; restore esp and the registers
.found: mov     [ThreadCurrent], ax
        mov     esp, [ebx + ThreadRegs.regs + 12]
        mov     ecx, 10
        mov     esi, ebx
        sub     esp, 36
        mov     edi, esp
        rep     movsd
        
        popfd
        popad
                
        ret

%endif
