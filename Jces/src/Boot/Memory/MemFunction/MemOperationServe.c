/*****************************************************************************
 memory_md.c

 access memory method all in this file , depend on the file memory_hal.h

 this file include three part
 E2P operation
 RAM operation
 MEM operation

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,6,23	.Created
 *****************************************************************************/
#include "DataOperation.h"
#include "SysConfigTarget.h"
#include "bootConfigProfile.h"
#include "MemBackUpServe.h"
#include "MemOperationServe.h"
#include "MemNvmOperation.h"
#include "SysConfig.h"
#include "NvmHal.h"
#include "SysConfigTarget.h"
#include "BootRam.h"
#include "bootConfigMethod.h"
#include "SecureCrypto.h"

/*****************************************************************************
 * The underlying layer must provide a abstraction of E2PROM as a stream.
 * To write consecutive bytes to address dest_addr,
 * first call E2P_stream_open(dest_addr) to open the strea.
 * Then, call E2P_stream_write_array or E2P_stream_fill_array once or multiple
 * times to feed the stream. Each call makes the current write address advance
 * accordingly. Writing is not guaranteed to be completed until after a call
 * to E2P_stream_flush(). This allows the low-level layer to use a buffer and
 * wait until a full E2PROM page is completed before writing it to E2PROM.
 *
 * The whole process is non-atomic, which means that:
 *  1. If tearing occurs between the call to E2P_stream_open(addr) and
 *     E2P_stream_flush(), the bytes between addr and the current address can
 *     have ANY values.
 *  2. When E2P_stream_flush() returns, the destination in E2PROM is guaranteed
 *     to contain the correct values.
 *  3. The contents E2PROM in addresses outside this destination are never
 *     affected
 *****************************************************************************/

/*****************************************************************************
 the variable used  for write E2p by flush way
 Size of one E2PROM page (now 64 bytes)used for save the data stream write to
 E2p, You can take a smaller value to save Ram space, larger values are useless
 ******************************************************************************/

/*****************************************************************************
 A buffer of bytes currently written in the stream but not yet written to
 E2PROM. This buffer is put in idata (upper IRAM) to save space in data
 (lower IRAM).
 the all variables must be in ram
 *****************************************************************************/

/************************************* variables ************************************/

#if BOOT__MEMORY__OPERATION

#pragma MEM_OPERATRION_SERVE_RAM_START

static 	PN8		 		X_A 			ptrFrom;
static 	u8 				X_A 			xValue;
		u8 				X_A				stream_buffer[EEPAGE_SIZE];
		u16				X_A 			stream_page_offset;
		u1				B_A				flush_flag;
		PN8 			X_A 			stream_page;
		PN8 			X_A 			stream_address;

MEM_OPERATRION_SERVE_RAM_END

#define ARRAY_FILL 				0x01
#define ARRAY_COPY_LEFT_RIGHT 	0x02
#define ARRAY_COPY_RIGHT_LEFT 	0x03


/*****************************************************************************
 Method E2p Write by flush way
 *****************************************************************************/
void E2P_stream_flush_clear(void)
{
	flush_flag 				= 0;
}

void E2P_stream_clear(void)
{
	flush_flag				= 0;
	stream_page				= MEMtPN8(NULL);
	stream_address			= MEMtPN8(NULL);
}

void E2P_stream_open(PN8 dest_addr)
{
	if (!isWriteAndCacheTheSame())
	{
		E2P_stream_flush();
	}

	if (!check_in_cach(dest_addr))
	{
		E2P_stream_flush();
		stream_page = MEMtPN8(U32_AND_U32(PNtMEM(dest_addr), NOR_EEPAGE_MASK));
		EP_EepromToRam((PX8)&stream_buffer[0], MEMtPM8(PNtMEM(stream_page)), EEPAGE_SIZE);
		flush_flag = 0;
	}
	setCacheDataType(getWriteEepType());
	stream_address = dest_addr;
	//TODO:check asm code of "(u8)dest_addr" NO
	stream_page_offset = U16_AND_U16(PNtMEM(dest_addr), EEPAGE_MASK);
}

