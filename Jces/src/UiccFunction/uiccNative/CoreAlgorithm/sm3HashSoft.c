/*
 * SM3 Hash alogrith 
 */

//Testing data from SM3 Standards
//http://www.oscca.gov.cn/News/201012/News_1199.htm 
// Sample 1
// Input:"abc"  
// Output:66c7f0f4 62eeedd9 d1f2d46b dc10e4e2 4167c487 5cf2f7a2 297da02b 8f4ba8e0

// Sample 2 
// Input:"abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"
// Outpuf:debe9ff9 2275b8a1 38604889 c18e5a4d 6fdb70e5 387e5765 293dcba3 9c0c5732

#include "sm3HashSoft.h"
#include <string.h>
#include <stdio.h>
#include "typedef.h"
#include "smPublicSoft.h"


/*
 * SM3 context setup
 */
void sm3_starts( sm3_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x7380166F;
    ctx->state[1] = 0x4914B2B9;
    ctx->state[2] = 0x172442D7;
    ctx->state[3] = 0xDA8A0600;
    ctx->state[4] = 0xA96F30BC;
    ctx->state[5] = 0x163138AA;
    ctx->state[6] = 0xE38DEE4D;
    ctx->state[7] = 0xB0FB0E4E;

}

void sm3_process( sm3_context *ctx, unsigned char buffer[64] )
{
  unsigned long xdata SS1, SS2, TT1, TT2, W[68]/*,W1[64]*/;
  unsigned long xdata A, B, C, D, E, F, G, H;
//	unsigned long xdata T[64];
	unsigned long xdata Temp1,Temp2,Temp3,Temp4,Temp5;
	unsigned char  idata j;
	
// 	for(j = 0; j < 16; j++)
// 	{
// 		T[j] = 0x79CC4519;
// 	}
// 	for(j =16; j < 64; j++)
// 	{
// 		T[j] = 0x7A879D8A;
// 	}
// 	
	for(j=0; j< 16; j++)
	{
		W[j] = GET_ULONG_BE(buffer+j*4);
	}

	for(j = 16; j < 68; j++ )
	{
		//W[j] = P1( W[j-16] ^ W[j-9] ^ ROTL(W[j-3],15)) ^ ROTL(W[j - 13],7 ) ^ W[j-6];
		//Why thd release's result is different with the debug's ?
		//Below is okay. Interesting, Perhaps VC6 has a bug of Optimizaiton.
		
		Temp1 = W[j-16] ^ W[j-9];
		Temp2 = ROTL(W[j-3],15);
		Temp3 = Temp1 ^ Temp2;
		Temp4 = P1(Temp3);
		Temp5 =  ROTL(W[j - 13],7 ) ^ W[j-6];
		W[j] = Temp4 ^ Temp5;
	}


// 	for(j =  0; j < 64; j++)
// 	{
//         W1[j] = W[j] ^ W[j+4];
// 	}

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];
	F = ctx->state[5];
	G = ctx->state[6];
	H = ctx->state[7];


	for(j =0; j < 64; j++)
	{
		
//		SS1 = ROTL((ROTL(A,12) + E + ROTL(T[j],j)), 7); 
		
// 		SS2 = SS1 ^ ROTL(A,12);
		
// 		if(j<16)
// 		{	
// 			TT1 = FF0(A,B,C) + D + SS2 + W1[j];
// 			TT2 = GG0(E,F,G) + H + SS1 + W[j];
// 		}
// 		else
// 		{
// 			TT1 = FF1(A,B,C) + D + SS2 + W1[j];
// 			TT2 = GG1(E,F,G) + H + SS1 + W[j];
// 		}
		
		if(j < 16)
		{
			SS1 = ROTL((ROTL(A,12) + E + ROTL(0x79CC4519,j)), 7); 
		}
		else
		{
			SS1 = ROTL((ROTL(A,12) + E + ROTL(0x7A879D8A,j)), 7); 
		}
		
		SS2 = SS1 ^ ROTL(A,12);
		
		if(j<16)
		{	
			TT1 = FF0(A,B,C) + D + SS2 + (W[j] ^ W[j+4]);
			TT2 = GG0(E,F,G) + H + SS1 + W[j];
		}
		else
		{
			TT1 = FF1(A,B,C) + D + SS2 + (W[j] ^ W[j+4]);
			TT2 = GG1(E,F,G) + H + SS1 + W[j];
		}

		D = C;
		C = ROTL(B,9);
		B = A;
		A = TT1;
		H = G;
		G = ROTL(F,19);
		F = E;
		E = P0(TT2);
	}
	
	ctx->state[0] ^= A;
	ctx->state[1] ^= B;
	ctx->state[2] ^= C;
	ctx->state[3] ^= D;
	ctx->state[4] ^= E;
	ctx->state[5] ^= F;
	ctx->state[6] ^= G;
	ctx->state[7] ^= H;

}

