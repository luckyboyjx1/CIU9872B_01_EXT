#ifndef __OPTSPACE_H__
#define __OPTSPACE_H__

#include "target.h"
#include "profile.h"
#include "global_variable.h"
#include "crypto_impl.h"
#include "MemOperation.h"
#include "typejava.h"

extern u8 edcFlag;
extern PX8		 									D_A 						JSP;
extern PX8 											D_A 						FP;
extern PX8											D_A							FPend;
extern u8											D_A 						FPsize;
extern PX8		 									D_A 						Top;
extern PF8		 									D_A 						JPC;
extern u32											X_A							VMStack[STACKSIZE/4];//modify by jiangxiaohui@20220324 ·Ö²ã

#if EMV_OPTIMIZATION_CODE

FORCE_INLINE u8 fetchByte(void)
{
	u8		D_A		value;

	value	= PF_R_U8(JPC); /* PC is either in E2P or in ROM... */
	PF8_INC(JPC);
	return (value);
}

FORCE_INLINE u16 fetchShort(void)
{
	u16		D_A		value;

	value	= PF_R_U16(JPC);
	JPC		= PF8_A_CONST(JPC, SIZE_SHORT_L);
	return (value);
}

FORCE_INLINE jref fetchReference(void)
{
	u16		D_A		value;

	value 	= PF_R_U16(JPC);
	JPC		= PF8_A_CONST(JPC, SIZE_REF_L);
	return (value);
}


#define  pushShort(value)    	{   PXS_W_U16(JSP, value); \
									JSP	= PX8_A_CONST(JSP, 2); \
									if (JSP > Top)		\
									{				\
										fatal_error(STACK_OVER_FLOW_ERROR);\
									}\
								}							
#else
extern u8 fetchByte(void);								
#endif

#endif
