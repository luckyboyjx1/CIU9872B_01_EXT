/*
 * crt0.S
 * 1 define and initial the stack pointer
 * 2 exception handler table
 * 3 call SystemInit
 * 4 go to __main in entry.o
 *
 * Copyright (C) 2016~2017  Hangzhou C-SKY Microsystems Co., Ltd
 * Modify by Jiang Long  on 2016-09-14
 */
// <<< Use Configuration Wizard in Context Menu >>>

.global __initial_sp

// <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:4>
// <i> Config the stack size for the application
#define		StackSize		0x2000


.section .csky_stack,"aw",@nobits
.align 2
Stack_Mem:
.space StackSize
__initial_sp:


#ifdef FAST_INTERRUPT_ENABLE
// <o> Fast Stack Size (in Bytes) <0x0-0xFFFFFFFF:4>
// <i> Config the stack size for the fast interrupt's handler
#define		FastStackSize		0x200
FastStack_Mem:
.space FastStackSize
__initial_fast_sp:
#endif

// <o> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
// <i> Config the heap size for the application
#define		HeapSize		0x200

.global __heap_start
.global __heap_end
.section .csky_heap,"aw",@nobits
.align 2
Heap_Mem:
__heap_start:
.space HeapSize
__heap_end:

/*
 * For importing variable or functions from other c or assemble files.
 */
.import main
.import SystemInit

/*
 * default service routine
 */
.global Reset_Handler
.global Misaligned_Access_Handler
.global Access_Error_Handler
.global Divided_By_Zero_Handler
.global Illegal_Handler
.global Privlege_Violation_Handler
.global Trace_Exection_Handler
.global Breakpoint_Exception_Handler
.global Unrecoverable_Error_Handler
.global Idly4_Error_Handler
.global Auto_INT_Handler
.global Auto_FINT_Handler
.global Reserved_HAI_Handler
.global Reserved_FP_Handler
.global TLB_Ins_Empty_Handler
.global TLB_Data_Empty_Handler
.global Default_handler

.weak Reset_Handler
.weak Misaligned_Access_Handler
.weak Access_Error_Handler
.weak Divided_By_Zero_Handler
.weak Illegal_Handler
.weak Privlege_Violation_Handler
.weak Trace_Exection_Handler
.weak Breakpoint_Exception_Handler
.weak Unrecoverable_Error_Handler
.weak Idly4_Error_Handler
.weak Auto_INT_Handler
.weak Auto_FINT_Handler
.weak Reserved_HAI_Handler
.weak Reserved_FP_Handler
.weak TLB_Ins_Empty_Handler
.weak TLB_Data_Empty_Handler
.weak Default_handler

.weak timer0_irq_handler
.weak timer1_irq_handler
.weak wdt_irq_handler
.weak nvm_irq_handler
.weak emmu_irq_handler
.weak secrst_irq_handler
.weak rng1_irq_handler
.weak dma_irq_handler
.weak SCIRST_irq_handler
.weak SCITx_irq_handler
.weak SCIRx_irq_handler
.weak SCIOverFlow_irq_handler
.weak gpio_irq_handler
.weak timer2_irq_handler
.weak CTIM_irq_handler

.export	ckcpu_vsr_table	/* Vector table base address. */
.section .exp_table,"ax",@progbits
/* Vector table space. */
$d:
.align 10
ckcpu_vsr_table:
	.long Reset_Handler
	.long Misaligned_Access_Handler
	.long Access_Error_Handler
	.long Divided_By_Zero_Handler
	.long Illegal_Handler
	.long Privlege_Violation_Handler
	.long Trace_Exection_Handler
	.long Breakpoint_Exception_Handler
	.long Unrecoverable_Error_Handler
	.long Idly4_Error_Handler
	.long Auto_INT_Handler
	.long Auto_FINT_Handler
	.long Reserved_HAI_Handler
	.long Reserved_FP_Handler
	.long TLB_Ins_Empty_Handler
	.long TLB_Data_Empty_Handler
.rept 16 //base 10
	.long Default_handler
.endr
	.long __timer0_irq_handler
	.long __timer1_irq_handler
	.long __wdt_irq_handler
	.long __nvm_irq_handler
	.long __emmu_irq_handler
	.long __secrst_irq_handler
	.long __rng1_irq_handler
	.long __dma_irq_handler
	.long __7816RST_irq_handler
	.long __7816Tx_irq_handler
	.long __7816Rx_irq_handler
	.long __7816OverFlow_irq_handler
	.long __gpio_irq_handler
	.long __timer2_irq_handler
	.long __CTIM_irq_handler
	.long Default_handler   
.rept 11 
	.long Default_handler
.endr
$t:
/* The ckcpu startup codes. */
.text
.align 2
	/*
	 * This is the codes first entry point. This is where it all begins...
	 */
Reset_Handler:
	/*
	 * Init psr value, enable exception, disable interrupt and fast interrupt.
	 * psr = 0x80000100
	 */
	bgeni	r7, 31
	bseti	r7, 8
	mtcr	r7, psr

	/*
	 * Setup initial vector base table for interrupts and exceptions
	 */
	lrw 	a3, ckcpu_vsr_table
	mtcr	a3, vbr
	
	/* Initialize the normal stack pointer from the linker definition. */
	lrw 	a3, __initial_sp
	mov		sp, a3

#ifdef FAST_INTERRUPT_ENABLE
	/* Initialize the fast interrupt stack pointer . */
	psrset	af
	lrw		r6, __initial_fast_sp
	mov		sp, r6
	psrclr	af
#endif
	psrset ie
	lrw a0,	SystemInit
	jsr a0
	lrw a0,	main
	jsr a0
	
	/* Should never get here. */
1:
	br 1b
/* 以下为芯片异常函数，默认为若函数，用户可根据需求进行函数实现 */
Misaligned_Access_Handler:
Access_Error_Handler:
Divided_By_Zero_Handler:
Illegal_Handler:
Privlege_Violation_Handler:
Trace_Exection_Handler:
Breakpoint_Exception_Handler:
Unrecoverable_Error_Handler:
Idly4_Error_Handler:
Auto_INT_Handler:
Auto_FINT_Handler:
Reserved_HAI_Handler:
Reserved_FP_Handler:
TLB_Ins_Empty_Handler:
TLB_Data_Empty_Handler:
Default_handler:
	br Default_handler
	rte

/* 以下为芯片中断函数，默认为若函数，用户可根据需求进行函数实现 */
__timer0_irq_handler:
	nie
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
    bsr	timer0_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	nir
	rte
__timer1_irq_handler:
	nie
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	timer1_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	nir
	rte
__wdt_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	wdt_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__nvm_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	nvm_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__emmu_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	emmu_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__secrst_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	secrst_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__rng1_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	rng1_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__dma_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	dma_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__7816RST_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	SCIRST_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__7816Tx_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	SCITx_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__7816Rx_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	SCIRx_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__7816OverFlow_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	SCIOverFlow_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__gpio_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	gpio_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte
__timer2_irq_handler:
	nie
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	timer2_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	nir
	rte
__CTIM_irq_handler:
	ipush
    subi    sp, sp, 4
    st.w    r15, (sp)
	bsr	CTIM_irq_handler
	ld.w     r15, (sp)
    addi    sp, sp, 4
    ipop
	rte