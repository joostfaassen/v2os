; Default kyboard input and display output
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn ModuleMode, calltable

dd TextStream_StreamFactory

%elifidn ModuleMode, variables4

        struc TextStream
.Stream resd 2
.Filter resd 2
        endstruc

%elifidn ModuleMode, implementation

TextStream_StreamFactory:
; in: eax:ecx = IStream
;     edx = instance
;     esi:edi = IFilter (esi = 0 for none)
; out: esi:edi = new interface (implements ITextStream)
;      cf set on error

        stc
        ret

TextStream_Read:
; in: ecx = buffer size
;     edx = instance
;     edi = buffer address
; out: ecx = size of data
;      cf set on buffer overflow

        stc
        ret

TextStream_Write:
; in: ecx = buffer size
;     edx = instance
;     esi = buffer address
; out: ecx = size of written data
;      cf set on error

        stc
        ret

TextStream_ReadLn:
; in: eax = iBuffer (0 to create one)
;     edx = instance
; out: eax = iBuffer (created or relocated)
;      ecx = size of the text
;      esi = text
;      cf set on error

        stc
        ret

TextStream_WriteLn:
; in: ecx = text size
;     edx = instance
;     esi = text
; out: ecx = size of written data
;      cf set on error

        stc
        ret

%endif
