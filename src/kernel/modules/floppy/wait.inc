; FDC wait, by EKS (futur@mad.scientist.com)
; A proud member of the V2_OS Kernel Coder Team
;----------------------------------------------
; revision 0.1

        mov ecx, [PIT_COUNT]
        add ecx, [PIT_BASEUNIT]

        ; wait for IRQ6 handler to signal command finished
.wait_irq:
        cmp ecx, [PIT_COUNT]
        jz .wait_expired
        test [FDC_STATUS], byte __FDC_DONE__
        jnz .wait_irq

        ; read in command result bytes
.wait_expired:
        push ecx
        xor ecx, ecx
        mov cl, 7
        mov edi, FDC_EXTENDED_STATUS
        mov dx, __FDC_MSR__
.getting_extended_status:
        in al, dx
        test al, 0x10
        jnz .no_more_status
        call FDC_getbyte
        stosb
        loop .getting_extended_status

.no_more_status:
        test [FDC_STATUS], byte __FDC_SENSEI__
        jz .bypass_sensei

        ; send a "sense interrupt status" command
        mov al, __FDC_CMD_SENSEI__
        call FDC_sendbyte
        call FDC_getbyte
        mov [FDC_SR0], al
        call FDC_getbyte
        mov [FDC_TRACK], al

.bypass_sensei:
        and [FDC_STATUS], byte 0xFF-__FDC_DONE__

        pop ecx
        test ecx, ecx
        jz .timed_out
        clc
        retn

.timed_out:
        ; check for diskchange
        mov dx, __FDC_DIR__
        in al, dx
        test al, 0x80
        jz .end_error
        or [FDC_STATUS], byte __FDC_DISKCHANGE__
.end_error:
        stc
        retn
