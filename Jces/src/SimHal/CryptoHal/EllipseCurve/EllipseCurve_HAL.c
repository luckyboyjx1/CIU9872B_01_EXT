/*
 * SM2_HAL.c
 *
 *  Created on: 2014-2-18
 *      Author: jiangxiaohui
 */

#include "EllipseCurve_API.h"
#include "chipHead.h"
#include "RSA_API.h"
#include "MemOperation.h"
#include "Crypto_API.h"
#include "miracl.h"
#include "MemGetReuseRam.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

#if WITH_SM2||WITH_ECC
u8 CONST SM2_256_DOMAINPARAMS[] = {
/* p */	0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* a */	0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,
/* b */	0x28, 0xE9, 0xFA, 0x9E, 0x9D, 0x9F, 0x5E, 0x34, 0x4D, 0x5A, 0x9E, 0x4B,
		0xCF, 0x65, 0x09, 0xA7, 0xF3, 0x97, 0x89, 0xF5, 0x15, 0xAB, 0x8F, 0x92,
		0xDD, 0xBC, 0xBD, 0x41, 0x4D, 0x94, 0x0E, 0x93,
/* Gx*/	0x32, 0xC4, 0xAE, 0x2C, 0x1F, 0x19, 0x81, 0x19, 0x5F, 0x99, 0x04, 0x46,
		0x6A, 0x39, 0xC9, 0x94, 0x8F, 0xE3, 0x0B, 0xBF, 0xF2, 0x66, 0x0B, 0xE1,
		0x71, 0x5A, 0x45, 0x89, 0x33, 0x4C, 0x74, 0xC7,
/* Gy*/	0xBC, 0x37, 0x36, 0xA2, 0xF4, 0xF6, 0x77, 0x9C, 0x59, 0xBD, 0xCE, 0xE3,
		0x6B, 0x69, 0x21, 0x53, 0xD0, 0xA9, 0x87, 0x7C, 0xC6, 0x2A, 0x47, 0x40,
		0x02, 0xDF, 0x32, 0xE5, 0x21, 0x39, 0xF0, 0xA0,
/* n */	0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0x72, 0x03, 0xDF, 0x6B, 0x21, 0xC6, 0x05, 0x2B,
		0x53, 0xBB, 0xF4, 0x09, 0x39, 0xD5, 0x41, 0x23
};

u8 CONST SM2_256_DOMAINPARAMS_REUSEDATA[] = {
/* Nn = (-N) ^(-1) mod (2 ^ 64), Nn is put in array as little-endian format here*/
		0x75, 0x09, 0x35, 0x72, 0x88, 0x9E, 0x7F, 0x32,
/* Qn = Q ^2 mod N , Qn is put in array as little-endian format here*/
		0x20, 0x4F, 0x11, 0x7C, 0xAF, 0x92, 0x11, 0x90, 0xFA, 0xA2, 0x6F, 0xDE,
		0x4A, 0x50, 0x64, 0x34, 0xD4, 0xE0, 0xFF, 0x3A, 0x4C, 0xC8, 0x0F, 0x62,
		0x3B, 0x3D, 0x2B, 0xA2, 0x12, 0xE4, 0xB5, 0x1E
};

#if WITH_SOFT_SM2||WITH_ECC
/*********************************************************************
 * Function name: ecpAAddB()
 * AbyteLen must not be smaller than BbyteLen
 * A = (A + B)
 *********************************************************************/
void ecpAAddB(PX8 APoint, u16 AbyteLen, PX8 BPoint, u16 BbyteLen, PX8 CPoint)
{
	big aPoint, bPoint, cPoint;

	miracl *mip;
	u16 digitNum;

	digitNum = AbyteLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	aPoint = mirvar(0);
	bPoint = mirvar(0);
	cPoint = mirvar(0);

	mip->IOBASE = 16;

	bytes_to_big(AbyteLen, APoint, aPoint);
	bytes_to_big(BbyteLen, BPoint, bPoint);

    // A = (A^(-1)) mod C
	fadd (aPoint, bPoint, cPoint);

    big_to_bytes(AbyteLen, cPoint, CPoint, true);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
    return;
}


/*********************************************************************
 * Function name: ecpAAddConst()
 * A = (A + ConstValue)
 *********************************************************************/
void ecpAAddConst(PX8 APoint, PX8 BPoint, u16 AbyteLen, u8 constValue)
{
	u8 cPoint[1];
	cPoint[0] = constValue;
	ecpAAddB(APoint, AbyteLen, cPoint,  sizeof(cPoint) , BPoint);
    return;
}

/*********************************************************************
 * Function name: ecpACompareB()
 *  Description: Big number Y substract Big number Z, the result is
 *               always put in big number Y.
 *********************************************************************/
u8 ecpACompareB(PX8 APoint, PX8 BPoint, u16 AbyteLen)
{
	big aPoint, bPoint;
	miracl *mip;
	u16 digitNum;
	u8 result;

	digitNum  = AbyteLen << 1;
	mip = mirsys(digitNum, 16);

	aPoint = mirvar(0);
	bPoint = mirvar(0);

	mip->IOBASE = 16;
	bytes_to_big(AbyteLen, APoint, aPoint);
	bytes_to_big(AbyteLen, BPoint, bPoint);

	/*
	 * Compares two big numbers Parameters:
	 * ←x
	 * ←y
	 * Returns:
	 * +1 ifx>y; 0 if x=y; -1 ifx<y
	 * */
	result = mr_compare(aPoint, bPoint);


    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

	if(0x01 == result)
	{
		return A_IS_BIGGER_THAN_B;
	}
	else if(0x00 == result)
	{
		return A_IS_EQUAL_WITH_B;
	}
	else
	{
		return A_IS_SMALLER_THAN_B;
	}
}

/*********************************************************************
 * Function name: ecpAAddBModC()
 * Description: Big number Y add big number Z in case of overflow, and
 *              then the result is reduced by modulus N, the final
 *              result is put in operandY.
 * D = (A + B) mod C
 *********************************************************************/
void ecpAAddBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
{

	big aPoint, bPoint, cPoint, dPoint, ePoint;
	u8 eValue[1];

	miracl *mip;
	u16 digitNum;

	eValue[0] = 1;
	digitNum = byteLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	aPoint = mirvar(0);
	bPoint = mirvar(0);
	cPoint = mirvar(0);
	dPoint = mirvar(0);
	ePoint = mirvar(0);

	mip->IOBASE = 16;
	bytes_to_big(byteLen, APoint, aPoint);
	bytes_to_big(byteLen, BPoint, bPoint);
	bytes_to_big(byteLen, CPoint, cPoint);
	bytes_to_big(sizeof(eValue), eValue, ePoint);

	mad(aPoint, ePoint, bPoint, cPoint, cPoint, dPoint);

    big_to_bytes(byteLen, dPoint, DPoint, true);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

    return;
}


/*********************************************************************
 * Function name: ecpASubBModC()
 *
 * D = (A - B) mod C
 *********************************************************************/
void ecpASubBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
{
//	big aPoint, bPoint, cPoint, dPoint, ePoint;
//	u8 eValue[1];
//	u8 		tempBuff[MAX_RSA_MUD_BYTE_LEN];
//
//	miracl *mip;
//	u16 digitNum;
//
//	digitNum = byteLen << 1;
//	mip = mirsys(digitNum, 16); // use hex internally
//
//	aPoint = mirvar(0);
//	bPoint = mirvar(0);
//	cPoint = mirvar(0);
//	dPoint = mirvar(0);
//	ePoint = mirvar(0);
//
//	eValue[0] = 1;
//	mip->IOBASE = 16;
//    ecpMem_Copy(tempBuff, APoint, byteLen);
//	bytes_to_big(byteLen, tempBuff, aPoint);
//    ecpMem_Copy(tempBuff, BPoint, byteLen);
//	bytes_to_big(byteLen, tempBuff, bPoint);
//    ecpMem_Copy(tempBuff, CPoint, byteLen);
//	bytes_to_big(byteLen, tempBuff, cPoint);
//	bytes_to_big(sizeof(eValue), eValue, ePoint);
//
//	// B = -B
//	subtract(cPoint, bPoint, bPoint);
////	negify(bPoint, bPoint);
//	mad(aPoint, ePoint, bPoint, cPoint, cPoint, dPoint);
//
//    big_to_bytes(byteLen, dPoint, tempBuff, true);
//    ecpMem_Copy(DPoint, tempBuff, byteLen);
//
//    //Cleans up after the current instance of MIRACL, and frees all internal variables.
//    //A subsequent call to mirsys() will re-initialise the MIRACL system
//    //Must be called after mirsys()
//    mirexit();
    return;
}



///*********************************************************************
// * Function name: ecpAMulBModC()
// *
// * D = (A * B) mod C
// *********************************************************************/
//void ecpAMulBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
//{
//
//	big aPoint, bPoint, cPoint, dPoint;
//
//	miracl *mip;
//	u16 digitNum;
//
//	digitNum = byteLen << 1;
//	mip = mirsys(digitNum, 16); // use hex internally
//
//	aPoint = mirvar(0);
//	bPoint = mirvar(0);
//	cPoint = mirvar(0);
//	dPoint = mirvar(0);
//
//	mip->IOBASE = 16;
//	bytes_to_big(byteLen, APoint, aPoint);
//	bytes_to_big(byteLen, BPoint, bPoint);
//	bytes_to_big(byteLen, CPoint, cPoint);
//
//	mad(aPoint, bPoint, aPoint, cPoint, cPoint, dPoint);
//
//    big_to_bytes(byteLen, dPoint, DPoint, true);
//
//    //Cleans up after the current instance of MIRACL, and frees all internal variables.
//    //A subsequent call to mirsys() will re-initialise the MIRACL system
//    //Must be called after mirsys()
//    mirexit();
//
//    return;
//}

/*********************************************************************
 * Function name: ecpAInvMultBModC()
 *
 * D = (A^(-1) * B) mod C
 *********************************************************************/
void ecpAInvMultBModC(PX8 APoint, u16 byteLen, PX8 BPoint, PX8 CPoint, PX8 DPoint)
{

	big aPoint, bPoint, cPoint, dPoint;

	miracl *mip;
	u16 digitNum;

	digitNum = byteLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	aPoint = mirvar(0);
	bPoint = mirvar(0);
	cPoint = mirvar(0);
	dPoint = mirvar(0);

	mip->IOBASE = 16;
	bytes_to_big(byteLen, APoint, aPoint);
	bytes_to_big(byteLen, BPoint, bPoint);
	bytes_to_big(byteLen, CPoint, cPoint);


    // A = (A^(-1)) mod C
    xgcd(aPoint, cPoint, aPoint, aPoint, aPoint);

    //D = (A * B) mod C
	mad(aPoint, bPoint, aPoint, cPoint, cPoint, dPoint);

    big_to_bytes(byteLen, dPoint, DPoint, true);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

    return;
}

