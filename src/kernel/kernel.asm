; V2_OS kernel version 0.70
; Copyright (C) 1999  V2_Lab
; Copyright (C) 2000, 2001  V2_OS Kernel Coders Team
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;
; More information is available at http://v2os.v2.nl/

%define DEBUG

%include "sdk.inc"

%define KeyboardLayout de	

%define LoadAddress 10000h ; must be the same as in the boot sector

%define InitData(label) label - Load + LoadAddress 	; Computes the absolute
			; address of a label in the loaded image.

%define RuntimeCode(label) label - LoadAddress + 4d8h	; Computes the relative
			; address for calls from init code to runtime code.
			; Use in combination with labels only (not with calltables).

        bits 32
        org 4d8h

Load:   cli
        cld
        jmp near Init

        align 4
Header:
.magic          dd 1badb002h
.flags          dd 00010000h
.checksum       dd 0 - 1badb002h - 00010000h
.header_addr    dd InitData(.magic)
.load_addr      dd InitData(Load)
.load_end_addr  dd InitData(EOF)
.bss_end_addr   dd InitData(EOF)
.entry_addr     dd InitData(Load)

Runtime:

; Static calltable
%define KernelSection calltable
%include "modules.inc"

; SysData: most values are defined at the end of this file
        dw mmIDT_size - 1               ; IDT
        dd mmIDT                        ; IDTBase
        dw mmGDT_size - 1               ; GDT
        dd mmGDT                        ; GDTBase
        dd 0            ; not defined   ; IDTHooks
        dw 30h * 4      ; not defined   ; IDTHooksSize
        dw 3ffh                         ; RM_IDT
        dd 0                            ; RM_IDTBase
        dd mmRMData                     ; RMDataBlock
        dd mmRMData_size                ; RMDataSize
        dd mmRMStack                    ; RMStack
        dd mmRMStack_size               ; RMStackSize
        dd mmRMData - RMRegs_size - 2   ; RMStruc
        dd $, $, Dummy, 1               ; KbdNotFound
	dd $, $, Dummy, 1		; KbdNotify
        dd ScancodeMap                  ; Scancodes
        dd 128                          ; ScancodeCount

%assign RealSysDataSize $ - Runtime
%assign ExpectedSysDataSize EndOfSysData - 500h

%if RealSysDataSize <> ExpectedSysDataSize
  %error SysData is RealSysDataSize bytes instead of ExpectedSysDataSize bytes
%endif

%define KernelSection DWORDvariables
%include "modules.inc"

Temp_Callback:
	istruc iCallback
	at iCallback.Callback, dd Temp_Echo
	iend

Temp_Buffer dd 0, 0

%define KernelSection variables
%include "modules.inc"

Temp_Echo:
	push ecx
	push edi
	mov ecx, 8
	mov edi, Temp_Buffer
	call [SysKbdStream.Read]
	jecxz .exit
	xchg esi, edi
%ifdef DEBUG
mov [esi + 5], byte 0
%endif
	add esi, byte 4
	mov ecx, 4
	call Display_Write
	xchg esi, edi
.exit	pop edi
	pop ecx
	ret

Start:
; TODO: execute "boot.script"
; TODO: execute keyboard input

        mov esi, Message
        mov ecx, Message_end - Message
        call Display_Write
	mov eax, Temp_Callback
	mov edx, SysData.KbdNotify
	call [SysHook.HookFirst]
        sti
	mov eax, [SysData.Scancodes]
.loop   hlt
	bt dword [eax], 1
	jnc .loop
        times 8 push dword 1ee7c0deh
        popa
        int3

IntHandler:
        cld
        pusha
        add dword [esp + 12], byte 4    ; saved esp
        mov ch, 8
        mov esi, CrashData
.loop   mov edi, [esi]
        add esi, 4
        pop ebx
        call Hex
        dec ch
        jnz .loop
        pop eax                         ; interrupt number
        mov edx, eax
        mov ebx, eax
        and al, 0fh
        mov cl, [HexDigits + eax]
        mov [CrashMsg.int + 1], cl
        shr ebx, 4
        mov cl, [HexDigits + ebx]
        mov [CrashMsg.int], cl
        cmp edx, byte 32
        jnb .nocode
        mov ebx, 27d0h
        bt ebx, edx
        jnc .nocode
        mov edi, CrashMsg.error
        pop ebx
        call Hex
.nocode mov edi, CrashMsg.eip
        call Hex
        mov dh, 6
        mov ebx, CrashMsg
        jmp short BSoD

Hex     mov cl, 8
.loop   mov eax, ebx
        shr eax, 28
        mov al, [HexDigits + eax]
        stosb
        shl ebx, 4
        dec cl
        jnz .loop
        ret     

OutOfMemory:
        cli
        mov dh, 3
        mov ebx, OutOfMemMsg

BSoD    mov edi, 0b8000h
        mov ecx, 1000
        mov eax, 1f201f20h
        rep stosd
        mov esi, BSoDMsg
        mov dl, 3
.loop   mov di, [esi]
        add esi, 2
        jmp short .start
.loop2  stosw
.start  lodsb
        or al, al
        jnz .loop2
        dec dl
        jnz .loop
        mov dl, dh
        mov esi, ebx    
        jmp short .loop

Dummy   clc
        ret

%define KernelSection implementation
%include "modules.inc"

        align 4
Message dd 'V', '2', '_', 'O', 'S', ' ', 'd', 'i', 'd', ' ', 'n', 'o', 't', ' ', 'c', 'r'
        dd 'a', 's', 'h', ' ', 'y', 'e', 't', '.', 0dh, 'P', 'r', 'e', 's', 's', ' ', 'E'
        dd 'S', 'C', ' ', 't', 'o', ' ', 'f', 'i', 'x', ' ', 't', 'h', 'a', 't', '.', 13
