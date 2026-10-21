/*****************************************************************************
 memory_md.h

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,6,23	.Created
 *****************************************************************************/

#ifndef __MEM_OPERATION_SERVE_H__
#define __MEM_OPERATION_SERVE_H__

#include "DataOperation.h"
#include "SysConfigTarget.h"
#include "bootConfigProfile.h"

#if BOOT__MEMORY__OPERATION
	/************************************* variables ************************************/

	extern u8 				X_A				stream_buffer[EEPAGE_SIZE];
	extern u16 				D_A 			stream_page_offset;
	extern u1				B_A				flush_flag;
	extern PN8 				D_A 			stream_page;
	extern PN8 				D_A 			stream_address;


	/*****************************************************************************
	 method memory write
	 memory type include RAM E2P ROM
	 write type include u8 u16 u32 arraycopy arrayfill
	 *****************************************************************************/
	extern void E2P_stream_flush_clear(void);
	extern void E2P_stream_clear(void);

	// the base method for E2p write by flush way
	extern  void E2P_stream_open(PN8 dest_addr);
	extern  void E2P_stream_flush(void);
	extern  void E2P_stream_write_array(PN8 src, u16 length);
	extern  void E2P_stream_fill_array(u16 length, u8 value);
	extern  void E2P_stream_Write(u16 length, u8 mode);

	extern  void E2P_stream_write_u8(u8 value);
	extern  void E2P_stream_write_u16(u16 value)												SMALL_MODE;
	extern  void E2P_stream_write_u32(u32 value)												SMALL_MODE;

	//E2p Write
	extern void E2P_write_u8_getType(PN8 dest, u8 value, u8 type);
	extern void E2P_write_u16_getType(PN8 dest, u16 value, u8 type);
	extern void E2P_write_u32_getType(PN8 dest, u32 value, u8 type)							SMALL_MODE;
	extern u16  E2P_write_array_getType(PN8 dest, PM8 src, u16 length, u8 type);
	extern void E2P_fill_array_getType(PN8 dest, u16 length, u8 value, u8 type);


	/*****************************************************************************
	 * Atomic writes
	 *****************************************************************************/

	/*
	 * This error is thrown whenever an atomic write would cause the transaction
	 * buffer to overflow
	 */
	/*
	 * Atomic write in E2P.
	 *
	 * dest must be an address in E2P.
	 *
	 * throws TRANSACTION_BUFFER_OVERFLOW on failure
	 */
	extern void E2P_writeArrayAtomic(memref dest, PM8 src, u16 length);

	extern u16 EP_EepromToRam(PX8 dest, PM8 src, u16 length);

//	extern boolean check_in_cach(PF8 ref);
#define check_in_cach(ref)	(U32_AND_U32(ref, NOR_EEPAGE_MASK) == PNtMEM(stream_page))

#else
	#define  E2P_stream_flush_clear()
	#define  E2P_stream_clear()
	#define  E2P_stream_open(dest_addr)
	#define  E2P_stream_flush()
	#define  E2P_stream_write_array(src, length)
	#define  E2P_stream_fill_array(length, value)
	#define  E2P_stream_Write(length, mode)
	#define  E2P_stream_write_u8(value)
	#define  E2P_stream_write_u16(value)
	#define  E2P_stream_write_u32(value)
	#define  E2P_write_u8_getType(dest, value, type)
	#define  E2P_write_u16_getType(dest, value, type)
	#define  E2P_write_u32_getType(dest, value, type)
	#define  E2P_write_array_getType(dest, src, length, type)
	#define  E2P_fill_array_getType(dest, length, value, type)
	#define  E2P_writeArrayAtomic(dest, src, length)
	#define  EP_EepromToRam(dest, src, length)
	#define  check_in_cach(ref)																0
#endif

#endif /* MEMOPERATIONSERVE_H_ */