/**********************************************************************
 * Function name: ecpAInvMultBModC()
 * Qxy = [A] * Pxy:
 * APoint must be 32 bytes
 **********************************************************************/
u8 ecpAMulPointXY(PM8 APoint, PM8 PointPXY, PX8 PointQXY)
{
	big A, B, POrBitCofficient;
	big aPoint, Gx, Gy;
	big rPoint;
	epoint * G;
	miracl * mip;
	u8 		tempBuff[SM2_DOMAIN_NLEN];

	u16 digitNum = SM2_DOMAIN_NLEN << 1; // G_order may be longer than bECKeyByteLength
	mip = mirsys(digitNum, 16); // use hex internally

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	aPoint = mirvar(0);
	Gx = mirvar(0);
	Gy = mirvar(0);
	rPoint = mirvar(0);

	mip->IOBASE = 16;

    ecpMem_Copy(tempBuff, SM2_DOMAINPARAMS_POINT_A, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, A);

    ecpMem_Copy(tempBuff, SM2_DOMAINPARAMS_POINT_B, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, B);

	ecpMem_Copy(tempBuff, SM2_DOMAINPARAMS_POINT_P, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, POrBitCofficient);

	ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);  /* Use PROJECTIVE if possible, else AFFINE coordinates */

	// set point Pxy
	G = epoint_init();
    ecpMem_Copy(tempBuff, PointPXY, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, Gx);

    ecpMem_Copy(tempBuff, (PointPXY + SM2_DOMAIN_NLEN), SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, Gy);
	if(TRUE != epoint_set(Gx, Gy, 0, G))
	{
		return POINT_NOT_IN_ELL;
	}

	// set point A
    ecpMem_Copy(tempBuff, APoint, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, aPoint);

	// cal Qxy = A * Pxy
	ecurve_mult(aPoint, G, G);

	// get point QXY
	epoint_get(G, Gx, Gy);

    big_to_bytes(SM2_DOMAIN_NLEN, Gx, tempBuff, true);
    ecpMem_Copy(PointQXY, tempBuff, SM2_DOMAIN_NLEN);

    big_to_bytes(SM2_DOMAIN_NLEN, Gy, tempBuff, true);
    ecpMem_Copy(PointQXY + SM2_DOMAIN_NLEN, tempBuff, SM2_DOMAIN_NLEN);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
	return CAL_SUCCESSED;
}



/**********************************************************************
 * Function name: ecpPointAddPoint()
 * Qxy = Mxy + Nxy
 **********************************************************************/

u8 ecpPointAddPoint(PX8 PointMxy, PM8 PointNxy, PX8 PointQXY)
{
	big A, B, POrBitCofficient;
	big mPointx, mPointy, nPointx, nPointy;
	epoint * mPointXy;
	epoint * nPointXy;
	miracl * mip;
	u8 		tempBuff[SM2_DOMAIN_NLEN];

	u16 digitNum = SM2_DOMAIN_NLEN << 1; // G_order may be longer than bECKeyByteLength
	mip = mirsys(digitNum, 16); // use hex internally

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	mPointx = mirvar(0);
	mPointy = mirvar(0);
	nPointx = mirvar(0);
	nPointy = mirvar(0);

	mip->IOBASE = 16;

    ecpMem_Copy(tempBuff, SM2_DOMAINPARAMS_POINT_A, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, A);
    ecpMem_Copy(tempBuff, SM2_DOMAINPARAMS_POINT_B, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, B);
    ecpMem_Copy(tempBuff, SM2_DOMAINPARAMS_POINT_P, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, POrBitCofficient);

	ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);  /* Use PROJECTIVE if possible, else AFFINE coordinates */

	// set point Mxy
	mPointXy = epoint_init();
    ecpMem_Copy(tempBuff, PointMxy, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, mPointx);

    ecpMem_Copy(tempBuff, (PointMxy + SM2_DOMAIN_NLEN), SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, mPointy);
	if(TRUE != epoint_set(mPointx, mPointy, 0, mPointXy))
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return POINT_NOT_IN_ELL;
	}
	epoint_set(mPointx, mPointy, 0, mPointXy);

	// set point Nxy
	nPointXy = epoint_init();
    ecpMem_Copy(tempBuff, PointNxy, SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, nPointx);

    ecpMem_Copy(tempBuff, (PointNxy + SM2_DOMAIN_NLEN), SM2_DOMAIN_NLEN);
	bytes_to_big(SM2_DOMAIN_NLEN, tempBuff, nPointy);
	if(TRUE != epoint_set(nPointx, nPointy, 0, nPointXy))
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return POINT_NOT_IN_ELL;
	}

	// Mxy = Mxy + Nxy
	ecurve_add(nPointXy, mPointXy);

	// get point QXY
	epoint_get(mPointXy, mPointx, mPointy);
	big_to_bytes(SM2_DOMAIN_NLEN, mPointx, tempBuff, true);
	ecpMem_Copy(PointQXY, tempBuff, SM2_DOMAIN_NLEN);

	big_to_bytes(SM2_DOMAIN_NLEN, mPointy, tempBuff, true);
	ecpMem_Copy(PointQXY + SM2_DOMAIN_NLEN, tempBuff, SM2_DOMAIN_NLEN);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

	return CAL_SUCCESSED;
}
 
#endif // WITH_SOFT_SM2

#endif //WITH_SM2

#if WITH_ECC
#define	ECC_PARAM_FP_OFFSET				0
#define	ECC_PARAM_A_OFFSET				MAX_ECC_KEY_BYTELEN
#define	ECC_PARAM_B_OFFSET				(MAX_ECC_KEY_BYTELEN * 2)
#define	ECC_PARAM_Gx_OFFSET				(MAX_ECC_KEY_BYTELEN * 3)
#define	ECC_PARAM_Gy_OFFSET				(MAX_ECC_KEY_BYTELEN * 4)
#define	ECC_PARAM_N_OFFSET				(MAX_ECC_KEY_BYTELEN * 5)

#define	ECC_WORK_TEMP_PRIKEY_OFFSET		0
#define	ECC_WORK_K_OFFSET				MAX_ECC_KEY_BYTELEN
#define	ECC_WORK_S_OFFSET				(MAX_ECC_KEY_BYTELEN * 2)
#define	ECC_WORK_Rx_OFFSET				(MAX_ECC_KEY_BYTELEN * 3)
#define	ECC_WORK_Ry_OFFSET				(MAX_ECC_KEY_BYTELEN * 4)
#define	ECC_WORK_E_OFFSET				(MAX_ECC_KEY_BYTELEN * 5)
//static u8 isprime(PX8 Des)
//{
//	return 1;
//}
//static void bigrand(PX8 Src, PX8 Des, u16 length)
//{
//	do
//	{
//		JCES_Random_number_generator(Des, length, ALG_SECURE_RANDOM);
//	}while(0x01 != ecpMem_Compare(Src, Des, length));
////	ecpMem_Set(Des, 0x01, length);
//}

// ************************************************************************
// Common Initialization function for ECC structures
// ************************************************************************
static void initECParams(ecpEccGfp_KeyGen_Param_t X_A * ECKeyGenParams, u8 X_A * workspaceCPU, u8 X_A * workspaceFX)
{
	ecpEccGfp_DomainParam_t X_A ECDomainParams;

	// Initialize EC domain parameter
	ECDomainParams.p.pPtr = g_ECKeyStore.pb_domainParams;
	ECDomainParams.p.len = g_ECKeyStore.b_PLength;
	ECDomainParams.a = ECDomainParams.p.pPtr + ECDomainParams.p.len;
	ECDomainParams.b = ECDomainParams.a + ECDomainParams.p.len;
	ECDomainParams.G.x = ECDomainParams.b + ECDomainParams.p.len + 1;
	ECDomainParams.G.y = ECDomainParams.G.x + ECDomainParams.p.len;
	ECDomainParams.n.pPtr = ECDomainParams.G.y + ECDomainParams.p.len;
	ECDomainParams.n.len = g_ECKeyStore.b_RLength;

	// Initialize calling structure for EC Key Generation
	ECKeyGenParams->pWorkspaceCPU = workspaceCPU;
	ECKeyGenParams->pWorkspaceFX = workspaceFX;
	ECKeyGenParams->domain = ECDomainParams;
}

//static void initECKeyExchange(ecpEccGfp_KeyExch_Param_t X_A * ECKeyExchange, u8 * privKey, u8 * pubKey, u8 X_A * commonSecret)
//{
//	ECKeyExchange->privKey = privKey;
//	ECKeyExchange->pubKey = pubKey;
//	ECKeyExchange->commonSecret = commonSecret;
//}

/****************************************************************************
 * Function name: doECOpertaion
 * Description: generate a signature from a hash value of message
 * Input Parameters:
 * 		u8 bHashLen 		  The length of source data
 * Output Parameters:		  The processing result code
 *	    signature length	  		 For success
 *		EC_PROCESS_FAIL				 For fail
 * NOTE:
 ****************************************************************************/
