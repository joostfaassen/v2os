V2_OS.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > V2_OS.img

# Ph33r my 31337 Makefile sk1LLz ;-)
modules := $(foreach inc,$(subst ",,$(sort $(filter "%.inc",$(shell grep '^[^;]' kernel/modules.inc)))),kernel/$(inc))

-include $(sort $(foreach inc,$(modules),$(dir $(inc))Makefile))

boot.bin: boot/boot.asm
	cd boot; nasm boot.asm -o../boot.bin -w+orphan-labels; cd ..

kernel.bin: kernel/kernel.asm kernel/modules.inc sdk/sdk.inc $(modules)
	cd kernel; nasm kernel.asm -o../kernel.bin -w+orphan-labels -i ../sdk/; cd ..

install: V2_OS.img FORCE
	dd if=v2_os.img of=/dev/fd0

clean: FORCE
	rm -f boot.bin kernel.bin V2_OS.img

FORCE:
