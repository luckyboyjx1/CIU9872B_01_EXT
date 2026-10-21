#ifndef __CRYPTO_DH__
#define __CRYPTO_DH__

#include "profile.h"

#if VM_WITH_DH

//-------------------------------------------------------------------------------------------------
// Define DH Key Reference Store Structure
//-------------------------------------------------------------------------------------------------
typedef struct DHKeyRefStore
{
	u8* pbParmP;
	u8* pbParmG;
	u8* pbParmX;
	u8* pbParmY;
	u16 wPrimeLength;
} DHKeyRefStore;

#define TYPE_DH_Y  11
#define TYPE_DH_G  12
#define TYPE_DH_P  13
#define TYPE_DH_K  14
#define TYPE_DH_X  15

#define DH_KEY_CALCU_ERR  0x55AA

#define LENGTH_DH_1024   (1024 >> 3)
#define LENGTH_DH_2048   (2048 >> 3)

extern u8 const DHPparam_Lenth1024[];
extern u8 const DHPparam_Lenth2048[];
extern u8 const DHGparam_Lenth1024[];
extern u8 const DHGparam_Lenth2048[];

extern void storeDHKeyRef(void);
extern void setDHDefaultPAndG(void);
extern void DH(void);
extern void genDHPriKeyX();
extern u16 genDHPubKey(u8* privKey,u8* pubKey,u8* G_Arr,u16 keyLeng,u8* P_Arr);
extern u16 modExpon(u8* privKey,u8* G_Arr,u16 keyLeng,u8* P_Arr,u8* pubKey);
#endif	//  VM_WITH_DH

#endif	// __CRYPTO_RSA__