u8 DoECOpertaion(u8 bHashLen)
{
	u8 					tempR[MAX_ECC_KEY_BYTELEN + 1];
	u8 					tempS[MAX_ECC_KEY_BYTELEN + 1];
	big A, B, POrBitCofficient, Gx, Gy, G_order, tR, S, priD, K, Hash;
	epoint * G;
	time_t seed;
	miracl *mip;
	u16 digitNum;

	u8 tmpBuff[MAX_ECC_KEY_BYTELEN];
	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 Rlength;
	u8 Slength;
	u8 tmpLen;
	u8 bECKeyByteLength;


	Rlength = 0;
	Slength = 0;
	tmpLen = g_ECKeyStore.b_RLength;
	bECKeyByteLength = g_ECKeyStore.b_PLength;

	digitNum = tmpLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	mip->IOBASE = 16;

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	Gx = mirvar(0);
	Gy = mirvar(0);
	G_order = mirvar(0);
	tR = mirvar(0);
	S = mirvar(0);
	priD = mirvar(0);
	K = mirvar(0);
	Hash = mirvar(0);

	initECParams(&ECKeyGenParams, NULL, NULL);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, A);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, B);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, POrBitCofficient);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.G.x, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Gx); // skip type value 0x04

    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.G.y, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Gy);

    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.n.pPtr, tmpLen);
    bytes_to_big(tmpLen, (const char *)tmpBuff, G_order);

    ecpMem_Copy(tmpBuff, g_ECKeyStore.pb_PrivateKeyS, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, priD);

	if(bHashLen > tmpLen)
	{
		bHashLen = tmpLen;
		ecpMem_Copy(MEMtPX8(tmpBuff), g_CryptoData.pbBuffer, bHashLen);
	}
	else
	{
		ecpMem_Set(MEMtPX8(tmpBuff), 0x00, (tmpLen - bHashLen));
		ecpMem_Copy(PX8_A_U16(MEMtPX8(tmpBuff), (tmpLen - bHashLen)), g_CryptoData.pbBuffer, bHashLen);
	}
	bytes_to_big(tmpLen, (const char *)tmpBuff, Hash);

	// the g_CryptoData.pbBuffer is structured as follows:
	// --------------------------------------------------------------------------------------------
	//         hash        | public key space for signature |      signature R |    signature S   |
	// --------------------------------------------------------------------------------------------
	//  length of bHashLen |          twice length of P     | length of tmpLen | length of tmpLen |
	// --------------------------------------------------------------------------------------------

	// init random number generator
	time(&seed);
	irand((unsigned long)seed);

	ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);  /* Use PROJECTIVE if possible, else AFFINE coordinates */

	G = epoint_init();
	epoint_set(Gx, Gy, 0, G);

	bigrand(G_order, K);
	ecurve_mult(K, G, G);
	epoint_get(G, tR, tR);
	divide(tR, G_order, G_order);

	// compute s
	xgcd(K, G_order, K, K, K);

	mad(priD, tR, Hash, G_order, G_order, S);
	mad(S, K, K, G_order, G_order, S);

	// obtain the result (r, s) = (ECKEYGEN_BASE_Qx, ECKEYGEN_BASE_Qy)
	big_to_bytes(tmpLen, tR, (char *)tempR, TRUE);
	big_to_bytes(tmpLen, S, (char *)tempS, TRUE);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

//	Rlength = getRealLength(&g_CryptoData.pbBuffer[bHashLen+bECKeyByteLength*2], tmpLen);
//	Slength = getRealLength(&g_CryptoData.pbBuffer[bHashLen+bECKeyByteLength*2+tmpLen], tmpLen);

//    ecpMem_Copy(tempR, MEMtPX8(R), tmpLen);
//    ecpMem_Copy(tempS, MEMtPX8(S), tmpLen);
	
	Rlength = getRealLength(tempR,tmpLen);
	Rlength = getRealLength2(tempR,Rlength);  
	Slength = getRealLength(tempS,tmpLen);
	Slength = getRealLength2(tempS,Slength);
	// return the encoded signature lengh
	EncodeSignature(tempR, &Rlength, tempS, &Slength);

//	EncodeSignature(&g_CryptoData.pbBuffer[bHashLen+bECKeyByteLength*2], &Rlength,
//			&g_CryptoData.pbBuffer[bHashLen+bECKeyByteLength*2+tmpLen], &Slength);
	return g_CryptoData.wBufferLen = Rlength + Slength + 6;
}

/****************************************************************************
 * Function name: GenECKeyPair
 * Description: Through this function the pair of keys (d,Q) are generated
 * Input Parameters:
 * 		none.
 * Output Parameters:
 *	    none.
 * NOTE: now just support ALG_EC_FP
 ****************************************************************************/
void GenECKeyPair(void)
{
	big priD, A, B, POrBitCofficient, Gx, Gy, G_order;
	epoint * G;
	time_t seed;
	miracl * mip;
	unsigned int digitNum;

	u8 tmpBuff[MAX_ECC_KEY_BYTELEN];
	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 tmpLen;
	u8 bECKeyByteLength;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_003();

	InitDefaultECDomainParams();

	tmpLen = g_ECKeyStore.b_RLength;
	bECKeyByteLength = g_ECKeyStore.b_PLength;

	digitNum = tmpLen << 1; // G_order may be longer than bECKeyByteLength
	mip = mirsys(digitNum, 16); // use hex internally

	mip->IOBASE = 16;

	priD = mirvar(0);
	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	Gx = mirvar(0);
	Gy = mirvar(0);
	G_order = mirvar(0);

	initECParams(&ECKeyGenParams, NULL, NULL);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, A);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, B);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, POrBitCofficient);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.G.x, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Gx); // skip type value 0x04

    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.G.y, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Gy);

    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.n.pPtr, tmpLen);
    bytes_to_big(tmpLen, (const char *)tmpBuff, G_order);

	// init random number generator
	time(&seed);
    irand((unsigned long)seed);

    ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);  /* Use PROJECTIVE if possible, else AFFINE coordinates */

    G = epoint_init();
    epoint_set(Gx, Gy, 0, G);

	// generate private key (less than order R)
	bigrand(G_order, priD);

	// generate public key
	ecurve_mult(priD, G, G);
	epoint_get(G, Gx, Gy);

    // store private key
	big_to_bytes(bECKeyByteLength, priD, (char *)tmpBuff, TRUE);
	memCopyNoAtomic(g_ECKeyStore.pb_PrivateKeyS, tmpBuff, bECKeyByteLength);

    // store public key
	// store type value
	memWriteU8NoAtomic(g_ECKeyStore.pb_PublicKeyQxy, 0x04);
    big_to_bytes(bECKeyByteLength, Gx, (char *)tmpBuff, TRUE);
    memCopyNoAtomic((g_ECKeyStore.pb_PublicKeyQxy + 1), tmpBuff, bECKeyByteLength);
    big_to_bytes(bECKeyByteLength, Gy, (char *)tmpBuff, TRUE);
    memCopyNoAtomic((g_ECKeyStore.pb_PublicKeyQxy + 1 + bECKeyByteLength), tmpBuff, bECKeyByteLength);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
}

/**********************************************************************************
 * Function name: ECVerify
 * Description: This function verifies a ECC signature.
 * input parameter:
 * 		PM8 inBuff             The source data address.
 * 		u16 inLen              The length of source data.
 *      PM8 sigBuff            The signature buffer.
 *      u16 sigLen             signature length.
 * 		PM8 mdBuff             md information.
 *      PM8 tmpBuff			   md temp data.
 *      u8 algType			   algorithm.
 * Output Parameters:		The processing result code
 *	    EC_VERIFY_OK		  		     For success
 *		EC_VERIFY_FAIL		             For fail
 * NOTE:
 **********************************************************************************/

#define ECC_VERIFY_WORK_TEMP_PUBKEY_OFFSET		0
#define ECC_VERIFY_WORK_TEMP_PUBKEY_X_OFFSET	ECC_VERIFY_WORK_TEMP_PUBKEY_OFFSET
#define ECC_VERIFY_WORK_TEMP_PUBKEY_Y_OFFSET	MAX_ECC_KEY_BYTELEN
#define ECC_VERIFY_WORK_TEMP_RPOINT_OFFSET		(MAX_ECC_KEY_BYTELEN * 2)
#define ECC_VERIFY_WORK_TEMP_RPOINT_X_OFFSET	ECC_VERIFY_WORK_TEMP_RPOINT_OFFSET
#define ECC_VERIFY_WORK_TEMP_RPOINT_Y_OFFSET	(MAX_ECC_KEY_BYTELEN * 3)
#define ECC_VERIFY_WORK_TEMP_S_OFFSET			(MAX_ECC_KEY_BYTELEN * 4)
#define ECC_VERIFY_WORK_TEMP_R_OFFSET			(MAX_ECC_KEY_BYTELEN * 5)
#define ECC_VERIFY_WORK_TEMP_E_OFFSET			(MAX_ECC_KEY_BYTELEN * 6)
/* pushShort(ECVerify(MEMtPN8(inBuff), inLen, MEMtPN8(sigBuff), sigLen, MEMtPN8(mdBuff), MEMtPN8(tmpBuff), algType));
*	inBuff:  message buffer
*	inLen:
*	sigBuff: sign result
*	sigLen:
*	mdBuff: hash update data
*	tmpBuff:
*	algType:HASH Type
*/
boolean ECVerify(PM8 inBuff, u16 inLen, PM8 sigBuff, u16 sigLen, PM8 mdBuff, PM8 tmpBuff, u8 algType,u16 preHashFlag)
{
	big A, B, POrBitCofficient, X, Y, G_order, V, U1, U2, tR, S, Hash;
	epoint * G, * public;
	time_t seed;
	miracl *mip;
	u16 digitNum;

	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 tmpWS[MAX_ECC_KEY_BYTELEN];
	u8 X_A hashType, hashLen, mdState;
	u8 X_A rLength, sLength;
	u8 bECKeyByteLength;
	u8 X_A orderLen;
	u8 X_A * rSign;
	u8 X_A * sSign;
	u8 X_A * hashBuff;

	mdState = HASH_INIT;
	bECKeyByteLength = g_ECKeyStore.b_PLength;
	orderLen = g_ECKeyStore.b_RLength;

	// the g_CryptoData.pbBuffer is structured as follows:
	// ----------------------------------------
	//  decode signature (r, s) |  hash value |
	// ----------------------------------------
	//     twice length of R    | length as R |
	// ----------------------------------------
	rSign = g_CryptoData.pbBuffer;
	sSign = rSign + orderLen;
	hashBuff = sSign + orderLen;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_005();

	// decode the formated signature first
	if(EC_VERIFY_FAIL == DecodeSignature(rSign, &rLength, sSign, &sLength, sigBuff, (u8)sigLen))
	{
		return EC_VERIFY_FAIL;
	}

	// verify the condition: r, s are in the interval [1, n-1], note prefix zeros
	if(rLength > orderLen || sLength > orderLen)
	{
		return EC_VERIFY_FAIL;
	}


	digitNum = orderLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	mip->IOBASE = 16;

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	X = mirvar(0);
	Y = mirvar(0);
	G_order = mirvar(0);
	V = mirvar(0);
	U1 = mirvar(0);
	U2 = mirvar(0);
	tR = mirvar(0);
	S = mirvar(0);
	Hash = mirvar(0);

	initECParams(&ECKeyGenParams, NULL, NULL);

	ecpMem_Copy(tmpWS, ECKeyGenParams.domain.a, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpWS, A);

	ecpMem_Copy(tmpWS, ECKeyGenParams.domain.b, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpWS, B);

	ecpMem_Copy(tmpWS, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpWS, POrBitCofficient);

	ecpMem_Copy(tmpWS, ECKeyGenParams.domain.G.x, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpWS, X); // skip type value 0x04

    ecpMem_Copy(tmpWS, ECKeyGenParams.domain.G.y, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpWS, Y);

    ecpMem_Copy(tmpWS, ECKeyGenParams.domain.n.pPtr, orderLen);
    bytes_to_big(orderLen, (const char *)tmpWS, G_order);

    // init random number generator
	time(&seed);
	irand((unsigned long)seed);

	ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);  /* Use PROJECTIVE if possible, else AFFINE coordinates */

    G = epoint_init();
    epoint_set(X, Y, 0, G);

    ecpMem_Copy(tmpWS, (g_ECKeyStore.pb_PublicKeyQxy + 1), bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpWS, X);
    ecpMem_Copy(tmpWS, (g_ECKeyStore.pb_PublicKeyQxy + 1 + bECKeyByteLength), bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpWS, Y);
    public = epoint_init();
    epoint_set(X, Y, 0, public);
	
	switch(algType)
	{
		case ALG_ECDSA_SHA:
			hashType = ALG_SHA_1;
			hashLen = SHA1_DIGEST_SIZE;
			break;
		case ALG_ECDSA_SHA_224:
			hashType = ALG_SHA_224;
			hashLen = SHA224_DIGEST_SIZE;
			break;
		case ALG_ECDSA_SHA_256:
			hashType = ALG_SHA_256;
			hashLen = SHA256_DIGEST_SIZE;
			break;
		case ALG_ECDSA_SHA_384:
			hashType = ALG_SHA_384;
			hashLen = SHA384_DIGEST_SIZE;
			break;
		case ALG_ECDSA_SHA_512:
			hashType = ALG_SHA_512;
			hashLen = SHA512_DIGEST_SIZE;
			break;
	}

	if(mdBuff != NULL)
	{
		mdState = HASH_UPDATE;
	}
	
	//compute the hash value, the hash function is SHA-1
	if(hashLen > orderLen)
	{
		hashLen = orderLen;
		if(preHashFlag == 0x5A5A)
		{
			memCopyToRam(hashBuff,inBuff,inLen);
		}
		else
		{
			HASHDoFinal((PM8)inBuff, inLen, MEMtPX8(hashBuff), mdState, mdBuff, tmpBuff, hashType);
		}
	}
	else
	{
		ecpMem_Set(MEMtPX8(hashBuff), 0x00, (orderLen - hashLen));
		if(preHashFlag == 0x5A5A)
		{
			memCopyToRam(PX8_A_U16(MEMtPX8(hashBuff), (orderLen - hashLen)),inBuff,inLen);
		}
		else
		{
			HASHDoFinal((PM8)inBuff, inLen, PX8_A_U16(MEMtPX8(hashBuff), (orderLen - hashLen)), mdState, mdBuff, tmpBuff, hashType);
		}
	}


	bytes_to_big(orderLen, (const char *)hashBuff, Hash);

	// verify the condition: r is in the interval [1, n-1], note prefix zeros
	bytes_to_big(rLength, (const char *)rSign, tR);
	bytes_to_big(sLength, (const char *)sSign, S);
	if((mr_compare(tR, G_order) >= 0) || (mr_compare(S, G_order) >= 0))
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialize the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_VERIFY_FAIL;
	}

	//compute the reverse value of s (mod order of G)
	xgcd(S, G_order, S, S, S);

	mad(Hash, S, S, G_order, G_order, U1);
	mad(tR, S, S, G_order, G_order, U2);

	ecurve_mult2(U2, public, U1, G, G);
	epoint_get(G, V, V);
	divide(V, G_order, G_order);

	if(mr_compare(V, tR) != 0)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_VERIFY_FAIL;
	}
    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
	return EC_VERIFY_OK;
}

