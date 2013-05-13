@echo off

if not %1_==clean_ goto notclean
  del boot.lst
  del kernel.lst
  del boot.bin
  del kernel.bin
  del V2_OS.img
  goto quit
:notclean

cd boot

if %1_==debug_ goto debug
if %2_==debug_ goto debug
  nasm boot.asm -o ..\boot.bin -w+orphan-labels
  cd ..\kernel
  nasm kernel.asm -o ..\kernel.bin -w+orphan-labels -i ..\sdk\
  goto endif
:debug
  nasm boot.asm -o ..\boot.bin -w+orphan-labels -l ..\boot.lst
  cd ..\kernel
  nasm kernel.asm -o ..\kernel.bin -w+orphan-labels -i ..\sdk\ -l ..\kernel.lst
:endif

cd ..
copy /b boot.bin+kernel.bin V2_OS.img

if not %1_==install_ goto notinstall
rem  echo Insert a floppy into drive A:
rem  pause
  fdimage -qv V2_OS.img a:
:notinstall

pause
:quit