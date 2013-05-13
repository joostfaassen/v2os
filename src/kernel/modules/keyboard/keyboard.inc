; Default keyboard driver
; Copyright (C) 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, calltable

dd Kbd_CreateTable, Kbd_AddEntry

dd Kbd_Read, Kbd_GetBuffer, Kbd_Flush

%endif
%ifidn KernelSection, init

Kbd_Init:

; Create a Malloc for table descriptors
        mov eax, iKbdTable_size
        mov ecx, 100h
        call [SysMallocFactory]
        jc near OutOfMemory
        mov [Kbd_TableMalloc], edi
        mov [Kbd_TableMalloc + 4], esi

; Create a Malloc for table entries
        mov eax, iKbdEntry_size
        call [SysMallocFactory]
        jc near OutOfMemory
        mov [Kbd_EntryMalloc], edi
        mov [Kbd_EntryMalloc + 4], esi

; Install the default layout
	mov al, 128	; eax was iKbdEntry_size before
        mov esi, InitData(Kbd_DefaultLayout)	; esi = layout
        call [SysKbd.CreateTable]		; eax = iKbdTable
        jc near OutOfMemory
	mov [Kbd_LShiftUp + iKbdCustomEntry.NextTable], eax
	mov [Kbd_RShiftUp + iKbdCustomEntry.NextTable], eax
	mov [Kbd_CurrentTable], eax
	mov edx, eax
	mov eax, Kbd_LShiftDown
	call [SysKbd.AddEntry]
	jc near OutOfMemory
	mov eax, Kbd_RShiftDown
	call [SysKbd.AddEntry]
	jc near OutOfMemory
	mov eax, Kbd_LShift2Up
	call [SysKbd.AddEntry]
	jc near OutOfMemory
	mov eax, Kbd_RShift2Up
	call [SysKbd.AddEntry]
	jc near OutOfMemory
	mov eax, 128
	mov esi, InitData(Kbd_ShiftLayout)
	call [SysKbd.CreateTable]
	jc near OutOfMemory
	mov [Kbd_LShiftDown + iKbdCustomEntry.NextTable], eax
	mov [Kbd_RShiftDown + iKbdCustomEntry.NextTable], eax
	mov edx, eax
	mov eax, Kbd_LShift2Down
	call [SysKbd.AddEntry]
	jc near OutOfMemory
	mov eax, Kbd_RShift2Down
	call [SysKbd.AddEntry]
	jc near OutOfMemory
	mov eax, Kbd_LShiftUp
	call [SysKbd.AddEntry]
	jc near OutOfMemory
	mov eax, Kbd_RShiftUp
	call [SysKbd.AddEntry]
	jc near OutOfMemory

; Install the keyboard interrupt handler
        mov eax, Kbd_Handler + 80000h
        mov edi, [SysData.IDTBase]
        add edi, 21h * 8
        stosd
        mov [edi], dword 8e00h

; Unmask IRQ 1
        in al, 21h
        and al, 0fdh
        out 21h, al

; Enable the keyboard
	mov al, 0f4h
	out 60h, al

%endif
%ifidn KernelSection, initdata

Kbd_DefaultLayout:
        db 28, 2, 1, 0
	dd 1bh, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0dfh, '´', 8
        dd 9, 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 0fch, '+', 0dh
	db 12, 3ch, 1eh, 0
        dd 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0f6h, 0e4h, '^'
        db 11, 56h, 2bh, 0
	dd '#', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-'
	db 1, 6eh, 37h, 0, '*', 0, 0, 0
        db 1, 72h, 39h, 0, ' ', 0, 0, 0
        db 1, 94h, 4ah, 0, '-', 0, 0, 0
        db 1, 9ch, 4eh, 0, '+', 0, 0, 0
        db 1, 0ach, 56h, 0, '<', 0, 0, 0
	dd 0