/****************************************************************************
 * Function name: CheckECDomainParams
 * Description: This function should be used to check that
 *              elliptic curve domain parameters over Fp or F2M are valid
 *              (here we just check the cofactor is not zero)
 * Input Parameters:
 * 		none
 * Output Parameters:		The processing result code
 *	    EC_DOMAINPARAMS_CHECK_OK		  		 For success
 *		EC_DOMAINPARAMS_CHECK_FAIL				 For fail
 * NOTE: now just check EC domain param on FP
 ****************************************************************************/
boolean CheckECDomainParams(void)
{
	big A, powerA, B, powerB, POrBitCofficient, Gx, Gy, G_order;
	epoint * G, * W;
	miracl * mip;
	u16 digitNum;

	u8 tmpBuff[MAX_ECC_KEY_BYTELEN];
	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 tmpLen;
	u8 bECKeyByteLength;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_006();

	tmpLen = g_ECKeyStore.b_RLength;
	bECKeyByteLength = g_ECKeyStore.b_PLength;

	digitNum = tmpLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	mip->IOBASE = 16;

	A = mirvar(0);
	powerA = mirvar(0);
	B = mirvar(0);
	powerB = mirvar(0);
	POrBitCofficient = mirvar(0);
	Gx = mirvar(0);
	Gy = mirvar(0);
	G_order = mirvar(0);

	initECParams(&ECKeyGenParams, NULL, NULL);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, A);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, B);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, POrBitCofficient);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.G.x, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Gx); // skip type value 0x04

    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.G.y, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Gy);

    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.n.pPtr, tmpLen);
    bytes_to_big(tmpLen, (const char *)tmpBuff, G_order);

	//check condition 1: P is an odd prime
	if(isprime(POrBitCofficient) == 0x00)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}

	//check condition 2: A, B, Gx, Gy are integers in the interval	[0, p-1]
	if(mr_compare(A, POrBitCofficient) >= 0)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	if(mr_compare(B, POrBitCofficient) >= 0)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	if(mr_compare(Gx, POrBitCofficient) >= 0)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	if(mr_compare(Gy, POrBitCofficient) >= 0)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
/*
	//check condition 3: 4a^3 + 27b^2 != 0 (mod p)
	//compute a^3
	power(A, 3, POrBitCofficient, powerA);

	//put 4*a^3 in ECKEYGEN_BASE_TempQx
	number = 4;
	mad(powerA, NUM, NUM, POrBitCofficient, POrBitCofficient, powerA);

	//compute b^2
	power(B, 2, POrBitCofficient, powerB);

	//compute 27b^2
	number = 27;
	mad(powerB, NUM, NUM, POrBitCofficient, POrBitCofficient, powerB);

	//compute the addition
	number = 1;
	mad(powerA, NUM, powerB, POrBitCofficient, POrBitCofficient, powerA);

	NUM->len = g_ECKeyStore.b_PLength;
	number = 0;
   	if(compare(powerA, NUM) == 0)
	{
		return EC_PROCESS_FAIL;
	}
*/
	//check condition 4: Gy^2 = Gx^3 + a*Gx + b (mod p) = Gx(Gx^2 + a) + b (mod p)
   	ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);
   	G = epoint_init();
   	if(!epoint_set(Gx, Gy, 0, G))
   	{
   	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
   	    //A subsequent call to mirsys() will re-initialise the MIRACL system
   	    //Must be called after mirsys()
   	    mirexit();
   		return EC_DOMAINPARAMS_CHECK_FAIL;
   	}

    //check condition 5: the order of G(x,y), R is prime
	if(isprime(G_order) == 0x00)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}

	//check condition 6: cofactor h
	if(g_ECKeyStore.bParamStatus & 0x01)
	{
		if(g_ECKeyStore.b_HValue == 0)
		{
		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
		    //A subsequent call to mirsys() will re-initialise the MIRACL system
		    //Must be called after mirsys()
		    mirexit();
			return EC_DOMAINPARAMS_CHECK_FAIL;
		}
	}

	//check condition 7: R*G is the infinity (or that R*G + G = G)
   	W = epoint_init();
   	ecurve_mult(G_order, G, W);
   	if(!point_at_infinity(W))
   	{
   	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
   	    //A subsequent call to mirsys() will re-initialise the MIRACL system
   	    //Must be called after mirsys()
   	    mirexit();
   		return EC_DOMAINPARAMS_CHECK_FAIL;
   	}

	//check condition 8 : p^B != 1 (mod n), for 1 <= B < 100, and p != n
   	//ignore here

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
	return EC_DOMAINPARAMS_CHECK_OK; //all the conditions are valid
}

/****************************************************************************
 * Function name: CheckECPrivateKey
 * Description: This is elliptic curve private key over Fp or F2M
 *              validation primitive.
 * Input Parameters:
 * 		none
 * Output Parameters:		The processing result code
 *	    EC_KEY_CHECK_OK		  		For success
 *		EC_KEY_CHECK_FAIL			For fail
 * NOTE: now just check EC domain param on FP
 ****************************************************************************/
u8 CheckECPrivateKey(void)
{
	big G_order, priD;
	miracl * mip;
	u16 digitNum;

	u8 tmpBuff[MAX_ECC_KEY_BYTELEN];
	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 X_A keyByteLen;
	u8 tmpLen;
	u8 * tmpPoint;

	keyByteLen = g_ECKeyStore.b_PLength;
	tmpLen = g_ECKeyStore.b_RLength;

	digitNum = tmpLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	mip->IOBASE = 16;

	G_order = mirvar(0);
	priD = mirvar(0);

	initECParams(&ECKeyGenParams, NULL, NULL);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.n.pPtr, tmpLen);
	bytes_to_big(tmpLen, (const char *)tmpBuff, G_order);

    ecpMem_Copy(tmpBuff, g_ECKeyStore.pb_PrivateKeyS, keyByteLen);
    bytes_to_big(keyByteLen, (const char *)tmpBuff, priD);

	//the private key is not 0, and smaller than R
	if(keyByteLen == 0)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_KEY_CHECK_FAIL;
	}
	else
	{
		tmpPoint = g_ECKeyStore.pb_PrivateKeyS;
		while(keyByteLen)
		{
			if(PN_R_U8(tmpPoint++))
			{
				break;
			}
			keyByteLen--;
		}
		if(keyByteLen == 0 || mr_compare(priD, G_order) >= 0)
		{
		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
		    //A subsequent call to mirsys() will re-initialise the MIRACL system
		    //Must be called after mirsys()
		    mirexit();
			return EC_KEY_CHECK_FAIL;
		}
	}
    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
	return EC_KEY_CHECK_OK;
}

/****************************************************************************
 * Function name: CheckECPublicKey
 * Description: This is elliptic curve public key over Fp or F2M
 *              validation primitive.
 * Input Parameters:
 * 		none
 * Output Parameters:		The processing result code
 *	    EC_KEY_CHECK_OK		  		For success
 *		EC_KEY_CHECK_FAIL			For fail
 * NOTE: now just check EC domain param on FP
 ****************************************************************************/
