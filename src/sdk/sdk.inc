; V2_OS Software Development Kit, include file for NASM
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

%ifndef SDK
%define SDK

%imacro struc 2.nolist
%push struc
%define %$strucname %1
[absolute %2_size]
%$strucname:
%endmacro


; *******************************
; * Interface IDs (preliminary) *
; *******************************

UnknownIID      equ 80000000h   ; used to compare interfaces
ServersIID      equ 80000001h   ; server manager

MemoryIID       equ 80000002h   ; system memory manager
MallocIID       equ 80000003h   ; plain Malloc
Malloc2IID      equ 80000004h   ; Malloc with Resize
MallocFactoryIID equ 80000005h  ; creates object Mallocs

ReadStreamIID   equ 80000007h   ; a stream for reading data
WriteStreamIID  equ 80000008h   ; a stream for writing data
ScreenIID       equ 8000000ch   ; a WriteStream with screen output

DirectoryIID    equ 8000000dh   ; a directory in a filesystem
FSObjectIID     equ 8000000eh   ; an object in a filesystem (file or directory)
SorageIID       equ 8000000fh   ; stream with positioning
FileIID         equ 80000010h   ; a file in a filesystem

CallbackIID     equ 80000011h   ; node of a double-linked list of callbacks
HookIID         equ 80000012h   ; head of a callback chain
CommandIID      equ 80000013h   ; text command line interface

HardwareIID     equ 80000006h   ; manages direct access to hardware
SegmentsIID     equ 80000014h   ; manager for segment descriptors
InterruptsIID   equ 80000015h   ; manager for interrupts and exceptions
ThreadsIID      equ 80000016h   ; basic multithreading engine
RealModeIID     equ 80000017h   ; simple access to real mode
TimerIID        equ 80000018h   ; Programmable Interval Timer interface
PICIID          equ 80000019h   ; Programmable Interrupt Controller interface
RTCIID          equ 8000001ah   ; Real Time Clock interface


; ***************************
; * Class IDs (preliminary) *
; ***************************

SystemCID       equ 80000001h   ; the basic API implemented in the kernel


; *************************
; * Interface definitions *
; *************************
;
; Defaults:
;
; Functions do not corrupt registers that are not listed under "out:";
; exception: arithmetic flags cf, pf, af, zf, sf, of.
; df is assumed to be clear.
; Functions in the static calltable ignore the parameter "instance".
;
; Interface pointers point to calltables defined by struc IXxxx.
; Instance pointers point to structures defined by struc iXxxx.
; If you know a better naming convention for interfaces and instances,
; please tell us.
; If there is no struc IXxxx, the interface pointer is not used at all.
; If there is no struc iXxxx, the instance pointer points to iUnknown.
;
; 64 bit arguments like edx:eax can be interpreted in two ways:
; - integers: with the first register as the high DWORD and
;    the second register as the low DWORD
; - interfaces: with the first register as the interface pointer and
;    the second register as the instance pointer
; In both cases, the first register is stored in memory 4 bytes higher
; than the second register.


; ************************
; * Component management *
; ************************

                struc iUnknown
.QueryInterface resd 1  ; in: eax = IID
                        ;     edx = instance
                        ; out: esi:edi = new interface
                        ;      cf set on error

.AddRef         resd 1  ; in: edx = instance

.Release        resd 1  ; in: edx = instance
                endstruc


                struc IServers
.List           resd 1  ; in: eax = IID
                        ;     ecx = enumeration callback
                        ;           in: ecx = CID
                        ;               edi = context
                        ;           out: edi = context
                        ;                cf set to abort the enumeration
                        ;     edx = instance
                        ;     edi = context
                        ; out: edi = context

.Get            resd 1  ; in: eax = IID
                        ;     ecx = CID
                        ;     edx = instance
                        ; out: esi:edi = new interface (implements iUnknown)
                        ;      cf set on error

.Add            resd 1  ; in: eax = IID
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

.Remove         resd 1  ; in: eax = IID
                        ;     ecx = CID
                        ;     edx = instance
                        ; out: cf set on error
                endstruc


; *********************
; * Memory management *
; *********************

                struc IMemory
