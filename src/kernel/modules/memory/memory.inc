; V2_OS memory managers
; Copyright (C) 2000, 2001  V2_OS Kernel Coders Team

; TODO: multithreading, better algorithms (balanced trees: AVL or RB?)

%ifidn KernelSection, init

; Initialize the memory management by freeing all memory that
; is to be managed. Currently, this is only the low 640 KB.

        mov al, 12h			; BIOS interrupt 12h, get memory size
        call [SysRealMode.Interrupt]	; ax returns the size of the contiguous
					; block that starts at 0
        mov ecx, [SysData.RMStruc]
        mov edi, mmFree			; start of free memory
        movzx ecx, word [ecx + RMRegs.ax]       ; retrieve ax value from int
        shl ecx, 10                             ; multiply KB size by 1024
        sub ecx, edi                            ; subtract memory used by the kernel
        call [SysMalloc.Free]                   ; free the memory to start managing it

; TODO: add the rest by freeing it
        
%endif
%ifidn KernelSection, calltable

dd Memory_Allocate
dd Memory_Free
dd Memory_Resize
dd Memory_GetInfo

dd Malloc_Allocate
dd Malloc_Free
dd Malloc_Resize

dd Memory_MallocFactory

%endif
%ifidn KernelSection, DWORDvariables

; Nodes are placed at the beginning of each free block.
; Free blocks must therefore be at least node_size bytes.
; This is achieved by aligning everything on 32 byte boundaries.

; In this temporary implementation, blocks are organized in two
; double-linked chains, one sorted by address, other by size.

                struc node
.sNext		resd 1		; node with next biggest size
.sPrev		resd 1		; node with next smallest size

.aNext		resd 1		; block with next biggest address
.aPrev		resd 1		; block with next smallest address

.Size		resd 1		; size of this free block
                endstruc

MemoryVars:
; The sentinel node is used to avoid checking for the end
; of a chain. Being a kernel variable, it has the smallest
; address and it has a Size value of 0ffffffffh. So any
; search will stop at it (address-based searches are top-down).

.null           istruc node

		at node.sNext,	dd .null	; used as start pointer
						; for size-based searches
		at node.sPrev,	dd .null

		at node.aNext,	dd .null
		at node.aPrev,	dd .null	; used as start pointer
						; for address-based searches
                at node.Size,   dd 0ffffffffh
                iend

%endif
%ifidn KernelSection, implementation

Memory_Allocate:
; in: eax = flags (must be 0 for now):
;           bits 0-2 = physical address limit:
;             000 = no restrictions
;             101 = the first 4 GB
;             110 = the first 16 MB
;             111 = the first 1 MB
;           bit 3 = alignment required, ebx is valid
;           bit 4 = bit 4 will be set in future Resize calls
;           bit 5 = bit 5 will be set in future Resize calls
;           bit 6 = block is going to be resized in the future
;     ebx = minimal alignment if bit 3 of eax is set:
;           lowest ebx bits of the address should be 0
;     ecx = minimal size of the block
; out: ecx = actual size of the block
;      edi = linear address of the block
;      cf set on error

	test eax, 70h ^ -1		; no flags implemented, but some
	jnz short .error		; can be ignored

	add ecx, byte 31		; round up to multiples of 32 bytes
	and ecx, byte 31 ^ -1
	jz short .error

; Search the block to use

	mov edi, MemoryVars.null	; start at the sentinel node
.loop1	mov edi, [edi + node.sNext]	; and search in order of increasing size
	cmp ecx, [edi + node.Size]
	ja short .loop1			; until requested size <= block size

	cmp edi, MemoryVars.null	; did we stop at the sentinel node?
	je short .error			; if yes, nothing found

	push eax
	push ebx
	push edx
	push esi

	mov ebx, [edi + node.sPrev]	; if no, remove from the size-sorted list
	mov edx, [edi + node.sNext]
	mov [ebx + node.sNext], edx
	mov [edx + node.sPrev], ebx

	mov ebx, [edi + node.aPrev]	; get the neighbours in the address-sorted
	mov edx, [edi + node.aNext]	; list for future use

	mov esi, [edi + node.Size]	; compute the size of remaining free space
	sub esi, ecx
	jz short .remove		; if nothing left, remove old node completely
	lea eax, [edi + ecx]		; otherwise, the remainder will take its place

	mov [ebx + node.aNext], eax	; the new block has the same position in the
	mov [edx + node.aPrev], eax	; address-sorted list as the old one
	mov [eax + node.aPrev], ebx	; fill in most data
	mov [eax + node.aNext], edx
	mov [eax + node.Size], esi

	mov ebx, edi			; start at the old node