Kbd_ShiftLayout:
        db 28, 2, 1, 0
	dd 1bh, '!', '"', 0a7h, '$', '%', '&', '/', '(', ')', '=', '?', '`', 8
        dd 9, 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 0dch, '*', 0dh
	db 12, 3ch, 1eh, 0
        dd 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0d6h, 0c4h, 0b0h
        db 11, 56h, 2bh, 0
	dd 27h, 'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_'
        db 1, 6eh, 37h, 0, '*', 0, 0, 0
        db 1, 72h, 39h, 0, ' ', 0, 0, 0
        db 13, 8eh, 47h, 0
	dd '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', ','
        db 1, 0ach, 56h, 0, '>', 0, 0, 0
	dd 0

%endif
%ifidn KernelSection, DWORDvariables

Kbd_TableMalloc         dd 0, 0
Kbd_EntryMalloc         dd 0, 0
Kbd_CurrentTable	dd 0
Kbd_BufferStart         dd Kbd_Buffer
Kbd_BufferEnd           dd Kbd_Buffer + 256
Kbd_ReadPos             dd Kbd_Buffer
Kbd_WritePos            dd Kbd_Buffer

%endif
%ifidn KernelSection, data

%ifdef DEBUG

Kbd_Buffer		equ 0b8000h + 23 * 160

ScancodeMap		equ 0b8000h + 22 * 160

%else

Kbd_Buffer              times 32 dd 0, 0

ScancodeMap             times 16 db 0

%endif

			struc iKbdCustomEntry, iKbdEntry
.NextTable		resd 1
			endstruc

Kbd_LShiftDown		istruc iKbdCustomEntry
at iKbdEntry.Callback,	dd Kbd_ShiftDown
at iKbdEntry.Flags,	db Kbd_Flag_Hook
at iKbdEntry.Key,	db 54h
at iKbdEntry.Index,	dw 2ah
			iend

Kbd_RShiftDown		istruc iKbdCustomEntry
at iKbdEntry.Callback,	dd Kbd_ShiftDown
at iKbdEntry.Flags,	db Kbd_Flag_Hook
at iKbdEntry.Key,	db 6ch
at iKbdEntry.Index,	dw 36h
			iend

Kbd_LShiftUp		istruc iKbdCustomEntry
at iKbdEntry.Callback,	dd Kbd_ShiftUp
at iKbdEntry.Flags,	db Kbd_Flag_Hook + Kbd_Flag_Release
at iKbdEntry.Key,	db 55h
at iKbdEntry.Index,	dw 2ah
			iend

Kbd_RShiftUp		istruc iKbdCustomEntry
at iKbdEntry.Callback,	dd Kbd_ShiftUp
at iKbdEntry.Flags,	db Kbd_Flag_Hook + Kbd_Flag_Release
at iKbdEntry.Key,	db 6dh
at iKbdEntry.Index,	dw 36h
			iend

Kbd_LShift2Down		istruc iKbdCustomEntry
at iKbdEntry.Callback,	dd Kbd_Shift2Down
at iKbdEntry.Flags,	db Kbd_Flag_Hook
at iKbdEntry.Key,	db 54h
at iKbdEntry.Index,	dw 2ah
			iend

Kbd_RShift2Down		istruc iKbdCustomEntry
at iKbdEntry.Callback,	dd Kbd_Shift2Down
at iKbdEntry.Flags,	db Kbd_Flag_Hook
at iKbdEntry.Key,	db 6ch
at iKbdEntry.Index,	dw 36h
			iend

Kbd_LShift2Up		istruc iKbdCustomEntry
at iKbdEntry.Callback,	dd Kbd_Shift2Up
at iKbdEntry.Flags,	db Kbd_Flag_Hook + Kbd_Flag_Release
at iKbdEntry.Key,	db 55h
at iKbdEntry.Index,	dw 2ah
			iend

