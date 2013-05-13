; Floppy Drive Controller: Motor Off
; by EKS (futur@mad.scientist.com)
;-----------------------------------
; revision 0.1

        test [FDC_STATUS], byte __FDC_MOTOR__
        jz .exit
        and [FDC_STATUS], byte 0xFF-__FDC_MOTOR__
        mov al, 0x0C
        mov dx, __FDC_DOR__
        out dx, al
.exit:
        retn
