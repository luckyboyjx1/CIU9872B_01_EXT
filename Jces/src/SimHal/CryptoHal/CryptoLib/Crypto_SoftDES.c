/************************************************************
  Copyright (C), 1988-1999, Eastcompeace. Co., Ltd.
  FileName: Crypto_SoftDES.c.
  Data Encryption Standard (DES) of the U.S Federal
  Information Processing Standard (FIPS)
  Author: Lizumeng.
  Version : 1.0.
  Date: 2010/04/16.
  Description: This source file contains functions used to compute
               DES.
  History:
      <author>    <time>    <version>    <desc>
      Lizumeng   10/04/16     1.0     build this moudle
*************************************************************/
#include "Crypto_SoftDES.h"
#include "SysConfigHalRam.h"

#if WITH_SOFT_DES

unsigned char DES_keyRegister[24]= {0};

CONST static unsigned char DES_leftshift[16]= {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

CONST static unsigned char DES_rightshift[16]= {0,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

CONST static unsigned char s[256] = {
		0xe0, 0x4f, 0xd7, 0x14, 0x2e, 0xf2, 0xbd, 0x81,
		0x3a, 0xa6, 0x6c, 0xcb, 0x59, 0x95, 0x03, 0x78,
		0x4f, 0x1c, 0xe8, 0x82, 0xd4, 0x69, 0x21, 0xb7,
		0xf5, 0xcb, 0x93, 0x7e, 0x3a, 0xa0, 0x56, 0x0d,
		0xf3, 0x1d, 0x84, 0xe7, 0x6f, 0xb2, 0x38, 0x4e,
		0x9c, 0x70, 0x21, 0xda, 0xc6, 0x09, 0x5b, 0xa5,
		0x0d, 0xe8, 0x7a, 0xb1, 0xa3, 0x4f, 0xd4, 0x12,
		0x5b, 0x86, 0xc7, 0x6c, 0x90, 0x35, 0x2e, 0xf9,
		0xad, 0x07, 0x90, 0xe9, 0x63, 0x34, 0xf6, 0x5a,
		0x12, 0xd8, 0xc5, 0x7e, 0xbc, 0x4b, 0x2f, 0x81,
		0xd1, 0x6a, 0x4d, 0x90, 0x86, 0xf9, 0x38, 0x07,
		0xb4, 0x1f, 0x2e, 0xc3, 0x5b, 0xa5, 0xe2, 0x7c,
		0x7d, 0xd8, 0xeb, 0x35, 0x06, 0x6f, 0x90, 0xa3,
		0x14, 0x27, 0x82, 0x5c, 0xb1, 0xca, 0x4e, 0xf9,
		0xa3, 0x6f, 0x90, 0x06, 0xca, 0xb1, 0x7d, 0xd8,
		0xf9, 0x14, 0x35, 0xeb, 0x5c, 0x27, 0x82, 0x4e,
		0x2e, 0xcb, 0x42, 0x1c, 0x74, 0xa7, 0xbd, 0x61,
		0x85, 0x50, 0x3f, 0xfa, 0xd3, 0x09, 0xe8, 0x96,
		0x4b, 0x28, 0x1c, 0xb7, 0xa1, 0xde, 0x72, 0x8d,
		0xf6, 0x9f, 0xc0, 0x59, 0x6a, 0x34, 0x05, 0xe3,
		0xca, 0x1f, 0xa4, 0xf2, 0x97, 0x2c, 0x69, 0x85,
		0x06, 0xd1, 0x3d, 0x4e, 0xe0, 0x7b, 0x53, 0xb8,
		0x94, 0xe3, 0xf2, 0x5c, 0x29, 0x85, 0xcf, 0x3a,
		0x7b, 0x0e, 0x41, 0xa7, 0x16, 0xd0, 0xb8, 0x6d,
		0x4d, 0xb0, 0x2b, 0xe7, 0xf4, 0x09, 0x81, 0xda,
		0x3e, 0xc3, 0x95, 0x7c, 0x52, 0xaf, 0x68, 0x16,
		0x16, 0x4b, 0xbd, 0xd8, 0xc1, 0x34, 0x7a, 0xe7,
		0xa9, 0xf5, 0x60, 0x8f, 0x0e, 0x52, 0x93, 0x2c,
		0xd1, 0x2f, 0x8d, 0x48, 0x6a, 0xf3, 0xb7, 0x14,
		0xac, 0x95, 0x36, 0xeb, 0x50, 0x0e, 0xc9, 0x72,
		0x72, 0xb1, 0x4e, 0x17, 0x94, 0xca, 0xe8, 0x2d,
		0x0f, 0x6c, 0xa9, 0xd0, 0xf3, 0x35, 0x56, 0x8b
		};


/*******************************************************************************
 * Function name: SoftwareDES
 * Description: single DES encryption or decryption function.
 * Input Parameters:   (precondition)
 *		unsigned char dataOut[8]             out data buffer
 *		unsigned char dataIn[8]		         input data buffer.
 *      unsigned char keypointer[8]          DES key buffer
 *      unsigned char mode                   encryption or decryption mode
 * Output Parameters:  (postcondition)
 *		none
 * NOTE: dataOut buffer and dataIn can be the same buffer
 ********************************************************************************/
static void SoftwareDES(unsigned char dataOut[8], unsigned char dataIn[8], unsigned char keypointer[8], unsigned char mode)
{
    unsigned char x,i,j,iter;
    unsigned char outblock[6];
    unsigned char plain[8];
	unsigned char key[8];


	ecpMem_Copy(plain,dataIn,8);
	ecpMem_Copy(key,keypointer,8);

    for(i=0; i<4; i++)         // IP permutation
    {
        x=plain[i];

        plain[i]=(plain[i]&0x0f)|((plain[i+4]&0x0f)<<4);

        plain[i+4]=(plain[i+4]&0xf0)|((x&0xf0)>>4);

    }

    for(i=0;i<2;i++)
    {
        x=plain[i];

        plain[i]=((plain[i+2]<<2)&0xcc)|(plain[i]&0x33);

        plain[i+2]= (plain[i+2]&0xcc)|((x&0xcc)>>2);

    }

    for(i=4;i<6;i++)
    {
        x=plain[i];

        plain[i]=((plain[i+2]<<2)&0xcc)|(plain[i]&0x33);

        plain[i+2]= (plain[i+2]&0xcc)|((x&0xcc)>>2);

    }

    for(i=0;i<8;i=i+2)
    {
        x=plain[i];

        plain[i]=((plain[i+1]<<1)&0xaa)|(plain[i]&0x55);

        plain[i+1]= (plain[i+1]&0xaa)|((x&0xaa)>>1);
    }

    x=plain[0];

    plain[0]=plain[6];

    plain[6]=plain[3];

    plain[3]=x;

    x=plain[1];

    plain[1]=plain[4];

    plain[4]=plain[7];

    plain[7]=x;

    ecpMem_Copy(dataOut,plain,8);

    plain[0]=((key[7]>>0)&0x80)|            // pc1

             ((key[6]>>1)&0x40)|

             ((key[5]>>2)&0x20)|

             ((key[4]>>3)&0x10)|

             ((key[3]>>4)&0x08)|

             ((key[2]>>5)&0x04)|

             ((key[1]>>6)&0x02)|

             ((key[0]>>7)&0x01);

    plain[1]=((key[7]<<1)&0x80)|

             ((key[6]>>0)&0x40)|

             ((key[5]>>1)&0x20)|

             ((key[4]>>2)&0x10)|

             ((key[3]>>3)&0x08)|

             ((key[2]>>4)&0x04)|

             ((key[1]>>5)&0x02)|

             ((key[0]>>6)&0x01);

    plain[2]=((key[7]<<2)&0x80)|

             ((key[6]<<1)&0x40)|

             ((key[5]<<0)&0x20)|

             ((key[4]>>1)&0x10)|

             ((key[3]>>2)&0x08)|

             ((key[2]>>3)&0x04)|

             ((key[1]>>4)&0x02)|

             ((key[0]>>5)&0x01);

    plain[3]=((key[7]<<3)&0x80)|

             ((key[6]<<2)&0x40)|

             ((key[5]<<1)&0x20)|

             ((key[4]<<0)&0x10);

    plain[4]=((key[7]<<6)&0x80)|

             ((key[6]<<5)&0x40)|

             ((key[5]<<4)&0x20)|

             ((key[4]<<3)&0x10)|

             ((key[3]<<2)&0x08)|

             ((key[2]<<1)&0x04)|

             ((key[1]<<0)&0x02)|

             ((key[0]>>1)&0x01);

    plain[5]=((key[7]<<5)&0x80)|

             ((key[6]<<4)&0x40)|

             ((key[5]<<3)&0x20)|

             ((key[4]<<2)&0x10)|

             ((key[3]<<1)&0x08)|

             ((key[2]<<0)&0x04)|

             ((key[1]>>1)&0x02)|

             ((key[0]>>2)&0x01);

    plain[6]=((key[7]<<4)&0x80)|

             ((key[6]<<3)&0x40)|

             ((key[5]<<2)&0x20)|

             ((key[4]<<1)&0x10)|

             ((key[3]<<0)&0x08)|

             ((key[2]>>1)&0x04)|

             ((key[1]>>2)&0x02)|

             ((key[0]>>3)&0x01);

    plain[7]=((key[3]<<3)&0x80)|

             ((key[2]<<2)&0x40)|

             ((key[1]<<1)&0x20)|

             ((key[0]<<0)&0x10);

    ecpMem_Copy(key,plain,8);

    for(iter=0;iter<16;iter++)       // 16 iterations
    {
	   	ecpMem_Copy(outblock,&dataOut[4],4);

        plain[0]=((outblock[3] & 0x01) << 7)|     	//ex0001,0370,0030,0007,0200,0001,0340,0030,0037,0200,0001,0370,0030,0007,0200,0001,0340,0030,0037,0200,

		         ((outblock[0] & 0xF8) >> 1)|

		         ((outblock[0] & 0x18) >> 3);

	    plain[1]=((outblock[0] & 0x07) << 5)|

		         ((outblock[1] & 0x80) >> 3)|

		         ((outblock[0] & 0x01) << 3)|

		         ((outblock[1] & 0xE0) >> 5);

	    plain[2]=((outblock[1] & 0x18) << 3)|

		         ((outblock[1] & 0x1F) << 1)|

		         ((outblock[2] & 0x80) >> 7);

	    plain[3]=((outblock[1] & 0x01) << 7)|

		         ((outblock[2] & 0xF8) >> 1)|

		         ((outblock[2] & 0x18) >> 3);

	    plain[4]=((outblock[2] & 0x07) << 5)|

		         ((outblock[3] & 0x80) >> 3)|

		         ((outblock[2] & 0x01) << 3)|

		         ((outblock[3] & 0xE0) >> 5);

	    plain[5]=((outblock[3] & 0x18) << 3)|

		         ((outblock[3]& 0x1F) << 1)|

		         ((outblock[0] & 0x80) >> 7);

       	if(mode == EN_DES) //En
       	{
		    i=DES_leftshift[iter];              // leftshift

		    while(i>0)
			{

                for(j=0;j<8;j=j+4)
				{
			        x=key[j];

			        key[j]=(key[j]<<1)|((key[j+1]&0x80)>>7);

			        key[j+1]=(key[j+1]<<1)|((key[j+2]&0x80)>>7);

		    	    key[j+2]=(key[j+2]<<1)|((key[j+3]&0x80)>>7);

			        key[j+3]=(key[j+3]<<1)|((x&0x80)>>3);
				 }

                i--;

            }

            outblock[0]=((key[1]<<5)&0x80)|    // pc2

                        ((key[2]>>1)&0x40)|

                        ((key[1]<<0)&0x20)|

                        ((key[2]<<4)&0x10)|

                        ((key[0]>>4)&0x08)|

                        ((key[0]>>1)&0x04)|

                        ((key[0]>>4)&0x02)|

                        ((key[3]>>4)&0x01);

			outblock[1]=((key[1]<<6)&0x80)|

                        ((key[0]<<4)&0x40)|

                        ((key[2]<<2)&0x20)|

                        ((key[1]>>2)&0x10)|

                        ((key[2]<<2)&0x08)|

                        ((key[2]>>3)&0x04)|

                        ((key[1]>>3)&0x02)|

                        ((key[0]>>4)&0x01);

			outblock[2]=((key[3]<<1)&0x80)|

                        ((key[0]<<6)&0x40)|

                        ((key[1]<<5)&0x20)|

                        ((key[0]<<3)&0x10)|

                        ((key[3]>>2)&0x08)|

                        ((key[2]>>2)&0x04)|

                        ((key[1]>>2)&0x02)|

                        ((key[0]>>6)&0x01);

			outblock[3]=((key[5]<<4)&0x80)|

                        ((key[6]<<6)&0x40)|

                        ((key[4]<<0)&0x20)|

                        ((key[5]>>3)&0x10)|

                        ((key[6]>>2)&0x08)|

                        ((key[7]>>3)&0x04)|

                        ((key[4]>>5)&0x02)|

                        ((key[5]>>4)&0x01);

			outblock[4]=((key[6]<<6)&0x80)|

                        ((key[6]>>1)&0x40)|

                        ((key[4]<<2)&0x20)|

                        ((key[6]<<0)&0x10)|

                        ((key[5]<<3)&0x08)|

                        ((key[6]>>1)&0x04)|

                        ((key[5]>>4)&0x02)|

                        ((key[7]>>4)&0x01);

			outblock[5]=((key[4]<<5)&0x80)|

                        ((key[7]>>1)&0x40)|

                        ((key[6]>>1)&0x20)|

                        ((key[5]<<2)&0x10)|

                        ((key[6]<<1)&0x08)|

                        ((key[4]<<2)&0x04)|

                        ((key[4]>>6)&0x02)|

                        ((key[4]>>4)&0x01);

       	}

		else//de
       	{
            i=DES_rightshift[iter];             // rightshift

		    while(i>0)
			{

                for(j=0;j<8;j=j+4)
				{
			        x=key[j+3];

			        key[j+3]=((key[j+3]>>1)|((key[j+2]&0x01)<<7))&0xf0;

			        key[j+2]=(key[j+2]>>1)|((key[j+1]&0x01)<<7);

		    	    key[j+1]=(key[j+1]>>1)|((key[j]&0x01)<<7);

			        key[j]=(key[j]>>1)|((x&0x10)<<3);
				 }

                i--;

            }

           outblock[0]=((key[1]<<5)&0x80)|    // pc2

                        ((key[2]>>1)&0x40)|

                        ((key[1]<<0)&0x20)|

                        ((key[2]<<4)&0x10)|

                        ((key[0]>>4)&0x08)|

                        ((key[0]>>1)&0x04)|

                        ((key[0]>>4)&0x02)|

                        ((key[3]>>4)&0x01);

			outblock[1]=((key[1]<<6)&0x80)|

                        ((key[0]<<4)&0x40)|

                        ((key[2]<<2)&0x20)|

                        ((key[1]>>2)&0x10)|

                        ((key[2]<<2)&0x08)|

                        ((key[2]>>3)&0x04)|

                        ((key[1]>>3)&0x02)|

                        ((key[0]>>4)&0x01);

			outblock[2]=((key[3]<<1)&0x80)|

                        ((key[0]<<6)&0x40)|

                        ((key[1]<<5)&0x20)|

                        ((key[0]<<3)&0x10)|

                        ((key[3]>>2)&0x08)|

                        ((key[2]>>2)&0x04)|

                        ((key[1]>>2)&0x02)|

                        ((key[0]>>6)&0x01);

			outblock[3]=((key[5]<<4)&0x80)|

                        ((key[6]<<6)&0x40)|

                        ((key[4]<<0)&0x20)|

                        ((key[5]>>3)&0x10)|

                        ((key[6]>>2)&0x08)|

                        ((key[7]>>3)&0x04)|

                        ((key[4]>>5)&0x02)|

                        ((key[5]>>4)&0x01);

			outblock[4]=((key[6]<<6)&0x80)|

                        ((key[6]>>1)&0x40)|

                        ((key[4]<<2)&0x20)|

                        ((key[6]<<0)&0x10)|

                        ((key[5]<<3)&0x08)|

                        ((key[6]>>1)&0x04)|

                        ((key[5]>>4)&0x02)|

                        ((key[7]>>4)&0x01);

			outblock[5]=((key[4]<<5)&0x80)|

                        ((key[7]>>1)&0x40)|

                        ((key[6]>>1)&0x20)|

                        ((key[5]<<2)&0x10)|

                        ((key[6]<<1)&0x08)|

                        ((key[4]<<2)&0x04)|

                        ((key[4]>>6)&0x02)|

                        ((key[4]>>4)&0x01);

        }

        plain[0]=plain[0] ^ outblock[0];     // XOR

        plain[1]=plain[1] ^ outblock[1];

        plain[2]=plain[2] ^ outblock[2];

        plain[3]=plain[3] ^ outblock[3];

        plain[4]=plain[4] ^ outblock[4];

        plain[5]=plain[5] ^ outblock[5];

        plain[7]=plain[5]&0x3f;               // 6 sets of 8 bits turn into 8 sets of 6 bits

        plain[6]=(plain[5]&0xc0)>>6 | ((plain[4]&0x0f)<<2);

        plain[5]=(plain[4]&0xf0)>>4 | ((plain[3]&0x03)<<4);

        plain[4]=(plain[3]&0xfc)>>2;

        plain[3]=plain[2]&0x3f;

        plain[2]=((plain[2]&0xc0)>>6) | ((plain[1]&0x0f)<<2);

        plain[1]=((plain[1]&0xf0)>>4) | ((plain[0]&0x03)<<4);

        plain[0]=(plain[0]&0xfc)>>2;

        ecpMem_Fill(outblock,0,6);                   // get s box

		x=0;

        for(i=0;i<8;i++)
        {
            j = plain[i]&0x3f;

            if((i==0) || (i==2) || (i==4) || (i==6))
            {
	           	if(j%2==0)
				{
	          		outblock[x] |= (s[32*i+j/2])&0xf0;
				}
	          	else
				{
	          		outblock[x] |= (s[32*i+j/2]&0x0f)<<4;
				}
	        }

	        else
	        {
	          	if(j%2==0)
				{
	          		outblock[x] |= (s[32*i+j/2]&0xf0)>>4;
				}

				else
				{
	          		outblock[x] |= s[32*i+j/2]&0x0f;
				}

				x++;
	       }

     }

     plain[0]=((outblock[1]<<7)&0x80)|   // p permutation

              ((outblock[0]<<5)&0x40)|

              ((outblock[2]<<1)&0x20)|

              ((outblock[2]<<1)&0x10)|

              ((outblock[3]<<0)&0x08)|

              ((outblock[1]>>2)&0x04)|

              ((outblock[3]>>3)&0x02)|

              ((outblock[2]>>7)&0x01);

     plain[1]=((outblock[0]<<0)&0x80)|

              ((outblock[1]<<5)&0x40)|

              ((outblock[2]<<4)&0x20)|

              ((outblock[3]>>2)&0x10)|

              ((outblock[0]<<0)&0x08)|

              ((outblock[2]>>4)&0x04)|

              ((outblock[3]<<0)&0x02)|

              ((outblock[1]>>6)&0x01);

     plain[2]=((outblock[0]<<1)&0x80)|

              ((outblock[0]<<6)&0x40)|

              ((outblock[2]<<5)&0x20)|

              ((outblock[1]<<2)&0x10)|

              ((outblock[3]<<3)&0x08)|

              ((outblock[3]>>3)&0x04)|

              ((outblock[0]>>4)&0x02)|

              ((outblock[1]>>7)&0x01);

     plain[3]=((outblock[2]<<2)&0x80)|

              ((outblock[1]<<3)&0x40)|

              ((outblock[3]<<3)&0x20)|

              ((outblock[0]<<2)&0x10)|

              ((outblock[2]<<1)&0x08)|

              ((outblock[1]>>3)&0x04)|

              ((outblock[0]>>3)&0x02)|

              ((outblock[3]>>7)&0x01);

     plain[0]=plain[0] ^ dataOut[0];    // exchange L and R

     plain[1]=plain[1] ^ dataOut[1];

     plain[2]=plain[2] ^ dataOut[2];

     plain[3]=plain[3]^ dataOut[3];

     ecpMem_Copy(dataOut,&dataOut[4],4);

     ecpMem_Copy(&dataOut[4],plain,4);

    }

    for(i=0;i<4;i++)         // exchange L and R
    {
        dataOut[i]^=dataOut[i+4];

        dataOut[i+4]^=dataOut[i];

        dataOut[i]^=dataOut[i+4];

    }

    x=dataOut[0];     // IP_1 permutation

    dataOut[0]=dataOut[3];

    dataOut[3]=dataOut[6];

    dataOut[6]=x;

    x=dataOut[1];

    dataOut[1]=dataOut[7];

    dataOut[7]=dataOut[4];

    dataOut[4]=x;

    for(i=0;i<8;i=i+2)
    {
        x=dataOut[i+1];

        dataOut[i+1]=((dataOut[i]>>1)&0x55)|(dataOut[i+1]&0xaa);

        dataOut[i]= (dataOut[i]&0x55)|((x&0x55)<<1);

    }

    for(i=0;i<2;i++)
    {
        x=dataOut[i+2];

        dataOut[i+2]=((dataOut[i]>>2)&0x33)|(dataOut[i+2]&0xcc);

        dataOut[i]= (dataOut[i]&0x33)|((x&0x33)<<2);
    }

    for(i=4;i<6;i++)
    {
        x=dataOut[i+2];

        dataOut[i+2]=((dataOut[i]>>2)&0x33)|(dataOut[i+2]&0xcc);

        dataOut[i]= (dataOut[i]&0x33)|((x&0x33)<<2);

    }

    for(i=0;i<4;i++)
    {
        x=dataOut[i];

        dataOut[i]=(dataOut[i]&0x0f)|((dataOut[i+4]&0x0f)<<4);

        dataOut[i+4]=(dataOut[i+4]&0xf0)|((x&0xf0)>>4);
    }
}

void SoftDes_loadDESKey(u8 keyByteLen, PX32 pExternKey)
{
	ecpMem_Copy(DES_keyRegister, MEMtPX8(pExternKey), keyByteLen);
	return;
}

/**********************************************************************************
 * Function name: HardwareDES
 * Description: This function does single DES kernel operation with hardware implement.
 * Input parameter:
 *      u8 dataOut[8]               cipher data.
 *      u8 dataIn[8]                plain data.
 *      u8 keyPointer[8]      DES key.
 *      u8 algType                  DES mode, encrypt or decrypt.
 * Output parameter:
 *		none.
 * NOTE: dataIn could be same as dataOut.
 **********************************************************************************/
void SoftDes_HardwareDES(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[8], u8 algType)
{
	switch(algType)
	{
		case MODE_ENCRYPT:
			SoftwareDES(dataOut, dataIn, pExternKey, MODE_ENCRYPT);
			break;
		case MODE_DECRYPT:
			SoftwareDES(dataOut, dataIn, pExternKey, MODE_DECRYPT);
			break;
	}
}
/**********************************************************************************
 * Function name: Hardware3DES2Key
 * Description: This function does triple DES 2 key kernel operation with hardware implement.
 * Input parameter:
 *      u8 dataOut[8]               cipher data.
 *      u8 dataIn[8]                plain data.
 *      u8 keyPointer[16]	    DES key.
 *      u8 algType                  DES mode, encrypt or decrypt.
 * Output parameter:
 *		none.
 * NOTE: dataIn could be same as dataOut.
 **********************************************************************************/
void SoftDes_Hardware3DES2Key(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[16], u8 algType)
{
	switch(algType)
	{
		case MODE_ENCRYPT:
			SoftwareDES(dataOut, dataIn, pExternKey, MODE_ENCRYPT);
			SoftwareDES(dataOut, dataOut, &pExternKey[8], MODE_DECRYPT);
			SoftwareDES(dataOut, dataOut, pExternKey, MODE_ENCRYPT);
			break;
		case MODE_DECRYPT:
			SoftwareDES(dataOut, dataIn, pExternKey, MODE_DECRYPT);
			SoftwareDES(dataOut, dataOut, &pExternKey[8], MODE_ENCRYPT);
			SoftwareDES(dataOut, dataOut, pExternKey, MODE_DECRYPT);
			break;
	}
}

/**********************************************************************************
 * Function name: Hardware3DES3Key
 * Description: This function does triple DES 3 key kernel operation with hardware implement.
 * Input parameter:
 *      u8 dataOut[8]               cipher data.
 *      u8 dataIn[8]                plain data.
 *      u8 keyPointer[24]	    DES key.
 *      u8 algType                  DES mode, encrypt or decrypt.
 * Output parameter:
 *		none.
 * NOTE: dataIn could be same as dataOut.
 *********************************************************************************/

void SoftDes_Hardware3DES3Key(u8 dataOut[8], u8 dataIn[8], u8 pExternKey[24], u8 algType)
{
    switch(algType)
	{
		case MODE_ENCRYPT:
			SoftwareDES(dataOut, dataIn, pExternKey, MODE_ENCRYPT);
			SoftwareDES(dataOut, dataOut, &pExternKey[8], MODE_DECRYPT);
			SoftwareDES(dataOut, dataOut, &pExternKey[16], MODE_ENCRYPT);
			break;
		case MODE_DECRYPT:
			SoftwareDES(dataOut, dataIn, &pExternKey[16], MODE_DECRYPT);
			SoftwareDES(dataOut, dataOut, &pExternKey[8], MODE_ENCRYPT);
			SoftwareDES(dataOut, dataOut, pExternKey, MODE_DECRYPT);
			break;
	}
}

void SoftDes_DoDes_Crypt(PX8 dataIn, PX8 dataOut, u8 en_deCrypt, PX32 pExternKey, u8 Key_type_len)
{
    switch(Key_type_len)
    {
		case BYTE_LENGTH_DES:     //DESXPCR.DESOP0 = 0,DESXPCR2.DESOP1 = 0,DESXPCR2.DESOP2 = 0
			HardwareDES( dataOut, dataIn, DES_keyRegister, en_deCrypt);
			break;
		case BYTE_LENGTH_3DES_2KEY:  //DESXPCR.DESOP0 = 1,DESXPCR2.DESOP1 = 0,DESXPCR2.DESOP2 = 0
			Hardware3DES2Key( dataOut, dataIn, DES_keyRegister, en_deCrypt);
			break;
		case BYTE_LENGTH_3DES_3KEY: //DESXPCR.DESOP0 = 1,DESXPCR2.DESOP1 = 0,DESXPCR2.DESOP2 = 1
			Hardware3DES3Key( dataOut, dataIn, DES_keyRegister, en_deCrypt);
			break;
		default:
			break;

    }
}

#endif    // WITH_SOFT_DES