Message_end:

HexDigits:
        db '0123456789ABCDEF'

BSoDMsg dw 8000h + 2*160 + 80-14
        db 'V2_OS crashed.', 0
        dw 8000h + 3*160 + 80-30
        db 'I am the Blue Screen of Death.', 0
        dw 8000h + 4*160 + 80-26
        db 'No one hears your screams.', 0

OutOfMemMsg:
        dw 8000h + 8*160 + 80-14
        db 'Out of memory.', 0
        dw 8000h + 9*160 + 80-30
        db 'We wish to hold the whole sky,', 0
        dw 8000h + 10*160 + 80-18
        db 'But we never will.', 0

CrashMsg:
        dw 8000h + 8*160 + 80-32
        db 'int: '
.int    db '00     error code: '
.error  db 'N/A     ', 0
        dw 8000h + 10*160 + 80-32
        db 'eax: '
.eax    db '00000000      esi: '
.esi    db '00000000', 0
        dw 8000h + 11*160 + 80-32
        db 'ebx: '
.ebx    db '00000000      edi: '
.edi    db '00000000', 0
        dw 8000h + 12*160 + 80-32
        db 'ecx: '
.ecx    db '00000000      ebp: '
.ebp    db '00000000', 0
        dw 8000h + 13*160 + 80-32
        db 'edx: '
.edx    db '00000000      esp: '
.esp    db '00000000', 0
        dw 8000h + 15*160 + 80-32
        db 'eip: '
.eip    db '00000000', 0

CrashData:
        dd CrashMsg.edi, CrashMsg.esi, CrashMsg.ebp, CrashMsg.esp
        dd CrashMsg.ebx, CrashMsg.edx, CrashMsg.ecx, CrashMsg.eax

%define KernelSection data
%include "modules.inc"

        align 8
Init:

; prepare the real mode IDT for the new IRQ mapping
        mov esi, 20h
        mov edx, esi
        mov edi, 80h
        mov ecx, 8
        rep movsd
        mov esi, 1c0h
        mov cl, 8
        rep movsd

; remap the IRQs to 20h-2fh by reprogramming the PIC
        mov esi, InitData(Init_PICsequence)
        mov dx, 20h
        call Init_SendSequence
        mov dl, 0a0h
        call Init_SendSequence

; move the runtime part to its final position in memory
        mov esi, InitData(Runtime)
        mov edi, mmRuntime
        mov cx, mmRuntime_size / 4
        rep movsd

; create exception handlers
        mov ebx, 0e9006ah                       ; push imm8; jmp rel32
        mov eax, IntHandler - mmIntHandlers - 7 ; = rel32
        mov cl, 30h
Init_HandlerLoop:
        mov [edi], ebx
        mov [edi + 3], eax
        add edi, byte 7
        sub eax, byte 7
        inc bh                  ; imm8 of the push
        loop Init_HandlerLoop

; create the IDT
        mov eax, mmIntHandlers + 80000h
        mov cl, 30h
Init_IDTLoop:
        mov [edi], eax
        mov dword [edi + 4], 00008e00h
        add edi, byte 8
        add eax, byte 7
        loop Init_IDTLoop

; copy the GDT to its final position
        mov esi, InitData(Init_GDT)
        mov cl, (Init_EndGDT - Init_GDT) / 4
        rep movsd
; fill unused descriptors with zeros
        xor eax, eax
        mov cx, (mmGDT_size - Init_EndGDT + Init_GDT) / 4
        rep stosd

        lgdt [SysData.GDT]
        jmp 8:InitData(Init_NewCS)

Init_NewCS:
        mov al, 10h
        mov ds, eax
        mov es, eax
        mov fs, eax
        mov gs, eax
        mov al, 18h
        mov ss, eax
        mov esp, mmFree         ; relocate before using the RM DataBlock!
        lidt [SysData.IDT]

; TODO: check for and initialize FPU (native exception mode)
; TODO: check for and evaluate multiboot information

%define KernelSection init
%include "modules.inc"

        jmp RuntimeCode(Start)

Init_SendSequence:
        outsb
        inc dl
        mov cl, 4
.loop   in al, 71h      ; delay
        in al, 71h      ; some more delay
        outsb
        loop .loop
        ret

Init_PICsequence:
        db 11h, 20h, 4, 1, 0ffh
        db 11h, 28h, 2, 1, 0ffh

        align 4
Init_GDT:
        dd 0, 0                         ; NULL segment

        dw 0ffffh, 0                    ; code segment
        db 0, 10011011b, 11001111b, 0

        dw 0ffffh, 0                    ; data segment
        db 0, 10010011b, 11001111b, 0

        dd 0                            ; stack segment
        db 0, 10010111b, 11000000b, 0

        dw 0ffffh, 0                    ; code segment for 
        db 0, 10011011b, 0, 0           ; the real mode portal

        dw 0ffffh, 0                    ; data segment for
        db 0, 10010011b, 0, 0           ; the real mode portal
Init_EndGDT:

%define KernelSection initdata
%include "modules.inc"

EOF:

; memory map

mmRuntime               equ Runtime
mmRuntime_size          equ Init - Runtime
mmIntHandlers           equ Init
mmIntHandlers_size      equ 30h * 7
mmIDT                   equ mmIntHandlers + mmIntHandlers_size
mmIDT_size              equ 30h * 8
mmGDT                   equ mmIDT + mmIDT_size
mmGDT_size              equ 0f000h - mmGDT + Runtime - 500h
mmRMStack               equ 0f000h
mmRMStack_size          equ 1000h
mmRMData                equ 10000h
mmRMData_size           equ 10000h
mmFree                  equ 20000h