boolean CheckECPublicKey(void)
{
	big A, B, POrBitCofficient, Qx, Qy, G_order;
	epoint * Q;
	miracl * mip;
	u16 digitNum;

	u8 tmpBuff[MAX_ECC_KEY_BYTELEN];
	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 tmpLen;
	u8 bECKeyByteLength;

	bECKeyByteLength = g_ECKeyStore.b_PLength;
	tmpLen = g_ECKeyStore.b_RLength;

	digitNum = tmpLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	mip->IOBASE = 16;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_007();

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	Qx = mirvar(0);
	Qy = mirvar(0);
	G_order = mirvar(0);

	memRefresh();

	initECParams(&ECKeyGenParams, NULL, NULL);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, A);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, B);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, POrBitCofficient);

	ecpMem_Copy(tmpBuff, (g_ECKeyStore.pb_PublicKeyQxy + 1), bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Qx); // skip type value 0x04

    ecpMem_Copy(tmpBuff, (g_ECKeyStore.pb_PublicKeyQxy + 1 + bECKeyByteLength), bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Qy);

    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.n.pPtr, tmpLen);
    bytes_to_big(tmpLen, (const char *)tmpBuff, G_order);

	if(g_ECKeyStore.b_PLength == 0)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_KEY_CHECK_FAIL;
	}
	else
	{
		// the public key is smaller than P
		if(mr_compare(Qx, POrBitCofficient) >= 0)
		{
		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
		    //A subsequent call to mirsys() will re-initialise the MIRACL system
		    //Must be called after mirsys()
		    mirexit();
			return EC_KEY_CHECK_FAIL;
		}
		if(mr_compare(Qy, POrBitCofficient) >= 0)
		{
		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
		    //A subsequent call to mirsys() will re-initialise the MIRACL system
		    //Must be called after mirsys()
		    mirexit();
			return EC_KEY_CHECK_FAIL;
		}

		// check the public point whether is on the elliptic curve
		ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);
		Q = epoint_init();
		if(!epoint_set(Qx, Qy, 0, Q))
		{
		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
		    //A subsequent call to mirsys() will re-initialise the MIRACL system
		    //Must be called after mirsys()
		    mirexit();
			return EC_KEY_CHECK_FAIL;
		}
	}
    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
	return EC_KEY_CHECK_OK;
}

/****************************************************************************
 * Function name: ECDiffieHellman
 * Description: This is elliptic curve (cofactor) Diffie-Hellman primitive
 *              function, through this function entity U can calculate a
 *              shared secret value with V
 * Input Parameters:
 * 		u8 bECKeyType       type of the  Diffie-Hellman (EC_DH or EC_Cofactor_DH)
 * Output Parameters:		The processing result code
 *	    secret message length  		 For success
 *		EC_PROCESS_FAIL				 For fail
 * NOTE:
 * modify by chenpingdi@20221022: 比亚迪外场反馈KeyAgreement.generateSecret时，ALG_EC_SVDP_DH_PLAIN和ALG_EC_SVDP_DHC_PLAIN不应该对生成的secret进行处理，即使高字节是0，也按照原始数据返回。java api中有明确说明：This algorithm returns the raw output of the derivation primitive
 ****************************************************************************/
u8 ECDiffieHellman(u8 bECDHType)
{
	big A, B, POrBitCofficient, Qx, Qy, priD;
	epoint * Q;
	miracl * mip;
	u16 digitNum;

	u8 tmpBuff[MAX_ECC_KEY_BYTELEN];
	u8 secretLen;
	u8 tmpLen;
	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 bECKeyByteLength;
	u8 X_A * commonSecret;

	secretLen = 0;
	commonSecret = g_CryptoData.pbBuffer;
	bECKeyByteLength = g_ECKeyStore.b_PLength;
	tmpLen = g_ECKeyStore.b_RLength;

	digitNum = tmpLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	mip->IOBASE = 16;

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	Qx = mirvar(0);
	Qy = mirvar(0);
	priD = mirvar(0);

	initECParams(&ECKeyGenParams, NULL, NULL);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, A);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, B);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, POrBitCofficient);

	ecpMem_Copy(tmpBuff, (g_ECKeyStore.pb_PublicKeyQxy + 1), bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Qx); // skip type value 0x04

    ecpMem_Copy(tmpBuff, (g_ECKeyStore.pb_PublicKeyQxy + 1 + bECKeyByteLength), bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Qy);

    ecpMem_Copy(tmpBuff, g_ECKeyStore.pb_PrivateKeyS, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, priD);

	ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);
	Q = epoint_init();
	epoint_set(Qx, Qy, 0, Q);

	//decide which type of DH to use
	if(ALG_EC_SVDP_DHC == bECDHType || ALG_EC_SVDP_DHC_PLAIN == bECDHType)
	{ //the elliptic curve cofactor Diffie-Hellman primitive
		big Cofactor;
		Cofactor = mirvar(0);
		bytes_to_big(1, (const char *)&g_ECKeyStore.b_HValue, Cofactor);

		//multiply cofactor H * dU
		mad(priD, Cofactor, priD, POrBitCofficient, POrBitCofficient, priD);

		//compute the elliptic curve point p(x, y) = H * dU * Qv
		ecurve_mult(priD, Q, Q);
	}
	else
	{ //EC_DH(the elliptic curve Diffie-Hellman primitive)
		//compute the elliptic curve point p(x, y) = dU * Qv
		ecurve_mult(priD, Q, Q);
	}

	//check the result (ECKEYGEN_BASE_Qx, ECKEYGEN_BASE_Qy) is not the infinity(not zero)
	//modify 07/20/09, use the fact: G + O = G
	if(point_at_infinity(Q))
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_PROCESS_FAIL;
	}

	//output the shared secret element z = ECKEYGEN_BASE_TempQx
	epoint_get(Q, Qx, Qy);
	big_to_bytes(bECKeyByteLength, Qx, (char *)commonSecret, TRUE);

	if(ALG_EC_SVDP_DHC == bECDHType || ALG_EC_SVDP_DH == bECDHType)
	{
		//if nessary, we can delete zeros in the front using the code below
//		secretLen = getRealLength(g_CryptoData.pbBuffer, bECKeyByteLength);	 //modify by chenpingdi@20221022: 未找到规范说明，但是验证了另外8家卡商的结果，都是未去掉0时计算SHA。测试用例：	ECC_generateSecret_01
		//compute SHA-1 value using the result from above
		HASHDoFinal((PM8)commonSecret, bECKeyByteLength, commonSecret, HASH_INIT, NULL, NULL, ALG_SHA_1);
		secretLen = SHA1_DIGEST_SIZE;
	}

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
	return EC_PROCESS_OK;
}

/**********************************************************************
 * Function name: ecpUniversalAMulPointXY()
 * Qxy = [A] * Pxy:
 * APoint must be 32 bytes
 **********************************************************************/
u8 ecpUniversalAMulPointXY(PM8 APoint, PM8 PointPXY, PX8 PointQXY, PX8 ECDomainParams)
{
	big A, B, POrBitCofficient;
	big aPoint, Gx, Gy;
	big rPoint;
	epoint * G;
	miracl * mip;
	u8 bECKeyByteLength = g_ECKeyStore.b_PLength;
	u8 		tempBuff[bECKeyByteLength];

	u16 digitNum = bECKeyByteLength << 1; // G_order may be longer than bECKeyByteLength
	mip = mirsys(digitNum, 16); // use hex internally

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	aPoint = mirvar(0);
	Gx = mirvar(0);
	Gy = mirvar(0);
	rPoint = mirvar(0);

	mip->IOBASE = 16;

    ecpMem_Copy(tempBuff, ECDomainParams, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, POrBitCofficient);

	ecpMem_Copy(tempBuff, ECDomainParams + bECKeyByteLength, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, A);

	ecpMem_Copy(tempBuff, ECDomainParams + bECKeyByteLength*2, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, B);

	ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);  /* Use PROJECTIVE if possible, else AFFINE coordinates */

	// set point Pxy
	G = epoint_init();
    ecpMem_Copy(tempBuff, PointPXY, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, Gx);

    ecpMem_Copy(tempBuff, (PointPXY + bECKeyByteLength), bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, Gy);
	if(TRUE != epoint_set(Gx, Gy, 0, G))
	{
		mirexit();
		return POINT_NOT_IN_ELL;
	}

	// set point A
    ecpMem_Copy(tempBuff, APoint, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, aPoint);

	// cal Qxy = A * Pxy
	ecurve_mult(aPoint, G, G);

	// get point QXY
	epoint_get(G, Gx, Gy);

    big_to_bytes(bECKeyByteLength, Gx, tempBuff, true);
    ecpMem_Copy(PointQXY, tempBuff, bECKeyByteLength);

    big_to_bytes(bECKeyByteLength, Gy, tempBuff, true);
    ecpMem_Copy(PointQXY + bECKeyByteLength, tempBuff, bECKeyByteLength);

    //check the result (ECKEYGEN_BASE_Gx, ECKEYGEN_BASE_Gy) is not the infinity(not zero)
    	//use the fact: G + O = G
    if(point_at_infinity(G))
	{
		mirexit();
		return POINT_NOT_IN_ELL;
	}

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
	return CAL_SUCCESSED;
}

/****************************************************************************
 * Function name: PublicKeyComputeYFromX
 * Description:
 * Input Parameters:
 *
 * Output Parameters:		The processing result code
 *	    EC_KEY_CHECK_OK		  		For success
 *		EC_KEY_CHECK_FAIL			For fail
 * NOTE:
 ****************************************************************************/
boolean PublicKeyComputeYFromX(PX8 xPointPtr, PX8 yPointPtr, PX8 ECDomainParams)
{
	big A, B, POrBitCofficient, Qx, Qy, G_order;
	epoint * Q;
	miracl * mip;
	u16 digitNum;

	u8 tempBuff[MAX_ECC_KEY_BYTELEN];
//	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 tmpLen;
	u8 bECKeyByteLength;

	bECKeyByteLength = g_ECKeyStore.b_PLength;
	tmpLen = g_ECKeyStore.b_RLength;

	digitNum = tmpLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_007();

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	Qx = mirvar(0);
	Qy = mirvar(0);
	G_order = mirvar(0);

	memRefresh();

//	initECParams(&ECKeyGenParams, NULL, NULL);

//	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
//	bytes_to_big(bECKeyByteLength, tmpBuff, A);
//
//	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
//	bytes_to_big(bECKeyByteLength, tmpBuff, B);
//
//	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
//	bytes_to_big(bECKeyByteLength, tmpBuff, POrBitCofficient);
//
//    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.n.pPtr, tmpLen);
//    bytes_to_big(tmpLen, tmpBuff, G_order);

	ecpMem_Copy(tempBuff, ECDomainParams, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, POrBitCofficient);

	ecpMem_Copy(tempBuff, ECDomainParams + bECKeyByteLength, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, A);

	ecpMem_Copy(tempBuff, ECDomainParams + bECKeyByteLength*2, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, B);

	ecpMem_Copy(tempBuff, xPointPtr, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, tempBuff, Qx); // skip type value 0x04

	if(g_ECKeyStore.b_PLength == 0)
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return EC_KEY_CHECK_FAIL;
	}
	else
	{
		// the public key is smaller than P
		if(mr_compare(Qx, POrBitCofficient) >= 0)
		{
		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
		    //A subsequent call to mirsys() will re-initialise the MIRACL system
		    //Must be called after mirsys()
		    mirexit();
			return EC_KEY_CHECK_FAIL;
		}

		// check the public point whether is on the elliptic curve
		ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);
		Q = epoint_init();
		if(!epoint_set(Qx, Qx, 1, Q))
		{
		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
		    //A subsequent call to mirsys() will re-initialise the MIRACL system
		    //Must be called after mirsys()
		    mirexit();
			return EC_KEY_CHECK_FAIL;
		}

		//output the shared yPointPtr element z = ECKEYGEN_BASE_TempQx
		epoint_get(Q, Qx, Qy);
		big_to_bytes(bECKeyByteLength, Qy, yPointPtr, TRUE);

		if(mr_compare(Qy, POrBitCofficient) >= 0)
		{
			//Cleans up after the current instance of MIRACL, and frees all internal variables.
			//A subsequent call to mirsys() will re-initialise the MIRACL system
			//Must be called after mirsys()
			mirexit();
			return EC_KEY_CHECK_FAIL;
		}

	}
    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();
	return EC_KEY_CHECK_OK;
}