.loop2	mov ebx, [ebx + node.sPrev]	; and search in order of decreasing size
	cmp esi, [ebx + node.Size]
	jnb .loop2			; until a smaller node is found
					; which becomes the new left neighbour
	mov edx, [ebx + node.sNext]	; get the right neioghbour
	mov [eax + node.sPrev], ebx	; and insert the remainder between them
	mov [ebx + node.sNext], eax
	mov [eax + node.sNext], edx
	mov [edx + node.sPrev], eax

.end	pop esi
	pop edx
	pop ebx
	pop eax
	clc
	ret

.error	stc
        ret

.remove	mov [ebx + node.aNext], edx
	mov [edx + node.aPrev], ebx
	jmp short .end

Memory_Free:
; in: ecx = size of the block
;     edi = linear address of the block
; out: ecx = actual size of freed memory

	push eax
	push ebx

	add ecx, byte 31		; round up to multiples of 32 bytes
	and ecx, byte 31 ^ -1
	push ecx			; this value will be returned

; Search where to insert the new free block

	mov eax, MemoryVars.null	; the .aPrev element points to the first node
.loop1	mov eax, [eax + node.aPrev]	; follow the chain, from high addresses to low
	cmp eax, edi			; we search a block that is below the new one
	jnb short .loop1

; Check if we can merge some blocks

	mov ebx, [eax + node.Size]	; size of the left neighbour (lower address)
	add ebx, eax			; end address = size + start
	cmp ebx, edi			; does the old block end where the new one starts?
	jne short .noMrgL		; if no, continue with the right neighbour
	add [eax + node.Size], ecx	; if yes, merge with the left neighbour
.MrgR	add ebx, ecx			; end address of the resulting block
	cmp ebx, [eax + node.aNext]	; test if the right neighbour can be merged too
	jne short .Sort			; if no, we're done merging
	mov ecx, [ebx + node.Size]	; ebx = right neighbour
	add [eax + node.Size], ecx	; if yes, merge the right neighbour into the left

; After both neighbours were merged, the right one is not an
; independent block anymore and must be removed from both lists.
; First, the address-sorted list.

					; eax already points to the left neighbour
	mov ecx, [ebx + node.aNext]	; get the new right neighbour
	mov [ecx + node.aPrev], eax	; update its backward pointer
	mov [eax + node.aNext], ecx	; and the forward pointer of the left neighbour

; Then, the size-sorted list, analogous to above, except that
; the left neighbour of the removed node is not the one in eax.

	mov ecx, [ebx + node.sNext]
	mov ebx, [ebx + node.sPrev]	; get the new left neighbour too
	mov [ecx + node.sPrev], ebx
	mov [ebx + node.sNext], ecx

; Restore the sort order in the size-sorted list by moving the merged block
; to a new position.
; First, remove it from the old position, just like above.

.Sort	mov ecx, [eax + node.sNext]
	mov ebx, [eax + node.sPrev]
	mov [ecx + node.sPrev], ebx
	mov [ebx + node.sNext], ecx

; Then, insert the block at a new position. Since its size increased,
; we only need to scan the right neighbours for one that's still bigger
; to find the position.

	mov ebx, eax			; start at the merged node
.Insert	mov ecx, [eax + node.Size]	; we search for a node larger than that
.loop2	mov ebx, [ebx + node.sNext]	; follow the chain
	cmp ecx, [ebx + node.Size]	; have we found one?
	ja short .loop2			; if no, continue searching

	mov ecx, [ebx + node.sPrev]	; get the new left neighbour
	mov [ebx + node.sPrev], eax	; update the right neighbour
	mov [ecx + node.sNext], eax	; update the left neighbour
	mov [eax + node.sPrev], ecx	; update the merged node
	mov [eax + node.sNext], ebx

	pop ecx
	pop ebx
	pop eax
        ret

; What if the left neighbour could not be merged?
; A new node is created at the start of the freed block.

; For now, the new block is also inserted into the chains
; and execution continues at MrgR above as if the node were
; a merged left neighbour.

.noMrgL mov [edi + node.Size], ecx	; set the size

; Insert into the address-sorted list.

	mov ebx, [eax + node.aNext]	; get the right neighbour
	mov [edi + node.aPrev], eax
	mov [edi + node.aNext], ebx
	mov [ebx + node.aPrev], edi	; update the neighbours
	mov [eax + node.aNext], edi

; Insert into the size-sorted list.
; Just insert at the start, it will be moved anyway.

	mov eax, [MemoryVars.null + node.sNext]	; update the null node
	mov [MemoryVars.null + node.sNext], edi
	mov [edi + node.sPrev], dword MemoryVars.null	; update the new node
	mov [edi + node.sNext], eax

; Prepare registers and resume execution above.

	mov eax, edi	; "merged block" is the new node
	mov ebx, edi	; same as eax
	jmp short .MrgR


