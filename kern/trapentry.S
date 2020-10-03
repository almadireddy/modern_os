/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
#include <kern/macro.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */

#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushq $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */

#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushq $0;							\
	pushq $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
   Find the PUSHA,POPA macros in kern/macro.h. Since amd64 doesn't support
   pusha,popa so you will find these macros handy.
 */

// https://wiki.osdev.org/Exceptions - ref for which ones need NOEC
TRAPHANDLER_NOEC(tDivide, T_DIVIDE)
TRAPHANDLER_NOEC(tDebug, T_DEBUG)
TRAPHANDLER_NOEC(tNmi, T_NMI)
TRAPHANDLER_NOEC(tBreakpoint, T_BRKPT)
TRAPHANDLER_NOEC(tOverflow, T_OFLOW)
TRAPHANDLER_NOEC(tBoundCheck, T_BOUND)
TRAPHANDLER_NOEC(tIllegalOpcode, T_ILLOP)
TRAPHANDLER_NOEC(tDeviceNotAvailable, T_DEVICE)
TRAPHANDLER(tDoubleFault, T_DBLFLT)

TRAPHANDLER(tInvalidStackSegment, T_TSS)
TRAPHANDLER(tSegmentNotPresent, T_SEGNP)
TRAPHANDLER(tStackException, T_STACK)
TRAPHANDLER(tGeneralProtection, T_GPFLT)
TRAPHANDLER(tPageFault, T_PGFLT)

TRAPHANDLER_NOEC(tFloatingPointError, T_FPERR)
TRAPHANDLER(tAlignmentCheck, T_ALIGN)
TRAPHANDLER_NOEC(tMachineCheck, T_MCHK)
TRAPHANDLER_NOEC(tSIMDError, T_SIMDERR)

TRAPHANDLER_NOEC(tSyscall, T_SYSCALL)
TRAPHANDLER(tDefault, T_DEFAULT)

/*
 * Lab 3: Your code here for _alltraps
 *
 * Hint: Be sure to review the x64 calling convention from lab1
 *       for how to pass a pointer to the trapframe.
 */


_alltraps:
subq $16, %rsp
movw %ds, 8(%rsp)
movw %es, 0(%rsp)
PUSHA
movl $GD_KD, %eax
movw %ax, %ds
movw %ax, %es
movw %ax, %ss
movw %ax, %fs
movw %ax, %gs
movq %rsp,%rdi 
call trap

spin:
    jmp spin
