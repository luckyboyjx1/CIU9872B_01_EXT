/*
 * LteAPI.c
 *
 *  Created on: 2016Äê6ÔÂ7ÈÕ
 *      Author: linguoming1
 */


#include "LteAPI.h"

#if LTE


u8 direction;
u8 paraPointer;
u16 para[10];

void initInterServiceVar(void)
{
	direction = 0;
	paraPointer = 0;
	ecpMem_Set(para,0,20);
}
void resetPointer(void)
{
	paraPointer = 0;
}

void sendParaObj(void)
{
	if(direction != 1) {
		paraPointer = 0;
		direction = 1;
	}

	para[paraPointer++] = popReference();
}

void sendParaShort(void)
{
	if(direction != 1) {
		paraPointer = 0;
		direction = 1;
	}

	para[paraPointer++] = popShort();
}

void rcvParaObj(void)
{
	if(direction != 2) {
		paraPointer = 0;
		direction = 2;
	}

	 pushReference(para[paraPointer++]);
}

void rcvParaShort(void)
{
	if(direction != 2) {
		paraPointer = 0;
		direction = 2;
	}
	 pushShort(para[paraPointer++]);
}


#endif
