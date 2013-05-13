; Send byte to FDC, by EKS (futur@mad.scientist.com)
; A proud member of the V2_OS Kernel Coder Team
;---------------------------------------------------
; revision 0.1

        push eax
        mov cl, 128

        mov dx, __FDC_MSR__
.waiting:
        in al, dx
        and al, 0xC0
        cmp al, 0x80
        jz .end
        in al, 0x80     ; inserts a delay
        dec cl
        jnz .waiting
        pop eax
        stc
        retn

.end:
        mov dx, __FDC_DATA__
        pop eax
        out dx, al
        clc
        retn
