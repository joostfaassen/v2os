; Disk access through the BIOS
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, DWORDvariables

Storage_Default:
.Read           dd Storage_Read
.Unread         dd Storage_Unread
.Write          dd Storage_Write
.Seek           dd Storage_Seek
.Position       dd 0, 0

%endif
%ifidn KernelSection, implementation

Storage_Read:
; in: ecx = buffer size
;     edx = instance
;     edi = buffer address
; out: ecx = size of data
;      cf set on buffer overflow

Storage_Unread:
; in: ecx = size of data to unread
;     edx = instance
;     edi = 

Storage_Write:
; in: ecx = buffer size
;     edx = instance
;     esi = buffer address
; out: ecx = size of written data
;      cf set on error

Storage_Seek:
; in: edx:eax = position (bytes)
;         edi = instance
; out: cf set on error

%endif
