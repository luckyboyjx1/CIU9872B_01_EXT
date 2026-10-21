#include "external_calls.h"
#include <curve25519_dh.h>
#include "base_folding8.h"

#define minusR_0    0xCF5D3ED0
#define minusR_1    0x812631A5
#define minusR_2    0x2F79CD65
#define minusR_3    0x4DEF9DEA
#define minusR_4    1
#define minusR_5    0
#define minusR_6    0
#define minusR_7    0

const U32 _w_P[8] = {
    0xFFFFFFED,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
    0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF
};

/* Maximum number of prime p that fits into 256-bits */
const U32 _w_maxP[8] = {   /* 2*P < 2**256 */
    0xFFFFFFDA,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
    0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
};

const U_WORD _w_NxBPO[16][K_WORDS] = {  /* n*BPO */
    W256(0,0,0,0,0,0,0,0),
    W256(0x5CF5D3ED,0x5812631A,0xA2F79CD6,0x14DEF9DE,0,0,0,0x10000000),
    W256(0xB9EBA7DA,0xB024C634,0x45EF39AC,0x29BDF3BD,0,0,0,0x20000000),
    W256(0x16E17BC7,0x0837294F,0xE8E6D683,0x3E9CED9B,0,0,0,0x30000000),
    W256(0x73D74FB4,0x60498C69,0x8BDE7359,0x537BE77A,0,0,0,0x40000000),
    W256(0xD0CD23A1,0xB85BEF83,0x2ED6102F,0x685AE159,0,0,0,0x50000000),
    W256(0x2DC2F78E,0x106E529E,0xD1CDAD06,0x7D39DB37,0,0,0,0x60000000),
    W256(0x8AB8CB7B,0x6880B5B8,0x74C549DC,0x9218D516,0,0,0,0x70000000),
    W256(0xE7AE9F68,0xC09318D2,0x17BCE6B2,0xA6F7CEF5,0,0,0,0x80000000),
    W256(0x44A47355,0x18A57BED,0xBAB48389,0xBBD6C8D3,0,0,0,0x90000000),
    W256(0xA19A4742,0x70B7DF07,0x5DAC205F,0xD0B5C2B2,0,0,0,0xA0000000),
    W256(0xFE901B2F,0xC8CA4221,0x00A3BD35,0xE594BC91,0,0,0,0xB0000000),
    W256(0x5B85EF1C,0x20DCA53C,0xA39B5A0C,0xFA73B66F,0,0,0,0xC0000000),
    W256(0xB87BC309,0x78EF0856,0x4692F6E2,0x0F52B04E,1,0,0,0xD0000000),
    W256(0x157196F6,0xD1016B71,0xE98A93B8,0x2431AA2C,1,0,0,0xE0000000),
    W256(0x72676AE3,0x2913CE8B,0x8C82308F,0x3910A40B,1,0,0,0xF0000000)
};

const EDP_BLINDING_CTX edp_custom_blinding = 
{
  W256(0xDB763B3D,0x2F33DD36,0xF8DC6F74,0x98C50273,0x8216E0D5,0x1AA522E6,0x6BFD2924,0x04BEFBE2),
  W256(0x73FF0C54,0x459E8BA8,0x84F9550B,0xDBF5F46A,0x6DB0E537,0x65F44FFD,0x2AED4E78,0x2CF92B82),
  {
    W256(0xAA6B12E6,0x7E6A2126,0x2E016899,0x80453AE0,0x6F300787,0x7A7E739E,0x264D5BA3,0x4C34AA79),
    W256(0x52EA83C8,0xE4156040,0x0792FA4C,0x97CA4A43,0x3C9611E1,0x2198FE81,0x41AEAAE6,0x11919AD1),
    W256(0x54A55516,0xCD8ADA8A,0x820A3D65,0xF85C2DB2,0x06667F3B,0xFF1E45F7,0x26BD6148,0x7F4E9D95),
    W256(0xA4986374,0x2C1BF6DE,0x57ABC779,0x7C913D04,0xFA1127FA,0x1D7E8692,0xDE0E0680,0xC636320A)
  }
};

/* x coordinate of base point */
const U8 ecp_BasePoint[32] = { 
    9,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 };
	
	
void ecp_TrimSecretKey(U8 *X)
{
    X[0] &= 0xf8;
    X[31] = (X[31] | 0x40) & 0x7f;
}

/* Convert little-endian byte array to little-endian word array */
U32* ecp_BytesToWords(OUT U32 *Y, IN const U8 *X)
{
    int i;
    M32 m;
    
    for (i = 0; i < 8; i++)
    {
        m.u8.b0 = *X++;
        m.u8.b1 = *X++;
        m.u8.b2 = *X++;
        m.u8.b3 = *X++;
        
        Y[i] = m.u32;
    }
    return Y;
}