.Allocate       resd 1  ; in: eax = flags:
                        ;           bits 0-2 = physical address limit:
                        ;             000 = no restrictions
                        ;             101 = the first 4 GB
                        ;             110 = the first 16 MB
                        ;             111 = the first 1 MB
                        ;           bit 3 = alignment required, ebx is valid
                        ;           bit 4 = bit 4 will be set in future Resize calls
                        ;           bit 5 = bit 5 will be set in future Resize calls
                        ;           bit 6 = block is going to be resized in the future
                        ;     ebx = minimal alignment if bit 3 of eax is set:
                        ;           lowest ebx bits of the address should be 0
                        ;     ecx = minimal size of the block
                        ;     edx = instance
                        ; out: ecx = actual size of the block
                        ;      edi = linear address of the block
                        ;      cf set on error

.Free           resd 1  ; in: ecx = size of the block
                        ;     edx = instance
                        ;     edi = linear address of the block
                        ; out: ecx = actual size of freed memory

.Resize         resd 1  ; in: eax = flags:
                        ;           bits 0-2 = physical address limit:
                        ;             000 = no restrictions
                        ;             101 = the first 4 GB
                        ;             110 = the first 16 MB
                        ;             111 = the first 1 MB
                        ;           bit 3 = alignment required, ebx is valid
                        ;           bit 4 = moving the block not allowed
                        ;           bit 5 = change the lower end of the block
                        ;     ebx = minimal alignment if bit 3 of eax is set:
                        ;           lowest ebx bits of the address should be 0
                        ;     ecx = old size of the block
                        ;     edx = instance
                        ;     esi = new size of the block
                        ;     edi = linear address of the block
                        ; out: ecx = actual new size of the block
                        ;      edi = new linear address of the block
                        ;      cf set on error

.GetInfo        resd 1  ; in: edx = instance
                        ; out: eax = total free memory
                        ;      ecx = largest free block
                endstruc


                struc IMalloc
.Allocate       resd 1  ; in: ecx = size of the block
                        ;     edx = instance
                        ; out: edi = linear address of the block
                        ;      cf set on error

.Free           resd 1  ; in: ecx = size of the block
                        ;     edx = instance
                        ;     edi = linear address of the block
                endstruc


                struc IMalloc2, IMalloc
.Resize         resd 1  ; in: ecx = old size of the block
                        ;     edx = instance
                        ;     esi = new size of the block
                        ;     edi = linear address of the block
                        ; out: edi = new linear address of the block
                        ;      cf set on error
                endstruc


; IMallocFactory interface pointer is a direct pointer to the creation function:
; in: eax = size of blocks to allocate from parent Malloc
;     ecx = size of objects that will be allocated
;     edx = instance
; out: esi:edi = new IMalloc
;      cf set on error


; ***************************
; * Data exchange (streams) *
; ***************************

                struc IReadStream
.Read           resd 1  ; in: ecx = size
                        ;     edx = instance
                        ;     edi = buffer address
                        ; out: ecx = data size
                        ;      cf set on error

.GetBuffer      resd 1  ; in: ecx = min. buffer size
                        ;     edx = instance
                        ;     edi = suggested buffer
                        ; out: ecx = data size
                        ;      esi = data address
                        ;      cf set on error

.Flush          resd 1  ; in: ecx = data size to flush
                        ;     edx = instance
			; out: cf set on error
		endstruc


                struc IWriteStream
.Write          resd 1  ; in: ecx = data size
                        ;     edx = instance
                        ;     esi = data address
                        ; out: ecx = size of not written data
                        ;      cf set on error

.GetBuffer      resd 1  ; in: ecx = min. buffer size
                        ;     edx = instance
                        ;     esi = suggested buffer
                        ; out: ecx = buffer size
                        ;      edi = buffer address
                        ;      cf set on error

.Flush          resd 1  ; in: ecx = data size to flush
                        ;     edx = instance
                        ; out: cf set on error
                endstruc


                struc IReadText
.ReadLine       resd 1  ; in: edx = instance
                        ; out: esi = text address
                        ;      ecx = text size
                        ;      cf set on EOF
                endstruc


                struc IWriteText
.WriteLine      resd 1  ; in: ecx = text size
                        ;     edx = instance
                        ;     esi = text address
                        ; out: ecx = size of not written data
                        ;      cf set on error
                endstruc


		struc IStream
