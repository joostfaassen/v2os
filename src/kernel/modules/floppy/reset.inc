; Reset the FDC, by EKS (futur@mad.scientist.com)
; A proud member of the V2_OS Kernel Coder Team
;------------------------------------------------
; revision 0.1

        mov al, 0
        mov dx, __FDC_DOR__
        out dx, al
        push dx

        mov [FDC_MOTORCOUNTDOWN], byte 0
        and [FDC_STATUS], byte 0xFF-__FDC_MOTOR__

        ; program the data rate (500KB/s)
        mov dx, __FDC_DRS__
        out dx, al

        ; re-enable interrupts
        pop dx
        mov al, 0x0C
        out dx, al

        ; resetting triggered an interrupt, handle it
        or [FDC_STATUS], byte __FDC_DONE__
        mov al, 1
        call FDC_wait

        ; specify drive timings (got these off the BIOS)
        mov al, __FDC_CMD_SPECIFY__
        call FDC_sendbyte
        mov al, 0xDF    ; SRT = 3ms, HUT = 240ms
        call FDC_sendbyte
        mov al, 0x02    ; HLT = 16ms, ND = 0
        call FDC_sendbyte

        ; clear "disk change" status
        mov al, 1
        call FDC_Seek
        call FDC_recalibrate

        and [FDC_STATUS], byte 0xFF-__FDC_DISKCHANGE__
        retn