Kbd_RShift2Up		istruc iKbdCustomEntry
at iKbdEntry.Callback,	dd Kbd_Shift2Up
at iKbdEntry.Flags,	db Kbd_Flag_Hook + Kbd_Flag_Release
at iKbdEntry.Key,	db 6dh
at iKbdEntry.Index,	dw 36h
			iend

%endif
%ifidn KernelSection, implementation

Kbd_ShiftDown:
; in: ebx = iKbdCustomEntry
;     ecx = index
; out: ecx corrupted
;      nc

	bts [ScancodeMap], ecx
	mov ecx, [ebx + iKbdCustomEntry.NextTable]
	clc
	mov [Kbd_CurrentTable], ecx
	ret

Kbd_ShiftUp:
; in: ebx = iKbdCustomEntry
;     ecx = index
; out: ecx corrupted
;      nc

	btr [ScancodeMap], ecx
	bt dword [ScancodeMap + 2ah >> 3], 2ah & 7
	setc cl
	bt dword [ScancodeMap + 36h >> 3], 36h & 7
	adc cl, 0
	jnz .exit
	mov ecx, [ebx + iKbdCustomEntry.NextTable]	; CMOV candidate
	mov [Kbd_CurrentTable], ecx			; CMOV candidate
.exit	ret

Kbd_Shift2Down:
; in: ecx = index
; out: nc

	bts [ScancodeMap], ecx
	clc
	ret

Kbd_Shift2Up:
; in ecx = index
; out: nc

	btr [ScancodeMap], ecx
	clc
	ret

Kbd_CreateTable:
; in: eax = size of the table in entries (power of 2)
;     esi = 0-terminated list of KbdChars structures
; out: eax = iKbdTable
;      esi = address of the terminating 0
;      cf set on error

	cmp eax, 101h
	cmc
	jc near .ret
        push ebx
	push ecx
        push edx
        push edi
        mov edi, [Kbd_TableMalloc + 4]		; edi = IMalloc for tables
        mov ecx, iKbdTable_size			; ecx = size of iKbdTable
        mov edx, [Kbd_TableMalloc]		; edx = iMalloc for tables
        call [edi + IMalloc.Allocate]		; edi = iKbdTable
        jc near .error
        mov ebx, edi				; ebx = iKbdTable
        lea ecx, [eax * 4]			; ecx = size of the table
        call [SysMalloc.Allocate]		; edi = hash table
        jc near .error
        mov [ebx + iKbdTable.Next], dword 0
        mov [ebx + iKbdTable.Prev], dword 0
        mov [ebx + iKbdTable.Table], edi
        mov ecx, eax				; ecx = number of entries
	mov [ebx + iKbdTable.Entries], word 1
        dec eax					; eax = mask
        mov [ebx + iKbdTable.Mask], ax
        xor eax, eax				; eax = 0
        rep stosd
	push ebx				; returned eax
	jmp short .start

.loop   mov ecx, iKbdEntry_size			; ecx = size of an entry
	mov eax, [Kbd_EntryMalloc + 4]		; eax = iMalloc for entries
	mov edx, [Kbd_EntryMalloc]		; edx = IMalloc for entries
	call [eax + IMalloc.Allocate]		; edi = entry1
	jc short .error2
	mov ebp, edi				; ebp = entry1
	call [eax + IMalloc.Allocate]		; edi = entry2
	jc short .error2
	mov ecx, ebx				; ch = key, ecx shr 16 = index
	lodsd					; eax = char
	mov cl, 0				; cl = flags: none
	mov [edi + iKbdEntry.Char], eax
	mov [edi + iKbdEntry.Flags], ecx
	mov edx, [esp]				; edx = iKbdTable
	xchg eax, edi				; eax = entry2, edi = char
	call [SysKbd.AddEntry]
	jc short .error2
	mov eax, ebp				; eax = entry1
	add ecx, 100h + Kbd_Flag_Release	; cl = flags: release, ch = key
	mov [eax + iKbdEntry.Char], edi
	mov [eax + iKbdEntry.Flags], ecx
	call [SysKbd.AddEntry]
	jc short .error2
	add ebx, 10200h				; ebx = next key and index
	dec bl					; bl = remaining count
	jnz short .loop