Memory_Resize:
; in: eax = flags:
;           bits 0-2 = physical address limit:
;             000 = no restrictions
;             101 = the first 4 GB
;             110 = the first 16 MB
;             111 = the first 1 MB
;           bit 3 = alignment required, ebx is valid
;           bit 4 = moving the block not allowed
;           bit 5 = change the lower end of the block
;     ebx = minimal alignment if bit 3 of eax is set:
;           lowest ebx bits of the address should be 0
;     ecx = old size of the block
;     esi = new size of the block
;     edi = linear address of the block
; out: ecx = actual new size of the block
;      edi = new linear address of the block
;      cf set on error

; TODO: check when growing doesn't require reallocation

	test eax, 32 ^ -1	; only bit 5 implemented
	jnz short .error

; compute the actual size values (round up)
	push esi		; stack: esi
	add esi, byte 31
	add ecx, byte 31
	and esi, byte 31 ^ -1
	and ecx, byte 31 ^ -1

; do we need to reallocate?
	cmp esi, ecx
	jna .shrink

; allocate a new block
	push edi		; stack: old block, esi
	xchg ecx, esi		; ecx = new size, esi = old size
	call Memory_Allocate	; edi = new block

; move the data
	push edi		; stack: new block, old block, esi
	or al, al
	jz short .plain
	add edi, ecx		; edi = end of new block
	sub edi, esi		; edi = start of old data in the new block
.plain	xchg ecx, esi		; ecx = old size, esi = new size
	xchg esi, [esp + 4]; esi = old block, stack: new block, new size, esi
	push ecx		; stack: old size, new block, new size, esi
	shr ecx, 2		; ecx = count of DWORDs to copy
	rep movsd		; ecx = 0, esi = end of old block, edi corrupted

; free the old block
	pop ecx			; ecx = old size, stack: new block, new size, esi
	sub esi, ecx		; esi = old block
	mov edi, esi		; edi = old block
	call Memory_Free
	pop edi			; edi = new block, stack: new size, esi
	pop ecx			; ecx = new size, stack: esi
	pop esi			; esi = esi, stack: ok
	clc
	ret

; shrinking: free the unneeded part
.shrink	sub ecx, esi		; ecx = size difference
	or al, al		; works because only bit 5 is implemented
	jz short .normal
	call Memory_Free
	add edi, ecx		; edi = not freed part
	jmp short .exit

.normal	add edi, esi		; edi = unneeded part
	call Memory_Free
	sub edi, esi		; edi = old block
.exit	mov ecx, esi		; ecx = new size
	pop esi			; esi = esi, stack: ok
	clc
	ret

.error  stc
        ret

Memory_GetInfo:
; out: eax = total free memory
;      ecx = largest free block

        ret

Malloc_Allocate:
; in: ecx = size of the block
; out: edi = linear address of the block
;      cf set on error

	push eax
	push ecx
	xor eax, eax
	call Memory_Allocate
	pop ecx
	pop eax
        ret

Malloc_Free:
; in: ecx = size of the block
;     edi = linear address of the block

	push ecx
	call Memory_Free
	pop ecx
        ret

Malloc_Resize:
; in: ecx = old size of the block
;     esi = new size of the block
;     edi = linear address of the block
; out: edi = new linear address of the block
;      cf set on error

	push eax
	push ecx
	xor eax, eax
	call Memory_Resize
	pop ecx
	pop eax
        ret

%endif

; *****************
; * MallocFactory *
; *****************

%ifidn KernelSection, DWORDvariables

                struc iOMalloc, iUnknown
.BlockSize      resd 1
.FirstBlock     resd 1
.LastBlock      resd 1
.ObjectSize     resd 1
.NextObject     resd 1
.RefCount       resd 1
                endstruc

OMalloc_Malloc  dd OMalloc_QueryInterface, Dummy, Dummy
                dd 1000h, 0, 0, iOMalloc_size, 0

OMalloc_Calltable:
                dd OMalloc_Allocate, OMalloc_Free

%endif
%ifidn KernelSection, init

        mov ecx, 1000h
        call [SysMalloc.Allocate]
        jc near OutOfMemory
        mov [OMalloc_Malloc + iOMalloc.FirstBlock], edi
        mov [OMalloc_Malloc + iOMalloc.LastBlock], edi
        mov [OMalloc_Malloc + iOMalloc.NextObject], edi
        mov [edi + 0ffch], dword 0
        lea eax, [edi + iOMalloc_size]
        mov ecx, (1000h - 4) / iOMalloc_size - 1
OMalloc_loop:
        mov [edi], eax
        add eax, byte iOMalloc_size
        add edi, byte iOMalloc_size
        loop OMalloc_loop
        mov [edi], dword 0

