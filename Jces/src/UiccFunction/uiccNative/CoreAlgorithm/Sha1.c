/*              secure hash algorithm 1 (sha1)
**
**              THIS APPLICATION NOTE IS SUBJECT TO THE
**              'CONDITIONS FOR USE OF APPLICATION NOTES'
**
**              joachim velten  11-nov-1999
**
**              (c)  1999  infineon technologies cc at sw
**
**              11-nov-1999: initial creation  jv
*/

/*              derived from 
**              SHA-1 in C by Steve Reid <steve@edmweb.com>, 100% Public Domain
**              and the cc application note sha1  V1.03: 13.01.1998
*/

/*              Test Vectors (from FIPS PUB 180-1)
**              "abc"
**              A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
**
**              "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
**              84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
**
**              A million repetitions of "a"
**              34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F


//============================================================================================
				说明：此处的SHA-1算法代码默认为大端模式，小端模式请定义宏 LittleEndian
//============================================================================================
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Sha1.h"
#include "hard_api.h"
//#include "Transaction.h"





// __align(4) SHA1_CTX  xdata G_Sha1Context;

#if 0

#ifdef LIB_HAL_LittleEndian
/*
**@decription:	将数据按照小端模式存储
**@parameter:	tgData数据缓冲区，len：数据长度,为4的倍数
**@return：		void
**
*/   
void convertBig2Little(unsigned char tgData[64],unsigned char len)
{
	unsigned long tempData;
	unsigned char i;

	for(i=0;i<len;i+=4)
	{
		//tempData = (tgData[i]<<24) | (tgData[i+1]<<16) | (tgData[i+2]<<8) | tgData[i+3];
		tempData = ByteBufToUWORD(&tgData[i]);
		tgData[i] = tempData & 0xFF;
		tgData[i+1] = (tempData>>8)&0xFF;
		tgData[i+2] = (tempData>>16)&0xFF;
		tgData[i+3] = tempData >>24;
	}
}
#endif



/*
 *@description :this function is used to calculate SHA1
 *
 *@param       :state[5],buffer[64]
 *
 *@history     :create @ 091111 
*/
// static void SHA1Transform(  unsigned long * state /*[5]*/,unsigned char * buffer /*[64]*/ )
void SHA1Transform(  unsigned long * state /*[5]*/,unsigned char * buffer /*[64]*/ )
{
        /*      Hash a single 512-bit block. This is the core of the algorithm
        */

        unsigned long idata a;
        unsigned long idata b;
        unsigned long idata c;
        unsigned long idata d;
        unsigned long idata e;
//        

        unsigned char idata t;
        unsigned char idata s;
        unsigned long idata kt;
        unsigned long idata temp;
        
		kt = 0x00;
        //      Copy context->state[] to working vars 
       
        a = state[0];	//state为unsigned long类型时  state的起始地址必须是4字节整数倍 
        b = state[1];
        c = state[2];
        d = state[3];
        e = state[4];

	/*	memcpy((BYTE*)&a, (BYTE*)state, 4);
		memcpy((BYTE*)&b, (BYTE*)(state+4), 4);
		memcpy((BYTE*)&c, (BYTE*)(state+8), 4);
		memcpy((BYTE*)&d, (BYTE*)(state+12), 4);
		memcpy((BYTE*)&e, (BYTE*)(state+16), 4);*/

        for ( t = 0; t < 80; t ++ ) {
        
                /* set constant kt 
                */
                switch( t ) {  
                
                        case 0:
                                kt = 0x5A827999;
                                break;
                        case 20:
                                kt = 0x6ED9EBA1;
                                break;
                        case 40:
                                kt = 0x8F1BBCDC;
                                break;
                        case 60:
                                kt = 0xCA62C1D6;
                                break;
						default:
								break;
                        }
                        
                s = t & 15;
                
                if( t >= 16 ) 
                {    
	/*注意：对于不同的编译器，由于数据存储方式不一样，( ( unsigned long * ) buffer )[s]的值可能会取值错误,如：ATMEL平台*/                                  
                       ( ( unsigned long * ) buffer ) [s] = Sn(
                                      ( ( unsigned long * ) buffer ) [(s+13) & 15] ^
                                      ( ( unsigned long * ) buffer ) [(s+8) & 15] ^
                                      ( ( unsigned long * ) buffer ) [(s+2) & 15] ^
                                      ( ( unsigned long * ) buffer ) [s],1
                                      );
                }
                       
                /*      calculate function ft(B,C,D)
                */
                if( t < 20 ) 
                {
                
                        temp = ( b & c ) | ( ( ~b ) & d );
                }
                else if( t < 40 ) 
                {
                
                        temp = b ^ c ^ d;
                }
                else if( t < 60 ) 
                {
                
                        temp = ( b & c ) | ( b & d ) | ( c & d );
                }
                else 
                {
               		temp = b ^ c ^ d;
                }
 	/*注意：对于不同的编译器，由于数据存储方式不一样，( ( unsigned long * ) buffer )[s]的值可能会取值错误,如：ATMEL平台*/                      
              temp +=  Sn( a, 5 ) + e + ( ( unsigned long * ) buffer )[s] + kt; 
                e = d;
                d = c;
                c = Sn( b, 30 );
                b = a;
                a = temp;
                }

        /*      Add the working vars back into context.state[] 
        */
         state[0] += a;//state为unsigned long*
         state[1] += b;
       state[2] += c;
         state[3] += d;
        state[4] += e;
	/*	*(unsigned long*) state += a;
		*(unsigned long*) (state+4) += b;
		*(unsigned long*) (state+8) += c;
		*(unsigned long*) (state+12) += d;
		*(unsigned long*) (state+16) += e;*/

        /*      Wipe variables 
        */
        a = b = c = d = e = 0;
}