/**********************************************************************
 * Function name: ECPointAdd()
 * ECPointC = ECPointA + ECPointB
 **********************************************************************/
u8 ECPointAdd(PM8 ECPointA, PM8 ECPointB, PM8 ECPointC)
{
	big A, B, POrBitCofficient;
	big mPointx, mPointy, nPointx, nPointy;
	epoint * mPointXy;
	epoint * nPointXy;
	miracl * mip;
	u8 		tempBuff[MAX_ECC_KEY_BYTELEN];
	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 bECKeyByteLength;

	u16 digitNum = SM2_DOMAIN_NLEN << 1; // G_order may be longer than bECKeyByteLength
	mip = mirsys(digitNum, 16); // use hex internally

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	mPointx = mirvar(0);
	mPointy = mirvar(0);
	nPointx = mirvar(0);
	nPointy = mirvar(0);

	mip->IOBASE = 16;

	initECParams(&ECKeyGenParams, NULL, NULL);
	bECKeyByteLength = g_ECKeyStore.b_PLength;

    ecpMem_Copy(tempBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, A);
    ecpMem_Copy(tempBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, B);
    ecpMem_Copy(tempBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, POrBitCofficient);

	ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);  /* Use PROJECTIVE if possible, else AFFINE coordinates */

	// set point Mxy
	mPointXy = epoint_init();
    ecpMem_Copy(tempBuff, ECPointA, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, mPointx);

    ecpMem_Copy(tempBuff, (ECPointA + bECKeyByteLength), bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, mPointy);
	if(TRUE != epoint_set(mPointx, mPointy, 0, mPointXy))
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return POINT_NOT_IN_ELL;
	}
//	epoint_set(mPointx, mPointy, 0, mPointXy);

	// set point Nxy
	nPointXy = epoint_init();
    ecpMem_Copy(tempBuff, ECPointB, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, nPointx);

    ecpMem_Copy(tempBuff, (ECPointB + bECKeyByteLength), bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, tempBuff, nPointy);
	if(TRUE != epoint_set(nPointx, nPointy, 0, nPointXy))
	{
	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
	    //A subsequent call to mirsys() will re-initialise the MIRACL system
	    //Must be called after mirsys()
	    mirexit();
		return POINT_NOT_IN_ELL;
	}

	// Mxy = Mxy + Nxy
	ecurve_add(nPointXy, mPointXy);

	// get point QXY
	epoint_get(mPointXy, mPointx, mPointy);
	big_to_bytes(bECKeyByteLength, mPointx, tempBuff, true);
	ecpMem_Copy(ECPointC, tempBuff, bECKeyByteLength);

	big_to_bytes(bECKeyByteLength, mPointy, tempBuff, true);
	ecpMem_Copy(ECPointC + bECKeyByteLength, tempBuff, bECKeyByteLength);

    //Cleans up after the current instance of MIRACL, and frees all internal variables.
    //A subsequent call to mirsys() will re-initialise the MIRACL system
    //Must be called after mirsys()
    mirexit();

	return CAL_SUCCESSED;
}

/**********************************************************************
 * Function name: ECPointAdd()
 * ECPointC = ECPointA - ECPointB
 **********************************************************************/
u8 ECPointSub(PM8 ECPointA, PM8 ECPointB, PM8 ECPointC)
{
	u8 	 tECPoint[MAX_ECC_KEY_BYTELEN + MAX_ECC_KEY_BYTELEN + 4];
	PX8  tECPointb;

	ecpMem_Copy(tECPoint, ECPointB, (g_ECKeyStore.b_PLength << 1) );
	tECPointb = PX8_A_U16(tECPoint, g_ECKeyStore.b_PLength);
	ecpASubBModC(g_ECKeyStore.pb_domainParams, g_ECKeyStore.b_PLength, tECPointb, g_ECKeyStore.pb_domainParams, tECPointb);
	return ECPointAdd(ECPointA, tECPoint, ECPointC);
}

/*
*description: KAT(密钥协商算法),ECKA-EG
*
*@@See      : 参考GP附录E,与EUICC规范，使用ECKA-EG协议计算密钥("EllipticCurveCryptographyVersion.pdf" Section_4.3.1)
*
*@Prerequisite:	ECCPara里的参数都初始化，并且所有参数是同一曲线类型的参数
*
*@function    :calcSHS
*
*@param       :u8 * publicKey
*@param       :u8 publicKeylen
*@param       :u8 keyRef
*@param       :u8 * privateKey
*@param       :u8 privateKeyLen   //该参数暂时未使用到
*@param       :u8 * shsBuf   :由于目前仅支持ECC 256 ，所以shsBuf 至少为128字节长
*
*@return      :shsLen
*/
u8 calcSHS(u8 *publicKey, u8 publicKeyLen, u8 keyRef, u8 *privateKey, u8 privateKeyLen, u8 *shsBuf)
{
	u8 pkValue[2*MAX_ECC_KEY_BYTELEN+1];
	big A, B, POrBitCofficient, Qx, Qy, priD;
	epoint * Q;
	miracl * mip;
	u16 digitNum;

	u8 tmpBuff[MAX_ECC_KEY_BYTELEN];
	u8 tmpLen;
	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
	u8 bECKeyByteLength;
	ecpEccGfp_DomainParam_t X_A ECDomainParams;
	u8	* hPoint;

	if(FALSE == setECDomainParams(keyRef)) //初始化ECC参数
	{
		return FALSE;
	}

	pkValue[0] = 0x04; //添加04
	ecpMem_Copy(&pkValue[1], publicKey, publicKeyLen);

	setECPubOrPrivKeyRef(TYPE_EC_FP_PUBLIC, NULL, pkValue); //赋值ECC公钥

	setECPubOrPrivKeyRef(TYPE_EC_FP_PRIVATE, privateKey, NULL); //赋值ECC私钥

	bECKeyByteLength = g_ECKeyStore.b_PLength;
	tmpLen = g_ECKeyStore.b_RLength;

	digitNum = tmpLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	mip->IOBASE = 16;

	A = mirvar(0);
	B = mirvar(0);
	POrBitCofficient = mirvar(0);
	Qx = mirvar(0);
	Qy = mirvar(0);
	priD = mirvar(0);

//	initECParams(&ECKeyGenParams, NULL, NULL);
	// Initialize EC domain parameter
	ECDomainParams.p.pPtr = g_ECKeyStore.pb_domainParams;
	ECDomainParams.p.len = g_ECKeyStore.b_PLength;
	ECDomainParams.a = ECDomainParams.p.pPtr + ECDomainParams.p.len;
	ECDomainParams.b = ECDomainParams.a + ECDomainParams.p.len;
	ECDomainParams.G.x = ECDomainParams.b + ECDomainParams.p.len + 1;
	ECDomainParams.G.y = ECDomainParams.G.x + ECDomainParams.p.len;
	ECDomainParams.n.pPtr = ECDomainParams.G.y + ECDomainParams.p.len;
	ECDomainParams.n.len = g_ECKeyStore.b_RLength;
	hPoint				= ECDomainParams.n.pPtr + ECDomainParams.n.len;

	// Initialize calling structure for EC Key Generation
	ECKeyGenParams.domain = ECDomainParams;

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, A);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, B);

	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
	bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, POrBitCofficient);

	ecpMem_Copy(tmpBuff, (g_ECKeyStore.pb_PublicKeyQxy + 1), bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Qx); // skip type value 0x04

    ecpMem_Copy(tmpBuff, (g_ECKeyStore.pb_PublicKeyQxy + 1 + bECKeyByteLength), bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, Qy);

    ecpMem_Copy(tmpBuff, g_ECKeyStore.pb_PrivateKeyS, bECKeyByteLength);
    bytes_to_big(bECKeyByteLength, (const char *)tmpBuff, priD);

	//L = h^-1 Mod n -->  L = h^-1 (* 1) Mod n
	ecpMem_Set(tmpBuff, 0, bECKeyByteLength);
	tmpBuff[bECKeyByteLength - 1] = 0x01;
	ecpAInvMultBModC(hPoint, bECKeyByteLength, tmpBuff, ECDomainParams.n.pPtr, (PX8)&shsBuf[0]); //L = h^-1 Mod n

	if(CAL_SUCCESSED != ecpUniversalAMulPointXY(hPoint, (g_ECKeyStore.pb_PublicKeyQxy + 1), tmpBuff, ECDomainParams.p.pPtr)) //compute the elliptic curve point p(x, y) = H * Qv
	{
		return FALSE;
	}

	ecpAMulBModC((PX8)g_ECKeyStore.pb_PrivateKeyS, bECKeyByteLength, (PX8)&shsBuf[0], ECDomainParams.n.pPtr, (PX8)&shsBuf[0]); //t = (d*L)Mod n

	if(CAL_SUCCESSED != ecpUniversalAMulPointXY(&shsBuf[0], tmpBuff, tmpBuff, ECDomainParams.p.pPtr)) //compute the elliptic curve point p(x, y) = t * Qv
	{
		return FALSE;
	}

	//output the shared secret element z = ECKEYGEN_BASE_TempQx
	ecpMem_Copy(&shsBuf[0], tmpBuff, 2 * bECKeyByteLength);

	return  2 * bECKeyByteLength;
}

#if UICC__AUTH__5G__SUPI

 void bigsmallendianswitch(u8 *big,u8 length,u8 *small)
 {
	 u8 i;

	 for (i=0;i<length;i++)
	 {
		 small[i]=big[length-i-1];
	 }
 }
