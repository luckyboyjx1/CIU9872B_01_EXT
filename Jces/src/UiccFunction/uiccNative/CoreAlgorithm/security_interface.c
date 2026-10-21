
#include  <stdio.h>
#include  <string.h>
#include  "typedef.h"
#include  "main.h"
#include  "gsmpublic.h"
#include  "interface.h"
#include  "security_interface.h"


/*
	安全服务接口
*/
/*
void security(void)
{
	BYTE idata ret;
	unsigned short idata block_no;
	
	block_no = (P1&0x7F) << 8 | P2; //块号
	
	ret = ReceiveData();
	if(TRUE == ret)
	{
		switch(RecData[0])
		{
			case 0x01:
				SP_accredit();
				break;
			case 0x02:
				SP_self();
				break;
			case 0x03:
				get_random();
				break;
			case 0x04:
				key_arrange();
				break;
		}
			
	}
}
*/

/*
	SP授权
*/
/*
void SP_accredit(void)
{
	
}
*/

/*
	SP自服务
*/
/*
void SP_self(void)
{
	
}

void get_random(void)
{

}

void key_arrange(void)
{

}
*/
