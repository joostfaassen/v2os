; Text stream functions
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn ModuleMode, calltable

dd TextStream_StreamFactory

%elifidn ModuleMode, variables4

                struc iReadText
; iUnknown
.QueryInterface resd 1
.AddRef         resd 1
.Release        resd 1
; iReadStream
.Buffer         resd 1

; iReadText

.RefCount       resd 1
.OldRead        resd 1
.OldPrefetch    resd 1
.Filter         resd 2
.IID            resd 1
.Buffer         resd 1
.Size           resd 1
                endstruc

TextStream_Calltable:
        dd TextStream_Read, TextStream_Write
        dd TextStream_ReadLn, TextStream_Writeln

%elifidn ModuleMode, implementation

TextStream_StreamFactory:
; in: eax:ecx = IStream
;
;     esi:edi = IFilter (esi = 0 for none)
; out: esi:edi = new interface (implements ITextStream)
;      cf set on error

        push edx
        mov edx, ecx
        call [ecx + iUnknown.QueryInterface]
        jnc .ok
        push edi
        mov ecx, iTextStream_size
        call [Malloc.Allocate]
        jc OutOfMemory
        mov ecx, edx
        mov dword [edi + iTextStream.QueryInterface], TextStream_QueryInterface
        mov dword [edi + iTextStream.AddRef], TextStream_AddRef
        mov dword [edi + iTextStream.Release], TextStream_Release
        mov dword [edi + iTextStream.RefCount], 1
        pop dword [edi + iTextStream.Stream]
        mov [edi + iTextStream.Stream + 4], eax
        mov [edi + iTextStream.Filter], ecx
        mov [edi + iTextStream.Filter + 4], esi
        call [edx + iUnknown.AddRef

        stc
        ret


TextStream_QueryInterface:
; in: eax = IID
;     edx = instance
; out: esi:edi = new interface
;      cf set on error

        cmp eax, StreamIID
        je .ok
        cmp eax, [edx + iTextStream.IID]
        jne TextStream_StreamFactory.error
.ok     mov esi, TextStream_Calltable
        mov edi, edx
        ret


TextStream_AddRef:
; in: edx = instance

        inc [edx + iTextStream.RefCount]
        ret


TextStream_Release:
; in: edx = instance

        dec [edx + iTextStream.RefCount]
        je .free
        clc
        ret

.free   mov ecx, iTextStream_size
        mov edi, edx
        jmp [Malloc.Free]


TextStream_Read:
; in: ecx = buffer size
;     edx = instance
;     edi = buffer address
; out: ecx = size of data
;      cf set on buffer overflow

        pusha
.loop   mov ecx, [edx + iTextStream.
        popa
        ret


TextStream_Write:
; in: ecx = buffer size
;     edx = instance
;     esi = buffer address
; out: ecx = size of written data
;      cf set on error


TextStream_ReadLn:
; in: eax = iBuffer (0 to create one)
;     edx = instance
; out: eax = iBuffer (created or relocated)
;      ecx = size of the text
;      esi = text
;      cf set on error


TextStream_WriteLn:
; in: ecx = text size
;     edx = instance
;     esi = text
; out: ecx = size of written data
;      cf set on error

        stc
        ret

%endif