%endif
%ifidn KernelSection, implementation    

Memory_MallocFactory:
; in: eax = size of objects that will be allocated
;     ecx = size of blocks to allocate from parent Malloc
; out: esi:edi = new IMalloc
;      cf set on error

	push ecx
        call [SysMalloc.Allocate]
        jc short .error
        push edx
        push edi
        mov esi, ecx
        mov ecx, iOMalloc_size
        mov edx, OMalloc_Malloc
        call OMalloc_Allocate
        jc short .error2
	xchg ebx, [esp]
        mov [edi + iUnknown.QueryInterface], dword OMalloc_QueryInterface
        mov [edi + iUnknown.AddRef], dword OMalloc_AddRef
        mov [edi + iUnknown.Release], dword OMalloc_Release
        mov [edi + iOMalloc.BlockSize], esi
        mov [edi + iOMalloc.FirstBlock], ebx
        mov [edi + iOMalloc.LastBlock], ebx
        mov [edi + iOMalloc.ObjectSize], eax
        mov [edi + iOMalloc.NextObject], ebx
        sub esi, 4				; esi = BlockSize - 4
        mov [edi + iOMalloc.RefCount], dword 1
        sub esi, eax				; esi = BlockSize - ObjectSize - 4
        lea edx, [ebx + eax]			; edx = second object
        sub esi, eax				; esi = BlockSize - 2*ObjectSize - 4
.loop   mov [ebx], edx
        add edx, eax
        add ebx, eax
        sub esi, eax
        jnb short .loop
	mov [ebx], dword 0
        pop ebx
        mov esi, OMalloc_Calltable
        pop edx
	clc
.error	pop ecx
	ret

.error2 pop edi
        mov ecx, esi
        call [SysMalloc.Free]
        pop edx
	pop ecx
	stc
        ret


OMalloc_QueryInterface:
; in: eax = IID
;     edx = instance
; out: esi:edi = new interface
;      cf set on error

        cmp eax, MallocIID
        jne short .error
        mov esi, OMalloc_Calltable
        mov edi, edx
        ret

.error  stc
        ret


OMalloc_AddRef:
; in: edx = instance

        inc dword [edx + iOMalloc.RefCount]
OMalloc_ret:
        ret


OMalloc_Release:
; in: edx = instance

        dec dword [edx + iOMalloc.RefCount]
        jnz short OMalloc_ret
        push eax
        push ebx
        push ecx
        push edi
        mov eax, [edx + iOMalloc.FirstBlock]
        mov ebx, [edx + iOMalloc.LastBlock]
        mov ecx, [edx + iOMalloc.BlockSize]
.loop   mov edi, eax
        mov eax, [eax + ecx - 4]
        call [SysMalloc.Free]
        cmp edi, ebx
        jne short .loop
        mov edi, edx
        mov edx, OMalloc_Malloc
        mov ecx, iOMalloc_size
        call OMalloc_Free
        mov edx, edi
        pop edi
        pop ecx
        pop ebx
        pop eax
        ret


OMalloc_Allocate:
; in: ecx = size of the block
;     edx = instance
; out: edi = linear address of the block
;      cf set on error

        cmp ecx, [edx + iOMalloc.ObjectSize]
        jne short .error
        mov edi, [edx + iOMalloc.NextObject]
        mov ecx, [edi]
        or ecx, ecx
        jz short .new
        mov [edx + iOMalloc.NextObject], ecx
        mov ecx, [edx + iOMalloc.ObjectSize]
	clc
        ret

.new    push edi
	mov ecx, [edx + iOMalloc.BlockSize]
        call [SysMalloc.Allocate]
        jc short .error2
        push eax
        mov eax, [edx + iOMalloc.LastBlock]
        mov [eax + ecx - 4], edi
        lea eax, [ecx - 4]
        mov ecx, [edx + iOMalloc.ObjectSize]
        push ebx
        sub eax, ecx
        mov [edx + iOMalloc.LastBlock], edi
        lea ebx, [edi + ecx]
        mov [edx + iOMalloc.NextObject], edi
        sub eax, ecx
.loop   mov [edi], ebx
        add ebx, ecx
        add edi, ecx
        sub eax, ecx
        jnb short .loop
        mov [edi], dword 0
        pop ebx
        pop eax
        pop edi
        clc
        ret

.error2 mov ecx, [edx + iOMalloc.ObjectSize]
	pop edi
.error  stc
        ret
        

OMalloc_Free:
; in: ecx = size of the block
;     edx = instance
;     edi = linear address of the block

        push eax
        mov eax, [edx + iOMalloc.NextObject]
        mov [edx + iOMalloc.NextObject], edi
        mov [edi], eax
        pop eax
        ret

%endif