.start  lodsd					; bl = count, bh = first key
	mov ebx, eax				; ebx shr 16 = first index
	or al, al
	jnz short .loop
.error2	sub esi, byte 4				; esi = address of terminating 0
	pop eax					; eax = iKbdTable
.error  pop edi
        pop edx
	pop ecx
        pop ebx
.ret    ret

Kbd_AddEntry:
; in: eax = iKbdEntry
;     edx = iKbdTable
; out: cf set on error (IMalloc.Allocate)

        push ebx
        push ecx
        mov ecx, [edx + iKbdTable.Table]		; ecx = hash table
        movzx ebx, byte [eax + iKbdEntry.Key]		; ebx = key
        and bl, [edx + iKbdTable.Mask]			; ebx = hash value
        lea ecx, [ecx + ebx * 4]			; ecx = entry in the table
        mov ebx, eax					; ebx = iKbdEntry
        xchg ebx, [ecx]					; ebx = next entry in the chain
        mov [eax + iKbdEntry.Next], ebx
        movzx ebx, word [edx + iKbdTable.Entries]	; ebx = old number of entries
        inc ebx						; ebx = new number of entries
        mov [edx + iKbdTable.Entries], bx
        add ebx, ebx					; ebx = optimal hash table size
        mov cl, [edx + iKbdTable.Mask]			;  cl = mask
        cmp cl, bl
        jnc short .end
        mov bl, cl					; ebx = old mask
        adc bl, bl					; ebx = new mask
        mov [edx + iKbdTable.Mask], bl
        push eax
        xor eax, eax					; eax = 0
        push esi
        push edi
        push ebp
        lea ecx, [eax + ebx * 4 + 4]			; ecx = new hash table size
        call [SysMalloc.Allocate]			; edi = new hash table
        jc short .error
        lea ecx, [ebx + 1]				; ecx = new number of table entries
        mov esi, edi					; esi = new hash table
        rep stosd
        mov edi, esi					; edi = new hash table
        mov esi, [edx + iKbdTable.Table]		; esi = old hash table
        mov bh, bl					;  bh = new mask
.loop   lodsd						; eax = entry
        or eax, eax
        jz short .next
	push dword 0
.loop2	push eax
	mov eax, [eax + iKbdEntry.Next]			; eax = next entry
	or eax, eax
	jnz short .loop2
	pop eax						; eax = last entry
.loop3  mov ebp, eax					; ebp = entry
	mov cl, [eax + iKbdEntry.Key]			; ecx = key
        and cl, bh					; ecx = table index
        xchg ebp, [edi + ecx * 4]			; ebp = next entry in the new chain
        mov [eax + iKbdEntry.Next], ebp
	pop eax						; eax = previous entry
        or eax, eax
        jnz short .loop3
.next   sub bl, 2					; bl = remaining entries * 2 - 1
        jnc short .loop
        xchg edi, [edx + iKbdTable.Table]		; edi = old hash table
        mov cl, bh					; ecx = new mask
        lea ecx, [ecx * 2 + 2]				; ecx = size of the old table
        call [SysMalloc.Free]
        clc
.error  pop ebp
        pop edi
        pop esi
        pop eax
.end    pop ecx
        pop ebx
        ret

Kbd_Handler:
        push eax
        in al, 60h
        push ebx
        push ecx
        push edx
        mov edx, [Kbd_CurrentTable]
        rol al, 1
        or edx, edx
        jz short .exit
.loop   movzx ebx, al
        mov ecx, [edx + iKbdTable.Table]
        and bl, [edx + iKbdTable.Mask]
        mov ebx, [ecx + ebx * 4]
	or ebx, ebx
	jz short .nextT
