; Programmable Interval Timer interface
; by EKS (futur@mad.scientist.com)
;--------------------------------------
; revision 0.1
; revision 0.2 (trem)

%ifidn ModuleMode, init_VP_broke_it

        ; Hook Interrupt handler in IDT
        mov eax, InitData(PIT_handler) + 80000h
        mov edi, [SysData.IDTBase]
        add edi, dword 0x20*8
        stosd
        mov [edi], dword 0x00008E00

        ; UnMask irq0
        in al, 0x21
        and al, 0xFE
        out 0x21, al


%elifidn ModuleMode, calltable_VP_broke_it

dd Timer_SetInterval
dd Timer_GetInterval
dd Timer_SetFowarder
dd Timer_GetFowarder
dd 0;Timer_Count
dd 0;Timer_BaseUnit

%elifidn ModuleMode, implementation

PIT_COUNT: dd 0                 ; number total of int
PIT_BASEUNIT: dd 18             ; number of int per second

;-------------------------------------------
; trem
activate:
        push ax
        cli     
        ; UnMask irq0
        in      al, 0x21
        and     al, 0xFE
        out     0x21, al
        sti
        pop     ax
        retn

;-------------------------------------------
; trem
; BE CARREFUL when you use it
desactivate:
        push ax
        cli     
        ; Mask irq0
        in      al, 0x21
        or      al, 0x01
        out     0x21, al
        sti
        pop     ax
        retn

;-------------------------------------------[ Exported: Timer.SetInterval ]----
; trem
; eax contain the frequence (number of int per second)
Timer_SetInterval:
        push    eax
        push    ecx
        push    edx
        mov     [PIT_BASEUNIT], eax
        cli
        mov     ecx, eax
        mov     al,00110110b  ; bit 7,6 = (00) timer counter 0
                              ; bit 5,4 = (11) write LSB then MSB
                              ; bit 3-1 = (011) generate square wave
                              ; bit 0 = (0) binary counter
        out     43h,al        ; prep PIT, counter 0, square wave&init count

        xor     edx, edx
        mov     eax, 1193180
        div     ecx
        mov     [PIT_INTERVAL], ax      

        out     40h,al
        mov     al, ah
        out     40h,al
        sti

        pop     edx
        pop     ecx
        pop     eax
        retn

;-------------------------------------------[ Exported: Timer.GetInterval ]----
; trem
; on return, eax contain the interval
Timer_GetInterval:
        mov     eax, [PIT_BASEUNIT]
        retn

;-------------------------------------------[ Exported: Timer.SetFowarder ]----
Timer_SetFowarder:
        mov [PIT_FOWARDER], eax
        retn

;-------------------------------------------[ Exported: Timer.GetFowarder ]----
Timer_GetFowarder:
        mov eax, [PIT_FOWARDER]
        retn

;----------------------------------------------------------[ IRQ0 HANDLER ]----
PIT_handler:
        push eax
        mov al, 0x20
        out 0x20, al
        sti

        ; Increase global counter
        inc dword [PIT_COUNT]

        mov eax, [FDC_MOTORCOUNTDOWN]
        test eax, eax
        jz .check_fdc_servicing

        dec eax
        mov [FDC_MOTORCOUNTDOWN], eax
        jmp short .bypass_fdc_service

.check_fdc_servicing:
        ; Check if FDC needs to be serviced
        test [FDC_STATUS], byte __FDC_MOTOR__
        jz .bypass_fdc_service

        ; Turning FDC motor off
        call FDC_motorOff

.bypass_fdc_service:
        mov eax, [PIT_FOWARDER]
        test eax, eax
        jz .bypass_fowarder

        call eax

.bypass_fowarder:
        pop eax
        iretd

%elifidn ModuleMode, variables

;------------------------------------------------------------------------------
; This value is the divider used for the timer tick frequency.  The tick freq
; can be calculated from 8MHz/value  A value of 0 is seen as 65536.  Only 16
; bits values are valid.
;-----------------
PIT_INTERVAL: dw 0


;------------------------------------------------------------------------------
; This is a pointer that can be used for an external routine to receive a
; a TICK notification.  Each time the timer is triggered, if this pointer is
; not equal to 0, this routine will be called.  The control should be returned
; using a "retn" instruction, and the EOI signal is must NOT be given.  The
; handler already took care of it.  All registers should be kept intact.
;-----------------
PIT_FOWARDER: dd 0
%endif