/* Convert little-endian word array to little-endian byte array */
U8* ecp_WordsToBytes(OUT U8 *Y, IN const U32 *X)
{
    int i;
    M32 m;
    
    for (i = 0; i < 32;)
    {
        m.u32 = *X++;
        Y[i++] = m.u8.b0;
        Y[i++] = m.u8.b1;
        Y[i++] = m.u8.b2;
        Y[i++] = m.u8.b3;
    }
    return Y;
}

void ecp_8Folds(U8* Y, const U32* X)
{
    int i, j;
    U8 a = 0;
    for (i = 32; i-- > 0;)
    {
        for (j = 8; j-- > 0;) a = (a << 1) + ((X[j] >> i) & 1);
        *Y++ = a;
    }
}

#define ECP_ADD_C0(Y,X,V) c.u64 = (U64)(X) + (V); Y = c.u32.lo;
#define ECP_ADD_C1(Y,X) c.u64 = (U64)(X) + c.u32.hi; Y = c.u32.lo;

#define ECP_SUB_C0(Y,X,V) c.s64 = (U64)(X) - (V); Y = c.u32.lo;
#define ECP_SUB_C1(Y,X) c.s64 = (U64)(X) + (S64)c.s32.hi; Y = c.u32.lo;

#define ECP_MULSET_W0(Y,b,X) c.u64 = (U64)(b)*(X); Y = c.u32.lo;
#define ECP_MULSET_W1(Y,b,X) c.u64 = (U64)(b)*(X) + c.u32.hi; Y = c.u32.lo;

#define ECP_MULADD_W0(Z,Y,b,X) c.u64 = (U64)(b)*(X) + (Y); Z = c.u32.lo;
#define ECP_MULADD_W1(Z,Y,b,X) c.u64 = (U64)(b)*(X) + (U64)(Y) + c.u32.hi; Z = c.u32.lo;

#define ECP_ADD32(Z,X,Y) c.u64 = (U64)(X) + (Y); Z = c.u32.lo;
#define ECP_ADC32(Z,X,Y) c.u64 = (U64)(X) + (U64)(Y) + c.u32.hi; Z = c.u32.lo;
#define ECP_SUB32(Z,X,Y) b.s64 = (S64)(X) - (Y); Z = b.s32.lo;
#define ECP_SBC32(Z,X,Y) b.s64 = (S64)(X) - (U64)(Y) + b.s32.hi; Z = b.s32.lo;

/* Computes Z = X+Y */
U32 ecp_Add(U32* Z, const U32* X, const U32* Y) 
{
    M64 c;

    ECP_ADD32(Z[0], X[0], Y[0]);
    ECP_ADC32(Z[1], X[1], Y[1]);
    ECP_ADC32(Z[2], X[2], Y[2]);
    ECP_ADC32(Z[3], X[3], Y[3]);
    ECP_ADC32(Z[4], X[4], Y[4]);
    ECP_ADC32(Z[5], X[5], Y[5]);
    ECP_ADC32(Z[6], X[6], Y[6]);
    ECP_ADC32(Z[7], X[7], Y[7]);
    return c.u32.hi;
}

/* Computes Z = X-Y */
S32 ecp_Sub(U32* Z, const U32* X, const U32* Y) 
{
    M64 b;
    ECP_SUB32(Z[0], X[0], Y[0]);
    ECP_SBC32(Z[1], X[1], Y[1]);
    ECP_SBC32(Z[2], X[2], Y[2]);
    ECP_SBC32(Z[3], X[3], Y[3]);
    ECP_SBC32(Z[4], X[4], Y[4]);
    ECP_SBC32(Z[5], X[5], Y[5]);
    ECP_SBC32(Z[6], X[6], Y[6]);
    ECP_SBC32(Z[7], X[7], Y[7]);
    return b.s32.hi;
}

