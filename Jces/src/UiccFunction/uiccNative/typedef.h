#ifndef    _type_def_h
#define	   _type_def_h
//#include  "eepromDefine.h"
//#include "FunctionSwitch.h"
//#include "FunctionSwitch.h"
//#include "Crypto_API.h"
#include "Crypto_Constant.h"

typedef	   	unsigned char    BYTE;
typedef 	signed char		 SBYTE;
typedef    	unsigned short   WORD;
typedef 	signed short	 SWORD;
typedef    	unsigned int     PWORD;  
typedef    	unsigned long    DWORD;
typedef     unsigned int     UWORD;
typedef		signed int		 SDWORD;
 

typedef 	signed short 	jshort;
typedef 	signed char 	jbyte;
//typedef 	signed char 	jboolean;
typedef     unsigned short  jref;
typedef	  	unsigned char   jbool;
typedef     unsigned short  objref_t;	        //引用应为2字节,by xwh 20130829


#define    code         				const
#define    FLASH                       const 
	
#define     memcmp_p      memcmp 
#define    	memcpy_G      memcpy

//extern void Random_number_generator(BYTE * rngStoreAddr, WORD rngLenth , BYTE mode);
//#define    Random_Number_Generator_Get(rngStoreAddr, rngLenth)  Random_number_generator(rngStoreAddr, rngLenth , 1)
#define    Random_Number_Generator_Get(rngStoreAddr, rngLenth)  Random_number_generator(rngStoreAddr, rngLenth , ALG_SECURE_RANDOM)  //产生真随机数

#define    ReceiveFirstByte				ReceiveByte
#define    sleep_mode    				SleepMode 
#define    flash_read                   ReadEeprom


#define BIT0    (1ul<<0)
#define BIT1    (1ul<<1)
#define BIT2    (1ul<<2)
#define BIT3    (1ul<<3)
#define BIT4    (1ul<<4)
#define BIT5    (1ul<<5)
#define BIT6    (1ul<<6)
#define BIT7    (1ul<<7)
#define BIT8    (1ul<<8)
#define BIT9    (1ul<<9)
#define BIT10   (1ul<<10)
#define BIT11   (1ul<<11)
#define BIT12   (1ul<<12)
#define BIT13   (1ul<<13)
#define BIT14   (1ul<<14)
#define BIT15   (1ul<<15)
#define BIT16   (1ul<<16)
#define BIT17   (1ul<<17)
#define BIT18   (1ul<<18)
#define BIT19   (1ul<<19)
#define BIT20   (1ul<<20)
#define BIT21   (1ul<<21)
#define BIT24   (1ul<<24)
#define BIT26   (1ul<<26)
#define BIT30   (1ul<<30)
#define BIT31   (1ul<<31)  
//----------------------------------------------
typedef		unsigned 	char		u8;
typedef		signed		char		s8;
typedef		unsigned 	short		u16;
typedef		signed		short		s16;
//typedef		unsigned 	int			u32;
//typedef		signed		int			s32;


//----------------------------------------------------------
#define    	TRUE     1
#define    	FALSE    0
#define    	EOL      0
#define    	EQUAL    0

/*
#define	   memcmp_p		   _memcmp_               //比较结果是有符号数，判断结果为 >EQUAL 或 < EQUAL 时使用该函数
#define	   memcmp_G		   _Memory_Cmp            //比较结果是无符号数，结果为 ==EQUAL 或 != EQUAL 时使用该函数
#define    memcpy_G        _Memory_Copy          
#define    memset_G        _Memory_Set
#define    memmove_G       _Memory_Move
*/
  

#define    T0_IS_ON 	1
#define    T0_IS_OFF 	0
#define    data	
#define    xdata
#define    idata
#define    bdata
//#define    CONST

#define FLASH_MODE
#define __packed


#define ILLEGAL_CHANNEL            (0xFF)
#undef  NULL
#define NULL  								((void *)0)

#define ADDR_IS_NULL                       0x00000000
#endif
