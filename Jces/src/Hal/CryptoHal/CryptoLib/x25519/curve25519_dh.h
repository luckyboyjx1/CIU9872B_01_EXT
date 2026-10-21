#ifndef __curve25519_dh_key_exchange_h__
#define __curve25519_dh_key_exchange_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseTypes.h"

#define ECP_VERSION_STR     "1.2.0"

// #define	USE_ASM_LIB 		1
	
#ifdef USE_ASM_LIB
#define U_WORD          U64
#define S_WORD          S64
#define WORDSIZE_64
#define W64(lo,hi)      ((U64)hi<<32)+lo
#else
#define U_WORD          U32
#define S_WORD          S32
#define WORDSIZE_32
#define W64(lo,hi)      lo,hi
#endif

#define K_BYTES         32
#define K_WORDS         (K_BYTES/sizeof(U_WORD))

#define W256(x0,x1,x2,x3,x4,x5,x6,x7) {W64(x0,x1),W64(x2,x3),W64(x4,x5),W64(x6,x7)}
U32 ecp_Add(U32* Z, const U32* X, const U32* Y);	
typedef struct
{
    U32 X[8];   /* x = X/Z */
    U32 Z[8];   /*  */
} XZ_POINT;	

/* Projective coordinates */
typedef struct {
    U_WORD x[K_WORDS];  /* x/z */
    U_WORD y[K_WORDS];  /* y/z */
    U_WORD z[K_WORDS];
    U_WORD t[K_WORDS];  /* xy/z */
} Ext_POINT;

/* pre-computed, extended point */
typedef struct
{
    U_WORD YpX[K_WORDS];        /* Y+X */
    U_WORD YmX[K_WORDS];        /* Y-X */
    U_WORD T2d[K_WORDS];        /* 2d*T */
    U_WORD Z2[K_WORDS];         /* 2*Z */
} PE_POINT;

/* pre-computed, Affine point */
typedef struct
{
    U_WORD YpX[K_WORDS];        /* Y+X */
    U_WORD YmX[K_WORDS];        /* Y-X */
    U_WORD T2d[K_WORDS];        /* 2d*T */
} PA_POINT;

typedef struct {
    U_WORD bl[K_WORDS];
    U_WORD zr[K_WORDS];
    PE_POINT BP;
} EDP_BLINDING_CTX;	
	
/* Return public key associated with sk */
/* sk will be trimmed on return */
void curve25519_dh_CalculatePublicKey(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk);         /* [32-bytes] IN/OUT: Your secret key */

/* Faster alternative for curve25519_dh_CalculatePublicKey */
/* sk will be trimmed on return */
void curve25519_dh_CalculatePublicKey_fast(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk);         /* [32-bytes] IN/OUT: Your secret key */

/* sk will be trimmed on return */
void curve25519_dh_CreateSharedKey(
    unsigned char *shared,      /* [32-bytes] OUT: Created shared key */
    const unsigned char *pk,    /* [32-bytes] IN: Other side's public key */
    unsigned char *sk);         /* [32-bytes] IN/OUT: Your secret key */

void curve25519_dh_CalculatePublicKey_half(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk);  	
#ifdef __cplusplus
}
#endif
#endif  /* __curve25519_dh_key_exchange_h__ */
