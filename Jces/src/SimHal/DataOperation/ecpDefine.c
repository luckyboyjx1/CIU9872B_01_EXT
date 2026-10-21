/*
 * ecpDefine.c
 *
 *  Created on: 2014Äê2ÔÂ28ÈÕ
 *      Author: zousonghua
 */

#include "ecpDefine.h"
#include "SysConfigTarget.h"
#include "chipHead.h"

u16	MEM_R_U16(memref d)
{
	return	(u16)( (PF_R_U8(d) << 8) | (PF_R_U8(d + 1)) );
}

void MEM_W_U16(memref d, u16 v)
{
	PD_W_U8(MEMtPX8(d), ((v & 0xff00) >> 8));
    PD_W_U8((MEMtPX8(d)+1), (v & 0x00ff));
}


u32	MEM_R_U32(memref d)
{
//	return	((PF_R_U8(d) << 24) | (PF_R_U8(d + 1) << 16) | (PF_R_U8(d + 2) << 8) | (PF_R_U8(d + 3) ));
	return ((PM_R_U8(d + 3) | (PM_R_U8(d + 2) << 8) | (PM_R_U8(d + 1) << 16) | (PM_R_U8(d) << 24)));
}

void MEM_W_U32(memref d, u32 v)
{
	PD_W_U8(MEMtPX8(d),((v & 0xff000000) >> 24));
	PD_W_U8((MEMtPX8(d)+1),((v & 0x00ff0000) >> 16));
	PD_W_U8((MEMtPX8(d)+2),((v & 0x0000ff00) >> 8));
	PD_W_U8((MEMtPX8(d)+3),(v & 0x000000ff));
}


u16 htoj16(u16 value)
{
	return ((LOW(value) << 8) | HIGH(value));
}
u32 htoj32(u32 x)
{
	return (((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) | ((x & 0x00ff0000) >> 8) | ((x & 0xff000000) >> 24));
}
#if BOOT__CODE_CHECK__FOR_TEST
	u16	U8t16(u8 v)
	{
		return (u16)v;
	}

	u32 U8t32(u8 v)
	{
		return (u32)v;
	}

	u8 U16t8(u16 v)
	{
		return (u8)v;
	}

	u32 U16t32(u16 v)
	{
		return (u32)v;
	}

	u8 U32t8(u32 v)
	{
		return (u8)v;
	}

	u16 U32t16(u32 v)
	{
		return (u16)v;
	}

	memref PXtMEM(PX8 v)
	{
		return (memref)v;
	}

	memref PRtMEM(PR8 v)
	{
		return (memref)v;
	}

	memref PNtMEM(PN8 v)
	{
		return (memref)v;
	}

	memref PCtMEM(PC8 v)
	{
		return (memref)v;
	}

	memref PFtMEM(PF8 v)
	{
		return (memref)v;
	}

	memref PMtMEM(PM8 v)
	{
		return (memref)v;
	}

	PI8 MEMtPI8(memref v)
	{
		return (PI8)v;
	}

	PI16 MEMtPI16(memref v)
	{
		return (PI16)v;
	}

	PI32 MEMtPI32(memref v)
	{
		return (PI32)v;
	}

	PD8 MEMtPD8(memref v)
	{
		return (PD8)v;
	}

	PD16 MEMtPD16(memref v)
	{
		return (PD16)v;
	}

	PD32 MEMtPD32(memref v)
	{
		return (PD32)v;
	}

	PX8 MEMtPX8(memref v)
	{
		return (PX8)v;
	}

	PX16 MEMtPX16(memref v)
	{
		return (PX16)v;
	}

	PX32 MEMtPX32(memref v)
	{
		return (PX32)v;
	}

	PR8 MEMtPR8(memref v)
	{
		return (PR8)v;
	}

	PR16 MEMtPR16(memref v)
	{
		return (PR16)v;
	}

	PR32 MEMtPR32(memref v)
	{
		return (PR32)v;
	}

	PN8 MEMtPN8(memref v)
	{
		return (PN8)v;
	}

	PN16 MEMtPN16(memref v)
	{
		return (PN16)v;
	}

	PN24 MEMtPN24(memref v)
	{
		return (PN24)v;
	}

	PN32 MEMtPN32(memref v)
	{
		return (PN32)v;
	}

	PC8 MEMtPC8(memref v)
	{
		return (PC8)v;
	}

	PC16 MEMtPC16(memref v)
	{
		return (PC16)v;
	}

	PC32 MEMtPC32(memref v)
	{
		return (PC32)v;
	}

	PF8 MEMtPF8(memref v)
	{
		return (PF8)v;
	}

	PF16 MEMtPF16(memref v)
	{
		return (PF16)v;
	}

	PF32 MEMtPF32(memref v)
	{
		return (PF32)v;
	}

	PM8 MEMtPM8(memref v)
	{
		return (PM8)v;
	}

	PM16 MEMtPM16(memref v)
	{
		return (PM16)v;
	}

	PM32 MEMtPM32(memref v)
	{
		return (PM32)v;
	}

	s16 U8tS16(u8 v)
	{
		return (s16)v;
	}

	s32 U8tS32(u8 v)
	{
		return (s32)v;
	}

	s16 U16tS16(u16 v)
	{
		return (s16)v;
	}

	s32 U16tS32(u16 v)
	{
		return (s32)v;
	}

	s32 U32tS32(u32 v)
	{
		return (s32)v;
	}

	u16 S16tU16(s16 v)
	{
		return (u16)v;
	}
#endif
