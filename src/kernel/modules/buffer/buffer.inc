; Default iBuffer implementation
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, implementation

        struc iBuffer_private
        resd iBuffer_size
.Refs   resd 1
        endstruc

Buffer_Create:
; in: ecx = buffer size
; out: ecx = iBuffer
;      cf set on error

        push edi
        add ecx, byte iBuffer_private_size
        call [Malloc.Allocate]
        jc .error
        add ecx, edi
        mov [ecx + iBuffer.QueryInterface], dword Error
        mov [ecx + iBuffer.AddRef], dword Buffer_AddRef
        mov [ecx + iBuffer.Release], dword Buffer_Release
        mov [ecx + iBuffer.Buffer], edi
        mov [ecx + iBuffer.End], ecx
        mov [ecx + iBuffer.Position], edi
        mov [ecx + iBuffer_private.Refs], dword 1
        pop edi
        clc
        ret

.error  pop edi
Error   stc
        ret

Buffer_AddRef:
; in: edx = instance

        inc dword [edx + iBuffer_private.Refs]
        ret

Buffer_Release:
; in: edx = instance

        dec dword [edx + iBuffer_private.Refs]
        jz .die
        ret

.die    push ecx
        push edi
        mov ecx, [edx + iBuffer.End]
        mov edi, [edx + iBuffer.Buffer]
        add ecx, iBuffer_private_size
        sub ecx, edi
        call [Malloc.Free]
        pop edi
        pop ecx
        ret

%endif
