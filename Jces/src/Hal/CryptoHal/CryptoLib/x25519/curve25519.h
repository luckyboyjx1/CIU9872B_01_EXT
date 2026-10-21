
#include "Crypto_Constant.h"

static const unsigned char Basepointasepoint[32] = {9};

//int curve25519_donna(u8 *output, const u8 *secret, const u8 *bp);

void curve25519_dh_CalculatePublicKey(
    unsigned char *pk,          /* [32-bytes] OUT: Public key */
    unsigned char *sk);         /* [32-bytes] IN/OUT: Your secret key */
		
void curve25519_dh_CreateSharedKey(
    unsigned char *shared,      /* [32-bytes] OUT: Created shared key */
    const unsigned char *pk,    /* [32-bytes] IN: Other side's public key */
    unsigned char *sk);         /* [32-bytes] IN/OUT: Your secret key */