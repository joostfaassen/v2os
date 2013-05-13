; V2_OS boot sector
; Copyright (C) 1999  V2_Lab
; Copyright (C) 2000, 2001  V2_OS Kernel Coders Team
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
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

        bits 16
        org 7c00h

        cli
        xor bx, bx
        mov ds, bx
        mov ss, bx
        mov sp, 7c00h

; A small CPU check to determine if we run on a 386
; see http://www.ddj.com/articles/1996/9611/9611n/9611n.htm for details
        push sp
        pop ax
        cmp ax,sp
        jz AtLeast286
WrongCPU:
        mov si, WrongCPUMsg
        jmp short WriteAndDie
AtLeast286:        
        mov word [bx + 24], WrongCPU    ; Install an exception handler
        mov [bx + 26], bx
        cwde                            ; and try a 32 bit instruction

; Save boot disk
        mov [BootDisk], dl

; Check for int 13h extensions
        xor bp, bp              ; bp=0 means LBA, bp=1 means CHS
        mov ah, 41h
        mov bx, 55aah
        int 13h
        jc NoExtensions
        cmp bx, 0aa55h
        jnz NoExtensions
        test cl, 1
        jz SkipCHS

NoExtensions:
        inc bp
        mov ah, 8
        xor di, di
        int 13h                 ; Get some information about the bootdisk
        jc Error

        and ecx, byte 00111111b
        mov [SectorsPerHead], ecx
        mov cl, dh
        inc cx
        mov [NumberOfHeads], ecx

SkipCHS mov ah, 0               ; Reset the bootdrive
        mov dl, [BootDisk]
        int 13h
        jc Error

        or dl,dl
        jns near LoadKernel

        call ReadSectors
        mov si, 8200h - 2 - 64  ; Point to the partition table
        mov cx, 4

PartLoop:
        cmp byte [si + 4], 6fh
        jz Found
        add si, byte 16
        loop PartLoop
        mov si, BadPartTableMsg

WriteAndDie:
        mov ah, 0eh             ; Teletype output
        xor bx, bx              ; Page 0, color 0 (graphic only)
.Loop   lodsb
        or al, al
        jz $
        int 10h
        jmp short .Loop

Error   mov si, ErrorMsg
        jmp short WriteAndDie

ReadSectors:
        mov di, 4
RetryLoop:
        mov ah, 42h
        mov si, DAP
        or bp, bp
        jz Load

        xor edx, edx
        mov eax, [DAP.Sector]
        div dword [SectorsPerHead]
        mov cx, dx
        inc cl
        mov dl, 0
        div dword [NumberOfHeads]
        mov ch, al
        shl ah, 6
        or cl, ah
        mov dh, dl
        mov ax, 0201h
        les bx, [DAP.Buffer]

Load    mov dl,[BootDisk]
        int 13h
        mov byte [DAP.Count], 1
        jnc Next
        mov ah, 0
        int 13h
        jc Error
        dec di
        jnz RetryLoop
        jmp short Error

Next    add word [DAP.BufferSeg], byte 20h
        inc dword [DAP.Sector]
        dec word [SectorCount]
        jnz ReadSectors
        ret

Found   sub cl, 4
        sub [BootPartition], cl
        mov eax, [si + 8]
        mov [DAP.Sector], eax

LoadKernel:
        mov ebx, [LoadPosition]
        shr ebx, 4
        mov [DAP.BufferSeg], bx
        mov eax, [KernelPosition]
        add [DAP.Sector], eax
        mov ax, [KernelSize]
        mov [SectorCount], ax
        call ReadSectors

; enable A20
        cli
        mov al, 0d1h            ; AT
        out 64h, al
        mov al, 3
        out 60h, al
        mov al, 2               ; PS/2
        out 92h, al

; switch to protected mode
        lgdt [GDTvalue]         ; load protected mode GDT
        mov eax, cr0
        or al, 1                ; set protected mode flag
        mov cr0, eax
        jmp 8:SetRegs           ; jump to 32 bit code

        bits 32                 ; 32 bit protected mode code
SetRegs xor ebx, ebx
        mov bl, 10h
        mov ds, ebx
        mov es, ebx
        mov fs, ebx
        mov gs, ebx
        mov ss, ebx
        mov bx, BootDisk - 12   ; Multiboot information
        mov eax, 2badb002h
        mov [ebx], al
        jmp [word LoadPosition]

; Initialized variables

; error messages
WrongCPUMsg     db '32 bit CPU required.', 0
BadPartTableMsg db 'Boot partition not found.', 0
ErrorMsg        db 'Error', 0

; the disk access packet for int 13h extensions
DAP             db 10h, 0
  .Count        dw 1
  .Buffer       dw 0
  .BufferSeg    dw 800h
  .Sector       dd 0
  .SectorHigh   dd 0

SectorCount     dw 1

GDTvalue        dw 17h, GDT - 8, 0

; Multiboot information structure
BootDisk        db 0
BootPartition   db 0
                dw 0ffffh       ; unused "subpartitions"

; Pad the boot sector to 512 bytes
        times 1e4h - ($ - $$) nop

LoadPosition    dd 10000h

GDT             dw 0ffffh, 0
                db 0, 10011011b, 11001111b, 0
                dw 0ffffh, 0
                db 0, 10010011b, 11001111b, 0

KernelPosition  dd 1
KernelSize      dw 20

; Boot signature
                dw 0aa55h

; Uninitialized variables
        section .bss
SectorsPerHead  resd 1
NumberOfHeads   resd 1