void E2P_stream_flush()
{	
	PN8 			I_A		des;
	PX8			    I_A		scr;
	u16 			I_A		offset;
	u16 			D_A		Mmin_addr ;
	u16				D_A		Mmax_addr ;
	u1				B_A 	isChanged ;
	
	PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_001();
	des = stream_page;
	scr = MEMtPX8(&stream_buffer[0]);
	isChanged = false;
	Mmin_addr = 0;
	Mmax_addr = 0;
	if (flush_flag)
	{
		for (offset = 0; offset < EEPAGE_SIZE; U16_INC(offset))
		{ // is the same data in catch and in eep
			if (PX_R_U8(scr) != PN_R_U8(des))
			{
				if (!isChanged)
				{
					Mmin_addr = offset;
					isChanged = true;
				}
				Mmax_addr = offset;
			}
			//scr += 1;
			PX8_INC(scr);
			PN8_INC(des);
			
		}
		if (isChanged)
		{ // Have data modified in ram ,need write back to eep

			//**back EEP data
//			Mmax_addr	= Mmax_addr << 2;
//			Mmin_addr	= Mmin_addr << 2;

			save_E2P(PNtMEM(PN8_A_U16(stream_page , Mmin_addr)), U16_A_U16(U16_S_U16(Mmax_addr , Mmin_addr) , 1));

			//** write EEP data

			//TODO: noatomic写的时候要注意。
			MemNvmOperation_WriteErase(PN8_A_U16(stream_page , Mmin_addr) ,MEMtPX8(&stream_buffer[Mmin_addr]), U16_A_U16(U16_S_U16(Mmax_addr , Mmin_addr) , 1));

			//** commit if is Atomic
			commitSaveE2p();
		}
		stream_page_offset = 0;
		flush_flag 		   = 0;
		stream_page		   = NULL; //主干版本有问题，此处发现，现在已将问题修改。
	}
}

void E2P_stream_write_array(PN8 src, u16 length)
{
	PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_002();
	
	/*fix bug:B100915-001
	 if ((src < (ref_to_pointer(stream_address))) && ((src + length) > (ref_to_pointer(stream_address))))
	 linguoming:stream_address must be converted to (u8 far *)stream_address in C51.Or comparison is incorrect*/
	//TODO:optimize the comparison
	if ((PNtMEM(src) < PNtMEM(stream_address)) && (PNtMEM(PN8_A_U16(src , length)) > PNtMEM(stream_address)))
	{
		E2P_stream_flush();
		//stream_address = stream_address + length - 1;
		stream_address = PN8_A_U16(stream_address , length);
		stream_address = PN8_S_U8(stream_address,1);
		/*fix bug:B100919-003
		 ptrFrom = (u8 far *)(src + length - 1);
		 */
		//ptrFrom = src + (u32) (length - 1);
		ptrFrom = PN8_A_U16(src , length);
		ptrFrom = PN8_S_U8(ptrFrom, 1);
		E2P_stream_Write(length, ARRAY_COPY_RIGHT_LEFT);
	}
	else
	{
		ptrFrom = src;
		E2P_stream_Write(length, ARRAY_COPY_LEFT_RIGHT);
	}
}

void E2P_stream_fill_array(u16 length, u8 value)
{
	xValue = value;
	E2P_stream_Write(length, ARRAY_FILL);
}

/*****************************************************************************
 * the mode hava three value : 0x01 ARRAY_FILL
 *                             0x02 ARRAY_COPY_LEFT_RIGHT
 *							   0x03 ARRAY_COPY_RIGHT_LEFT.
 *
 *****************************************************************************/
