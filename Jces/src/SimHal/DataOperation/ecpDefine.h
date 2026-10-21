#ifndef __ECP_DEFINE_H__
#define	__ECP_DEFINE_H__

/********************************************************数据存储类型定义******************************************************/
/*****************************************************************************************************************************/
#define	B_A												 	/* bit */
#define D_A													/* data */
#define I_A													/* idata */
#define X_A													/* xdata */
#define R_A													/* data/idata/xdata */
#define N_A													/* far NVM */
#define C_A													/* far code */
#define F_A													/* far NVM/Code*/
#define M_A													/* RAM_AREA/NVM_AREA */
/*****************************************************************************************************************************/
/*****************************************************************************************************************************/

/********************************************************方法存储类型定义******************************************************/
/*****************************************************************************************************************************/
#define	SMALL_MODE
#define CONST								const

#define STATIC								static
#undef  NULL
#define NULL  								((void *)0)
#define MEM_NULL							0
#define REF_NULL							0x0000
#define	REF_INVALID  						0xFFFF					
#define FAR
#define AT
#define REENTRANT
#define IRQ 								
#define SAVE_OT                             
#define NO_OT                               
#define RESTORE_OT
#define WEAK 							/*__attribute__((weak))*/
#define FORCE_INLINE
#define INLINE
#define  INIT_VALUE                  (u8)0
#define  BYTE2BIT_UNIT				 (u8)8
/*****************************************************************************************************************************/
/*****************************************************************************************************************************/

/********************************************************数据类型定义**********************************************************/
/*****************************************************************************************************************************/
#define	true								1
#define false								0

typedef unsigned char           			u1;
typedef unsigned char           			u8;
typedef signed char             			s8;
typedef unsigned short          			u16;
typedef signed short            			s16;
typedef unsigned long           			u32;
typedef signed long             			s32;
typedef signed long long                    s64;
typedef unsigned long long                  u64;
typedef unsigned char           			boolean;

#define CL_REREST_FLAG       0xA5
#define CT_REREST_FLAG       0xA3
#define SPI_REREST_FLAG      0xA9
#define VM_REREST_FLAG       0xAB
#define I2C_REREST_FLAG      0xAD
typedef int TYPE_JUP_BUF[16];

typedef u16                     			jref;
typedef u32                     			memref;

typedef u8 D_A *							PD8;
typedef u16	D_A *							PD16;
typedef u32	D_A *							PD32;

typedef u8 I_A *							PI8;
typedef u16	I_A *							PI16;
typedef u32	I_A *							PI32;

typedef u8 X_A *							PX8;
typedef u16	X_A *							PX16;
typedef u32	X_A *							PX32;

typedef u8 R_A *							PR8;
typedef u16	R_A *							PR16;
typedef u32	R_A *							PR32;

typedef u8 N_A *							PN8;
typedef u16	N_A *							PN16;
typedef memref								PN24;
typedef u32	N_A *							PN32;

typedef u8	C_A *							PC8;
typedef u16	C_A *							PC16;
typedef u32	C_A *							PC32;

typedef u8	F_A *							PF8;
typedef u16	F_A *							PF16;
typedef u32	F_A *							PF32;

typedef u8	M_A *							PM8;
typedef u16	M_A *							PM16;
typedef u32	M_A *							PM32;

#define HIGH(x) 							U16t8(U16_SHR_U8(x, 8))
#define LOW(x)  							U16t8(U16_AND_U16(x, 0x00ff))
/*****************************************************************************************************************************/
/*****************************************************************************************************************************/

/********************************************************强制类型转换**********************************************************/
/*****************************************************************************************************************************/
#if BOOT__CODE_CHECK__FOR_TEST
	extern u16	U8t16(u8 v);
	extern u32 U8t32(u8 v);
	extern u8  U16t8(u16 v);
	extern u32 U16t32(u16 v);
	extern u8  U32t8(u32 v);
	extern u16 U32t16(u32 v);

	extern memref PXtMEM(PX8 v);
	extern memref PRtMEM(PR8 v);
	extern memref PNtMEM(PN8 v);
	extern memref PCtMEM(PC8 v);
	extern memref PFtMEM(PF8 v);
	extern memref PMtMEM(PM8 v);


	extern PI8  MEMtPI8(memref v);
	extern PI16 MEMtPI16(memref v);
	extern PI32 MEMtPI32(memref v);

	extern PD8  MEMtPD8(memref v);
	extern PD16 MEMtPD16(memref v);
	extern PD32 MEMtPD32(memref v);

	extern PX8  MEMtPX8(memref v);
	extern PX16 MEMtPX16(memref v);
	extern PX32 MEMtPX32(memref v);

	extern PR8  MEMtPR8(memref v);
	extern PR16 MEMtPR16(memref v);
	extern PR32 MEMtPR32(memref v);

	extern PN8  MEMtPN8(memref v);
	extern PN16 MEMtPN16(memref v);
	extern PN24 MEMtPN24(memref v);
	extern PN32 MEMtPN32(memref v);

	extern PC8  MEMtPC8(memref v);
	extern PC16 MEMtPC16(memref v);
	extern PC32 MEMtPC32(memref v);

	extern PF8  MEMtPF8(memref v);
	extern PF16 MEMtPF16(memref v);
	extern PF32 MEMtPF32(memref v);

	extern PM8  MEMtPM8(memref v);
	extern PM16 MEMtPM16(memref v);
	extern PM32 MEMtPM32(memref v);
