%ifidn KernelSection, init

%endif
%ifidn KernelSection, calltable

istruc ICommand
iend

dd Command_Hook

%endif
%ifidn KernelSection, DWORDvariables

Command_Hook:
.Next           dd $
.Prev           dd $ - 4
.Callback       dd Dummy
.Mutex          dd 1

%endif
%ifidn KernelSection, implementation

%endif