.Read		resd 1	; see IReadSteam.Read
.GetReadBuffer	resd 1	; see IReadStream.Getbuffer
.FlushRead	resd 1	; see IReadStream.Flush
.Write		resd 1	; see IWriteStream.Write
.GetWriteBuffer	resd 1	; see IWriteStream.GetBuffer
.FlushWrite	resd 1	; see IWriteStream.Flush
		endstruc


                struc ITextStream, IStream
.ReadLine       resd 1  ; see IReadText.ReadLine
.WriteLine      resd 1  ; see IWriteText.WriteLine
                endstruc


                struc ITerminal, ITextStream
.DisableEcho    resd 1  ; in: edx = instance
                        ; out: cf set on error

.EnableEcho     resd 1  ; in: edx = instance
                        ; out: cf set on error
                endstruc


                struc IScreen, IWriteText ; UCS-4
.GetSize        resd 1  ; in: edx = instance
                        ; out: eax = width
                        ;      edx = height
                        ;      esi = physical width (in mm)
                        ;      edi = physical height (in mm)

.SetSize        resd 1  ; in: eax = width
                        ;     ecx = height
                        ;     edx = instance
                        ;     esi = physical width (in mm)
                        ;     edi = physical height (in mm)
                        ; out: cf set on error

.GetCursor      resd 1  ; in: edx = instance
                        ; out: eax = X
                        ;      ecx = Y

.SetCursor      resd 1  ; in: eax = X
                        ;     ecx = Y
                        ;     edx = instance
                        ; out: cf set on error
                endstruc


; ******************************
; * Files (persistent storage) *
; ******************************

                struc IDirectory
.Open           resd 1  ; in: eax = name (UTF-8)
                        ;     ecx = size of the name
                        ;     edx = instance
                        ; out: esi:edi = created IFSObject
                        ;      cf set on error

.Create         resd 1  ; in: eax = name (UTF-8)
                        ;     ecx = size of the name
                        ;     edx = instance
                        ;     esi = CID of the object to create
                        ; out: esi:edi = created IFSObject
                        ;      cf set on error

.List           resd 1  ; in: ecx = enumeration callback:
                        ;           in: ecx = size of the name
                        ;               esi = name (UTF-8)
                        ;               edi = context
                        ;           out: edi = context
                        ;     edx = instance
                        ;     edi = context
                        ; out: edi = context

.Mount          resd 1  ; in: eax = name (UTF-8)
                        ;     ecx = size of the name
                        ;     edx = instance
                        ;     esi:edi = IDirectory to mount
                        ; out: cf set on error

.Unmount        resd 1  ; in: eax = name (UTF-8)
                        ;     ecx = size of the name
                        ;     edx = instance
                        ; out: cf set on error
                endstruc


                struc IFSObject
.GetName        resd 1  ; in: edx = instance
                        ; out: eax = name (UTF-8)
                        ;      ecx = size of the name

.SetName        resd 1  ; in eax = name (UTF-8)
                        ;    ecx = size of the name
                        ;    edx = instance
                        ; out: cf set on error

.Delete         resd 1  ; in: edx = instance
                        ; out: cf set on error
; any ideas?
                endstruc


                struc IStorage, IStream
.Seek           resd 1  ; in: edx:eax = position (bytes)
                        ;         edi = instance
                        ; out: cf set on error

.Position       resd 2  ; position (bytes)

.Size           resd 2  ; size (bytes)

.Factor1        resd 1  ; bytes per sector

.Factor2        resd 1  ; sectors per head

.Factor3        resd 1  ; heads per cylinder
                endstruc


                struc IFile, IStorage
.SetSize        resd 1  ; in: edx:eax = new size (bytes)
                        ;     ecx = instance
                        ; out: cf set on error
; any ideas?
                endstruc


; ***********************
; * Hooks and callbacks *
; ***********************

                struc iCallback
.Next           resd 1  ; older ICallback, called after this one

.Prev           resd 1  ; newer ICallback, called before this one

.Callback       resd 1  ; function pointer
                        ; parameters are defined by the hook
                endstruc


                struc iHook, iCallback
.Mutex          resd 1  ; 0 = locked, 1 = free
                endstruc


                struc IHook
.HookFirst      resd 1  ; in: eax = iCallback
                        ;     edx = instance

.HookLast       resd 1  ; in: eax = iCallback
                        ;     edx = instance

.Unhook         resd 1  ; in: eax = iCallback
                        ;     edx = instance
                endstruc


; *******************************
; * Command line user interface *
; *******************************

                struc ICommand