/* Computes Z = X+Y mod P */
void ecp_AddReduce(U32* Z, const U32* X, const U32* Y) 
{
    M64 c;
    c.u32.hi = ecp_Add(Z, X, Y) * 38;

    /* Z += c.u32.hi * 38 */
    ECP_ADD_C0(Z[0], Z[0], c.u32.hi);
    ECP_ADD_C1(Z[1], Z[1]);
    ECP_ADD_C1(Z[2], Z[2]);
    ECP_ADD_C1(Z[3], Z[3]);
    ECP_ADD_C1(Z[4], Z[4]);
    ECP_ADD_C1(Z[5], Z[5]);
    ECP_ADD_C1(Z[6], Z[6]);
    ECP_ADD_C1(Z[7], Z[7]);

    /* One more carry at most */
    ECP_ADD_C0(Z[0], Z[0], c.u32.hi*38);
    ECP_ADD_C1(Z[1], Z[1]);
    ECP_ADD_C1(Z[2], Z[2]);
    ECP_ADD_C1(Z[3], Z[3]);
    ECP_ADD_C1(Z[4], Z[4]);
    ECP_ADD_C1(Z[5], Z[5]);
    ECP_ADD_C1(Z[6], Z[6]);
    ECP_ADD_C1(Z[7], Z[7]);
}

/* Computes Z = X-Y mod P */
void ecp_SubReduce(U32* Z, const U32* X, const U32* Y) 
{
    M64 c;
    c.u32.hi = ecp_Sub(Z, X, Y) & 38;

    ECP_SUB_C0(Z[0], Z[0], c.u32.hi);
    ECP_SUB_C1(Z[1], Z[1]);
    ECP_SUB_C1(Z[2], Z[2]);
    ECP_SUB_C1(Z[3], Z[3]);
    ECP_SUB_C1(Z[4], Z[4]);
    ECP_SUB_C1(Z[5], Z[5]);
    ECP_SUB_C1(Z[6], Z[6]);
    ECP_SUB_C1(Z[7], Z[7]);

    ECP_SUB_C0(Z[0], Z[0], c.u32.hi & 38);
    ECP_SUB_C1(Z[1], Z[1]);
    ECP_SUB_C1(Z[2], Z[2]);
    ECP_SUB_C1(Z[3], Z[3]);
    ECP_SUB_C1(Z[4], Z[4]);
    ECP_SUB_C1(Z[5], Z[5]);
    ECP_SUB_C1(Z[6], Z[6]);
    ECP_SUB_C1(Z[7], Z[7]);
}

void ecp_Mod(U32 *X)
{
    U32 T[8];
    U32 c = (U32)ecp_Sub(X, X, _w_P);

    /* set T = 0 if c=0, else T = P */

    T[0] = c & 0xFFFFFFED;
    T[1] = T[2] = T[3] = T[4] = T[5] = T[6] = c;
    T[7] = c >> 1;

    ecp_Add(X, X, T);   /* X += 0 or P */

    /* In case there is another P there */

    c = (U32)ecp_Sub(X, X, _w_P);

    /* set T = 0 if c=0, else T = P */

    T[0] = c & 0xFFFFFFED;
    T[1] = T[2] = T[3] = T[4] = T[5] = T[6] = c;
    T[7] = c >> 1;

    ecp_Add(X, X, T);   /* X += 0 or P */
}

/* Computes Y = b*X */
static void ecp_mul_set(U32* Y, U32 b, const U32* X) 
{
    M64 c;
    ECP_MULSET_W0(Y[0], b, X[0]);
    ECP_MULSET_W1(Y[1], b, X[1]);
    ECP_MULSET_W1(Y[2], b, X[2]);
    ECP_MULSET_W1(Y[3], b, X[3]);
    ECP_MULSET_W1(Y[4], b, X[4]);
    ECP_MULSET_W1(Y[5], b, X[5]);
    ECP_MULSET_W1(Y[6], b, X[6]);
    ECP_MULSET_W1(Y[7], b, X[7]);
    Y[8] = c.u32.hi;
}

/* Computes Y += b*X */
/* Addition is performed on lower 8-words of Y */
static void ecp_mul_add(U32* Y, U32 b, const U32* X) 
{
    M64 c;
    ECP_MULADD_W0(Y[0], Y[0], b, X[0]);
    ECP_MULADD_W1(Y[1], Y[1], b, X[1]);
    ECP_MULADD_W1(Y[2], Y[2], b, X[2]);
    ECP_MULADD_W1(Y[3], Y[3], b, X[3]);
    ECP_MULADD_W1(Y[4], Y[4], b, X[4]);
    ECP_MULADD_W1(Y[5], Y[5], b, X[5]);
    ECP_MULADD_W1(Y[6], Y[6], b, X[6]);
    ECP_MULADD_W1(Y[7], Y[7], b, X[7]);
    Y[8] = c.u32.hi;
}

