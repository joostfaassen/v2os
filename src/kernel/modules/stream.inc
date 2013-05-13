; Default stream implementations
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn ModuleMode, init

; Input:

; Install the keyboard interrupt handler
        mov eax, Keyboard_Handler + 80000h
        mov edi, [SysData.IDTBase]
        add edi, 21h * 8
        stosd
        mov [edi], dword 8e00h

; Unmask IRQ 1
        in al, 21h
        and al, 0fdh
        out 21h, al

; Output:

; Set 80x25 color text mode
        mov eax, [SysData.RMStruc]
        mov [eax + RMRegs.ax], word 3
        mov al, 10h
        call [RealMode.Interrupt]

%elifidn ModuleMode, variables4

        struc Keyboard_Table
.next   resd 1
.prev   resd 1
.data   resd 1
.low    resb 1
.high   resb 1
        endstruc

StreamVars:
.Cursor         dd 0b8000h
.NextLine       dd 0b8000h + 160
.KbdTable       dd Keyboard_DefaultTable
.KbdTable_size  dd 4
.Hooks          dd 0
.HookCount      dd 0
.KbdBuffer      dd 0
.ScancodeMap    dd 0

StreamCallTable dd Stream_Read, Stream_Write

%elifidn ModuleMode, data

Kbd_Buffer times 64 db 0

Kbd_modShift    equ 1
Kbd_modCapsLock equ 2
Kbd_modNumLock  equ 4

kbdAccentAcute  equ 80000000h
kbdAccentGrave  equ 80000001h
kbdAlt          equ 80000002h
kbdAltShift     equ 80000003h
kbdBackspace    equ 80000004h
kbdCapsLock     equ 80000005h
kbdCircumflex   equ 80000006h
kbdCtrl         equ 80000007h
kbdCtrlShift    equ 80000008h
kbdDel          equ 80000009h
kbdDoubleShift  equ 8000000ah
kbdDown         equ 8000000bh
kbdEnd          equ 8000000ch
kbdEnter        equ 8000000dh
kbdEsc          equ 8000000eh
kbdHome         equ 8000000fh
kbdInsert       equ 80000010h
kbdLeft         equ 80000011h
kbdNumLock      equ 80000012h
kbdPgDown       equ 80000013h
kbdPgUp         equ 80000014h
kbdRight        equ 80000015h
kbdScrollLock   equ 80000016h
kbdShift        equ 80000017h
kbdTab          equ 80000018h
kbdUp           equ 80000019h


%ifidn KeyboardLayout, DE

Keyboard_Default:
        dd kbdEsc, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0dfh, kbdAccentAcute, kbdBackspace
        dd kbdTab, 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 0fch, '+', kbdEnter
        dd kbdCtrl, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0f6h, 0e4h, kbdCircumflex
        dd kbdShift, '#', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', kbdShift
        dd '*', kbdAlt, ' ', kbdCapsLock, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, kbdNumLock, kbdScrollLock
        dd kbdHome, kbdUp, kbdPgUp, '-', kbdLeft, 0, kbdRight, '+', kbdEnd, kbdDown, kbdPgDown, kbdInsert, kbdDel, 0, 0, '<'

Keyboard_Shift:
        dd kbdEsc, '!', '"', 0a7h, '$', '%', '&', '/', '(', ')', '=', '?', kbdAccentGrave, kbdBackspace
        dd kbdTab, 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 0dch, '*', kbdEnter
        dd kbdCtrlShift, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0d6h, 0c4h, 0b0h
        dd kbdDoubleShift, 27h, 'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', kbdDoubleShift
        dd '*', kbdAltShift, ' ', kbdCapsLock, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, kbdNumLock, kbdScrollLock
        dd '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', ',', 0, 0, '>'

%else

  %error TODO: US keyboard layout

%endif

%elifidn ModuleMode, variables

Keyboard_DefaultTable:
                db 1, 3ah
                dd Keyboard_Default
                db 1, 2bh
                dd Keyboard_Shift

%elifidn ModuleMode, implementation

Stream_Read:
; in: ecx = buffer size
;     edi = buffer address
; out: ecx = size of data
;      cf set on buffer overflow

        xor ecx, ecx
        ret

Stream_Write:
; in: ecx = buffer size
;     esi = buffer address
; out: ecx = size of written data
;      cf set on buffer overflow

        push eax
        push edx
        push esi
        push edi
        push ecx
        mov edi, [StreamVars.Cursor]
        mov edx, [StreamVars.NextLine]
        jmp short .start

.loop   cmp eax, 20h
        jb short .miss
        mov ah, 7
.out    stosw
        cmp edi, edx
        jnb short .LF

.start  sub ecx, byte 4
        jb short .end
        lodsd
        cmp eax, byte 7eh
        ja short .miss
        cmp eax, 0ah
        je short .LF
        cmp eax, 0dh
        jne short .loop

.CR     cmp ecx, byte 4
        jb short .LF
        cmp dword [esi], 0ah
        sete al
        shl eax, 2
        add esi, eax
        sub ecx, eax

.LF     mov edi, edx
        cmp edx, 0b8000h + 160 * 25
        jae short .scroll
        add edx, 160
        jmp short .start

.miss   mov eax, '?' + 800h
        jmp short .out

.scroll mov eax, esi
        mov edx, ecx
        mov esi, 0b8000h + 160
        mov edi, 0b8000h
        mov ecx, 160 * 24
        rep movsd
        mov esi, eax
        mov eax, 7000700h
        mov ecx, 40
        rep stosd
        mov edi, 0b8000h + 160 * 24
        mov ecx, edx
        jmp short .start

.end    add ecx, byte 4
        pop eax
        sub eax, ecx
        mov ecx, eax
        mov [StreamVars.Cursor], edi
        mov [StreamVars.NextLine], edx
        mov dx, 3d4h
        mov al, 0fh
        out dx, al
        inc dl
        mov eax, edi
        shr eax, 1
        out dx, al
        dec dl
        mov al, 0eh
        out dx, al
        shr eax, 8
        and al, 3fh
        out dx, al
        pop edi
        pop esi
        pop edx
        pop eax
        clc
        ret

Keyboard_Handler:
        push eax
        in al, 60h
        mov [0b8000h], al
        mov al, 20h
        out 20h, al
        pop eax
        iretd

%endif
