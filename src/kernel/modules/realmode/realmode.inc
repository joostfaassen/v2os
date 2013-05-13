; V2_OS RealMode portal
; Copyright (C) 2000, 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, calltable

dd RealMode_Call
dd RealMode_Interrupt

%endif
%ifidn KernelSection, implementation

RealMode_Interrupt:
; in: al = interrupt number
        push eax
        push ebx
        push ecx
        movzx eax, al
        mov ecx, [SysData.RM_IDTBase]
        mov ecx, [ecx + eax * 4]
        mov eax, [SysData.RMStackSize]
        mov ebx, [SysData.RMStack]
        mov [ebx + eax - (RMRegs_size + 2) + RMRegs.ip], ecx
        pushfd
        pop ecx
        and ch, -2      ; cli
        mov [ebx + eax - (RMRegs_size + 2) + RMRegs.params], cx
        mov [ebx + eax - (RMRegs_size + 2) + RMRegs.eflags], ecx
        mov ecx, 2
        call [SysRealMode.Call]
        pop ecx
        pop ebx
        pop eax
        ret

RealMode_Call:
; in: eax = size of the stack
;     ebx = linear address of the stack (16 byte aligned)
;     ecx = size of stack parameters
        push gs
        push fs
        push es
        push ds
        pushad
        pushfd
        mov [ebx + eax - RMRegs_size + RMRegs._esp], esp
        mov [ebx + eax - RMRegs_size + RMRegs._ss], ss
        mov ebp, eax
        sub ebp, ecx
        sub ebp, RMRegs_size - RMRegs.cs
        mov [ebx + eax - RMRegs_size + RMRegs._sp], bp
        sub ebp, RMRegs.cs
        shr ebx, 4
        cli
        jmp 20h:.thunk

        bits 16
.thunk  mov ax, 28h
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        lidt [SysData.RM_IDT]
        mov eax, cr0
        and al, -2
        mov cr0, eax
        jmp 0:.rm

.rm     mov ss, bx
        mov sp, bp
        mov dword [bp + RMRegs._ret], .retf
        popfd
        popad
        pop ds
        pop es
        pop fs
        pop gs
        retf

.retf   push bp                 ; save output bp value on the old stack
        mov bp, sp              ; save the old stack pointer in bp
        mov sp, [bp + RMRegs._sp - (RMRegs._esp - 2)]
                                ; the new stack pointer for output registers
        push bp                 ; save the old stack pointer on the new stack
        mov bp, [bp]            ; restore output value bp from the old stack
        push gs                 ; save output register values on the stack
        push fs
        push es
        push ds
        pushad
        pushfd
        cli
        mov bp, sp                              ; restore the old stack
        movzx ebp, word [bp + RMRegs.ip]        ; pointer from the new stack
        xor eax, eax
        mov ax, ss
        shl eax, 4
        add ebp, eax    ; calculate the linear address of the stack pointer
        mov eax, cr0
        or al, 1
        mov cr0, eax
        jmp 8:.pm

        bits 32
.pm     lss esp, [cs:ebp + 2]
        popfd
        popad
        pop ds
        pop es
        pop fs
        pop gs
        lidt [SysData.IDT]
        ret

%endif
