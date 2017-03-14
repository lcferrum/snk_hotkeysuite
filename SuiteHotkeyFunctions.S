.file "SuiteHotkeyFunctions.S"
.text                                 /* Code section */
#ifndef _WIN64
/* Make functions visible to linker, STDCALL name mangling */
.globl _SinglePressCtrlAltEventHandler@12
.globl _SinglePressShiftAltEventHandler@12
.globl _SinglePressCtrlShiftEventHandler@12
/* Defining symbols, COFF format, storage-class 2 (external), attribute 32 (function) */
.def _SinglePressCtrlAltEventHandler@12;   .scl 2; .type 32; .endef
.def _SinglePressShiftAltEventHandler@12;  .scl 2; .type 32; .endef
.def _SinglePressCtrlShiftEventHandler@12; .scl 2; .type 32; .endef
    
_SinglePressCtrlAltEventHandler@12:
	/* 
	Function prototype is bool __stdcall KeyPressEventHandler(this, WPARAM, KBDLLHOOKSTRUCT*)
	4 DWORDs are passed on the stack (ESP offsets w/ pushed EBX): 
	 0x04 Return address
	 0x08 "this" pointer to KeyTriplet
	 0x0C wParam
	 0x10 pointer to KBDLLHOOKSTRUCT
	KeyTriplet:
	 0x00 hk_sc
	 0x04 hk_ext
	 0x08 hk_state
	 0x0C hk_engaged
	 0x10 hk_down_tick
	KBDLLHOOKSTRUCT:
	 0x00 vkCode;
	 0x04 scanCode;
	 0x08 flags;
	 0x0C time;
	 0x10 dwExtraInfo;
	*/
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=0 (used as bool return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, CTRL or ALT (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x1D, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag0 */
	je flag0
	cmpl $0x38, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je flag1
	popl %ebx
	ret $12                           /* Clean up call stack (12 bytes) conforming to STDCALL convention and return EAX */
_SinglePressShiftAltEventHandler@12:
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=0 (used as bool return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, SHIFTR or ALT (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je flag0
	cmpl $0x36, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je flag0
	cmpl $0x38, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je flag1
	popl %ebx
	ret $12                           /* Clean up call stack (12 bytes) conforming to STDCALL convention and return EAX */
_SinglePressCtrlShiftEventHandler@12:
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=0 (used as bool return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, CTRL or SHIFTR (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je flag0
	cmpl $0x1D, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag1 */
	je flag1
	cmpl $0x36, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je flag0
	popl %ebx
	ret $12                           /* Clean up call stack (12 bytes) conforming to STDCALL convention and return EAX */
test_ext:                             /* KBDLLHOOKSTRUCT.scanCode was matched with third key (KeyTriplet.hk_sc) - test if extended-key flags match */
	movl 0x08(%edx), %edx             /* Move KBDLLHOOKSTRUCT.flags to EDX */
	andl $0x01, %edx                  /* EDX=KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED */
	cmpl 0x04(%ebx), %edx             /* If KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED!=KeyTriplet.hk_ext JMP exit */
	jne exit
	movl $2, %ecx                     /* ECX=010b (HK_FLAG) */
flag1:                                /* KBDLLHOOKSTRUCT.scanCode was matched with second key */
	addl %ecx, %ecx                   /* Double ECX (HK_FLAG) - same as shifting left by 1, but may be faster on P4 */ 
flag0:                                /* KBDLLHOOKSTRUCT.scanCode was matched with first key */
	cmpl $0x0101, 0x0C(%esp)          /* If wParam==WM_KEYUP JMP key_up */
	je key_up
	cmpl $0x0105, 0x0C(%esp)          /* If wParam==WM_SYSKEYUP JMP key_up */
	je key_up
	orl %ecx, 0x08(%ebx)              /* KeyTriplet.hk_state|=HK_FLAG */
	cmpl $0x07, 0x08(%ebx)            /* If KeyTriplet.hk_state==111b JMP sp_eng */
	je sp_eng
	popl %ebx
	ret $12
sp_eng:                               /* Single hotkey press occured */
	movl $1, %eax                     /* EAX=1 (used as bool return value) */
	cmpl $1, 0x0C(%ebx)               /* If KeyTriplet.hk_engaged==1 JMP exit */
	je exit
	movl $1, 0x0C(%ebx)               /* KeyTriplet.hk_engaged=1 */
	/* Call Shit */
	popl %ebx
	ret $12
key_up:                               /* WM_KEYUP/WM_SYSKEYUP event */
	notl %ecx
	andl %ecx, 0x08(%ebx)             /* KeyTriplet.hk_state&=~HK_FLAG */
	movl $0, 0x0C(%ebx)               /* KeyTriplet.hk_engaged=0 */
exit:
	popl %ebx
	ret $12
#endif