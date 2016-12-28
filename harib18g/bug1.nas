[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
	EXTERN	_api_putchar
	EXTERN	_api_end
[FILE "bug1.c"]
[SECTION .text]
	GLOBAL	_HariMain
_HariMain:
	PUSH	EBP
	MOV	EBP,ESP
	SUB	ESP,112
	PUSH	97
	MOV	BYTE [-102+EBP],97
	CALL	_api_putchar
	PUSH	98
	MOV	BYTE [-10+EBP],98
	CALL	_api_putchar
	PUSH	99
	MOV	BYTE [11+EBP],99
	CALL	_api_putchar
	CALL	_api_end
	LEAVE
	RET