void E2P_stream_Write(u16 length, u8 mode)
{
	u16			X_A		chunk_length;
	u16			X_A		copyLength;
	PN8 				dest ;
	
	PATCH_BOOT_MEMORY_MEMOPERATIONSERVE_003();
	
	dest = MEMtPN8(0);

	if (mode == ARRAY_COPY_RIGHT_LEFT)
	{
		dest = stream_address;
		//chunk_length = EEPAGE_SIZE - ((~(u8) stream_address) & EEPAGE_MASK);
		chunk_length = U16_S_U16(EEPAGE_SIZE , U16_AND_U16(U16_INV(U32t16(PNtMEM(stream_address))), EEPAGE_MASK));
	}
	else
	{
		//chunk_length = EEPAGE_SIZE - ((u8) stream_address & EEPAGE_MASK);
		chunk_length = U16_S_U16(EEPAGE_SIZE , U16_AND_U16(U32t16(PNtMEM(stream_address)), EEPAGE_MASK));
	}
	while (length > 0)
	{
		//flush_flag = 1;
		if (length < chunk_length)
		{
			chunk_length = length;
		}
		switch (mode)
		{
		case ARRAY_FILL:
			ecpMem_Set((PM8)&stream_buffer[stream_page_offset], (s8)xValue, (s32)chunk_length);
			flush_flag = 1;
			break;
		case ARRAY_COPY_LEFT_RIGHT:
			EP_EepromToRam(MEMtPX8(&stream_buffer[stream_page_offset]), MEMtPM8(&ptrFrom[0]), chunk_length);
			/*fix bug:B100919-003
			 ptrFrom += chunk_length;
			 */
			//ptrFrom += (u32) chunk_length;
			ptrFrom = PN8_A_U16(ptrFrom , chunk_length);
			flush_flag = 1;
			break;
		case ARRAY_COPY_RIGHT_LEFT:

			E2P_stream_open(dest);
			/*fix bug:B100919-003
			 ptrFrom -= chunk_length ;
			 */
			//ptrFrom -= (u32) chunk_length;
			ptrFrom = PN8_S_U16(ptrFrom , chunk_length);
			stream_page_offset = U16_AND_U16(U32t16(U32_A_U16(PNtMEM(PN8_S_U16(dest , chunk_length)), 1)), EEPAGE_MASK);
			for (copyLength = 0; copyLength < chunk_length; U16_INC(copyLength))
			{
				/*fix bug:B100919-003
				 stream_buffer[stream_page_offset + copyLength] = *(ptrFrom + 1 + copyLength);
				 */
				//stream_buffer[stream_page_offset + copyLength] = *(ptrFrom + (u32) (1 + copyLength));
				stream_buffer[U16_A_U16(stream_page_offset , copyLength)] = PM_R_U8(PM8_A_U8((PN8_A_U16(ptrFrom , copyLength)), 1));
			}
			flush_flag = 1;
			break;
		default:
			break;
		}

		if (mode == ARRAY_COPY_RIGHT_LEFT)
		{
			//length -= chunk_length;
			length = U16_S_U16(length , chunk_length);
			if (length > 0)
			{
				E2P_stream_flush();
				//EP_RamToEeprom((NVM_U8_P)(dest & NOR_EEPAGE_MASK),(XDATA_U8_P)&stream_buffer[0], EEPAGE_SIZE );
				//MemNvmOperation_WriteErase(MEMtPN8(U32_AND_U32(dest, NOR_EEPAGE_MASK)), MEMtPX8(&stream_buffer[0]), EEPAGE_SIZE );
			}
			//dest -= chunk_length;
			dest = PN8_S_U16(dest , chunk_length);
		}
		else
		{
			//stream_page_offset += chunk_length;
			//stream_address += chunk_length;
			//length -= chunk_length;
			stream_page_offset = U16_A_U16(stream_page_offset , chunk_length);
			stream_address = PN8_A_U16(stream_address , chunk_length);
			length = U16_S_U16(length , chunk_length);
			if (U16_AND_U16(PNtMEM(stream_address), EEPAGE_MASK) == 0)
			{
				E2P_stream_flush();
				if(length != 0)
				{
					E2P_stream_open(stream_address);
				}
			}
		}
		chunk_length = EEPAGE_SIZE;
	}
}
//@@@@@@@@@@@@@@ end of base operation

//and of E2P operation @@@@@@@@@@@@@@
/*****************************************************************************
 *  E2p_stream_write
 *****************************************************************************/

void E2P_stream_write_u8(u8 value)
{
 	stream_buffer[stream_page_offset] = value;
	flush_flag = 1;

	if (stream_page_offset == EEPAGE_MASK)
	{
		E2P_stream_open(PN8_A_U16(stream_page , EEPAGE_SIZE));	// read next page in caches
	}
	else
	{
		//stream_page_offset++;
		//stream_address++;
		U16_INC(stream_page_offset);
		stream_address = PN8_A_U8(stream_address , 1);
	}
}

void E2P_stream_write_u16(u16 value)	SMALL_MODE
{
	u16		D_A		tmp = value;	// less code

	E2P_stream_write_u8((u8)U16_SHR_U8(tmp, 8));
	E2P_stream_write_u8((u8)tmp);
}

