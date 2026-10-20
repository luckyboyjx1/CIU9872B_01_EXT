/*************************************************************
// <<< Use Configuration Wizard in Context Menu >>>
*************************************************************/

/*************************************************************
// <m> Entry Point <2>
// <i> config start function for this application
*************************************************************/
ENTRY( Reset_Handler )

MEMORY
{
  FLASH(RX) : 
/*************************************************************
// <o> Memory Start Address (in Bytes) <0x0-0xFFFFFFFF:0x100>
// <i> config start address of RAM area for this application
*************************************************************/
  ORIGIN = 0x1FFD0000 , 
/*************************************************************
// <o> Memory Length (in Bytes) <0x0-0xFFFFFFFF:0x1>
// <i> config the length of RAM area for this application
*************************************************************/
  LENGTH = 0x7800
  RAM(RWX) : 
/*************************************************************
// <o> Memory Start Address (in Bytes) <0x0-0xFFFFFFFF:0x100>
// <i> config start address of RAM area for this application
*************************************************************/
  ORIGIN = 0x20000000 , 
/*************************************************************
// <o> Memory Length (in Bytes) <0x0-0xFFFFFFFF:0x1>
// <i> config the length of RAM area for this application
*************************************************************/
  LENGTH = 0x5000
}

SECTIONS
{
    .text : {
		. = ALIGN(0x4) ;
		KEEP(*crt0.o(.exp_table))
		__stext = . ;
		*(.text)
		*(.text*)
		*(.text.*)
		*(.gnu.warning)
		*(.stub)
		*(.gnu.linkonce.t*)
		*(.glue_7t)
		*(.glue_7)
		*(.jcr)

		KEEP (*(.init))
		KEEP (*(.fini))
		
		/* This is special code area at the end of the normal
		   text section.  It contains a small lookup table at
		   the start followed by the code pointed to by entries
		   in the lookup table.  
		 */
		. = ALIGN (4) ;
		PROVIDE(__ctbp = .);
		*(.call_table_data)
		*(.call_table_text)
		
		. = ALIGN(0x10) ;
		__etext = . ;
    } > FLASH

    .rodata : {
		. = ALIGN(0x4) ;
		__srodata = .;
		*(.rdata)
		*(.rdata*)
		*(.rdata1)
		*(.rdata.*)
		*(.rodata)
		*(.rodata1)
		*(.rodata*)
		*(.rodata.*)
		*(.rodata.str1.4)
		. = ALIGN(0x10) ;
		__erodata = .;
    } > FLASH

	.pading : {
		FILL(0xFF);
		BYTE(0xFF);
		. = ORIGIN(FLASH) + LENGTH(FLASH);
	} > FLASH	
	
    .data : {
		. = ALIGN(0x4) ;
		__sdata = . ;
		__data_start = . ;
		data_start = . ;
		*(.got.plt)
		*(.got)
		*(.gnu.linkonce.r*)
		*(.data)
		*(.data*)
		*(.data1)
		*(.data.*)
		*(.gnu.linkonce.d*)
		*(.data1)
		*(.gcc_except_table)
		*(.gcc_except_table*)
		*(.sdata) 
		*(.sdata.*)
		*(.gnu.linkonce.s.*)
		*(__libc_atexit)
		*(__libc_subinit)
		*(__libc_subfreeres)
		*(.note.ABI-tag)
		
		. = ALIGN(0x10) ; 
		__edata = . ;
	} > RAM

    .bss : {
		. = ALIGN(0x4) ;
		__sbss = ALIGN(0x4) ;
		__s_ram_bss_1 = . ;
		*(.dynsbss)
		*(.sbss)
		*(.sbss.*)
		*(.scommon)
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(0x10) ;
		__e_ram_bss_1 = .;
		KEEP(*crt0.o(.csky_stack))
		KEEP(*crt0.o(.csky_heap))
		__end = . ;
		end = . ;
	} > RAM
}