/*
 *@description :this function is used to initial the struct
 *
 *@param       : SHA1_CTX struct
 * 
 *@history     :create @ 091111
*/
void SHA1Init( SHA1_CTX * context ) 
{
	    memset((BYTE*)context,0,sizeof(SHA1_CTX));
        /*      SHA1 initialization
        */
        context->state[0] = 0x67452301;
        context->state[1] = 0xEFCDAB89;
        context->state[2] = 0x98BADCFE;
        context->state[3] = 0x10325476;
        context->state[4] = 0xC3D2E1F0;
        context->count[0] = 0;
		context->count[1] = 0;	
}

/*
*@description: 计算SHA1算法的中间数据. 
*
*fucntion    : SHA1Update
*
*@param      : SHA1_CTX *context     SHA1算法的数据结构
*@param      : unsigned char *dat    输入的源数据的起始ram地址
*@param      : unsigned int len      输入数据的长度
*@param      : BYTE mode             0表示源数据来自于ram数组；此时fileAddr无意义
*                                    1表示源数据来自于E2地址; 此时input无意义
*@param      : UWORD addr            输入的源数据的起始E2地址
*
*return      : void
*
*@history    : created by WG@20070627
*
*/
void SHA1Update(SHA1_CTX * context, unsigned char* dat, unsigned int len, BYTE mode, UWORD addr) 
{

        /*      Run your data through this
        */
        unsigned int idata i;
        unsigned char idata j;
	
		if(len == 0)
		{
			return;
		}

        j = ( unsigned long ) context ->count[0] >> 3 & 63; /* need only lowest 11 bits,计算最后一块的字节数*/
        
        if( ( context ->count[0] += (( unsigned long ) len << 3) ) < (( unsigned long ) len << 3 )) context ->count[1] ++; 
        
        if( (j + len) > 63 ) 
        {
				if(mode == 0x00)
				{
                	memcpy( (BYTE*)(context ->buffer + j), dat, i = 64 - j );
				}
				else
				{
//					VMReadBuf((BYTE*)(context ->buffer + j), addr, i = 64 - j);		  
					memmove((BYTE*)(context ->buffer + j), (BYTE*)addr, i = 64 - j);
				}
#ifdef LIB_HAL_LittleEndian
				convertBig2Little((unsigned char *)context ->buffer,64);
#endif               
                SHA1Transform((unsigned long*) context->state, (BYTE*)context->buffer );//SHA1Transform((unsigned long*) context->state, (BYTE*)context->buffer );//
           
                for ( ; i + 63 < len; i += 64 ) 
                {
					if(mode == 0x00)
					{
						memcpy( (BYTE*)(context->buffer + 0), dat + i, 64 );  /* must copy dat, may be in e2 or rom space !*/
					}
					else
					{
//						VMReadBuf((BYTE*)(context ->buffer + 0), addr+i, 64);				  
						memmove((BYTE*)(context ->buffer + 0), (BYTE*)(addr+i), 64);
					}
						
#ifdef LIB_HAL_LittleEndian
					convertBig2Little((unsigned char *)context ->buffer,64);
#endif
 
                    SHA1Transform((unsigned long*) context->state, (BYTE*)context->buffer );//SHA1Transform((unsigned long*) context->state, (BYTE*)context->buffer );//

                }
				
                j = 0;
        }
        else 
        {
                i = 0;
        }

		if(mode == 0x00)
		{
        	memcpy( (BYTE*)(context ->buffer + j), dat + i, len - i );
		}
		else
		{
//			 VMReadBuf((BYTE*)(context ->buffer + j), addr+i, len - i);		   	
			memmove((BYTE*)(context ->buffer + j), (BYTE*)(addr+i), len - i);
		}
}