.loop2	cmp al, [ebx + iKbdEntry.Key]
        jne short .next
        test byte [ebx + iKbdEntry.Flags], Kbd_Flag_Hook
        movzx ecx, word [ebx + iKbdEntry.Index]
        jnz short .hook
        test byte [ebx + iKbdEntry.Flags], Kbd_Flag_Release
        jnz short .keyup
        bts [ScancodeMap], ecx
        mov eax, [Kbd_WritePos]
        mov ebx, [ebx + iKbdEntry.Char]
%ifdef DEBUG
mov bh, 7
%endif
        mov [eax], ecx
        mov [eax + 4], ebx
        add eax, 8
        cmp eax, [Kbd_BufferEnd]
        jb short .nowrap
        mov eax, [Kbd_BufferStart]      ; CMOV candidate
.nowrap cmp eax, [Kbd_ReadPos]
        je short .exit
        mov [Kbd_WritePos], eax         ; CMOV candidate
.exit   mov al, 20h
        out 20h, al
	mov edx, [SysData.KbdNotify + iCallback.Next]
	call [edx + iCallback.Callback]
	pop edx
        pop ecx
        pop ebx
        pop eax
        iret

.keyup  btr [ScancodeMap], ecx
        jmp short .exit

.hook   call [ebx + iKbdEntry.Callback]
        jnc short .exit
.next   mov ebx, [ebx + iKbdEntry.Next]
        or ebx, ebx
        jnz short .loop2
.nextT  mov edx, [edx + iKbdTable.Next]
        or edx, edx
        jnz near .loop ;short?
	ror al, 1
	mov edx, [SysData.KbdNotFound + iCallback.Next]
	call [edx + iCallback.Callback]
        jmp short .exit

Kbd_Read:
; in: ecx = size
;     edi = buffer address
; out: ecx = data size
;      cf set on error

	and ecx, byte -8		; ecx rounded down to multiples of 8
	jz short .exit
	push eax
	push ebx
	push esi
	push edi
	xor ebx, ebx			; ebx = 0 (return value for ecx)
	mov esi, [Kbd_ReadPos]		; esi = start of the source data
	mov eax, [Kbd_WritePos]		; eax = end of source data
	sub eax, esi			; eax = size of source data (< 0 if it wraps)
	je short .empty
	ja short .1piece
	mov eax, [Kbd_BufferEnd]	; eax = end of source buffer
	sub eax, esi			; eax = size of 1st piece of data
	cmp ecx, eax
	jbe short .last
	xchg eax, ecx			; eax = dest buffer size, ecx = size of 1st piece
	sub eax, ecx			; eax = size of data to read from 2nd piece
	mov ebx, ecx			; ebx = size of 1st piece
	shr ecx, 2			; ecx = number of DWORDs to copy
	rep movsd			; ecx = 0
	mov ecx, eax			; ecx = remaining size of data to copy
	mov esi, [Kbd_BufferStart]	; esi = start of the 2nd piece
	mov eax, [Kbd_WritePos]		; eax = end of 2nd piece
	sub eax, esi			; eax = size of 2nd piece
	jz short .empty2
.1piece	cmp ecx, eax
	jb short .last
	mov ecx, eax	; CMOV candidate, ecx = min(source data, data to copy)
.last	add ebx, ecx			; ebx = total size of copied data
	shr ecx, 2			; ecx = number of DWORDs to copy
	rep movsd			; ecx = 0, esi = remaining data
	cmp esi, [Kbd_BufferEnd]
	jb .empty2
	mov esi, [Kbd_BufferStart]	; CMOV candidate
.empty2	mov [Kbd_ReadPos], esi
.empty	mov ecx, ebx
	pop edi
	pop esi
	pop ebx
	pop eax
.exit	ret

Kbd_GetBuffer:
; in: ecx = min. buffer size
;     edi = suggested buffer
; out: ecx = data size
;      esi = data address
;      cf set on error


Kbd_Flush:
; in: ecx = offset to add to internal pointers
; out: cf set on error

stc
ret

%endif
