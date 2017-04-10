.file "AsmHotkeyFunctions.S"
.text                                 /* Code section */
#define LONG_PRESS_DURATION 2500      /* 2.5 sec (keep in mind that this define is compared with derivative of GetTickCount() that can have resolution up to 55ms) */
#ifndef _WIN64
/* Make functions visible to linker, CDECL name mangling */
.globl _SinglePressCtrlAltEventHandler
.globl _SinglePressShiftAltEventHandler
.globl _SinglePressCtrlShiftEventHandler
.globl _LongPressCtrlAltEventHandler
.globl _LongPressShiftAltEventHandler
.globl _LongPressCtrlShiftEventHandler
/* Defining symbols, COFF format, storage-class 2 (external), attribute 32 (function) */
.def _SinglePressCtrlAltEventHandler;   .scl 2; .type 32; .endef
.def _SinglePressShiftAltEventHandler;  .scl 2; .type 32; .endef
.def _SinglePressCtrlShiftEventHandler; .scl 2; .type 32; .endef
.def _LongPressCtrlAltEventHandler;     .scl 2; .type 32; .endef
.def _LongPressShiftAltEventHandler;    .scl 2; .type 32; .endef
.def _LongPressCtrlShiftEventHandler;   .scl 2; .type 32; .endef
.def __imp__CreateProcessW@40;          .scl 2; .type 32; .endef
.def __imp__CloseHandle@4;              .scl 2; .type 32; .endef
    