/*
 * SM3 process buffer
 */
void sm3_update( sm3_context *ctx, unsigned char *input, int ilen )
{
    int xdata fill;
    unsigned long xdata left;

    if( ilen <= 0 )
		{
        return;
		}

    left = ctx->total[0] & 0x3F;
		
    fill = 64 - left;

    ctx->total[0] += ilen;
		
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < (unsigned long) ilen )
		{
        ctx->total[1]++;
		}

    if( left && (ilen >= fill) )
    {
        memcpy( (void *) (ctx->buffer + left), (void *) input, fill );
        sm3_process( ctx, ctx->buffer );
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
        sm3_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }

    if( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),(void *) input, ilen );
    }
}

static const unsigned char sm3_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SM3 final digest
 */
void sm3_finish( sm3_context *ctx, unsigned char output[32] )
{
    unsigned long xdata last, padn;
    unsigned long xdata high, low;
    unsigned char xdata msglen[8];
		unsigned char idata i;

    high = ( ctx->total[0] >> 29 ) | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_BE( high, msglen);
    PUT_ULONG_BE( low,  msglen+4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sm3_update( ctx, (unsigned char *) sm3_padding, padn );
    sm3_update( ctx, msglen, 8 );

		for(i=0; i<8 ;i ++)
		{
			PUT_ULONG_BE(ctx->state[i],output+i*4);
		}
 
}

/*
 * output = SM3( input buffer )
 */
void sm3( unsigned char *input, int ilen, unsigned char output[32] )
{
    sm3_context xdata ctx;

    sm3_starts( &ctx );
    sm3_update( &ctx, input, ilen );
    sm3_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sm3_context ) );
}

#if 0

/*
 * SM3 HMAC context setup
 */
void sm3_hmac_starts( sm3_context *ctx, unsigned char *key, int keylen )
{
    int xdata i;
    unsigned char xdata sum[32];

    if( keylen > 64 )
    {
        sm3( key, keylen, sum );
        keylen = 32;
        key = sum;
    }

    memset( ctx->ipad, 0x36, 64 );
    memset( ctx->opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
        ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
    }

    sm3_starts( ctx);
    sm3_update( ctx, ctx->ipad, 64 );

    memset( sum, 0, sizeof( sum ) );
}

/*
 * SM3 HMAC process buffer
 */
void sm3_hmac_update( sm3_context *ctx, unsigned char *input, int ilen )
{
    sm3_update( ctx, input, ilen );
}

/*
 * SM3 HMAC final digest
 */
void sm3_hmac_finish( sm3_context *ctx, unsigned char output[32] )
{
    int xdata hlen;
    unsigned char xdata tmpbuf[32];

    //is224 = ctx->is224;
    hlen =  32;

    sm3_finish( ctx, tmpbuf );
    sm3_starts( ctx );
    sm3_update( ctx, ctx->opad, 64 );
    sm3_update( ctx, tmpbuf, hlen );
    sm3_finish( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-SM#( hmac key, input buffer )
 */
void sm3_hmac( unsigned char *key, int keylen,unsigned char *input, int ilen,unsigned char output[32] )
{
    sm3_context xdata ctx;

    sm3_hmac_starts( &ctx, key, keylen);
    sm3_hmac_update( &ctx, input, ilen );
    sm3_hmac_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sm3_context ) );
}

#endif





