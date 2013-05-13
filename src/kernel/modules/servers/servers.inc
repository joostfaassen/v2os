; 19/01/2001 trem input interface deleted


%ifidn KernelSection, init

%endif
%ifidn KernelSection, calltable

dd Servers_List
dd Servers_Get
dd Servers_Add
dd Servers_Remove

%endif
%ifidn KernelSection, implementation

Servers_List:
; in: eax = IID
;     ecx = enumeration callback
;           in: ecx = CID
;               edi = context
;           out: edi = context
;                cf set to abort the enumeration
;     edx = instance
;     edi = context
; out: edi = context

        ret

Servers_Get:
;
; TEMPORARY: only interfaces to the System component are returned
;            (see modules/unknown/unknown.inc)
;
; in: eax = IID
;     ecx = CID
;     edx = instance
; out: esi:edi = new interface (implements iUnknown)
;      cf set on error

        cmp ecx, SystemCID
        je short Unknown_QueryInterface
        stc
        ret

Servers_Add:
; in: eax = IID
;     ecx = CID
;     edx = instance
;     esi = factory function
;           in: eax = IID
;               ecx = CID
;               ebx = context
;           out: esi:edi = new interface
;                cf set on error
;     edi = context
; out: cf set on error

        stc
        ret

Servers_Remove:
; in: eax = IID
;     ecx = CID
;     edx = instance
; out: cf set on error

        stc
        ret

%endif
