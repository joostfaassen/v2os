; V2_OS hooks for a single processor
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, calltable

dd Hooks_HookFirst
dd Hooks_HookLast
dd Hooks_Unhook

%endif
%ifidn KernelSection, implementation

Hooks_HookFirst:
; in: eax = iCallback
;     edx = instance

        push ebx
        cli
        mov ebx, [edx + iCallback.Next]
        mov [edx + iCallback.Next], eax
        mov [ebx + iCallback.Prev], eax
        mov [eax + iCallback.Next], ebx
        mov [eax + iCallback.Prev], edx
        sti
        pop ebx
        ret

Hooks_HookLast:
; in: eax = iCallback
;     edx = instance

        push ebx
        cli
        mov ebx, [edx + iCallback.Prev]
        mov [edx + iCallback.Prev], eax
        mov [ebx + iCallback.Next], eax
        mov [eax + iCallback.Prev], ebx
        mov [eax + iCallback.Next], edx
        sti
        pop ebx
        ret

Hooks_Unhook:
; in: eax = iCallback
;     edx = instance

        push eax
        push ebx
        cli
        mov ebx, [eax + iCallback.Next]
        mov eax, [eax + iCallback.Prev]
        mov [eax + iCallback.Next], ebx
        mov [ebx + iCallback.Prev], eax
        sti
        pop ebx
        pop eax
        ret

%endif
