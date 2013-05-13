; Floppy Drive Controller interface (NEC 765 and compatibles)
; by EKS (futur@mad.scientist.com)
;---------------------------------
; revision 0.1

%ifidn ModuleMode, init


%elifidn ModuleMode, implementation

Floppy_calltable:
dd FDC_GetInfo
dd FDC_Read
dd FDC_Write
dd FDC_Seek
dd FDC_FormatTrack
dd FDC_SetOffDelay
dd FDC_WakeupDevice
dd FDC_reset
dd FDC_motorOff
dd FDC_motorOn

; FDC related ports
%define __FDC_DOR__             0x3F2
%define __FDC_MSR__             0x3F4
%define __FDC_DRS__             0x3F4
%define __FDC_DATA__            0x3F5
%define __FDC_DIR__             0x3F7
%define __FDC_CCR__             0x3F7

; FDC related command values
%define __FDC_CMD_FORMAT__      0x4D
%define __FDC_CMD_READ__        0xE6
%define __FDC_CMD_RECAL__       0x07
%define __FDC_CMD_SEEK__        0x0F
%define __FDC_CMD_SENSEI__      0x08
%define __FDC_CMD_SPECIFY__     0x03
%define __FDC_CMD_VERSION__     0x10
%define __FDC_CMD_WRITE__       0xC5


; Default starting countdown value, 40/18.2 ticks makes it 2 seconds
%define __FDC_COUNTDOWN__       40

; Status bits
%define __FDC_MOTOR__           0x01
%define __FDC_DISKCHANGE__      0x02
%define __FDC_DONE__            0x04
%define __FDC_SENSEI__          0x08


FDC_Read:
        retn
FDC_Write:
        retn
FDC_GetInfo:
        retn
FDC_Seek:
        retn
FDC_FormatTrack:
        retn

FDC_SetOffDelay:
        mov [FDC_COUNTDOWNSTART], eax
        retn

FDC_WakeupDevice:
        pushad
        call FDC_motorOn
        popad
        retn

FDC_reset:
        call FDC_motorOff
        retn

FDC_wait:
%include "modules/floppy/wait.inc"

FDC_motorOff:
%include "modules/floppy/motoroff.inc"

FDC_motorOn:
%include "modules/floppy/motoron.inc"

FDC_init:
%include "modules/floppy/reset.inc"

FDC_sendbyte:
%include "modules/floppy/sendbyte.inc"

FDC_getbyte:
%include "modules/floppy/getbyte.inc"

FDC_recalibrate:
        retn



; bit 0: motor status, 0=off, 1=on
; bit 7-1: reserved
FDC_STATUS: db 0+ __FDC_MOTOR__
FDC_EXTENDED_STATUS: Times 7 db 0

FDC_SR0: db 0
FDC_TRACK: db 0
FDC_HEAD: db 0
FDC_SECTOR: db 0

FDC_MOTORCOUNTDOWN: dd __FDC_COUNTDOWN__
FDC_COUNTDOWNSTART: dd __FDC_COUNTDOWN__

%endif