/* Computes Z = Y + b*X and return carry */
void ecp_WordMulAddReduce(U32 *Z, const U32* Y, U32 b, const U32* X) 
{
    M64 c;
    ECP_MULADD_W0(Z[0], Y[0], b, X[0]);
    ECP_MULADD_W1(Z[1], Y[1], b, X[1]);
    ECP_MULADD_W1(Z[2], Y[2], b, X[2]);
    ECP_MULADD_W1(Z[3], Y[3], b, X[3]);
    ECP_MULADD_W1(Z[4], Y[4], b, X[4]);
    ECP_MULADD_W1(Z[5], Y[5], b, X[5]);
    ECP_MULADD_W1(Z[6], Y[6], b, X[6]);
    ECP_MULADD_W1(Z[7], Y[7], b, X[7]);

    /* Z += c.u32.hi * 38 */
    ECP_MULADD_W0(Z[0], Z[0], c.u32.hi, 38);
    ECP_ADD_C1(Z[1], Z[1]);
    ECP_ADD_C1(Z[2], Z[2]);
    ECP_ADD_C1(Z[3], Z[3]);
    ECP_ADD_C1(Z[4], Z[4]);
    ECP_ADD_C1(Z[5], Z[5]);
    ECP_ADD_C1(Z[6], Z[6]);
    ECP_ADD_C1(Z[7], Z[7]);

    /* One more time at most */
    ECP_MULADD_W0(Z[0], Z[0], c.u32.hi, 38);
    ECP_ADD_C1(Z[1], Z[1]);
    ECP_ADD_C1(Z[2], Z[2]);
    ECP_ADD_C1(Z[3], Z[3]);
    ECP_ADD_C1(Z[4], Z[4]);
    ECP_ADD_C1(Z[5], Z[5]);
    ECP_ADD_C1(Z[6], Z[6]);
    ECP_ADD_C1(Z[7], Z[7]);
}

/* Computes Z = X*Y mod P. */
/* Output fits into 8 words but could be greater than P */
void ecp_MulReduce(U32* Z, const U32* X, const U32* Y) 
{
    U32 T[16];

    ecp_mul_set(T+0, X[0], Y);
    ecp_mul_add(T+1, X[1], Y);
    ecp_mul_add(T+2, X[2], Y);
    ecp_mul_add(T+3, X[3], Y);
    ecp_mul_add(T+4, X[4], Y);
    ecp_mul_add(T+5, X[5], Y);
    ecp_mul_add(T+6, X[6], Y);
    ecp_mul_add(T+7, X[7], Y);

    /* We have T = X*Y, now do the reduction in size */

    ecp_WordMulAddReduce(Z, T, 38, T+8);
}

/* Computes Z = X*Y */
void ecp_Mul(U32* Z, const U32* X, const U32* Y) 
{
    ecp_mul_set(Z+0, X[0], Y);
    ecp_mul_add(Z+1, X[1], Y);
    ecp_mul_add(Z+2, X[2], Y);
    ecp_mul_add(Z+3, X[3], Y);
    ecp_mul_add(Z+4, X[4], Y);
    ecp_mul_add(Z+5, X[5], Y);
    ecp_mul_add(Z+6, X[6], Y);
    ecp_mul_add(Z+7, X[7], Y);
}

/* Computes Z = X*Y mod P. */
void ecp_SqrReduce(U32* Y, const U32* X) 
{
    /* TBD: Implementation is based on multiply */
    /*      Optimize for squaring */

    U32 T[16];

    ecp_mul_set(T+0, X[0], X);
    ecp_mul_add(T+1, X[1], X);
    ecp_mul_add(T+2, X[2], X);
    ecp_mul_add(T+3, X[3], X);
    ecp_mul_add(T+4, X[4], X);
    ecp_mul_add(T+5, X[5], X);
    ecp_mul_add(T+6, X[6], X);
    ecp_mul_add(T+7, X[7], X);

    /* We have T = X*X, now do the reduction in size */

    ecp_WordMulAddReduce(Y, T, 38, T+8);
}

/* Computes Z = X*Y mod P. */
void ecp_MulMod(U32* Z, const U32* X, const U32* Y) 
{
    ecp_MulReduce(Z, X, Y);
    ecp_Mod(Z);
}