u8 CONST CURVES_25519_DOMAINPARAMS[] = {
/* p */	0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7,
/* a */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x6D, 0x06,
/* b */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
/* Gx*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
/* Gy*/	0x20, 0xAE, 0x19, 0xA1, 0xB8, 0xA0, 0x86, 0xB4, 0xE0, 0x1E, 0xDD, 0x2C,
		0x77, 0x48, 0xD1, 0x4C, 0x92, 0x3D, 0x4D, 0x7E, 0x6D, 0x7C, 0x61, 0xB2,
		0x29, 0xE9, 0xC5, 0xA2, 0x7E, 0xCE, 0xD3, 0xD9,

/* n */	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x14, 0xDE, 0xF9, 0xDE, 0xA2, 0xF7, 0x9C, 0xD6,
		0x58, 0x12, 0x63, 0x1A, 0x5C, 0xF5, 0xD3, 0xED
};
#define CURVES_25519_BYTE_LENGTH 32
void HED_X25519 (unsigned char *shared_secret, const unsigned char *private_key, const unsigned char *public_key)
{
	;
}

void ECCKeyPair_ProfileA(u8 * pkBuf_smallendian, u8 * skBuf_smallendian)
{
//	u8 skBuf_bigendian[32];
//	u8 pkBuf_bigendian[32];
//	Random_number_generator(skBuf_bigendian, 32,1);
//
////	SYSCLKCFG ^= (1<<12) ;                      /* APB div = 1 */
////   SYSCLKCFG |= CPU_36MHZ|PKE_72MHZ;
//	HED_X25519(pkBuf_bigendian,skBuf_bigendian,NULL);
//
//	bigsmallendianswitch(skBuf_bigendian,32,skBuf_smallendian);
//	bigsmallendianswitch(pkBuf_bigendian,32,pkBuf_smallendian);
//	return;

	Random_number_generator(skBuf_smallendian, 32,1);

	cf_curve25519_mul_base(pkBuf_smallendian,skBuf_smallendian);

	return;
}
/*
*@description: 获取sharekey
*
*@param      : mode: profile_a/profile_b
							 HNPublicKey: Home Network Public Key 
							 USIMSKey: ECCKeyPair输出的私钥Key
*							 shareKey:输出的sharekey,这里的ShareKey是32字节。是运算结果的x坐标（sec1_v2 3.3.1）
*
*@returns   :void
*
*@history   :create by yanjr@20190123
*           :modify by xuxiaojun@20211014 同步5G SimCard\5GNFC_Control\CMCC\scr\hardapi\CIU98M25\ECCf5G.c代码
*/
void ECCKeyAgreet_ProfileA(u8 * HNPublicKey_smallendian, u8 * USIMSKey_smallendian, u8 * shareKey_smallendian)
{
//	u8 HNPublicKey_bigendian[32];
//	u8 USIMSKey_bigendian[32];
//	u8 shareKey_bigendian[32];
//
//	bigsmallendianswitch(HNPublicKey_smallendian,32,HNPublicKey_bigendian);
//	bigsmallendianswitch(USIMSKey_smallendian,32,USIMSKey_bigendian);
//	HED_X25519(shareKey_bigendian,USIMSKey_bigendian,HNPublicKey_bigendian);
//	bigsmallendianswitch(shareKey_bigendian,32,shareKey_smallendian);

	cf_curve25519_mul(shareKey_smallendian, USIMSKey_smallendian, HNPublicKey_smallendian);

	return;
}

//暂定
u8 CONST HedEcc256Para[]={
	/*p*/	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	/*a*/	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,
	/*b*/	0x5A, 0xC6, 0x35, 0xD8, 0xAA, 0x3A, 0x93, 0xE7, 0xB3, 0xEB, 0xBD, 0x55,
			0x76, 0x98, 0x86, 0xBC, 0x65, 0x1D, 0x06, 0xB0, 0xCC, 0x53, 0xB0, 0xF6,
			0x3B, 0xCE, 0x3C, 0x3E, 0x27, 0xD2, 0x60, 0x4B,
	/*g*/	0x04, 0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8, 0xBC, 0xE6,
			0xE5, 0x63, 0xA4, 0x40, 0xF2, 0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33,
			0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96, 0x4F, 0xE3, 0x42,
			0xE2, 0xFE, 0x1A, 0x7F, 0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E,
			0x16, 0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE, 0xCB, 0xB6, 0x40,
			0x68, 0x37, 0xBF, 0x51, 0xF5,
	/*r*/	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E,
			0x84, 0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
};
void ECCPointDecompress(PX8 HNPublicKey)
{
	setECDomainParams(ECCCure_P_256_KeyParaRef); //初始化ECC参数

	PublicKeyComputeYFromX(&HNPublicKey[1], &HNPublicKey[33], g_ECKeyStore.pb_domainParams);

	HNPublicKey[0] = 0x04;
}


//unsigned int HED_ECCGenKey(ECC_Parameters *ECC_Param)
//{
//	return 0;
//}

void ECCKeyPair_ProfileB(u8 * pkBuf, u8 * skBuf)
{
//	unsigned int sk[8];  //sk[8]
//	unsigned int pk[16]; //pk[16]
//
//	ECC_Parameters  ECC_Param;
//	ECC_Param.argPtr=HedEcc256Para;
//	ECC_Param.priKeyPtr=sk;
//	ECC_Param.pubKeyPtr=pk;
//	HED_ECCGenKey(&ECC_Param);
//	ecpMem_Copy(pkBuf,(u8 *)pk,64);
//	ecpMem_Copy(skBuf,(u8 *)sk,32);
//	return;

	u8 pkdata[65];

	GenECCKeyPair(pkdata, skBuf, ECCCure_P_256_KeyParaRef);

	ecpMem_Copy(pkBuf, &pkdata[1], 64); //不保存04
}

//unsigned int HED_ECKA_BSI(ECKA_Parameters *ECC_Param )
//{
//	return 0;
//}
/*
*@description: 获取sharekey
*
*@param      : mode: profile_a/profile_b
							 HNPublicKey: Home Network Public Key 
							 USIMSKey: ECCKeyPair输出的私钥Key
*							 shareKey:输出的sharekey,这里的ShareKey是32字节。是运算结果的x坐标（sec1_v2 3.3.1）
*
*@returns   :void
*
*@history   :create by yanjr@20190123
*           :modify by xuxiaojun@20211014 同步5G SimCard\5GNFC_Control\CMCC\scr\hardapi\CIU98M25\ECCf5G.c代码
*/
//注意四字节对齐,和ECCKeyPair_ProfileB统一
void ECCKeyAgreet_ProfileB(u8 * HNPublicKey, u8 * USIMSKey, u8 * shareKey)
{
//	unsigned int st;
//	unsigned int usimsk[8];
//	unsigned int hnpk[16];
//
//	u8 share[64];
//	ECKA_Parameters eccPara;
//
//	ecpMem_Copy(usimsk,USIMSKey,32);
//	ecpMem_Copy(hnpk,HNPublicKey,64);
//
//	eccPara.argPtr=HedEcc256Para;
//	eccPara.priKeyPtr=usimsk;
//	eccPara.pubKeyPtr=hnpk;
//	eccPara.oDataPtr=share;
//
//	st = HED_ECKA_BSI(&eccPara);
//
//    if ( (st != 0))
//    {
//		while(1);
//
//    }
//
//    ecpMem_Copy(shareKey,share,32);
//	return;

	calcSHS(&HNPublicKey[0], 64, ECCCure_P_256_KeyParaRef, &USIMSKey[0], 32, &shareKey[0]);
}

