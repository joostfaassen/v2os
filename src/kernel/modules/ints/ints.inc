%ifidn KernelSection, init_not_working

        xor ebx, ebx            ; no flags
        mov ecx, 30h * 4        ; a dword for every interrupt
        call [Memory.Allocate]
        jc OutOfMemory
        mov [SysData.IDTHooks], eax
        mov edi, eax
        mov ecx, 30h * IDTHook.size
        call [Memory.Allocate]
        mov cl, 30h
        mov esi, eax
Hooks   stosd
        add eax, 16
        loop Hooks
        mov edi, esi
        xor eax, eax
        mov cl, 30h
        mov esi, [SysData.IDTBase]
Links   stosd                   ; Next
        stosd                   ; Previous
        movsd                   ; Value
        movsd
        loop Links

%endif
%ifidn KernelSection, calltable

dd Interrupts_Hook 
dd Interrupts_Unhook

%endif
%ifidn KernelSection, implementation

Interrupts_Hook:
Interrupts_Unhook:

        stc
        ret

%endif