.Execute        resd 1  ; in: edx = instance
                        ;     esi:edi = IReadStream (UCS-4)
                        ; out: cf set on error

.Add            resd 1  ; in: eax = callback
                        ;           in: ebx = context
                        ;               esi:edi = parameters as IReadStream (UCS-4)
                        ;           out: cf set on error
                        ;     ecx = size of the command
                        ;     edx = instance
                        ;     esi = command (UCS-4)
                        ;     edi = context
                        ; out: eax = old handler
                        ;      edi = old context
                        ;      cf set on error

.Remove         resd 1  ; in: eax = handler
                        ;     ecx = size of the command(bytes)
                        ;     edx = instance
                        ;     esi = command (UCS-4)
                        ;     edi = context
                        ; out: cf set on error
                endstruc


; ************
; * Keyboard *
; ************

                struc iKbdEntry
.Next           resd 1
.Char:
.Callback       resd 1  ; in: ebx = iKbdEntry
                        ; out: cf set to continue processing
.Flags          resb 1
    Kbd_Flag_Release	equ 1
    Kbd_Flag_Hook	equ 2

.Key            resb 1	; scancode rol 1
.Index          resw 1
                endstruc


                struc iKbdTable
.Next           resd 1
.Prev           resd 1
.Table          resd 1
.Entries        resw 1  ; number of entries
.Mask           resb 1
                align 4, resb 1
                endstruc


; Preliminary keyboard interface:

                struc KbdChars
.Count          resb 1
.FirstKey       resb 1  ; key = scancode rol 1
.FirstIndex     resw 1
.Data:                  ; an array of .Count UCS-4 characters follows
                endstruc


                struc IKeyboard
.CreateTable    resd 1  ; in: eax = size of the table in entries (power of 2)
			;     esi = 0-terminated list of KbdChars structures
                        ; out: eax = iKbdTable
                        ;      cf set on error

.AddEntry       resd 1  ; in: eax = iKbdEntry
                        ;     edx = iKbdTable
                        ; out: cf set on error
                endstruc


;******************
;* Multithreading *
;******************

		struc Regs
.eflags:
.flags  	resw 1
        	resw 1
.edi:
.di     	resw 1
        	resw 1
.esi:
.si     	resw 1
        	resw 1
.ebp:
.bp     	resw 1
        	resw 1
.esp:
.sp     	resw 1
		resw 1
.ebx:
.bx:
.bl     	resb 1
.bh     	resb 1
        	resw 1
.edx:
.dx:
.dl     	resb 1
.dh     	resb 1
        	resw 1
.ecx:
.cx:
.cl     	resb 1
.ch     	resb 1
        	resw 1
.eax:
.ax:
.al     	resb 1
.ah     	resb 1
        	resw 1
		endstruc

		struc FPUState

		struc iCPU

		endstruc


		%macro iThread_definition 0
.CPU		resd 1	; iCPU
.StackLimit	resd 1
.Pause		resd 1
.Resume		resd 1	; in: edx = instance

.ExceptionHook	resd 2	; the interface pointer points to a function:
			; in: eax = iException
			;     edx = instance
.Prev		resd 1
.Next		resd 1
		%endmacro

struc iThread_positive_offsets
iThread_definition
endstruc

[absolute -iThread_positive_offsets_size]
iThread:
iThread_definition
iThread_size:
__SECT__


                struc IThreads
.CreateThread   resd 1	; in: edx = instance
			; out: eax = iThread
			;      cf set on error

.Resume         resd 1	; in: edx = iThread to resume
			; out: cf set if the thread runs already
                endstruc


;*************
;* Real Mode *
;*************

                struc IRealMode
.Call           resd 1  ; in: eax = size of the RM stack
                        ;     ebx = address of the RM stack (16 byte aligned)
                        ;     ecx = size of parameters on the RM stack
                        ;     edx = instance

.Interrupt      resd 1  ; in: al = interrupt number
                        ;    edx = instance
			;    RMRegs structure pointed to by [SysData.RMStruc]
			; out: RMRegs structure pointed to by [SysData.RMStruc]
                endstruc


struc RMRegs
.eflags:
.flags  resw 1
        resw 1
.edi:
.di     resw 1
        resw 1
.esi:
.si     resw 1
        resw 1
.ebp:
.bp     resw 1
        resw 1
        resd 1  ; not used (esp)