#else
	#define	U8t16(v)							(u16)(v)
	#define	U8t32(v)							(u32)(v)
	#define U16t8(v)							(u8)(v)
	#define U16t32(v)							(u32)(v)
	#define U32t8(v)							(u8)(v)
	#define U32t16(v)							(u16)(v)

	#define PXtMEM(v)							(memref)(v)
	#define PRtMEM(v)							(memref)(v)
	#define PNtMEM(v)							(memref)(v)
	#define PCtMEM(v)							(memref)(v)
	#define PFtMEM(v)							(memref)(v)
	#define PMtMEM(v)							(memref)(v)

	#define MEMtPI8(v)							(PI8)(v)
	#define MEMtPI16(v)							(PI16)(v)
	#define MEMtPI32(v)							(PI32)(v)

	#define MEMtPD8(v)							(PD8)(v)
	#define MEMtPD16(v)							(PD16)(v)
	#define MEMtPD32(v)							(PD32)(v)

	#define MEMtPX8(v)							(PX8)(memoryToPoint((memref)(v)))
	#define MEMtPX16(v)							(PX16)(memoryToPoint((memref)(v)))
	#define MEMtPX32(v)							(PX32)(memoryToPoint((memref)(v)))

	#define MEMtPR8(v)							(PR8)(v)
	#define MEMtPR16(v)							(PR16)(v)
	#define MEMtPR32(v)							(PR32)(v)

	#define MEMtPN8(v)							(PN8)(v)
	#define MEMtPN16(v)							(PN16)(v)
	#define MEMtPN24(v)                         (PN24)(v)
	#define MEMtPN32(v)							(PN32)(v)

	#define MEMtPC8(v)							(PC8)(v)
	#define MEMtPC16(v)							(PC16)(v)
	#define MEMtPC32(v)							(PC32)(v)

	#define MEMtPF8(v)							(PF8)(v)
	#define MEMtPF16(v)							(PF16)(v)
	#define MEMtPF32(v)							(PF32)(v)

	#define MEMtPM8(v)							(PM8)(v)
	#define MEMtPM16(v)							(PM16)(v)
	#define MEMtPM32(v)							(PM32)(v)
#endif
/*****************************************************************************************************************************/
/*****************************************************************************************************************************/

/*************************************************无符号转有符号强制类型转换(只用在字节码解释上)*********************************/
/*****************************************************************************************************************************/
#if BOOT__CODE_CHECK__FOR_TEST
	extern s16 U8tS16(u8 v);
	extern s32 U8tS32(u8 v);
	extern s16 U16tS16(u16 v);
	extern s32 U16tS32(u16 v);
	extern s32 U32tS32(u32 v);
	extern u16 S16tU16(s16 v);
#else
	#define U8tS16(v)							((s16)(s8)(v))
	#define U8tS32(v)							((s32)(s8)(v))
	#define U16tS16(v)							(s16)(v)
	#define U16tS32(v)							((s32)(s16)(v))
	#define U32tS32(v)							(s32)(v)
	#define S16tU16(v)							(u16)(v)
#endif

/*****************************************************************************************************************************/
/*****************************************************************************************************************************/

/*************************************************只允许ecpDefine.h文件使用的**************************************************/
/*****************************************************************************************************************************/
extern PF8 memoryToPoint(u32 address);
extern u32 pointToMemory(PF8 point);
extern u32 htoj32(u32);
extern u16 htoj16(u16 value);
extern void faultDetected(void);
u16	MEM_R_U16(memref d);
void MEM_W_U16(memref d, u16 v);

extern u32	MEM_R_U32(memref d);
extern void MEM_W_U32(memref d, u32 v);
#endif