/* Courtesy of DJB */
/* Return out = 1/z mod P */
void ecp_Inverse(U32 *out, const U32 *z) 
{
  int i;
  U32 t0[8],t1[8],z2[8],z9[8],z11[8];
  U32 z2_5_0[8],z2_10_0[8],z2_20_0[8],z2_50_0[8],z2_100_0[8];

  /* 2 */               ecp_SqrReduce(z2,z);
  /* 4 */               ecp_SqrReduce(t1,z2);
  /* 8 */               ecp_SqrReduce(t0,t1);
  /* 9 */               ecp_MulReduce(z9,t0,z);
  /* 11 */              ecp_MulReduce(z11,z9,z2);
  /* 22 */              ecp_SqrReduce(t0,z11);
  /* 2^5 - 2^0 = 31 */  ecp_MulReduce(z2_5_0,t0,z9);

  /* 2^6 - 2^1 */       ecp_SqrReduce(t0,z2_5_0);
  /* 2^7 - 2^2 */       ecp_SqrReduce(t1,t0);
  /* 2^8 - 2^3 */       ecp_SqrReduce(t0,t1);
  /* 2^9 - 2^4 */       ecp_SqrReduce(t1,t0);
  /* 2^10 - 2^5 */      ecp_SqrReduce(t0,t1);
  /* 2^10 - 2^0 */      ecp_MulReduce(z2_10_0,t0,z2_5_0);

  /* 2^11 - 2^1 */      ecp_SqrReduce(t0,z2_10_0);
  /* 2^12 - 2^2 */      ecp_SqrReduce(t1,t0);
  /* 2^20 - 2^10 */     for (i = 2;i < 10;i += 2) { 
                            ecp_SqrReduce(t0,t1); 
                            ecp_SqrReduce(t1,t0); }
  /* 2^20 - 2^0 */      ecp_MulReduce(z2_20_0,t1,z2_10_0);

  /* 2^21 - 2^1 */      ecp_SqrReduce(t0,z2_20_0);
  /* 2^22 - 2^2 */      ecp_SqrReduce(t1,t0);
  /* 2^40 - 2^20 */     for (i = 2;i < 20;i += 2) { 
                            ecp_SqrReduce(t0,t1); 
                            ecp_SqrReduce(t1,t0); }
  /* 2^40 - 2^0 */      ecp_MulReduce(t0,t1,z2_20_0);

  /* 2^41 - 2^1 */      ecp_SqrReduce(t1,t0);
  /* 2^42 - 2^2 */      ecp_SqrReduce(t0,t1);
  /* 2^50 - 2^10 */     for (i = 2;i < 10;i += 2) { 
                            ecp_SqrReduce(t1,t0); 
                            ecp_SqrReduce(t0,t1); }
  /* 2^50 - 2^0 */      ecp_MulReduce(z2_50_0,t0,z2_10_0);

  /* 2^51 - 2^1 */      ecp_SqrReduce(t0,z2_50_0);
  /* 2^52 - 2^2 */      ecp_SqrReduce(t1,t0);
  /* 2^100 - 2^50 */    for (i = 2;i < 50;i += 2) { 
                            ecp_SqrReduce(t0,t1); 
                            ecp_SqrReduce(t1,t0); }
  /* 2^100 - 2^0 */     ecp_MulReduce(z2_100_0,t1,z2_50_0);

  /* 2^101 - 2^1 */     ecp_SqrReduce(t1,z2_100_0);
  /* 2^102 - 2^2 */     ecp_SqrReduce(t0,t1);
  /* 2^200 - 2^100 */   for (i = 2;i < 100;i += 2) { 
                            ecp_SqrReduce(t1,t0); 
                            ecp_SqrReduce(t0,t1); }
  /* 2^200 - 2^0 */     ecp_MulReduce(t1,t0,z2_100_0);

  /* 2^201 - 2^1 */     ecp_SqrReduce(t0,t1);
  /* 2^202 - 2^2 */     ecp_SqrReduce(t1,t0);
  /* 2^250 - 2^50 */    for (i = 2;i < 50;i += 2) { 
                            ecp_SqrReduce(t0,t1); 
                            ecp_SqrReduce(t1,t0); }
  /* 2^250 - 2^0 */     ecp_MulReduce(t0,t1,z2_50_0);

  /* 2^251 - 2^1 */     ecp_SqrReduce(t1,t0);
  /* 2^252 - 2^2 */     ecp_SqrReduce(t0,t1);
  /* 2^253 - 2^3 */     ecp_SqrReduce(t1,t0);
  /* 2^254 - 2^4 */     ecp_SqrReduce(t0,t1);
  /* 2^255 - 2^5 */     ecp_SqrReduce(t1,t0);
  /* 2^255 - 21 */      ecp_MulReduce(out,t1,z11);
}