.ebx:
.bx:
.bl     resb 1
.bh     resb 1
        resw 1
.edx:
.dx:
.dl     resb 1
.dh     resb 1
        resw 1
.ecx:
.cx:
.cl     resb 1
.ch     resb 1
        resw 1
.eax:
.ax:
.al     resb 1
.ah     resb 1
        resw 1
.ds     resw 1
.es     resw 1
.fs     resw 1
.gs     resw 1
.ip     resw 1
.cs     resw 1

._ret   resw 2  ; return address is set by the portal code

.params:        ; stack parameters that are removed by the
                ; real mode routine should be inserted here

._esp   resd 1  ; the protected mode stack pointer is saved
._ss    resw 1  ; here by the portal code

._sp    resw 1  ; used to synchronize stacks before and
                ; after execution of the routine
.size   endstruc


; *******************
; * Hardware access *
; *******************

                struc IHardware
; should be a hardware manager, any ideas?
.Get            resd 1
.Lock           resd 1
.Unlock         resd 1
.Add            resd 1
.Remove         resd 1
                endstruc


                struc ISegments
.Get            resd 1
.Set            resd 1
.Create         resd 1
.Destroy        resd 1
                endstruc


                struc IInterrupts
.Hook           resd 1
.Unhook         resd 1
                endstruc


                struc ITimer
.SetInterval    resd 1
.GetInterval    resd 1
.SetFowarder    resd 1
.GetFowarder    resd 1
.Count          resd 1
.BaseUnit       resd 1
                endstruc


                struc IPIC
.SetMask        resd 1
.SetMaskBit     resd 1
.ClearMaskBit   resd 1
.SetRMBase      resd 1
.SetPMBase      resd 1
                endstruc


                struc IRTC
.ReadTime       resd 1
.SetTime        resd 1
.ReadAlarm      resd 1
.SetAlarm       resd 1
                endstruc


; ************************************
; * Static calltable and system data *
; ************************************

                absolute 500h
SysUnknown:
.QueryInterface resd 1
.AddRef         resd 1
.Release        resd 1

SysServers:
.List           resd 1
.Get            resd 1
.Add            resd 1
.Remove         resd 1

SysMemory:
.Allocate       resd 1
.Free           resd 1
.Resize         resd 1
.GetInfo        resd 1

SysMalloc:
.Allocate       resd 1
.Free           resd 1
.Resize         resd 1

SysMallocFactory resd 1

SysTextStream:	; TODO

SysHardware:
.Get            resd 1
.Lock           resd 1
.Unlock         resd 1
.Add            resd 1
.Remove         resd 1

SysUCS4_to_UTF8 resd 1
SysUTF8_to_UCS4 resd 1

; SysStreamFactory resd 1

SysDirectory:
.Open           resd 1
.Create         resd 1
.List           resd 1
.Mount          resd 1
.Unmount        resd 1

SysHook:
.HookFirst      resd 1
.HookLast       resd 1
.Unhook         resd 1

SysCommand:
.Execute        resd 1
.Add            resd 1
.Remove         resd 1

SysCommandHook  resd 1

SysKbd:
.CreateTable    resd 1
.AddEntry	resd 1

SysKbdStream:
.Read		resd 1
.GetBuffer	resd 1
.Flush		resd 1

SysSegments:
.Get            resd 1
.Set            resd 1
.Create         resd 1
.Destroy        resd 1

SysInterrupts:
.Hook           resd 1
.Unhook         resd 1

SysThreads:
.Start          resd 1
.Stop           resd 1
.Resume         resd 1

SysRealMode:
.Call           resd 1
.Interrupt      resd 1

SysData:
.IDT            resw 1
.IDTBase        resd 1
.GDT            resw 1
.GDTBase        resd 1
.IDTHooks       resd 1
.IDTHooksSize   resw 1
.RM_IDT         resw 1
.RM_IDTBase     resd 1
.RMDataBlock    resd 1
.RMDataSize     resd 1
.RMStack        resd 1
.RMStackSize    resd 1
.RMStruc        resd 1
.KbdNotFound    resb iHook_size ; Callbacks: in: al = scancode
				;                edx = instance
.KbdNotify	resb iHook_size	; Callbacks: in: edx = instance
.Scancodes      resd 1          ; scancode map
.ScancodeCount  resd 1          ; size in bits

%assign EndOfSysData $

                section .text

%endif