_LongPressCtrlAltEventHandler:
	/* 
	Function prototype is bool __cdecl KeyPressEventHandler(KeyTriplet*, WPARAM, KBDLLHOOKSTRUCT*)
	4 DWORDs are passed on the stack (ESP offsets w/ pushed EBX): 
	 0x04 Return address
	 0x08 Pointer to KeyTriplet
	 0x0C wParam
	 0x10 Pointer to KBDLLHOOKSTRUCT
	KeyTriplet:
	 0x00 hk_sc
	 0x04 hk_ext
	 0x08 hk_state
	 0x0C hk_engaged
	 0x10 hk_down_tick
	 0x14 hk_cmdline_s
	 0x18 hk_cmdline_l
	 0x1C hk_pi
	 0x2C hk_si
	KBDLLHOOKSTRUCT:
	 0x00 vkCode
	 0x04 scanCode
	 0x08 flags
	 0x0C time
	 0x10 dwExtraInfo
	*/
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, CTRL or ALT (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je lp_test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x1D, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag0 */
	je lp_flag0
	cmpl $0x38, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je lp_flag1
	popl %ebx
	ret                               /* Return EAX (conforming to CDECL convention, stack is cleared by caller) */
_LongPressShiftAltEventHandler:
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, SHIFTR or ALT (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je lp_test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je lp_flag0
	cmpl $0x36, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je lp_flag0
	cmpl $0x38, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je lp_flag1
	popl %ebx
	ret                               /* Return EAX (conforming to CDECL convention, stack is cleared by caller) */
_LongPressCtrlShiftEventHandler:
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, CTRL or SHIFTR (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je lp_test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je lp_flag0
	cmpl $0x1D, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag1 */
	je lp_flag1
	cmpl $0x36, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je lp_flag0
	popl %ebx
	ret                               /* Return EAX (conforming to CDECL convention, stack is cleared by caller) */
lp_test_ext:                          /* KBDLLHOOKSTRUCT.scanCode was matched with third key (KeyTriplet.hk_sc) - test if extended-key flags match */
	movl 0x08(%edx), %ecx             /* Move KBDLLHOOKSTRUCT.flags to ECX */
	andl $0x01, %ecx                  /* ECX=KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED */
	cmpl 0x04(%ebx), %ecx             /* If KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED!=KeyTriplet.hk_ext JMP lp_exit_default */
	jne lp_exit_default
	movl $2, %ecx                     /* ECX=010b (HK_FLAG) */
lp_flag1:                             /* KBDLLHOOKSTRUCT.scanCode was matched with second key */
	addl %ecx, %ecx                   /* Double ECX (HK_FLAG) - same as shifting left by 1, but may be faster on P4 */ 
lp_flag0:                             /* KBDLLHOOKSTRUCT.scanCode was matched with first key */
	cmpl $0x0101, 0x0C(%esp)          /* If wParam==WM_KEYUP JMP lp_key_up */
	je lp_key_up
	cmpl $0x0105, 0x0C(%esp)          /* If wParam==WM_SYSKEYUP JMP lp_key_up */
	je lp_key_up
	orl %ecx, 0x08(%ebx)              /* KeyTriplet.hk_state|=HK_FLAG */
	cmpl $0x07, 0x08(%ebx)            /* If KeyTriplet.hk_state==111b JMP lp_engaged */
	je lp_engaged
	popl %ebx
	ret    
lp_engaged:                           /* Hotkey press occured */
	movl $1, %eax                     /* EAX=true (used as return value) */
	cmpl $1, 0x0C(%ebx)               /* If KeyTriplet.hk_engaged!=1 JMP lp_set_time */
	jne lp_set_time
	popl %ebx
	ret
lp_set_time:
	movl $1, 0x0C(%ebx)               /* KeyTriplet.hk_engaged=1 */ 
	movl 0x0C(%edx), %ecx             /* Copy KBDLLHOOKSTRUCT.time to KeyTriplet.hk_down_tick */  
	movl %ecx, 0x10(%ebx)
	popl %ebx
	ret
lp_key_up:
	notl %ecx
	andl %ecx, 0x08(%ebx)             /* KeyTriplet.hk_state&=~HK_FLAG */
	cmpl $1, 0x0C(%ebx)               /* If KeyTriplet.hk_engaged==1 JMP lp_release */
	je lp_release
	popl %ebx
	ret
lp_release:
	movl $0, 0x0C(%ebx)               /* KeyTriplet.hk_engaged=0 */
	movl 0x0C(%edx), %ecx             /* ECX=KBDLLHOOKSTRUCT.time */  
	subl 0x10(%ebx), %ecx             /* If KBDLLHOOKSTRUCT.time<KeyTriplet.hk_down_tick JMP lp_exit_default - excludes moment of system timer wrap around after 49.7 days of uptime */
	jl lp_exit_default
	cmpl $LONG_PRESS_DURATION, %ecx   /* If KBDLLHOOKSTRUCT.time-KeyTriplet.hk_down_tick>LONG_PRESS_DURATION ECX=KeyTriplet.hk_cmdline_l else ECX=KeyTriplet.hk_cmdline_s */
	movl 0x18(%ebx), %ecx
	jg lp_longpress
	movl 0x14(%ebx), %ecx
lp_longpress:
	/* Push CreateProcess parameters to stack */
	leal 0x1C(%ebx), %eax
	pushl %eax                        /* lpProcessInformation=KeyTriplet.hk_pi */
	leal 0x2C(%ebx), %eax
	pushl %eax                        /* lpStartupInfo=KeyTriplet.hk_si */
	pushl $0                          /* lpCurrentDirectory=NULL */
	pushl $0                          /* lpEnvironment=NULL */
	pushl $0x80                       /* dwCreationFlags=HIGH_PRIORITY_CLASS */
	pushl $0                          /* bInheritHandles=FALSE */
	pushl $0                          /* lpThreadAttributes=NULL */
	pushl $0                          /* lpProcessAttributes=NULL */
	pushl %ecx                        /* lpCommandLine=ECX (cmdline pointer) */
	pushl $0                          /* lpApplicationName=NULL */
	call *__imp__CreateProcessW       /* Call CreateProcess - it's STDCALL function and callee clears the stack */
	cmpl $0, %eax
	je lp_exit_default                /* If CreateProcess returned 0 JMP lp_exit_default (function will return false in this case because EAX=0) */
	pushl 0x1C(%ebx)
	call *__imp__CloseHandle          /* Call CloseHandle(KeyTriplet.hk_pi.hProcess) - STDCALL */
	pushl 0x20(%ebx)
	call *__imp__CloseHandle          /* Call CloseHandle(KeyTriplet.hk_pi.hThread) - STDCALL */
	xorl %eax, %eax                   /* EAX=false (used as return value) */
lp_exit_default:
	popl %ebx
	ret	

_SinglePressCtrlAltEventHandler:
	/* 
	Function prototype is bool __cdecl KeyPressEventHandler(KeyTriplet*, WPARAM, KBDLLHOOKSTRUCT*)
	4 DWORDs are passed on the stack (ESP offsets w/ pushed EBX): 
	 0x04 Return address
	 0x08 Pointer to KeyTriplet
	 0x0C wParam
	 0x10 Pointer to KBDLLHOOKSTRUCT
	KeyTriplet:
	 0x00 hk_sc
	 0x04 hk_ext
	 0x08 hk_state
	 0x0C hk_engaged
	 0x10 hk_down_tick
	 0x14 hk_cmdline_s
	 0x18 hk_cmdline_l
	 0x1C hk_pi
	 0x2C hk_si
	KBDLLHOOKSTRUCT:
	 0x00 vkCode
	 0x04 scanCode
	 0x08 flags
	 0x0C time
	 0x10 dwExtraInfo
	*/
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, CTRL or ALT (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je sp_test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x1D, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag0 */
	je sp_flag0
	cmpl $0x38, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je sp_flag1
	popl %ebx
	ret                               /* Return EAX (conforming to CDECL convention, stack is cleared by caller) */
_SinglePressShiftAltEventHandler:
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, SHIFTR or ALT (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je sp_test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je sp_flag0
	cmpl $0x36, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je sp_flag0
	cmpl $0x38, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je sp_flag1
	popl %ebx
	ret                               /* Return EAX (conforming to CDECL convention, stack is cleared by caller) */
_SinglePressCtrlShiftEventHandler:
	pushl %ebx                        /* Saving EBX to stack */
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	movl 0x10(%esp), %edx             /* Move pointer to KBDLLHOOKSTRUCT to EDX */
	movl 0x08(%esp), %ebx             /* Move pointer to KeyTriplet to EBX */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, CTRL or SHIFTR (ordered based on assumed use freqency) */
	movl 0x00(%ebx), %ecx             /* ECX=KeyTriplet.hk_sc */
	cmpl %ecx, 0x04(%edx)             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je sp_test_ext
	movl $1, %ecx                     /* ECX=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je sp_flag0
	cmpl $0x1D, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag1 */
	je sp_flag1
	cmpl $0x36, 0x04(%edx)            /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je sp_flag0
	popl %ebx
	ret                               /* Return EAX (conforming to CDECL convention, stack is cleared by caller) */
sp_test_ext:                          /* KBDLLHOOKSTRUCT.scanCode was matched with third key (KeyTriplet.hk_sc) - test if extended-key flags match */
	movl 0x08(%edx), %ecx             /* Move KBDLLHOOKSTRUCT.flags to ECX */
	andl $0x01, %ecx                  /* ECX=KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED */
	cmpl 0x04(%ebx), %ecx             /* If KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED!=KeyTriplet.hk_ext JMP exit */
	jne sp_exit_default
	movl $2, %ecx                     /* ECX=010b (HK_FLAG) */
sp_flag1:                             /* KBDLLHOOKSTRUCT.scanCode was matched with second key */
	addl %ecx, %ecx                   /* Double ECX (HK_FLAG) - same as shifting left by 1, but may be faster on P4 */ 
sp_flag0:                             /* KBDLLHOOKSTRUCT.scanCode was matched with first key */
	cmpl $0x0101, 0x0C(%esp)          /* If wParam==WM_KEYUP JMP sp_key_up */
	je sp_key_up
	cmpl $0x0105, 0x0C(%esp)          /* If wParam==WM_SYSKEYUP JMP sp_key_up */
	je sp_key_up
	orl %ecx, 0x08(%ebx)              /* KeyTriplet.hk_state|=HK_FLAG */
	cmpl $0x07, 0x08(%ebx)            /* If KeyTriplet.hk_state==111b JMP sp_engaged */
	je sp_engaged
	popl %ebx
	ret    
sp_engaged:                           /* Single hotkey press occured */
	cmpl $1, 0x0C(%ebx)               /* If KeyTriplet.hk_engaged==1 JMP exit w/ EAX=true */
	je sp_exit_true
	movl $1, 0x0C(%ebx)               /* KeyTriplet.hk_engaged=1 */ 
	/* Push CreateProcess parameters to stack */
	leal 0x1C(%ebx), %eax
	pushl %eax                        /* lpProcessInformation=KeyTriplet.hk_pi */
	leal 0x2C(%ebx), %eax
	pushl %eax                        /* lpStartupInfo=KeyTriplet.hk_si */
	pushl $0                          /* lpCurrentDirectory=NULL */
	pushl $0                          /* lpEnvironment=NULL */
	pushl $0x80                       /* dwCreationFlags=HIGH_PRIORITY_CLASS */
	pushl $0                          /* bInheritHandles=FALSE */
	pushl $0                          /* lpThreadAttributes=NULL */
	pushl $0                          /* lpProcessAttributes=NULL */
	pushl 0x14(%ebx)                  /* lpCommandLine=KeyTriplet.hk_cmdline_s */
	pushl $0                          /* lpApplicationName=NULL */
	call *__imp__CreateProcessW       /* Call CreateProcess - it's STDCALL function and callee clears the stack */
	cmpl $0, %eax
	je sp_exit_true                   /* If CreateProcess returned 0 JMP sp_exit_true */
	pushl 0x1C(%ebx)
	call *__imp__CloseHandle          /* Call CloseHandle(KeyTriplet.hk_pi.hProcess) - STDCALL */
	pushl 0x20(%ebx)
	call *__imp__CloseHandle          /* Call CloseHandle(KeyTriplet.hk_pi.hThread) - STDCALL */
sp_exit_true:
	movl $1, %eax                     /* EAX=true (used as return value) */
	popl %ebx
	ret	
sp_key_up:                            /* WM_KEYUP/WM_SYSKEYUP event */
	notl %ecx
	andl %ecx, 0x08(%ebx)             /* KeyTriplet.hk_state&=~HK_FLAG */
	movl $0, 0x0C(%ebx)               /* KeyTriplet.hk_engaged=0 */
sp_exit_default:
	popl %ebx
	ret
#else
/* Make functions visible to linker, no mangling for x64 */
.globl SinglePressCtrlAltEventHandler
.globl SinglePressShiftAltEventHandler
.globl SinglePressCtrlShiftEventHandler
.globl LongPressCtrlAltEventHandler
.globl LongPressShiftAltEventHandler
.globl LongPressCtrlShiftEventHandler
/* Defining symbols, COFF format, storage-class 2 (external), attribute 32 (function) */
.def SinglePressCtrlAltEventHandler;   .scl 2; .type 32; .endef
.def SinglePressShiftAltEventHandler;  .scl 2; .type 32; .endef
.def SinglePressCtrlShiftEventHandler; .scl 2; .type 32; .endef
.def LongPressCtrlAltEventHandler;     .scl 2; .type 32; .endef
.def LongPressShiftAltEventHandler;    .scl 2; .type 32; .endef
.def LongPressCtrlShiftEventHandler;   .scl 2; .type 32; .endef
.def __imp_CreateProcessW;             .scl 2; .type 32; .endef
.def __imp_CloseHandle;                .scl 2; .type 32; .endef

/*
	MS changed the way SEH is handled with x64 ABI
	On x86 SEH was handled by walking EXCEPTION_REGISTRATION chain, stored on stack, which head is pointed at by [FS:0]
	On x64 SEH is handled by walking call stack itself with the help of function table stored in .pdata section and unwind/exception handling data in .xdata section
	Because exception information no longer stored on stack, but stored in PE header itself, it's not susceptible to buffer overflow and there is no extra code for maintaining EXCEPTION_REGISTRATION chain
	The downside is that every function should be now described (it's start, end offsets and prolog code) using special pseudo operations and epilog code must follow a strict set of rules for SEH to recognize it (there are only two allowed epilog forms)
	Exception is leaf functions - functions that do not change any non-volatile registers (including RSP)
	If this convention is abandoned, SEH won't work past the first function on call stack that abandoned it and unhandled exception code will be called instead (system default or set with SetUnhandledExceptionFilter)

	Functions below have multiple entries/exits and don't have traditional prolog/epilog
	Such functions are impossible to describe using provided SEH-related pseudo operations
	While it is possible to rewrite them to be SEH-compatible, perfomance and code size will suffer from such changes
	So using SEH here actually contradicts the idea behind implementing hotkey functions in assembler - low level optimization
	
	That's why we are abandoning SEH compatibility for these functions
	Moreover, all the SEH related data is stopped being generated for the binary altogether using -fno-asynchronous-unwind-tables compiler option
	Anyway, SEH is not used in SnK HotkeySuite and being able to catch unhandled exceptions is more than enough here
*/

LongPressCtrlAltEventHandler:
	/* 
	Function prototype is bool KeyPressEventHandler(KeyTriplet*, WPARAM, KBDLLHOOKSTRUCT*)
	Stack is misaligned by 8 bytes
	1 QWORD + 32-byte shadow space is passed on the stack (RSP offsets): 
	 0x00 Return address
	 0x08 Shadow space
	 0x10 Shadow space
	 0x18 Shadow space
	 0x20 Shadow space
	Arguments are passed in registers:
	 RCX Pointer to KeyTriplet
	 RDX wParam
	 R8  Pointer to KBDLLHOOKSTRUCT
	KeyTriplet:
	 0x00 hk_sc
	 0x04 hk_ext
	 0x08 hk_state
	 0x0C hk_engaged
	 0x10 hk_down_tick
	 0x18 hk_cmdline_s
	 0x20 hk_cmdline_l
	 0x28 hk_pi
	 0x40 hk_si
	KBDLLHOOKSTRUCT:
	 0x00 vkCode
	 0x04 scanCode
	 0x08 flags
	 0x0C time
	 0x10 dwExtraInfo
	*/
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, CTRL or ALT (ordered based on assumed use freqency) */
	movl 0x04(%r8), %r9d              /* R9D=KBDLLHOOKSTRUCT.scanCode */
	cmpl 0x00(%rcx), %r9d             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je lp_test_ext
	movl $1, %r10d                    /* R10D=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x1D, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag0 */
	je lp_flag0
	cmpl $0x38, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je lp_flag1
	ret                               /* Return EAX (conforming to x64 calling convention, stack is cleared by caller) */
LongPressShiftAltEventHandler:
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, SHIFTR or ALT (ordered based on assumed use freqency) */
	movl 0x04(%r8), %r9d              /* R9D=KBDLLHOOKSTRUCT.scanCode */
	cmpl 0x00(%rcx), %r9d             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je lp_test_ext
	movl $1, %r10d                    /* R10D=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je lp_flag0
	cmpl $0x36, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je lp_flag0
	cmpl $0x38, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je lp_flag1
	ret                               /* Return EAX (conforming to x64 calling convention, stack is cleared by caller) */
LongPressCtrlShiftEventHandler:
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, CTRL or SHIFTR (ordered based on assumed use freqency) */
	movl 0x04(%r8), %r9d              /* R9D=KBDLLHOOKSTRUCT.scanCode */
	cmpl 0x00(%rcx), %r9d             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je lp_test_ext
	movl $1, %r10d                    /* R10D=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je lp_flag0
	cmpl $0x1D, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag1 */
	je lp_flag1
	cmpl $0x36, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je lp_flag0
	ret                               /* Return EAX (conforming to x64 calling convention, stack is cleared by caller) */
lp_test_ext:                          /* KBDLLHOOKSTRUCT.scanCode was matched with third key (KeyTriplet.hk_sc) - test if extended-key flags match */
	movl 0x08(%r8), %r9d              /* Move KBDLLHOOKSTRUCT.flags to R9D */
	andl $0x01, %r9d                  /* R9D=KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED */
	cmpl 0x04(%rcx), %r9d             /* If KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED!=KeyTriplet.hk_ext JMP exit */
	jne lp_exit_default
	movl $2, %r10d                    /* R10D=010b (HK_FLAG) */
lp_flag1:                             /* KBDLLHOOKSTRUCT.scanCode was matched with second key */
	addl %r10d, %r10d                 /* Double R10D (HK_FLAG) - same as shifting left by 1, but may be faster on P4 */ 
lp_flag0:                             /* KBDLLHOOKSTRUCT.scanCode was matched with first key */
	cmpq $0x0101, %rdx                /* If wParam==WM_KEYUP JMP lp_key_up */
	je lp_key_up
	cmpq $0x0105, %rdx                /* If wParam==WM_SYSKEYUP JMP lp_key_up */
	je lp_key_up
	orl %r10d, 0x08(%rcx)             /* KeyTriplet.hk_state|=HK_FLAG */
	cmpl $0x07, 0x08(%rcx)            /* If KeyTriplet.hk_state==111b JMP lp_engaged */
	je lp_engaged
	ret    
lp_engaged:                           /* Hotkey press occured */
	movl $1, %eax                     /* EAX=true (used as return value) */
	cmpl $1, 0x0C(%rcx)               /* If KeyTriplet.hk_engaged!=1 JMP lp_set_time */
	jne lp_set_time
	ret
lp_set_time:
	movl $1, 0x0C(%rcx)               /* KeyTriplet.hk_engaged=1 */ 
	movl 0x0C(%r8), %r11d             /* Copy KBDLLHOOKSTRUCT.time to KeyTriplet.hk_down_tick */  
	movl %r11d, 0x10(%rcx)
	ret
lp_key_up:
	notl %r10d
	andl %r10d, 0x08(%rcx)            /* KeyTriplet.hk_state&=~HK_FLAG */
	cmpl $1, 0x0C(%rcx)               /* If KeyTriplet.hk_engaged==1 JMP lp_release */
	je lp_release
	ret
lp_release:
	movl $0, 0x0C(%rcx)               /* KeyTriplet.hk_engaged=0 */
	movl 0x0C(%r8), %r11d             /* R11D=KBDLLHOOKSTRUCT.time */  
	subl 0x10(%rcx), %r11d            /* If KBDLLHOOKSTRUCT.time<KeyTriplet.hk_down_tick JMP lp_exit_default - excludes moment of system timer wrap around after 49.7 days of uptime */
	jl lp_exit_default
	cmpl $LONG_PRESS_DURATION, %r11d  /* If KBDLLHOOKSTRUCT.time-KeyTriplet.hk_down_tick>LONG_PRESS_DURATION RDX=KeyTriplet.hk_cmdline_l else RDX=KeyTriplet.hk_cmdline_s */
	movq 0x20(%rcx), %rdx
	jg lp_longpress
	movq 0x18(%rcx), %rdx
lp_longpress:
	/* Push CreateProcess parameters to stack */
	pushq %rbx                        /* Saving RBX to stack, now stack is 16-byte aligned */
	subq $80, %rsp                    /* Prepairing 80 bytes on stack: 48-bytes for parameters, 32-bytes for shadow space */
	leaq 0x28(%rcx), %rbx             /* Saving KeyTriplet.hk_pi to RBX because RCX is volatile and will be trashed */
	movq %rbx, 0x48(%rsp)             /* lpProcessInformation=KeyTriplet.hk_pi */
	leaq 0x40(%rcx), %rax
	movq %rax, 0x40(%rsp)             /* lpStartupInfo=KeyTriplet.hk_si */
	movq $0, 0x38(%rsp)               /* lpCurrentDirectory=NULL */
	movq $0, 0x30(%rsp)               /* lpEnvironment=NULL */
	movl $0x80, 0x28(%rsp)            /* dwCreationFlags=HIGH_PRIORITY_CLASS */
	movl $0, 0x20(%rsp)               /* bInheritHandles=FALSE */
	movq $0, %r9                      /* lpThreadAttributes=NULL */
	movq $0, %r8                      /* lpProcessAttributes=NULL */
	/* RDX was set previously: lpCommandLine=KeyTriplet.hk_cmdline_s/KeyTriplet.hk_cmdline_l */
	movq $0, %rcx                     /* lpApplicationName=NULL */
	/* Call CreateProcess - it's x64 function and caller clears the stack */
	movq __imp_CreateProcessW(%rip), %rax
	call *%rax
	cmpl $0, %eax
	je lp_skip_closehandle            /* If CreateProcess returned 0 JMP lp_skip_closehandle (function will return false in this case because EAX=0) */
	movq 0x00(%rbx), %rcx
	movq __imp_CloseHandle(%rip), %rax
	call *%rax                        /* Call CloseHandle(KeyTriplet.hk_pi.hProcess) */
	movq 0x08(%rbx), %rcx
	movq __imp_CloseHandle(%rip), %rax
	call *%rax                        /* Call CloseHandle(KeyTriplet.hk_pi.hThread) */
	xorl %eax, %eax                   /* EAX=false (used as return value) */
lp_skip_closehandle:
	addq $80, %rsp                    /* Free allocated space on the stack */  
	popq %rbx                         /* Restore RBX */
lp_exit_default:
	ret	
	
SinglePressCtrlAltEventHandler:
	/* 
	Function prototype is bool KeyPressEventHandler(KeyTriplet*, WPARAM, KBDLLHOOKSTRUCT*)
	Stack is misaligned by 8 bytes
	1 QWORD + 32-byte shadow space is passed on the stack (RSP offsets): 
	 0x00 Return address
	 0x08 Shadow space
	 0x10 Shadow space
	 0x18 Shadow space
	 0x20 Shadow space
	Arguments are passed in registers:
	 RCX Pointer to KeyTriplet
	 RDX wParam
	 R8  Pointer to KBDLLHOOKSTRUCT
	KeyTriplet:
	 0x00 hk_sc
	 0x04 hk_ext
	 0x08 hk_state
	 0x0C hk_engaged
	 0x10 hk_down_tick
	 0x18 hk_cmdline_s
	 0x20 hk_cmdline_l
	 0x28 hk_pi
	 0x40 hk_si
	KBDLLHOOKSTRUCT:
	 0x00 vkCode
	 0x04 scanCode
	 0x08 flags
	 0x0C time
	 0x10 dwExtraInfo
	*/
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, CTRL or ALT (ordered based on assumed use freqency) */
	movl 0x04(%r8), %r9d              /* R9D=KBDLLHOOKSTRUCT.scanCode */
	cmpl 0x00(%rcx), %r9d             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je sp_test_ext
	movl $1, %r10d                    /* R10D=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x1D, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag0 */
	je sp_flag0
	cmpl $0x38, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je sp_flag1
	ret                               /* Return EAX (conforming to x64 calling convention, stack is cleared by caller) */
SinglePressShiftAltEventHandler:
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, SHIFTR or ALT (ordered based on assumed use freqency) */
	movl 0x04(%r8), %r9d              /* R9D=KBDLLHOOKSTRUCT.scanCode */
	cmpl 0x00(%rcx), %r9d             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je sp_test_ext
	movl $1, %r10d                    /* R10D=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je sp_flag0
	cmpl $0x36, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je sp_flag0
	cmpl $0x38, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==ALT JMP flag1 */
	je sp_flag1
	ret                               /* Return EAX (conforming to x64 calling convention, stack is cleared by caller) */
SinglePressCtrlShiftEventHandler:
	xorl %eax, %eax                   /* EAX=false (used as return value) */
	/* Sequentially check if KBDLLHOOKSTRUCT.scanCode matches KeyTriplet.hk_sc, SHIFTL, CTRL or SHIFTR (ordered based on assumed use freqency) */
	movl 0x04(%r8), %r9d              /* R9D=KBDLLHOOKSTRUCT.scanCode */
	cmpl 0x00(%rcx), %r9d             /* If KBDLLHOOKSTRUCT.scanCode==KeyTriplet.hk_sc JMP test_ext */
	je sp_test_ext
	movl $1, %r10d                    /* R10D=001b (used as HK_FLAG for KeyTriplet.hk_state) */
	cmpl $0x2A, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==SHIFTL JMP flag0 */
	je sp_flag0
	cmpl $0x1D, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==CTRL JMP flag1 */
	je sp_flag1
	cmpl $0x36, %r9d                  /* If KBDLLHOOKSTRUCT.scanCode==SHIFTR JMP flag0 */
	je sp_flag0
	ret                               /* Return EAX (conforming to x64 calling convention, stack is cleared by caller) */
sp_test_ext:                          /* KBDLLHOOKSTRUCT.scanCode was matched with third key (KeyTriplet.hk_sc) - test if extended-key flags match */
	movl 0x08(%r8), %r9d              /* Move KBDLLHOOKSTRUCT.flags to R9D */
	andl $0x01, %r9d                  /* R9D=KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED */
	cmpl 0x04(%rcx), %r9d             /* If KBDLLHOOKSTRUCT.flags&LLKHF_EXTENDED!=KeyTriplet.hk_ext JMP exit */
	jne sp_exit_default
	movl $2, %r10d                    /* R10D=010b (HK_FLAG) */
sp_flag1:                             /* KBDLLHOOKSTRUCT.scanCode was matched with second key */
	addl %r10d, %r10d                 /* Double R10D (HK_FLAG) - same as shifting left by 1, but may be faster on P4 */ 
sp_flag0:                             /* KBDLLHOOKSTRUCT.scanCode was matched with first key */
	cmpq $0x0101, %rdx                /* If wParam==WM_KEYUP JMP sp_key_up */
	je sp_key_up
	cmpq $0x0105, %rdx                /* If wParam==WM_SYSKEYUP JMP sp_key_up */
	je sp_key_up
	orl %r10d, 0x08(%rcx)             /* KeyTriplet.hk_state|=HK_FLAG */
	cmpl $0x07, 0x08(%rcx)            /* If KeyTriplet.hk_state==111b JMP sp_engaged */
	je sp_engaged
	ret    
sp_engaged:                           /* Single hotkey press occured */
	cmpl $1, 0x0C(%rcx)               /* If KeyTriplet.hk_engaged==1 JMP exit w/ EAX=true */
	je sp_exit_true
	movl $1, 0x0C(%rcx)               /* KeyTriplet.hk_engaged=1 */ 
	/* Push CreateProcess parameters to stack */
	pushq %rbx                        /* Saving RBX to stack, now stack is 16-byte aligned */
	subq $80, %rsp                    /* Prepairing 80 bytes on stack: 48-bytes for parameters, 32-bytes for shadow space */
	leaq 0x28(%rcx), %rbx             /* Saving KeyTriplet.hk_pi to RBX because RCX is volatile and will be trashed */
	movq %rbx, 0x48(%rsp)             /* lpProcessInformation=KeyTriplet.hk_pi */
	leaq 0x40(%rcx), %rax
	movq %rax, 0x40(%rsp)             /* lpStartupInfo=KeyTriplet.hk_si */
	movq $0, 0x38(%rsp)               /* lpCurrentDirectory=NULL */
	movq $0, 0x30(%rsp)               /* lpEnvironment=NULL */
	movl $0x80, 0x28(%rsp)            /* dwCreationFlags=HIGH_PRIORITY_CLASS */
	movl $0, 0x20(%rsp)               /* bInheritHandles=FALSE */
	movq $0, %r9                      /* lpThreadAttributes=NULL */
	movq $0, %r8                      /* lpProcessAttributes=NULL */
	movq 0x18(%rcx), %rdx             /* lpCommandLine=KeyTriplet.hk_cmdline_s */
	movq $0, %rcx                     /* lpApplicationName=NULL */
	/* Call CreateProcess - it's x64 function and caller clears the stack */
	movq __imp_CreateProcessW(%rip), %rax
	call *%rax
	cmpl $0, %eax
	je sp_skip_closehandle            /* If CreateProcess returned 0 JMP sp_skip_closehandle */
	movq 0x00(%rbx), %rcx
	movq __imp_CloseHandle(%rip), %rax
	call *%rax                        /* Call CloseHandle(KeyTriplet.hk_pi.hProcess) */
	movq 0x08(%rbx), %rcx
	movq __imp_CloseHandle(%rip), %rax
	call *%rax                        /* Call CloseHandle(KeyTriplet.hk_pi.hThread) */
sp_skip_closehandle:
	addq $80, %rsp                    /* Free allocated space on the stack */  
	popq %rbx                         /* Restore RBX */
sp_exit_true:
	movl $1, %eax                     /* EAX=true (used as return value) */
	ret	
sp_key_up:                            /* WM_KEYUP/WM_SYSKEYUP event */
	notl %r10d
	andl %r10d, 0x08(%rcx)            /* KeyTriplet.hk_state&=~HK_FLAG */
	movl $0, 0x0C(%rcx)               /* KeyTriplet.hk_engaged=0 */
sp_exit_default:
	ret
#endif