/* Y = X + X */
void ecp_MontDouble(XZ_POINT *Y, const XZ_POINT *X)
{
    U_WORD A[K_WORDS], B[K_WORDS];
    /*  x2 = (x+z)^2 * (x-z)^2 */
    /*  z2 = ((x+z)^2 - (x-z)^2)*((x+z)^2 + ((A-2)/4)((x+z)^2 - (x-z)^2)) */
    ecp_AddReduce(A, X->X, X->Z);       /* A = (x+z) */
    ecp_SubReduce(B, X->X, X->Z);       /* B = (x-z) */
    ecp_SqrReduce(A, A);                /* A = (x+z)^2 */
    ecp_SqrReduce(B, B);                /* B = (x-z)^2 */
    ecp_MulReduce(Y->X, A, B);          /* x2 = (x+z)^2 * (x-z)^2 */
    ecp_SubReduce(B, A, B);             /* B = (x+z)^2 - (x-z)^2 */
    /* (486662-2)/4 = 121665 */
    ecp_WordMulAddReduce(A, A, 121665, B);
    ecp_MulReduce(Y->Z, A, B);          /* z2 = (B)*((x+z)^2 + ((A-2)/4)(B)) */
}

/* return P = P + Q, Q = 2Q */
void ecp_Mont(XZ_POINT *P, XZ_POINT *Q, IN const U_WORD *Base)
{
    U_WORD A[K_WORDS], B[K_WORDS], C[K_WORDS], D[K_WORDS], E[K_WORDS];
    /* x3 = ((x1-z1)(x2+z2) + (x1+z1)(x2-z2))^2*zb     zb=1 */
    /* z3 = ((x1-z1)(x2+z2) - (x1+z1)(x2-z2))^2*xb     xb=Base */
    ecp_SubReduce(A, P->X, P->Z);   /* A = x1-z1 */
    ecp_AddReduce(B, P->X, P->Z);   /* B = x1+z1 */
    ecp_SubReduce(C, Q->X, Q->Z);   /* C = x2-z2 */
    ecp_AddReduce(D, Q->X, Q->Z);   /* D = x2+z2 */
    ecp_MulReduce(A, A, D);         /* A = (x1-z1)(x2+z2) */
    ecp_MulReduce(B, B, C);         /* B = (x1+z1)(x2-z2) */
    ecp_AddReduce(E, A, B);         /* E = (x1-z1)(x2+z2) + (x1+z1)(x2-z2) */
    ecp_SubReduce(B, A, B);         /* B = (x1-z1)(x2+z2) - (x1+z1)(x2-z2) */
    ecp_SqrReduce(P->X, E);         /* x3 = ((x1-z1)(x2+z2) + (x1+z1)(x2-z2))^2 */
    ecp_SqrReduce(A, B);            /* A = ((x1-z1)(x2+z2) - (x1+z1)(x2-z2))^2 */
    ecp_MulReduce(P->Z, A, Base);   /* z3 = ((x1-z1)(x2+z2) - (x1+z1)(x2-z2))^2*Base */

    /* x4 = (x2+z2)^2 * (x2-z2)^2 */
    /* z4 = ((x2+z2)^2 - (x2-z2)^2)*((x2+z2)^2 + 121665((x2+z2)^2 - (x2-z2)^2)) */
    /* C = (x2-z2) */
    /* D = (x2+z2) */
    ecp_SqrReduce(A, D);            /* A = (x2+z2)^2 */
    ecp_SqrReduce(B, C);            /* B = (x2-z2)^2 */
    ecp_MulReduce(Q->X, A, B);      /* x4 = (x2+z2)^2 * (x2-z2)^2 */
    ecp_SubReduce(B, A, B);         /* B = (x2+z2)^2 - (x2-z2)^2 */
    ecp_WordMulAddReduce(A, A, 121665, B);
    ecp_MulReduce(Q->Z, A, B);      /* z4 = B*((x2+z2)^2 + 121665*B) */
}

/* Constant-time measure: */
/* Use different set of parameters for bit=0 or bit=1 with no conditional jump */
/* */
#define ECP_MONT(n) j = (k >> n) & 1; ecp_Mont(PP[j], QP[j], X)

/* -------------------------------------------------------------------------- */
/* Return point Q = k*P */
/* K in a little-endian byte array */
void ecp_PointMultiply(
    OUT U8 *PublicKey, 
    IN const U8 *BasePoint, 
    IN const U8 *SecretKey, 
    IN int len)
{
    int i, j, k;
    U_WORD X[K_WORDS];
    XZ_POINT P, Q, *PP[2], *QP[2];

    ecp_BytesToWords(X, BasePoint);

    while (len-- > 0)
    {
        k = SecretKey[len];
        for (i = 0; i < 8; i++, k <<= 1)
        {
            /* P = kG, Q = (k+1)G */
            if (k & 0x80)
            {
                ecp_Add(P.Z, X, edp_custom_blinding.zr);    /* P.Z = random */
                ecp_MulReduce(P.X, X, P.Z);
                ecp_MontDouble(&Q, &P);

                PP[1] = &P; PP[0] = &Q;
                QP[1] = &Q; QP[0] = &P;

                while (++i < 8) { k <<= 1; ECP_MONT(7); }
                while (len > 0)
                {
                    k = SecretKey[--len];
                    ECP_MONT(7);
                    ECP_MONT(6);
                    ECP_MONT(5);
                    ECP_MONT(4);
                    ECP_MONT(3);
                    ECP_MONT(2);
                    ECP_MONT(1);
                    ECP_MONT(0);
                }

                ecp_Inverse(Q.Z, P.Z);
                ecp_MulMod(X, P.X, Q.Z);
                ecp_WordsToBytes(PublicKey, X);
                return;
            }
        }
    }
    /* K is 0 */
    mem_fill(PublicKey, 0, 32);
}

