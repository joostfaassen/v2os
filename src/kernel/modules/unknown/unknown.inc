; iUnknown interface of the V2_OS kernel
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, calltable

dd Unknown_QueryInterface
dd Dummy
dd Dummy

%endif
%ifidn KernelSection, DWORDvariables

Unknown_Interfaces:
        dd UnknownIID, 0
        dd ServersIID, SysServers
        dd MemoryIID, SysMemory
        dd MallocIID, SysMalloc
        dd Malloc2IID, SysMalloc
        dd MallocFactoryIID, SysMallocFactory
        dd DirectoryIID, SysDirectory
        dd CommandIID, SysCommand
        dd SegmentsIID, SysSegments
        dd InterruptsIID, SysInterrupts
        dd ThreadsIID, SysThreads
        dd RealModeIID, SysRealMode
Unknown_Interfaces_end:

%endif
%ifidn KernelSection, implementation

Unknown_QueryInterface:
; in: eax = IID
;     edx = instance
; out: esi:edi = new interface
;      cf set on error

        cmp eax, HookIID
        je .hook
        mov esi, Unknown_Interfaces
.loop   cmp eax, [esi]
        je .found
        add esi, 8
        cmp esi, Unknown_Interfaces_end
        jne .loop
        stc
        ret

.found  mov esi, [esi + 4]
        mov edi, SysUnknown
        ret

.hook   mov esi, SysHook
        mov edi, SysCommandHook
        ret

%endif
