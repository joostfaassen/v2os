; Programmable Interrupt controller interface

%ifidn ModuleMode, init

%elifidn ModuleMode, implementation

Pic_calltable:
dd PIC_SetMask
dd PIC_SetMaskBit
dd PIC_ClearMaskBit
dd PIC_SetRMBase
dd PIC_SetPMBase

PIC_SetMask:
PIC_SetMaskBit:
PIC_ClearMaskBit:
PIC_SetRMBase:
PIC_SetPMBase:

%endif