void ecp_PointMultiply_half(
    OUT U8 *PublicKey, 
    IN const U8 *BasePoint, 
    IN const U8 *SecretKey, 
    IN int len)
{
  int j, k;
  U_WORD X[K_WORDS];
  XZ_POINT P, Q, *PP[2], *QP[2];
	
	U32 TemPX[8] = {0x1909D9E4,0x1F4E892D,0x90AA601B,0x7A0CEF2F,0xB8297FFE,0xE776ECAF,0x5FA2D9BF,0x4BF5AD3F};
	U32 TemPZ[8] = {0xF70E041C,0xF46E4E5A,0x3FF5A1B3,0x34F1C698,0x3A13226F,0xAC8C1FE8,0x35E9582E,0x07B1B86C};	
	U32 TemQX[8] = {0x18AD6C82,0xD21D6773,0x80180884,0x02C193DB,0x2CEB912F,0x33EF6F0B,0x9BB1C5A4,0x96A41299};
	U32 TemQZ[8] = {0x0FAA64AE,0x9DBEFDEB,0x4FFB0889,0x634F3799,0x380B7153,0x3B6BC5B8,0x16498FBD,0x3FBBA35B};	

  ecp_BytesToWords(X, BasePoint);
	memcpy(P.X, TemPX, 32);
	memcpy(P.Z, TemPZ, 32);
	memcpy(Q.X, TemQX, 32);
	memcpy(Q.Z, TemQZ, 32);

	PP[1] = &P; PP[0] = &Q;
	QP[1] = &Q; QP[0] = &P;	

    while (len > 0)
	{
      k = SecretKey[--len];
      ECP_MONT(7);
      ECP_MONT(6);
      ECP_MONT(5);
      ECP_MONT(4);
      ECP_MONT(3);
      ECP_MONT(2);
      ECP_MONT(1);
      ECP_MONT(0);
      }

    ecp_Inverse(Q.Z, P.Z);
    ecp_MulMod(X, P.X, Q.Z);
    ecp_WordsToBytes(PublicKey, X);
    return;
}

void edp_AddAffinePoint(Ext_POINT *p, const PA_POINT *q)
{
    U_WORD a[K_WORDS], b[K_WORDS], c[K_WORDS], d[K_WORDS], e[K_WORDS];
    ecp_SubReduce(a, p->y, p->x);           /* A = (Y1-X1)*(Y2-X2) */
    ecp_MulReduce(a, a, q->YmX);
    ecp_AddReduce(b, p->y, p->x);           /* B = (Y1+X1)*(Y2+X2) */
    ecp_MulReduce(b, b, q->YpX);
    ecp_MulReduce(c, p->t, q->T2d);         /* C = T1*2d*T2 */
    ecp_AddReduce(d, p->z, p->z);           /* D = Z1*2*Z2 (Z2=1)*/
    ecp_SubReduce(e, b, a);                 /* E = B-A */
    ecp_AddReduce(b, b, a);                 /* H = B+A */
    ecp_SubReduce(a, d, c);                 /* F = D-C */
    ecp_AddReduce(d, d, c);                 /* G = D+C */

    ecp_MulReduce(p->x, e, a);              /* E*F */
    ecp_MulReduce(p->y, b, d);              /* H*G */
    ecp_MulReduce(p->t, e, b);              /* E*H */
    ecp_MulReduce(p->z, d, a);              /* G*F */
}

