[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "a_nask.nas"]

		GLOBAL	_api_putchar
		GLOBAL	_api_end, _api_putstr0
		GLOBAL	_api_openwin, _api_boxfilwin, _api_putstrwin

[SECTION .text]

_api_putchar:	; void api_putchar(int c);
		MOV		EDX,1
		MOV		AL,[ESP+4]		; c
		INT		0x40
		RET

_api_end:	; void api_end(void);
		MOV		EDX,4
		INT		0x40
_api_putstr0: ;void api_putstr0(char *s)
		PUSH 	EBX
		MOV 	EDX, 2
		MOV 	EBX, [ESP + 8]
		INT 	0x40
		POP 	EBX
		RET
_api_openwin: ; int api_openwin(char *buf, int xsize, int ysize, int colinv, char * title)
		PUSH 	EDI
		PUSH 	ESI
		PUSH 	EBX
		MOV 	EDX, 5
		MOV 	EBX,[ESP + 16]; BUF
		MOV 	ESI,[ESP + 20]
		MOV 	EDI,[ESP + 24]
		MOV 	EAX,[ESP + 28]
		MOV 	ECX,[ESP + 32]
		INT 	0X40
		POP		EBX
		POP 	ESI
		POP 	EDI
		RET
_api_putstrwin: ; int api_putstrwin(int win, int x, int y, int col, int len, char *str)
		PUSH 	EDI
		PUSH 	ESI
		PUSH 	EBP
		PUSH 	EBX
		MOV 	EDX, 6 ; 第六种api
		MOV 	EBX,[ESP + 20]; win
		MOV 	ESI,[ESP + 24];	x
		MOV 	EDI,[ESP + 28];	y
		MOV 	EAX,[ESP + 32];	col
		MOV 	ECX,[ESP + 36];	len
		MOV 	EBP,[ESP + 40];	str
		INT 	0X40
		POP		EBX
		POP 	EBP
		POP 	ESI
		POP 	EDI
		RET
_api_boxfilwin: ; int api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col)
		PUSH 	EDI
		PUSH 	ESI
		PUSH 	EBP
		PUSH 	EBX
		MOV 	EDX, 7 ; 第七种api
		MOV 	EBX,[ESP + 20]; win
		MOV 	EAX,[ESP + 24];	x0
		MOV 	ECX,[ESP + 28];	y0
		MOV 	ESI,[ESP + 32];	x1
		MOV 	EDI,[ESP + 36];	y1
		MOV 	EBP,[ESP + 40];	col
		INT 	0X40
		POP		EBX
		POP 	EBP
		POP 	ESI
		POP 	EDI
		RET

