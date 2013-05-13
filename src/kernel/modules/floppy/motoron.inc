; Floppy Drive Controller: Motor On
; by EKS (futur@mad.scientist.com)
;----------------------------------
; revision 0.1

        test [FDC_STATUS], byte __FDC_MOTOR__
        jnz .exit
        or [FDC_STATUS], byte __FDC_MOTOR__
        mov al, 0x1C
        mov dx, __FDC_DOR__
        out dx, al
        mov eax, [FDC_COUNTDOWNSTART]
        mov [FDC_MOTORCOUNTDOWN], eax
.exit:
        retn