void edp_DoublePoint(Ext_POINT *p)
{
    U_WORD a[K_WORDS], b[K_WORDS], c[K_WORDS], d[K_WORDS], e[K_WORDS];

    ecp_SqrReduce(a, p->x);         /* A = X1^2 */
    ecp_SqrReduce(b, p->y);         /* B = Y1^2 */
    ecp_SqrReduce(c, p->z);         /* C = 2*Z1^2 */
    ecp_AddReduce(c, c, c);
    ecp_SubReduce(d, _w_maxP, a);   /* D = -A */

    ecp_SubReduce(a, d, b);         /* H = D-B */
    ecp_AddReduce(d, d, b);         /* G = D+B */
    ecp_SubReduce(b, d, c);         /* F = G-C */
    ecp_AddReduce(e, p->x, p->y);   /* E = (X1+Y1)^2-A-B = (X1+Y1)^2+H */
    ecp_SqrReduce(e, e);
    ecp_AddReduce(e, e, a);

    ecp_MulReduce(p->x, e, b);      /* E*F */
    ecp_MulReduce(p->y, a, d);      /* H*G */
    ecp_MulReduce(p->z, d, b);      /* G*F */
    ecp_MulReduce(p->t, e, a);      /* E*H */
}

void edp_BasePointMult(
    OUT Ext_POINT *S, 
    IN const U_WORD *sk, 
    IN const U_WORD *R)
{
  int i = 16;
  U8 cut[32];
	
	Ext_POINT PS;

	const U32 X[8] = {0x54A65496,0x61C9ACC3,0x8800B9FF,0x81C9F230,0xD367876D,0x3FD30CAD,0x195C5E01,0x14B78011};
	const U32 Y[8] = {0xCB0973B6,0x7E16ACB6,0x963AFCD4,0xD10E7FF2,0xF3D98082,0x2304D5EA,0x51FF37F3,0x1B9CD2D2};
	const U32 Z[8] = {0x691672E2,0x166EA10D,0xF143F5D0,0x1A5985E2,0x892CBC15,0x25352B5A,0x0CE7B254,0xE68FA01C};
	const U32 T[8] = {0xD3EE6AD2,0xD9F34E0C,0x52A537F0,0x2A69DAD7,0xA52F851E,0xCB21D951,0x00066B00,0xCC1DDD21};
	memcpy((U8 *)PS.t, (U8 *)T ,32);
	memcpy((U8 *)PS.x, (U8 *)X ,32);
	memcpy((U8 *)PS.y, (U8 *)Y ,32);
	memcpy((U8 *)PS.z, (U8 *)Z ,32);

	ecp_8Folds(cut, sk);

  do 
  {
    edp_DoublePoint(&PS);
    edp_AddAffinePoint(&PS, &_w_base_folding8[cut[i]]);
  } while (i++ < 31);
	
	memcpy(S, &PS ,32*4);
}

/* -- DH key exchange interfaces ----------------------------------------- */

/* Return R = a*P where P is curve25519 base point */
void x25519_BasePointMultiply(OUT U8 *r, IN const U8 *sk)
{
    Ext_POINT S;
    U_WORD t[K_WORDS];

    ecp_BytesToWords(t, sk);
    edp_BasePointMult(&S, t, edp_custom_blinding.zr);

    /* Convert ed25519 point to x25519 point */
    
    /* u = (1 + y)/(1 - y) = (Z + Y)/(Z - Y) */

    ecp_AddReduce(S.t, S.z, S.y);
    ecp_SubReduce(S.z, S.z, S.y);
    ecp_Inverse(S.z, S.z);
    ecp_MulMod(S.t, S.t, S.z);
    ecp_WordsToBytes(r, S.t);
}

/* Return public key associated with sk */
void curve25519_dh_CalculatePublicKey_fast(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk)          /* [32-bytes] IN/OUT: Your secret key */
{
    ecp_TrimSecretKey(sk);
    /* Use faster method */
    x25519_BasePointMultiply(pk, sk);
}

/* Return public key associated with sk */
void curve25519_dh_CalculatePublicKey(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk)          /* [32-bytes] IN/OUT: Your secret key */
{
    ecp_TrimSecretKey(sk);
    ecp_PointMultiply(pk, ecp_BasePoint, sk, 32);
}

void curve25519_dh_CalculatePublicKey_half(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk)          /* [32-bytes] IN/OUT: Your secret key */
{
    ecp_TrimSecretKey(sk);
    ecp_PointMultiply_half(pk, ecp_BasePoint, sk, 16);
}

/* Create a shared secret */
void curve25519_dh_CreateSharedKey(
    unsigned char *shared,      /* [32-bytes] OUT: Created shared key */
    const unsigned char *pk,    /* [32-bytes] IN: Other side's public key */
    unsigned char *sk)          /* [32-bytes] IN/OUT: Your secret key */
{
    ecp_TrimSecretKey(sk);
    ecp_PointMultiply(shared, pk, sk, 32);
}