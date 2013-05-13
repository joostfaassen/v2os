; Get byte from FDC, by EKS (futur@mad.scientist.com)
; A proud member of the V2_OS Kernel Coder Team
;----------------------------------------------------
; revision 0.1

        mov cl, 128

        mov dx, __FDC_MSR__
.waiting:
        in al, dx
        and al, 0xD0
        cmp al, 0xD0
        jz .end
        in al, 0x80     ; inserts a delay
        dec cl
        jnz .waiting
        stc
        retn

.end:
        mov dx, __FDC_DATA__
        in al, dx
        clc
        retn