void E2P_stream_write_u32(u32 value)	SMALL_MODE
{
	u32		D_A		tmp = value;	// less code
	E2P_stream_write_u16((u16)U32_SHR_U8(tmp, 16));
	E2P_stream_write_u16((u16)tmp);
}

/*****************************************************************************
 *  E2p NO ATOMIC Write
 *****************************************************************************/

void E2P_write_u8_getType(PN8 dest, u8 value, u8 type)
{
	if((U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page)) && !isInProgress() && (gBackUp_cacheType != WRITEEP_LOST))
	{
		setWriteEepType(gBackUp_cacheType);
	}
	else
	{
		setWriteEepType(type);
	}
	E2P_stream_open(dest);
	stream_buffer[stream_page_offset] = value;
	flush_flag = 1;
}

void E2P_write_u16_getType(PN8 dest, u16 value, u8 type)
{
	if((U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page)) && !isInProgress() && (gBackUp_cacheType != WRITEEP_LOST))
	{
		setWriteEepType(gBackUp_cacheType);
	}
	else
	{
		setWriteEepType(type);
	}
	E2P_stream_open(dest);
	stream_buffer[stream_page_offset] = U16t8(U16_SHR_U8(value, 8));

	if (stream_page_offset == EEPAGE_MASK)
	{
		flush_flag = 1;
		E2P_stream_open(PN8_A_U8(dest, 1));
	}
	else
	{
		//stream_page_offset++;
		U16_INC(stream_page_offset);
	}

	stream_buffer[stream_page_offset] = (u8) value;
	flush_flag = 1;
}

void E2P_write_u32_getType(PN8 dest, u32 value, u8 type)		SMALL_MODE
{
	if((U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page)) && !isInProgress() && (gBackUp_cacheType != WRITEEP_LOST))
	{
		setWriteEepType(gBackUp_cacheType);
	}
	else
	{
		setWriteEepType(type);
	}
	E2P_stream_open(dest);
	E2P_stream_write_u32(value);
}

u16 E2P_write_array_getType(PN8 dest, PM8 src, u16 length, u8 type)
{
	if((U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page)) && !isInProgress() && (gBackUp_cacheType != WRITEEP_LOST))
	{
		setWriteEepType(gBackUp_cacheType);
	}
	else
	{
		setWriteEepType(type);
	}
	E2P_stream_open(dest);
	E2P_stream_write_array(src, length);
	return ECP_ERR_OK;
}

void E2P_fill_array_getType(PN8 dest, u16 length, u8 value, u8 type)
{
	if((U32_AND_U32(PNtMEM(dest), NOR_EEPAGE_MASK) == PNtMEM(stream_page)) && !isInProgress() && (gBackUp_cacheType != WRITEEP_LOST))
	{
		setWriteEepType(gBackUp_cacheType);
	}
	else
	{
		setWriteEepType(type);
	}
	E2P_stream_open(dest);
	E2P_stream_fill_array(length, value);
}

u16 EP_EepromToRam(PX8 dest, PM8 src, u16 length)
{
	if(!isInE2P(src))
	{

		SC_MemCopyRam2RamAPI(dest, src, length);

	}

	else if(flush_flag)
	{
		while(length)
		{
			if (!check_in_cach(src))
			{
				PX_W_U8(dest , PM_R_U8(src));
			}
			else
			{
				PX_W_U8(dest , stream_buffer[U16_AND_U16(PMtMEM(src), EEPAGE_MASK)]);
			}

			PX8_INC(dest);
			PM8_INC(src);
			U16_DEC(length);
		}
	}
	else
	{
		while(length)
		{
			PX_W_U8(dest , PM_R_U8(src));
			PX8_INC(dest);
			PM8_INC(src);
			U16_DEC(length);
		}
	}

	return ECP_ERR_OK;
}

//boolean check_in_cach(PN8 ref)
//{
//	if(U32_AND_U32(ref, NOR_EEPAGE_MASK) != PNtMEM(stream_page))
//	{
//		return  false;
//	}
//	else
//	{
//		return true;
//	}
//}

#endif //BOOT__MEMORY__OPERATION
/**************************end of RAM operation*************************************/