//sec1-v2.pdf 3.6.1 ANS X9.63 Key Derivation Function
//modify by xuxiaojun@20211014 将该函数从hard_api.c中移动到这里，便于代码分层
u8 ANS_X963_KDF_Hal(u8* Z,u8 ZLen, u8 keydatalen,u8* shareInfo, u8 shareInfoLen, u8* K)
{	
	#define		SHA256_DIGEST_SHIFT_BIT		0x05
	#define 	EC_256_PUBLICKEYLEN			(ALG_EC_FP_KEY_256 * 2)
	#define 	EC_256_DOMAIN_NLEN			ALG_EC_FP_KEY_256
	#define 	EC_256_DOMAIN_NLEN_MASK		0x1F

	//the u8 type value 'BlockNum' limit the max message length not longer than 8160 bytes.
	u8 		X_A 			tempbuff[SHA256_DIGEST_SIZE];
	u8 		X_A 			HASHmdBuff[SHA256_ICV_SIZE];
	u8 		X_A 			HASHtmpBuff[SHA256_BLOCK_SIZE];
	u8 		X_A 			tempValue[4];
	u8 		D_A 			BlockNum;
	u8 		D_A 			remainlen;
	u32 		D_A 		temp;

	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_014();
	BlockNum 	= U16t8(keydatalen>> SHA256_DIGEST_SHIFT_BIT); // klen/v
	remainlen 	= keydatalen & EC_256_DOMAIN_NLEN_MASK;

	temp					= 0;
	while (BlockNum--)
	{
		U32_INC(temp);
		PXB_W_U32(tempValue, temp);
		if(shareInfoLen == 0)
		{
			HASHUpdate(Z, ZLen, HASH_INIT, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
			HASHDoFinal(tempValue, 4, K, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
		}
		else
		{
			HASHUpdate(Z, ZLen, HASH_INIT, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
			HASHUpdate(tempValue, 4, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
			HASHDoFinal(shareInfo, shareInfoLen, K, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
		}
		K = PX8_A_U8(K, SHA256_DIGEST_SIZE);
	}
	if (remainlen)
	{
		U32_INC(temp);
		PXB_W_U32(tempValue, temp);
		if(shareInfoLen == 0)
		{
			HASHUpdate(Z, ZLen, HASH_INIT, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
			HASHDoFinal(tempValue, 4, tempbuff, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
		}
		else
		{
			HASHUpdate(Z, ZLen, HASH_INIT, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
			HASHUpdate(tempValue, 4, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
			HASHDoFinal(shareInfo, shareInfoLen, tempbuff, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_256);
		}
		ecpMem_Copy(K, tempbuff, remainlen);
	}


	return 0;
}
/**
*modify by xuxiaojun@20211014 将该函数从hard_api.c中移动到这里，便于代码分层; 同步5G SimCard\5GNFC_Control\CMCC\scr\hardapi\CIU98M25\ECCf5G.c代码
*/
void AESEncrypt_Hal(u8* Key, u8* aIn,u8* aOut)
{
	SymCipherDofinal(aIn, 16, aOut, Key, LENGTH_AES_128, NULL, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, (MODE_ENCRYPT  | SYM_ECB_MODE), SBC_INIT, ALG_AES);
}
#endif //UICC__AUTH__5G__SUPI

#if WITH_SM2
/*********************************************************************
 * Function name: SM2_ECP_Getw()
 *
 * w = log2 inv CPoint / 2 - 1
 *********************************************************************/
u8 SM2_ECP_Getw(PX8 CPoint, u16 byteLen)
{
	big cPoint;
	u32 w;
	miracl *mip;
	u16 digitNum;

	digitNum = byteLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	cPoint = mirvar(0);

	mip->IOBASE = 16;

	bytes_to_big(byteLen, CPoint, cPoint);

	w = logb2(cPoint);

	w = (w/2)*2 + ((w%2 == 0)?0:1); //top integer of w

	w = (w/2) + ((w%2 == 0)?0:1); //top integer of w

	w -= 1;

	//Cleans up after the current instance of MIRACL, and frees all internal variables.
	//A subsequent call to mirsys() will re-initialise the MIRACL system
	//Must be called after mirsys()
	mirexit();

	return w;
}
/*********************************************************************
 * Function name: SM2_ECP_TransX_toInteger()
 *
 * ponitX1 = 2^w + (pointX & (2^w ?1))
 *********************************************************************/
u32 SM2_ECP_TransX_toX1(u32 w, PX8 pointX, u16 byteLen, PX8 pointX1)
{
	big pointx, pointx1, bigE;
	big tmp;
	u16 index;
	u32 num;

	u8 eValue[1];

	eValue[0] = 1;

	miracl *mip;
	u16 digitNum;

	digitNum = byteLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	pointx = mirvar(0);
	pointx1 = mirvar(0);
	bigE = mirvar(0);

	mip->IOBASE = 16;
	bytes_to_big((u32)byteLen, pointX, pointx);
	bytes_to_big(sizeof(eValue), eValue, bigE);

	expb2(w, tmp);// tmp = 2^w

	subtract(tmp, bigE, pointx1); //pointx1 =  2^w -1

	big_to_bytes(byteLen, pointx1, pointX1, true);

	big_to_bytes(byteLen, pointx, pointX, true);

	for(index = 0; index < byteLen; index++)
	{
		pointX1[index] &= pointX[index];
	}

	bytes_to_big((u32)byteLen, pointX1, pointx1);

	add(tmp,pointx1,pointx1); // pointx1 = tmp + pointx1

	num = big_to_bytes(byteLen, pointx1, pointX1, true);

    mirexit();

    return num;

}
/*********************************************************************
 * Function name: ecpCalT()
 * Six big numbers x,y,z,w,q and r. On exit q=(x.y+z)/w and r contains the
 * remainder. If w and q are not distinct variables then only the remainder
 * is returned; if q and r are not distinct then only the quotient is returned.
 * The addition of z is not done if x and z (or y and z) are the same
 *********************************************************************/
void SM2_ECP_GetT(PX8 X, PX8 Y, PX8 Z, PX8 W, PX8 Q, PX8 tR, u16 byteLen)
{
	big bigX, bigY, bigZ, bigW, bigQ, bigR;
	miracl *mip;
	u16 digitNum;
	u8 		tempBuf[SM2_DOMAIN_NLEN];

	if(byteLen > SM2_DOMAIN_NLEN)
	{
		return;
	}

	digitNum = byteLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	bigX = mirvar(0);
	bigY = mirvar(0);
	bigZ = mirvar(0);
	bigW = mirvar(0);
	bigQ = mirvar(0);
	bigR = mirvar(0);

	mip->IOBASE = 16;
	ecpMem_Copy(tempBuf, X, byteLen);
	bytes_to_big(byteLen, tempBuf, bigX);

	ecpMem_Copy(tempBuf, Y, byteLen);
	bytes_to_big(byteLen, tempBuf, bigY);

	ecpMem_Copy(tempBuf, Z, byteLen);
	bytes_to_big(byteLen, tempBuf, bigZ);

	ecpMem_Copy(tempBuf, W, byteLen);
	bytes_to_big(byteLen, tempBuf, bigW);

	mad(bigX,bigY,bigZ,bigW,bigQ,bigR); // q=(x.y+z)/w and r contains the remainder

	big_to_bytes(byteLen, bigR, tempBuf, true);
	ecpMem_Copy(tR, tempBuf, byteLen);

	mirexit();
	return;

}
// [h*x](PointXY)
void SM2_ECP_GetU(PX8 h, PX8 X, PX8 pointXY, u16 byteLen)
{
	big bigH;
	big bigX;
	big bigZ;
	big bigW;
	big bigR;
	big bigQ;
	miracl *mip;
	u16 digitNum;
	u8  X_A     z[1];
	u8  X_A     w[1];
	u8  X_A     q[byteLen];

	z[0] = 0;
	w[0] = 1;

	digitNum = byteLen << 1;
	mip = mirsys(digitNum, 16); // use hex internally

	bigX 				= mirvar(0);
	bigH 				= mirvar(0);
	bigZ                = mirvar(0);
	bigW                = mirvar(0);
	bigQ                = mirvar(0);
	bigR                = mirvar(0);

	mip->IOBASE = 16;
	bytes_to_big(byteLen, h, bigH);
	bytes_to_big(byteLen, X, bigX);
	bytes_to_big(byteLen, z, bigZ);
	bytes_to_big(byteLen, w, bigW);

	mad(bigX,bigH,bigZ,bigW,bigQ,bigR); // q=(x.y+z)/w and r contains the remainder

	big_to_bytes(SM2_DOMAIN_NLEN, bigQ, q, true);

	mirexit();

	ecpAMulPointXY(q, pointXY, pointXY);

	return;
}
/*
*@function: GMSM2KeyExchange
*/
typedef struct
{
    const unsigned char *AID;
    const unsigned char *BID;
    const unsigned int  *PublicKey;
    const unsigned int  *PrivateKey;
    const unsigned int  *BPublicKey;
    const unsigned int  *TempPrivateKey;
    const unsigned int  *TempPubKeyX;
    const unsigned int  *BTempPubKey;
    unsigned int         oLen;
    unsigned int         DataType;
    unsigned int         iEccSel;
} SM2KeyExch_Parameters;


void SM2_KeyExchange(PM8 Pa,PM8 Pb,PM8 tmpPubOfA,PM8 tmpPubOfB,PM8 priOfA,PM8 tmpPriOfA,PM8 IDa,PM8 IDb,u8 IdBytesLen_A,u8 IdBytesLen_B,PM8 outBuffer,u8 exchangeKeyLen,u8 bMode)
{
	u8 	    X_A	    nBuf[SM2_DOMAIN_NLEN];
	u8      X_A     tmpPubOfB_x[SM2_ELEN];
	u8      X_A     tmpPubOfA_x[SM2_ELEN];
	u8      X_A     qBuf[SM2_ELEN];
	u8      X_A     ta[SM2_ELEN];
	PX8		X_A		tempZa;
	PX8		X_A		tempZb;
	PX8     X_A     transX1;
	PX8     X_A     transX2;
	PX8     X_A     tmp;
	PX8     X_A     tmp2;
	PX8	    zBuffer;
	u32     D_A w;
	u32     num;

	ecpMem_Copy(nBuf, SM2_DOMAINPARAMS_POINT_N, SM2_DOMAIN_NLEN);

	ecpMem_Copy(tmpPubOfB_x, tmpPubOfB, SM2_ELEN);
	ecpMem_Copy(tmpPubOfA_x, tmpPubOfA, SM2_ELEN);

	//ZA=H256(ENTLA∥IDA∥a∥b∥xG∥yG∥xA∥yA)
	tempZa = MEMtPX8(MemGetReuseRam_GetTempArrBuffer(TEMP_ARR_BUF_SIZE));// 0x118: tempZa(32) + tempZb(32) + transX1(32) + transX2(32) + tmp(32) + tmp2(64)
	SM2_ECP_GetZa(IDa, IdBytesLen_A, Pa, tempZa);

	//ZB=H256(ENTLB∥IDB∥a∥b∥xG∥yG∥xB∥yB)
	tempZb = PX8_A_U8(tempZa, 32); // SM3_DIGEST_SIZE
	SM2_ECP_GetZa(IDb, IdBytesLen_B, Pb, tempZb);

	w = SM2_ECP_Getw(nBuf, SM2_DOMAIN_NLEN);

	//x1' = 2^w + (x1 & (2^w  - 1))
	transX1 = PX8_A_U8(tempZb, SM3_DIGEST_SIZE);
	num = SM2_ECP_TransX_toX1(w, tmpPubOfA_x, SM2_ELEN, transX1); // tmpPubOfA_x is X1'

	//x2' = 2^w + (x2 & (2^w  - 1))
	transX2 = PX8_A_U8(transX1, num);
	num = SM2_ECP_TransX_toX1(w, tmpPubOfB_x, SM2_ELEN, transX2); // tmpPubOfB_x is X2'

	//[x2']Rb
	tmp = PX8_A_U8(transX2, num);
	ecpAMulPointXY(transX2, tmpPubOfB, tmp); // tmp must be 32 bytes

	//Pb + [x2']Rb
	tmp2 = PX8_A_U8(tmp, 32); //tmp2 64 bytes
	ecpPointAddPoint(Pb, tmp, tmp2);

	//ta = (da + x1' * ra) mod n
	SM2_ECP_GetT(transX1, tmpPriOfA, priOfA, nBuf, qBuf, ta, SM2_DOMAIN_NLEN);

	//[h*ta](Pb + [x2']Rb)
	//h[0] = 1;
	//SM2_ECP_GetU(h, ta, Pb, SM2_ELEN);
	ecpAMulPointXY(ta, tmp2, tmp2);

	// KA = KDF(xU∥yU∥ZA∥ZB;klen)
	zBuffer = transX1; //复用
	memCopyToRam(zBuffer, tmp2, SM2_PUBLICKEYLEN);
	if(bMode == GMSM2KeyExchange_SEND_MODE)
	{
		memCopyToRam(zBuffer + SM2_PUBLICKEYLEN, tempZa, 32);
		memCopyToRam(zBuffer + SM2_PUBLICKEYLEN + 32, tempZb, 32);
	}
	else
	{
		memCopyToRam(zBuffer + SM2_PUBLICKEYLEN, tempZb, 32);
		memCopyToRam(zBuffer + SM2_PUBLICKEYLEN + 32, tempZa, 32);
	}

	memFillToRam(zBuffer + SM2_PUBLICKEYLEN + 32 + 32, 0, 4);
	SM2_ECP_KDF2(zBuffer, SM2_PUBLICKEYLEN + 32 + 32 + 4, exchangeKeyLen, outBuffer);
}
#endif
#endif