/*
*@Function: SHA1Final
*
*@Descrip:   该函数根据shaContext的值计算20字节哈希SHA1，
             shaContext中的值是上一次计算结果的缓存(计算最后一步)
*
*@See:       无
*@Param:     shaContext    哈希运算过程中的中间值
*@Param:     inLength:     数据长度
*@Param:     inAddr:	   输入数据的地址
*@Param:     *result:      输出
*@Return:    无
*
*@Author:	lgy@20150527
*
*@History:  V1.0:创建函数
*/
void SHA1Final(SHA1_CTX* shaContext , BYTE * result)
{
	BYTE idata i,filldata;
	BYTE xdata  finalcount[8];	

	for( i = 0; i < 8; i++ ) 
    {
        finalcount[i] = ( unsigned char ) ( (shaContext->count[(i >= 4 ? 0 : 1)] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 ); 
    }

    filldata = 0x80;

    SHA1Update(shaContext,&filldata, 1,0,0 );   /*fill 0x80*/

    filldata = 0x00;

    while( (shaContext->count[0] & 504) != 448 ) 
    {
        SHA1Update(shaContext,&filldata, 1,0,0 );         /*fill 0x00 until 448*/       
    }

    SHA1Update(shaContext,finalcount, 8,0,0 );      /*Add initial data length add calculate SHA1*/

    /* Wipe variables 
    */
    memset( (BYTE*)shaContext->buffer, 0, 64 );
    memset((BYTE*) shaContext->count, 0, 8 );
    memset(finalcount, 0, 8 );
    
    for( i = 0; i < 20; i++ )
    {
        *(result + i) = (unsigned char) ( (shaContext->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255 );
    }

	memset( (BYTE*)shaContext->state, 0, 20 );
}

/*
 *@description :this function is used to implement Secure Hash Algorithm1，该函数的使用是独立的，只需要输入源数据和数据长度即可计算出HASH值
 *
 *@param       :unsigned char* digest     输出的HASH值，长度固定为20字节
 *@param       :unsigned char* sourcedata 输入的源数据的起始ram地址
 *@param       :unsigned int length       输入数据的长度
 *@param      : BYTE mode                 0表示源数据来自于ram数组/1表示源数据来自于E2地址
 *@param      : UWORD fileAddr            输入的源数据的起始E2地址 
 *
 */
void SHA1DoFinalSingleCall(unsigned char * digest /*[20]*/, unsigned char* sourcedata,unsigned int length, BYTE mode,UWORD fileAddr) 
{
	SHA1_CTX xdata context;
			   
	SHA1Init(&context);      /*初始化结构体*/

	SHA1Update(&context,sourcedata,length,mode,fileAddr);

	SHA1Final(&context, digest);
}

#endif
