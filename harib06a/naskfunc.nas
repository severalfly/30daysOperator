; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; ƒIƒuƒWƒFƒNƒgƒtƒ@ƒCƒ‹‚ðì‚éƒ‚[ƒh	
[INSTRSET "i486p"]				; 486‚Ì–½—ß‚Ü‚ÅŽg‚¢‚½‚¢‚Æ‚¢‚¤‹Lq
[BITS 32]						; 32ƒrƒbƒgƒ‚[ƒh—p‚Ì‹@ŠBŒê‚ðì‚ç‚¹‚é
[FILE "naskfunc.nas"]			; ƒ\[ƒXƒtƒ@ƒCƒ‹–¼î•ñ

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL	_load_tr
		GLOBAL	_asm_inthandler20, _asm_inthandler21
		GLOBAL	_asm_inthandler27, _asm_inthandler2c
		GLOBAL	_memtest_sub
		GLOBAL	_taskswitch3, _taskswitch4
		EXTERN	_inthandler20, _inthandler21
		EXTERN	_inthandler27, _inthandler2c

[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS ‚Æ‚¢‚¤ˆÓ–¡
		POP		EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS ‚Æ‚¢‚¤ˆÓ–¡
		RET

_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

_load_tr:		; void load_tr(int tr);
		LTR		[ESP+4]			; tr
		RET

_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD


; 以下为memtest_sub 实现代码，因为编译器会自动优化，所以此检查内存代码只能使用汇编语言写，此处拒绝优化
_memtest_sub:	; unsigined int memtest_sub(unsigned int start, unsigned int end)
		PUSH EDI
		PUSH ESI
		PUSH EBX
		MOV ESI, 0xaa55aa55
		MOV EDI, 0x55aa55aa
		MOV EAX,[ESP+12+4] ; i = start
mts_loop:
		MOV 		EBX, EAX
		ADD 		EBX, 0xffc
		MOV		EDX, [EBX]
		MOV		[EBX], ESI
		XOR		DWORD [EBX], 0xffffffff
		CMP		EDI, [EBX]
		JNE		mts_fin
		XOR		DWORD [EBX], 0xffffffff
		CMP		ESI, [EBX]
		JNE		mts_fin
		MOV		[EBX], EDX
		ADD 		EAX, 0x1000
		CMP		EAX, [ESP + 12 +8]

		JBE 		mts_loop
		POP 		EBX
		POP 		ESI
		POP 		EDI
		RET
mts_fin:
		MOV 		[EBX], EDX
		POP 		EBX
		POP 		ESI
		POP 		EDI
		RET

_taskswitch3:	; void taskswitch3(void);
		JMP		3*8:0
		RET

_taskswitch4:	; void taskswitch4(void);
		JMP		4*8:0
		RET


