; Copyright (C) 2000, 2001  V2_OS Kernel Coders Team

%ifidn KernelSection, calltable
  %include "modules/unknown/unknown.inc"
  %include "modules/servers/servers.inc"
  %include "modules/memory/memory.inc"
;  %include "modules/tstream/tstream.inc"
  %include "modules/hardware/hardware.inc"
  %include "modules/filters/filters.inc"
  %include "modules/dir/dir.inc"
  %include "modules/hooks/hooks.inc"
  %include "modules/command/command.inc"
  %include "modules/keyboard/keyboard.inc"
  %include "modules/segments/segments.inc"
  %include "modules/ints/ints.inc"
  %include "modules/threads/threads.inc"
  %include "modules/realmode/realmode.inc"
%else
  %include "modules/segments/segments.inc"
  %include "modules/ints/ints.inc"
  %include "modules/realmode/realmode.inc"      ; finished
  %include "modules/memory/memory.inc"          ; only IMalloc and IMallocFactory work
						; and only for the first 640 KB
;  %include "modules/buffer/buffer.inc"         ; finished, instantiation unclear
  %include "modules/hooks/hooks.inc"            ; finished
  %include "modules/threads/threads.inc"
  %include "modules/unknown/unknown.inc"        ; finished, must be updated all the time
  %include "modules/servers/servers.inc"
  %include "modules/hardware/hardware.inc"
  %include "modules/filters/filters.inc"        ; finished, outdated
  %include "modules/display/display.inc"        ; Write works
  %include "modules/keyboard/keyboard.inc"
;  %include "modules/tstream/tstream.inc"
  %include "modules/storage/storage.inc"
  %include "modules/dir/dir.inc"
  %include "modules/command/command.inc"
%endif
