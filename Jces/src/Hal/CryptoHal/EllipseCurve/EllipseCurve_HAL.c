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

//#include "curve25519.h"
//#include "curve25519_dh.h"

#if WITH_SM2
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

#if WITH_SOFT_SM2


#define	SM2_PARAM_FP_OFFSET				0
#define	SM2_PARAM_A_OFFSET				(SM2_PARAM_FP_OFFSET + SM2_DOMAIN_NLEN)
#define	SM2_PARAM_B_OFFSET				(SM2_PARAM_A_OFFSET + SM2_DOMAIN_NLEN)
#define	SM2_PARAM_Gx_OFFSET				(SM2_PARAM_B_OFFSET + SM2_DOMAIN_NLEN)
#define	SM2_PARAM_Gy_OFFSET				(SM2_PARAM_Gx_OFFSET + SM2_DOMAIN_NLEN)
#define	SM2_PARAM_N_OFFSET				(SM2_PARAM_Gy_OFFSET + SM2_DOMAIN_NLEN)
#define	SM2_PARAM_LENGTH				(SM2_PARAM_N_OFFSET + SM2_DOMAIN_NLEN)

#define	SM2_WORK_TEMP_PRIKEY_OFFSET		0
#define	SM2_WORK_K_OFFSET				SM2_DOMAIN_NLEN
#define	SM2_WORK_S_OFFSET				(SM2_DOMAIN_NLEN * 2)
#define	SM2_WORK_Rx_OFFSET				(SM2_DOMAIN_NLEN * 3)
#define	SM2_WORK_Ry_OFFSET				(SM2_DOMAIN_NLEN * 4)
#define	SM2_WORK_E_OFFSET				(SM2_DOMAIN_NLEN * 5)

void initSM2Params(ECCCPARM * pEccCurve, PX8 workspaceCPU, u8 X_A * workspaceFX)
{ 	
	//assign Characterisitic
	pEccCurve->Characteristic 						= 0;
	
	//assign Modulus
	pEccCurve->Modulus.Cookie 						= ECC_CURVE_MODULUS_COOKIE;
	pEccCurve->Modulus.super_.Data 					= MEMtPX8(SM2_DOMAINPARAMS_POINT_P);
	pEccCurve->Modulus.super_.BytesAllocated 		= SM2_DOMAIN_NLEN;
	pEccCurve->Modulus.super_.BitLength				= SM2_DOMAIN_NLEN << 3;
	
	//assign CoefA
	pEccCurve->CoefA.Cookie 						= ECC_CURVE_COEF_A_COOKIE;
	pEccCurve->CoefA.super_.Data 					= MEMtPX8(SM2_DOMAINPARAMS_POINT_A);
	pEccCurve->CoefA.super_.BytesAllocated 			= SM2_DOMAIN_NLEN;
	pEccCurve->CoefA.super_.BitLength				= SM2_DOMAIN_NLEN << 3;
	
	//assign CoefB
	pEccCurve->CoefB.Cookie 						= ECC_CURVE_COEF_B_COOKIE;
	pEccCurve->CoefB.super_.Data 					= MEMtPX8(SM2_DOMAINPARAMS_POINT_B);
	pEccCurve->CoefB.super_.BytesAllocated 			= SM2_DOMAIN_NLEN;
	pEccCurve->CoefB.super_.BitLength				= SM2_DOMAIN_NLEN << 3;
	
	//assign Basepoint.X
	pEccCurve->Basepoint.X.Cookie 					= ECC_CURVE_BASEPOINT_X_COOKIE;
	pEccCurve->Basepoint.X.super_.Data 				= MEMtPX8(SM2_DOMAINPARAMS_POINT_GX);
	pEccCurve->Basepoint.X.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	pEccCurve->Basepoint.X.super_.BitLength			= SM2_DOMAIN_NLEN << 3;
	
	//assign Basepoint.Y
	pEccCurve->Basepoint.Y.Cookie 					= ECC_CURVE_BASEPOINT_Y_COOKIE;
	pEccCurve->Basepoint.Y.super_.Data 				= MEMtPX8(SM2_DOMAINPARAMS_POINT_GY);
	pEccCurve->Basepoint.Y.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	pEccCurve->Basepoint.Y.super_.BitLength			= SM2_DOMAIN_NLEN << 3;
	
	//assign BasepointOrder
	pEccCurve->BasepointOrder.Cookie 				= ECC_CURVE_BASEPOINTORDER_COOKIE;
	pEccCurve->BasepointOrder.super_.Data 			= MEMtPX8(SM2_DOMAINPARAMS_POINT_N);
	pEccCurve->BasepointOrder.super_.BytesAllocated = SM2_DOMAIN_NLEN;
	pEccCurve->BasepointOrder.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
}


/**********************************************************************
 * Function name: ecpAInvMultBModC()
 * Qxy = [A] * Pxy:
 * APoint must be 32 bytes
 **********************************************************************/
u8 ecpAMulPointXY(PM8 APoint, PM8 PointPXY, PX8 PointQXY)
{
	u8 			tmpPrivteKeyBuf[SM2_PRIVEYLEN];
	u8 			tmppublicKeyBuf[SM2_PUBLICKEYLEN];
	
	u8 			outputBuf[SM2_PUBLICKEYLEN];
	u8 			tmpBuff[GET_MIN_TEMP_MEM_ECC_DH( (SM2_PUBLICKEYLEN << 3), (SM2_PUBLICKEYLEN << 3) )];
	
	ECCCPARM  	myPEccCurve;
	AFFINEPOINT Y, Q; //public key
	TCLONG 		pk, mem;
	CLIB_STATUS 			clResult;   // clResult in order to check the API


	pk.Cookie 					= ECC_SCALAR_CLONG_COOKIE;
	pk.super_.Data 				= tmpPrivteKeyBuf;
	pk.super_.BytesAllocated 	= SM2_PRIVEYLEN;
	pk.super_.BitLength			= SM2_PRIVEYLEN << 3;
	ecpMem_Copy(MEMtPX8(pk.super_.Data), APoint, SM2_PRIVEYLEN);
	
	Y.X.Cookie 					= ECC_P_IN_X_COOKIE;
	Y.X.super_.Data 			= tmppublicKeyBuf;
	Y.X.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	Y.X.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	Y.Y.Cookie 					= ECC_P_IN_Y_COOKIE;
	Y.Y.super_.Data 			= MEMtPX8(&tmppublicKeyBuf[SM2_DOMAIN_NLEN]);
	Y.Y.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	Y.Y.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	ecpMem_Copy(MEMtPX8(Y.X.super_.Data), PointPXY, SM2_DOMAIN_NLEN);
	ecpMem_Copy(MEMtPX8(Y.Y.super_.Data), PX8_A_U16(PointPXY, SM2_DOMAIN_NLEN), SM2_DOMAIN_NLEN);

	mem.Cookie 					= TEMPMEM_CLONG_COOKIE;
	mem.super_.Data 			= tmpBuff;
	mem.super_.BytesAllocated 	= sizeof(tmpBuff);
	mem.super_.BitLength		= sizeof(tmpBuff) << 3;
	
	
	Q.X.Cookie 					= ECC_SHARED_SECRET_X_COOKIE;
	Q.X.super_.Data 			= outputBuf;
	Q.X.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	Q.X.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	Q.Y.Cookie 					= (u16)ECC_SHARED_SECRET_Y_COOKIE;
	Q.Y.super_.Data 			= MEMtPX8(&outputBuf[SM2_DOMAIN_NLEN]);
	Q.Y.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	Q.Y.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	initSM2Params(&myPEccCurve, NULL, NULL);
		/// call infineon sle97 crypto lib
	// open CryptoLibrary
	clResult = CryptoOpenLibrary();
	if (CLIB_FAILED(clResult)) // if failed, then end
	{
		while(1);
	}
	
	//??RSA????DEEPSLEEP??,?????
	UserSetHaltMode();

	
	// change to privilage mode
	setControlRegPrivMode((int)&__set_CONTROL, 2);
	
//	do
//	{
		clResult = ECC_DH( &myPEccCurve, &pk, &Y, &mem, &Q);
//	} while (clResult == CLIB_STATUS_ERROR_ECC_PROJ_ZERO_POINT_TRY_AGAIN );


	// change to unprivilage mode
	setControlRegPrivMode((int)&__set_CONTROL, 3);

//	if (CLIB_FAILED(clResult)) // if failed, then end
//	{
//		while(1);
//	}

	CryptoCloseLibrary();
	if( (clResult != CLIB_STATUS_SUCCESS_UNSPECIFIC) && (clResult != CLIB_STATUS_WARNING_CLNGBITMDF) )
	{
		return POINT_NOT_IN_ELL;
	}
	
	ecpMem_Copy(PointQXY, MEMtPX8(Q.X.super_.Data), SM2_DOMAIN_NLEN);
	ecpMem_Copy(PX8_A_U16(PointQXY, SM2_DOMAIN_NLEN), MEMtPX8(Q.Y.super_.Data), SM2_DOMAIN_NLEN);
	
	return CAL_SUCCESSED;
}



/**********************************************************************
 * Function name: ecpPointAddPoint()
 * Qxy = Mxy + Nxy
 **********************************************************************/

u8 ecpPointAddPoint(PX8 PointMxy, PM8 PointNxy, PX8 PointQXY)
{
	u8 			XPointXY[SM2_PUBLICKEYLEN];
	u8 			YPointXY[SM2_PUBLICKEYLEN];
	u8 			QPointXY[SM2_PUBLICKEYLEN];

	ECCCPARM  	myPEccCurve;
	AFFINEPOINT X, Y, Q; //public key
	CLIB_STATUS 			clResult;   // clResult in order to check the API

	X.X.Cookie 					= ECC_P_IN_X_COOKIE;
	X.X.super_.Data 			= XPointXY;
	X.X.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	X.X.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	X.Y.Cookie 					= ECC_P_IN_Y_COOKIE;
	X.Y.super_.Data 			= MEMtPX8(&XPointXY[SM2_DOMAIN_NLEN]);
	X.Y.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	X.Y.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	ecpMem_Copy(MEMtPX8(X.X.super_.Data), PointMxy, SM2_DOMAIN_NLEN);
	ecpMem_Copy(MEMtPX8(X.Y.super_.Data), PX8_A_U16(PointMxy, SM2_DOMAIN_NLEN), SM2_DOMAIN_NLEN);
	
	Y.X.Cookie 					= ECC_P_IN_X_COOKIE;
	Y.X.super_.Data 			= YPointXY;
	Y.X.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	Y.X.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	Y.Y.Cookie 					= ECC_P_IN_Y_COOKIE;
	Y.Y.super_.Data 			= MEMtPX8(&YPointXY[SM2_DOMAIN_NLEN]);
	Y.Y.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	Y.Y.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	ecpMem_Copy(MEMtPX8(Y.X.super_.Data), PointNxy, SM2_DOMAIN_NLEN);
	ecpMem_Copy(MEMtPX8(Y.Y.super_.Data), PX8_A_U16(PointNxy, SM2_DOMAIN_NLEN), SM2_DOMAIN_NLEN);
	
	Q.X.Cookie 					= ECC_P_OUT_X_COOKIE;
	Q.X.super_.Data 			= QPointXY;
	Q.X.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	Q.X.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	Q.Y.Cookie 					= (u16)ECC_P_OUT_Y_COOKIE;
	Q.Y.super_.Data 			= MEMtPX8(&QPointXY[SM2_DOMAIN_NLEN]);
	Q.Y.super_.BytesAllocated 	= SM2_DOMAIN_NLEN;
	Q.Y.super_.BitLength		= SM2_DOMAIN_NLEN << 3;
	
	initSM2Params(&myPEccCurve, NULL, NULL);
		/// call infineon sle97 crypto lib
	// open CryptoLibrary
	clResult = CryptoOpenLibrary();
	if (CLIB_FAILED(clResult)) // if failed, then end
	{
		while(1);
	}
	
	//??RSA????DEEPSLEEP??,?????
	UserSetHaltMode();

	
	// change to privilage mode
	setControlRegPrivMode((int)&__set_CONTROL, 2);
	
	do
	{
		clResult = ECC_Add( &myPEccCurve, &X, &Y, &Q);
	} while (clResult == CLIB_STATUS_ERROR_ECC_PROJ_ZERO_POINT_TRY_AGAIN );


	// change to unprivilage mode
	setControlRegPrivMode((int)&__set_CONTROL, 3);

	CryptoCloseLibrary();
	if( (clResult != CLIB_STATUS_SUCCESS_UNSPECIFIC) && (clResult != CLIB_STATUS_WARNING_CLNGBITMDF) )
	{
		return POINT_NOT_IN_ELL;
	}
	
	ecpMem_Copy(PointQXY, MEMtPX8(Q.X.super_.Data), SM2_DOMAIN_NLEN);
	ecpMem_Copy(PX8_A_U16(PointQXY, SM2_DOMAIN_NLEN), MEMtPX8(Q.Y.super_.Data), SM2_DOMAIN_NLEN);
	
	return CAL_SUCCESSED;
}
 
#endif // WITH_SOFT_SM2

#endif //WITH_SM2

 void bigsmallendianswitch(u8 *big,u8 length,u8 *small)
 {
	 u8 i;

	 for (i=0;i<length;i++)
	 {
		 small[i]=big[length-i-1];
	 }
 }


#if WITH_ECC


#define	ECC_PARAM_FP_OFFSET				0
#define	ECC_PARAM_A_OFFSET				(ECC_PARAM_FP_OFFSET + MAX_ECC_KEY_BYTELEN)
#define	ECC_PARAM_B_OFFSET				(ECC_PARAM_A_OFFSET + MAX_ECC_KEY_BYTELEN)
#define	ECC_PARAM_Gx_OFFSET				(ECC_PARAM_B_OFFSET + MAX_ECC_KEY_BYTELEN)
#define	ECC_PARAM_Gy_OFFSET				(ECC_PARAM_Gx_OFFSET + MAX_ECC_KEY_BYTELEN)
#define	ECC_PARAM_N_OFFSET				(ECC_PARAM_Gy_OFFSET + MAX_ECC_KEY_BYTELEN)
#define	ECC_PARAM_LENGTH				(ECC_PARAM_N_OFFSET + MAX_ECC_KEY_BYTELEN)

#define	ECC_WORK_TEMP_PRIKEY_OFFSET		0
#define	ECC_WORK_K_OFFSET				MAX_ECC_KEY_BYTELEN
#define	ECC_WORK_S_OFFSET				(MAX_ECC_KEY_BYTELEN * 2)
#define	ECC_WORK_Rx_OFFSET				(MAX_ECC_KEY_BYTELEN * 3)
#define	ECC_WORK_Ry_OFFSET				(MAX_ECC_KEY_BYTELEN * 4)
#define	ECC_WORK_E_OFFSET				(MAX_ECC_KEY_BYTELEN * 5)


// ************************************************************************
// Common Initialization function for ECC structures
// ************************************************************************
static void initECParams(MHECC_PARAM *myEccInitParam, u8 X_A * workSpaceParam, u8 X_A * oDataBuf)
{
	u8 tempPLength;
	InitDefaultECDomainParams();
	tempPLength	=  g_ECKeyStore.b_PLength;
	ecpMem_Copy(&workSpaceParam[ECC_PARAM_FP_OFFSET], g_ECKeyStore.pb_domainParams, tempPLength);
	ecpMem_Copy(&workSpaceParam[ECC_PARAM_A_OFFSET], g_ECKeyStore.pb_domainParams + tempPLength, tempPLength);
	ecpMem_Copy(&workSpaceParam[ECC_PARAM_B_OFFSET], g_ECKeyStore.pb_domainParams + tempPLength * 2, tempPLength);
	ecpMem_Copy(&workSpaceParam[ECC_PARAM_Gx_OFFSET], g_ECKeyStore.pb_domainParams + tempPLength * 3 + 1, tempPLength );
  ecpMem_Copy(&workSpaceParam[ECC_PARAM_Gy_OFFSET], g_ECKeyStore.pb_domainParams + tempPLength * 4 + 1, tempPLength);
//	if(tempPLength == g_ECKeyStore.b_RLength)
//	{
		ecpMem_Copy(&workSpaceParam[ECC_PARAM_N_OFFSET], g_ECKeyStore.pb_domainParams + tempPLength * 5 + 1, g_ECKeyStore.b_RLength);
//	}
//	else
//	{
//		// do something 
//	}
	
	// Initialize EC domain parameter
	myEccInitParam->pu8P = MEMtPX8(&workSpaceParam[ECC_PARAM_FP_OFFSET]);
	myEccInitParam->pu8A = MEMtPX8(&workSpaceParam[ECC_PARAM_A_OFFSET]);
	myEccInitParam->pu8B = MEMtPX8(&workSpaceParam[ECC_PARAM_B_OFFSET]);
	myEccInitParam->pu8N = MEMtPX8(&workSpaceParam[ECC_PARAM_N_OFFSET]);
	myEccInitParam->pu8Gx = MEMtPX8(&workSpaceParam[ECC_PARAM_Gx_OFFSET]);
	myEccInitParam->pu8Gy = MEMtPX8(&workSpaceParam[ECC_PARAM_Gy_OFFSET]);
	myEccInitParam->pu8Space = oDataBuf;
	myEccInitParam->u16BitLen = (u16)(tempPLength<< 3);
	
	switch(myEccInitParam->u16BitLen)
	{
		case 192:
			myEccInitParam->u32SpaceLen = MH_ECC192_SPACE_LEN;
			break;
		case 224:
			myEccInitParam->u32SpaceLen = MH_ECC224_SPACE_LEN;
			break;
		case 256:
			myEccInitParam->u32SpaceLen = MH_ECC256_SPACE_LEN;
			break;
		case 384:
			myEccInitParam->u32SpaceLen = MH_ECC384_SPACE_LEN;
			break;
		case 521:
			myEccInitParam->u32SpaceLen = MH_ECC521_SPACE_LEN;
			break;
		default:
			break;
	}
  myEccInitParam->u32Crc = MHCRC_CalcBuff(0xffff, myEccInitParam, sizeof(MHECC_PARAM)-4); 
}


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
	/**
     * Information of workSpace bytes:
     * -------------------------------------------------------------------------------------------
	 * | FP   				 	| paramA  				|paramB   				|  paramG  					|  paramN  				|
	 * | MAX_ECC_KEY_BYTELEN	| MAX_ECC_KEY_BYTELEN 	|MAX_ECC_KEY_BYTELEN 	| MAX_ECC_KEY_BYTELEN	* 2 |  MAX_ECC_KEY_BYTELEN  |
	 * -------------------------------------------------------------------------------------------
	 */
	
	u8 			X_A		workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
	
	 /**
     * Information of workSpace bytes:
     * -------------------------------------------------------------------------------------------
	 * | PrivareKey(priD) 	 	| K(Random Bytes)		|S(=1/k * (e+r * dU) mod n)	| R (= K * G)  				|  E  					|
	 * | MAX_ECC_KEY_BYTELEN	| MAX_ECC_KEY_BYTELEN 	|MAX_ECC_KEY_BYTELEN 		| MAX_ECC_KEY_BYTELEN	* 2 |  MAX_ECC_KEY_BYTELEN  |
	 * -------------------------------------------------------------------------------------------
	 */
	u8 			X_A		workSpaceTemp[(MAX_ECC_KEY_BYTELEN * 6)];
	u8 			X_A		oDataBuffer[(MAX_ECC_KEY_BYTELEN * 2)];
	u8 			tempR[MAX_ECC_KEY_BYTELEN + 4];
	u8 			tempS[MAX_ECC_KEY_BYTELEN + 4];
	u8 			argBuf[MH_ECC521_SPACE_LEN];
	
	MHECC_PARAM   myEccInitParam;
	MHECC_KEY			myEccParam;
	MHECC_SIG_VER_CALL SignCall;
	MH_ECC_INNER_BASE InBase;
	MHECC_SIGN SignRs;
	u32 	resultVal;
	uint8_t u8Base = 0;
	
	//u8  random[32] = {0x13,0x99,0x81,0x83,0x61,0x60,0x65,0x85,0xE4,0xC6,0x97,0x55,0x19,0xEF,0x43,0xD8,0xB4,0xF1,0x3B,0x62,0xBA,0x29,0x1C,0x6A,0xF3,0xC0,0xDE,0x98,0xCA,0x16,0xAE,0xD7};

	u8 			X_A		tmpLen;
	u8 			X_A		bECKeyByteLength;
	u8 			X_A		Rlength;
	u8 			X_A		Slength;
	
	Rlength		= 0;
	Slength		= 0;
	tmpLen				= g_ECKeyStore.b_RLength;
	bECKeyByteLength = g_ECKeyStore.b_PLength;
	resultVal = MH_RET_ECC_FAILURE;
	
	ecpMem_Copy(&workSpaceTemp[ECC_WORK_TEMP_PRIKEY_OFFSET], g_ECKeyStore.pb_PrivateKeyS, bECKeyByteLength);
	
		//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);

//	if(bHashLen > tmpLen)
//	{
//		bHashLen = tmpLen;
//		ecpMem_Copy(MEMtPX8(&workSpaceTemp[ECC_WORK_E_OFFSET]), g_CryptoData.pbBuffer, bHashLen);
//	}
//	else
//	{
//		ecpMem_Set(MEMtPX8(&workSpaceTemp[ECC_WORK_E_OFFSET]), 0x00, (tmpLen - bHashLen));
//		ecpMem_Copy(PX8_A_U16(MEMtPX8(&workSpaceTemp[ECC_WORK_E_OFFSET]), (tmpLen - bHashLen)), g_CryptoData.pbBuffer, bHashLen);
//	}
	
	ecpMem_Set(MEMtPX8(&workSpaceTemp[ECC_WORK_E_OFFSET]), 0x00, MAX_ECC_KEY_BYTELEN);
	ecpMem_Copy(MEMtPX8(&workSpaceTemp[ECC_WORK_E_OFFSET]), g_CryptoData.pbBuffer, bHashLen);
	
	initECParams(&myEccInitParam, workSpaceParam, argBuf);
	
	resultVal = MHECC_ParamInit(&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	resultVal = MH_RET_ECC_FAILURE;
	myEccParam.pu8D = MEMtPX8(&workSpaceTemp[ECC_WORK_TEMP_PRIKEY_OFFSET]);
	myEccParam.pu8PubX = NULL;
	myEccParam.pu8PubY = NULL;
	myEccParam.u16Bits = myEccInitParam.u16BitLen;
	myEccParam.u32Crc = MHCRC_CalcBuff(0xffff, &myEccParam, sizeof(MHECC_KEY)-4); 
	
	//set key
	resultVal = MHECC_SetKey(&myEccParam,&InBase,&u8Base,&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	resultVal = MH_RET_ECC_FAILURE;
	SignRs.pu8SignR = MEMtPX8(oDataBuffer);
  SignRs.pu8SignS = MEMtPX8(&oDataBuffer[MAX_ECC_KEY_BYTELEN]);
	SignCall.pu8Sign = &SignRs;
	SignCall.pInBase = &InBase;
	SignCall.pParam = &myEccInitParam;
	SignCall.pu8E = MEMtPX8(&workSpaceTemp[ECC_WORK_E_OFFSET]);
	SignCall.pu8Msg = NULL;
	SignCall.u16ELen = bHashLen;
	SignCall.u32MLen = 0;

	MHECC_HashE(&SignCall);
	SignCall.u32Crc = MHCRC_CalcBuff(0xffff, &SignCall, sizeof(MHECC_SIG_VER_CALL)-4);     
	resultVal = MHECC_EcdsaSign(&SignCall);
	
	
		//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);
	
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	else
	{
		ecpMem_Copy(tempR, oDataBuffer, tmpLen);
		Rlength = getRealLength(tempR,tmpLen);
		Rlength = getRealLength2(tempR,Rlength);  
		
		ecpMem_Copy(tempS, (oDataBuffer + MAX_ECC_KEY_BYTELEN),  tmpLen);	
		Slength = getRealLength(tempS,tmpLen);
		Slength = getRealLength2(tempS,Slength);
	
		EncodeSignature(tempR, &Rlength, tempS, &Slength);
		return g_CryptoData.wBufferLen = Rlength + Slength + 6;
	}
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
	u8 workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
	u8 argBuf[MH_ECC521_SPACE_LEN];
	u8 			tmpPrivteKeyBuf[MAX_ECC_KEY_BYTELEN];
	u8 			tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN*2];
	u32      resultVal;
	MH_ECC_INNER_BASE InBase;
	uint8_t u8Base = 0;
	
	MHECC_PARAM   myEccInitParam;
	MHECC_KEY			myEccParam;
	
	memset(&myEccInitParam,0,sizeof(myEccInitParam));
	
		//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);
	
	resultVal = MH_RET_ECC_FAILURE;

	initECParams(&myEccInitParam, workSpaceParam, argBuf);

	resultVal = MHECC_ParamInit(&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	resultVal = MH_RET_ECC_FAILURE;
	myEccParam.pu8D = MEMtPX8(tmpPrivteKeyBuf);
	myEccParam.pu8PubX = MEMtPX8(tmppublicKeyBuf);
	myEccParam.pu8PubY = MEMtPX8(&tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN]);
	myEccParam.u16Bits = myEccInitParam.u16BitLen;
	myEccParam.u32Crc = MHCRC_CalcBuff(0xffff, &myEccParam, sizeof(MHECC_KEY)-4); 
	resultVal = MHECC_GenKey(&myEccParam,&myEccInitParam);
	
			//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
//	resultVal = MH_RET_ECC_FAILURE;
//	//set key to Check the legality of the key
//	resultVal = MHECC_SetKey(&myEccParam,&InBase,&u8Base,&myEccInitParam);
//	if(MH_RET_ECC_SUCCESS != resultVal)
//	{
//		while(1);
//	}
//	
	memWriteU8NoAtomic(g_ECKeyStore.pb_PublicKeyQxy, 0x04);
	memCopyNoAtomic((g_ECKeyStore.pb_PublicKeyQxy + 1), tmppublicKeyBuf, g_ECKeyStore.b_PLength);
	memCopyNoAtomic((g_ECKeyStore.pb_PublicKeyQxy + g_ECKeyStore.b_PLength + 1), (tmppublicKeyBuf + MAX_ECC_KEY_BYTELEN), g_ECKeyStore.b_PLength);
	memCopyNoAtomic(g_ECKeyStore.pb_PrivateKeyS, tmpPrivteKeyBuf, g_ECKeyStore.b_PLength);
		
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
#define ECC_VERIFY_WORK_TEMP_R_OFFSET			(MAX_ECC_KEY_BYTELEN * 2)
#define ECC_VERIFY_WORK_TEMP_S_OFFSET			(MAX_ECC_KEY_BYTELEN * 3)
#define ECC_VERIFY_WORK_TEMP_E_OFFSET			(MAX_ECC_KEY_BYTELEN * 4)

//#define ECC_VERIFY_WORK_TEMP_PUBKEY_OFFSET		0
//#define ECC_VERIFY_WORK_TEMP_PUBKEY_X_OFFSET	ECC_VERIFY_WORK_TEMP_PUBKEY_OFFSET
//#define ECC_VERIFY_WORK_TEMP_PUBKEY_Y_OFFSET	MAX_ECC_KEY_BYTELEN
//#define ECC_VERIFY_WORK_TEMP_RPOINT_OFFSET		(MAX_ECC_KEY_BYTELEN * 2)
//#define ECC_VERIFY_WORK_TEMP_RPOINT_X_OFFSET	ECC_VERIFY_WORK_TEMP_RPOINT_OFFSET
//#define ECC_VERIFY_WORK_TEMP_RPOINT_Y_OFFSET	(MAX_ECC_KEY_BYTELEN * 3)
//#define ECC_VERIFY_WORK_TEMP_S_OFFSET			(MAX_ECC_KEY_BYTELEN * 4)
//#define ECC_VERIFY_WORK_TEMP_R_OFFSET			(MAX_ECC_KEY_BYTELEN * 5)
//#define ECC_VERIFY_WORK_TEMP_E_OFFSET			(MAX_ECC_KEY_BYTELEN * 6)
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
	/**
     * Information of workSpace bytes:
     * -------------------------------------------------------------------------------------------
	 * | FP   				 	| paramA  				|paramB   				|  paramG  					|  paramN  				|
	 * | MAX_ECC_KEY_BYTELEN	| MAX_ECC_KEY_BYTELEN 	|MAX_ECC_KEY_BYTELEN 	| MAX_ECC_KEY_BYTELEN	* 2 |  MAX_ECC_KEY_BYTELEN  |
	 * -------------------------------------------------------------------------------------------
	 */
	
	u8 		X_A		workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
	
	 /**
     * Information of workSpace bytes:
     * -------------------------------------------------------------------------------------------
	 * | PublicKey(= K * G) 	   | R			                |S	                    |  E  					
	 * | MAX_ECC_KEY_BYTELEN * 2 | MAX_ECC_KEY_BYTELEN  	|MAX_ECC_KEY_BYTELEN 		| MAX_ECC_KEY_BYTELEN 	
	 * -------------------------------------------------------------------------------------------
	 */
	u8 		X_A		workSpaceTemp[(MAX_ECC_KEY_BYTELEN * 7)];
	
	u8 argBuf[MH_ECC521_SPACE_LEN];
	MHECC_PARAM   myEccInitParam;
	MHECC_KEY			myEccParam;
	MHECC_SIG_VER_CALL SignCall;
	MH_ECC_INNER_BASE InBase;
	MHECC_SIGN SignRs;

	PX8 	X_A   publicKey;
	PX8		X_A		rsBuf;
	PX8 	X_A		hashBuf;
	u8 X_A * rSign;
	u8 X_A * sSign;
	u32 resultVal;
	
	u8 X_A hashType, hashLen, mdState;
	u8 X_A rLength, sLength;
	u8 X_A bECKeyByteLength;
	u8 X_A orderLen;
	uint8_t u8Base = 0;
	hashType = 0;
	hashLen = 0;
	resultVal = MH_RET_ECC_FAILURE;
	mdState = HASH_INIT;
	bECKeyByteLength = g_ECKeyStore.b_PLength;
	orderLen = g_ECKeyStore.b_RLength;

	// the g_CryptoData.pbBuffer is structured as follows:
	// ----------------------------------------
	//  decode signature (r, s) |  hash value |
	// ----------------------------------------
	//     twice lenght of R    | length as R |
	// ----------------------------------------
	rSign = g_CryptoData.pbBuffer;
	sSign = rSign + orderLen;

	publicKey = MEMtPX8(&workSpaceTemp[ECC_VERIFY_WORK_TEMP_PUBKEY_OFFSET]);
	rsBuf			= MEMtPX8(&workSpaceTemp[ECC_VERIFY_WORK_TEMP_R_OFFSET]);
	hashBuf		= MEMtPX8(&workSpaceTemp[ECC_VERIFY_WORK_TEMP_E_OFFSET]);
	
	mdState = HASH_INIT;
	
		//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);
	
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
	
	initECParams(&myEccInitParam, workSpaceParam, argBuf);
	resultVal = MHECC_ParamInit(&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	resultVal = MH_RET_ECC_FAILURE;
	
	ecpMem_Copy(MEMtPX8(rsBuf), rSign, rLength);
	if((ecpMem_Compare(MEMtPX8(rsBuf), MEMtPX8(myEccInitParam.pu8N), bECKeyByteLength) >= 0) )
	{
		return EC_VERIFY_FAIL;
	}
	
	ecpMem_Copy(MEMtPX8(rsBuf + MAX_ECC_KEY_BYTELEN), sSign, sLength);
	if((ecpMem_Compare(MEMtPX8(rsBuf + MAX_ECC_KEY_BYTELEN), MEMtPX8(myEccInitParam.pu8N), bECKeyByteLength) >= 0) )
	{
		return EC_VERIFY_FAIL;
	}
	
	ecpMem_Copy(MEMtPX8(publicKey), (g_ECKeyStore.pb_PublicKeyQxy + 1), bECKeyByteLength);
	ecpMem_Copy(MEMtPX8(&publicKey[MAX_ECC_KEY_BYTELEN]), (g_ECKeyStore.pb_PublicKeyQxy + 1 + bECKeyByteLength), bECKeyByteLength);
  //ecpMem_Copy(MEMtPX8(publicKey), (g_ECKeyStore.pb_PublicKeyQxy + 1), (bECKeyByteLength * 2));
	
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
//	if(hashLen > orderLen)
//	{
//		hashLen = orderLen;
//		HASHDoFinal((PM8)inBuff, inLen, MEMtPX8(hashBuf), mdState, mdBuff, tmpBuff, hashType);
//	}
//	else
//	{
//		ecpMem_Set(MEMtPX8(hashBuf), 0x00, (orderLen - hashLen));
//		HASHDoFinal((PM8)inBuff, inLen, PX8_A_U16(MEMtPX8(hashBuf), (orderLen - hashLen)), mdState, mdBuff, tmpBuff, hashType);
//	}
	
	ecpMem_Set(MEMtPX8(hashBuf), 0x00, MAX_ECC_KEY_BYTELEN);
	HASHDoFinal((PM8)inBuff, inLen, MEMtPX8(hashBuf), mdState, mdBuff, tmpBuff, hashType);
		
	myEccParam.pu8D = NULL;
	myEccParam.pu8PubX = MEMtPX8(publicKey);
	myEccParam.pu8PubY = MEMtPX8(&publicKey[MAX_ECC_KEY_BYTELEN]);
	myEccParam.u16Bits = myEccInitParam.u16BitLen;
	myEccParam.u32Crc = MHCRC_CalcBuff(0xffff, &myEccParam, sizeof(MHECC_KEY)-4); 
	
	//set key
	resultVal = MHECC_SetKey(&myEccParam,&InBase,&u8Base,&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	resultVal = MH_RET_ECC_FAILURE;
	
	SignRs.pu8SignR = MEMtPX8(rsBuf);
	SignRs.pu8SignS = MEMtPX8(rsBuf + MAX_ECC_KEY_BYTELEN);
	SignCall.pu8Sign = &SignRs;
	SignCall.pInBase = &InBase;
	SignCall.pParam = &myEccInitParam;
	SignCall.pu8E = hashBuf;
	SignCall.pu8Msg = NULL;
	SignCall.u16ELen = hashLen;
	SignCall.u32MLen = 0;
	
	SignCall.u32Crc = MHCRC_CalcBuff(0xffff, &SignCall, sizeof(MHECC_SIG_VER_CALL)-4);   
	resultVal = MHECC_EcdsaVerify(&SignCall);
	
			//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);
	
	if(MH_RET_ECC_SUCCESS == resultVal)
	{
		return EC_VERIFY_OK;
	}
	else
	{
		return EC_VERIFY_FAIL;
	}
	
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
	 /**
     * Information of workSpace bytes:
     * -------------------------------------------------------------------------------------------
	 * | FP   				 	| paramA  				|paramB   				|  paramG  					|  paramN  				|
	 * | MAX_ECC_KEY_BYTELEN	| MAX_ECC_KEY_BYTELEN 	|MAX_ECC_KEY_BYTELEN 	| MAX_ECC_KEY_BYTELEN	* 2 |  MAX_ECC_KEY_BYTELEN  |
	 * -------------------------------------------------------------------------------------------
	 */
	u8		pkBuf[521],XBuf[521],YBuf[521];
	u8 		argBuf[MH_ECC521_SPACE_LEN];
	MHECC_PARAM   myEccInitParam;
	MHECC_KEY			myEccParam;
	MHECC_POINT_MUL_CALL  Pointmulcall;
	MHECC_POINT 		PointA,PointR;
	MHBN_BIGNUM			pk;
	u32		 resultVal;
	u8 		X_A 	workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];


	
	if(g_ECKeyStore.bParamStatus & 0x01)
	{
		if(g_ECKeyStore.b_HValue == 0)
		{
			return EC_DOMAINPARAMS_CHECK_FAIL;
		}
	}
	
	//check condition 1: P is an odd prime
	if(0x00 == (memReadU8(PM8_A_U16(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.b_PLength - 1))) & 0x01))
	{
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}

	//check condition 2: A, B, Gx, Gy are integers in the interval	[0, p-1]
	if (ECC_compare(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.pb_domainParams + g_ECKeyStore.b_PLength), g_ECKeyStore.b_PLength) != 0x01)
	{
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}	
	if (ECC_compare(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.pb_domainParams + g_ECKeyStore.b_PLength * 2), g_ECKeyStore.b_PLength) != 0x01)
	{
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}	
	if (ECC_compare(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.pb_domainParams + g_ECKeyStore.b_PLength * 3 + 1 ), g_ECKeyStore.b_PLength) != 0x01)
	{
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}	
	if (ECC_compare(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.pb_domainParams + g_ECKeyStore.b_PLength * 4 + 1), g_ECKeyStore.b_PLength) != 0x01)
	{
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}	
	
	//modify by jiangxiaohui@20220526 : check condition 5: the order of G(x,y) - R is prime (see sec1_ECC-v2(2009).pdf  Page 17)
	if(0x00 == (memReadU8(PM8_A_U16(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.b_PLength * 5) + g_ECKeyStore.b_RLength)) & 0x01))
	{
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	if(g_ECKeyStore.b_RLength == g_ECKeyStore.b_PLength)
	{
		if(0x00 == ECC_compare((g_ECKeyStore.pb_domainParams + g_ECKeyStore.b_PLength * 5 + 1), g_ECKeyStore.pb_domainParams, g_ECKeyStore.b_PLength))
		{
			return EC_DOMAINPARAMS_CHECK_FAIL;
		}
	}
	else if(g_ECKeyStore.b_RLength > g_ECKeyStore.b_PLength)
	{
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	
			//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);
	
	
	initECParams(&myEccInitParam, workSpaceParam, argBuf);
	resultVal = MHECC_ParamInit(&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		//while(1);
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	resultVal = MH_RET_ECC_FAILURE;
	
	PointR.pu8X = MEMtPX8(XBuf);
	PointR.pu8Y = MEMtPX8(YBuf);
	PointA.pu8X = MEMtPX8(XBuf);
	PointA.pu8Y = MEMtPX8(YBuf);
	LOADBIGNUM(pk,&pkBuf[0],g_ECKeyStore.b_PLength);
	
	ecpMem_Copy(MEMtPX8(XBuf), (g_ECKeyStore.pb_domainParams + g_ECKeyStore.b_PLength * 3 + 1 ), g_ECKeyStore.b_PLength);
	ecpMem_Copy(MEMtPX8(YBuf), (g_ECKeyStore.pb_domainParams + g_ECKeyStore.b_PLength * 4 + 1 ), g_ECKeyStore.b_PLength);
	memset(pkBuf,0,sizeof(pkBuf));
	pkBuf[g_ECKeyStore.b_PLength -1] = 1;
	
	Pointmulcall.pPointR = &PointR;
	Pointmulcall.pPointA = &PointA;
	Pointmulcall.pK = &pk;
	Pointmulcall.u32Crc = MHCRC_CalcBuff(0xffff, &Pointmulcall, sizeof(MHECC_POINT_MUL_CALL)-4);   
	
	resultVal = MHECC_PointMul(&Pointmulcall,&myEccInitParam);
	
	if(MH_RET_ECCPOINT_SUCCESS != resultVal)
	{
		//while(1);
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	
				//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);

//	initECParams(&myEccInitParam, workSpaceParam, NULL);

//	libResult = HED_ECCCurveCheck(&myEccInitParam);
//	
//	if(libResult == 0)
//	{
//		//while(1);
//		return EC_DOMAINPARAMS_CHECK_FAIL;
//	}
	
	return EC_DOMAINPARAMS_CHECK_OK;
	
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
//	u32 argBuf[(((MAX_ECC_KEY_BYTELEN >> 2) * 14) + 8)];
//	u8  workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
//	u8 	tmpprivateKeyBuf[MAX_ECC_KEY_BYTELEN];
//	u8  resultVal;
//	
//	ECC_Init_Parameters   myEccInitParam;
//	ECC_Parameters				myEccParam;
//	
//	resultVal = 1;
//	
	u8 * X_A tmpPoint;
	u8 X_A keyByteLen, orderLen = g_ECKeyStore.b_RLength;

	//the private key is not 0, and smaller than R
	keyByteLen = g_ECKeyStore.b_PLength;
	tmpPoint = g_ECKeyStore.pb_PrivateKeyS;
	while (keyByteLen)
	{
		if (*tmpPoint++)
		{
			break;
		}
		keyByteLen--;
	}

	if (keyByteLen == 0)  // all key byte are zeros
	{
		return EC_KEY_CHECK_FAIL;
	}
	else
	{
		if (keyByteLen > orderLen)
		{
			return EC_KEY_CHECK_FAIL;
		}
		else if (keyByteLen == orderLen)
		{
			if (ECC_compare((g_ECKeyStore.pb_domainParams + keyByteLen * 5 + 1), g_ECKeyStore.pb_PrivateKeyS, keyByteLen) != 0x01)
			{
				return EC_KEY_CHECK_FAIL;
			}
		}
	}

//	initECParams(&myEccInitParam, workSpaceParam, argBuf);
//	
//	resultVal = HED_ECCInitPara(&myEccInitParam);
//	
//	if(EC_PROCESS_OK != resultVal)
//	{
//		//while(1);
//		return EC_KEY_CHECK_FAIL;
//	}

//	ecpMem_Copy(tmpprivateKeyBuf, g_ECKeyStore.pb_PrivateKeyS, keyByteLen);
//	
//	myEccParam.priKeyPtr = (unsigned int *)tmpprivateKeyBuf;
//	myEccParam.argPtr = (unsigned int *)argBuf;
//	
//	resultVal = HED_ECCIsLegalPriKey(&myEccParam);
//	if(EC_PROCESS_OK != resultVal)
//	{
//		//while(1);
//		return EC_KEY_CHECK_FAIL;
//	}
//	
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
	u8		pkBuf[521],XBuf[521],YBuf[521];
	u8 		argBuf[MH_ECC521_SPACE_LEN];
	MHECC_PARAM   myEccInitParam;
	MHECC_KEY			myEccParam;
	MHECC_POINT_MUL_CALL  Pointmulcall;
	MHECC_POINT 		PointA,PointR;
	MHBN_BIGNUM			pk;
	u32		 resultVal;
	u8 		X_A 	workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
	
	
	//check condition 1: Px, Py are integers in the interval	[0, p-1]
	if (ECC_compare(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.pb_PublicKeyQxy + 1), g_ECKeyStore.b_PLength) != 0x01)
	{
		return EC_KEY_CHECK_FAIL;
	}	
	if (ECC_compare(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.pb_PublicKeyQxy + g_ECKeyStore.b_PLength + 1), g_ECKeyStore.b_PLength) != 0x01)
	{
		return EC_KEY_CHECK_FAIL;
	}	
	
				//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);
	
	
	initECParams(&myEccInitParam, workSpaceParam, argBuf);
	resultVal = MHECC_ParamInit(&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		//while(1);
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	resultVal = MH_RET_ECC_FAILURE;
	
	PointR.pu8X = MEMtPX8(XBuf);
	PointR.pu8Y = MEMtPX8(YBuf);
	PointA.pu8X = MEMtPX8(XBuf);
	PointA.pu8Y = MEMtPX8(YBuf);
	LOADBIGNUM(pk,&pkBuf[0],g_ECKeyStore.b_PLength);
	
	ecpMem_Copy(MEMtPX8(XBuf), (g_ECKeyStore.pb_PublicKeyQxy + 1), g_ECKeyStore.b_PLength);
	ecpMem_Copy(MEMtPX8(YBuf), (g_ECKeyStore.pb_PublicKeyQxy + g_ECKeyStore.b_PLength + 1), g_ECKeyStore.b_PLength);
	memset(pkBuf,0,sizeof(pkBuf));
	pkBuf[g_ECKeyStore.b_PLength -1] = 1;
	
	Pointmulcall.pPointR = &PointR;
	Pointmulcall.pPointA = &PointA;
	Pointmulcall.pK = &pk;
	Pointmulcall.u32Crc = MHCRC_CalcBuff(0xffff, &Pointmulcall, sizeof(MHECC_POINT_MUL_CALL)-4);   
	
	resultVal = MHECC_PointMul(&Pointmulcall,&myEccInitParam);
	
	if(MH_RET_ECCPOINT_SUCCESS != resultVal)
	{
		//while(1);
		return EC_DOMAINPARAMS_CHECK_FAIL;
	}
	
				//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);


	
//	u32 argBuf[(((MAX_ECC_KEY_BYTELEN >> 2) * 14) + 8)];
//	u8  workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
//	u8 	tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN << 1];
//	u8  resultVal;
//	
//	ECC_Init_Parameters   myEccInitParam;
//	ECC_Parameters				myEccParam;
//	
//	resultVal = 1;
//	
//	//check condition 1: Px, Py are integers in the interval	[0, p-1]
//	if (ECC_compare(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.pb_PublicKeyQxy + 1), g_ECKeyStore.b_PLength) != 0x01)
//	{
//		return EC_KEY_CHECK_FAIL;
//	}	
//	if (ECC_compare(g_ECKeyStore.pb_domainParams, (g_ECKeyStore.pb_PublicKeyQxy + g_ECKeyStore.b_PLength + 1), g_ECKeyStore.b_PLength) != 0x01)
//	{
//		return EC_KEY_CHECK_FAIL;
//	}	
//	
//	initECParams(&myEccInitParam, workSpaceParam, argBuf);
//	
//	resultVal = HED_ECCInitPara(&myEccInitParam);
//	if(EC_PROCESS_OK != resultVal)
//	{
//		return EC_KEY_CHECK_FAIL;
//		//while(1);
//	}

//	ecpMem_Copy(tmppublicKeyBuf, (g_ECKeyStore.pb_PublicKeyQxy + 1), g_ECKeyStore.b_PLength);
//	ecpMem_Copy((tmppublicKeyBuf + g_ECKeyStore.b_PLength), (g_ECKeyStore.pb_PublicKeyQxy + g_ECKeyStore.b_PLength + 1), g_ECKeyStore.b_PLength);
//	
//	myEccParam.pubKeyPtr = (unsigned int *)tmppublicKeyBuf;
//	myEccParam.argPtr = (unsigned int *)argBuf;
//	
//	resultVal = HED_ECCIsLegalPubKey(&myEccParam);
//	if(EC_PROCESS_OK != resultVal)
//	{
//		return EC_KEY_CHECK_FAIL;
//		//while(1);
//	}
//	
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
 * modify by chenpingdi@20221022: ???????KeyAgreement.generateSecret?,ALG_EC_SVDP_DH_PLAIN?ALG_EC_SVDP_DHC_PLAIN???????secret????,??????0,??????????java api??????:This algorithm returns the raw output of the derivation primitive
 ****************************************************************************/
u8 ECDiffieHellman(u8 bECDHType)
{
	 /**
   * Information of workSpace bytes:
     * -------------------------------------------------------------------------------------------
	 * | FP   				 	| paramA  				|paramB   				|  paramG  					|  paramN  				|
	 * | MAX_ECC_KEY_BYTELEN	| MAX_ECC_KEY_BYTELEN 	|MAX_ECC_KEY_BYTELEN 	| MAX_ECC_KEY_BYTELEN	* 2 |  MAX_ECC_KEY_BYTELEN  |
	 * -------------------------------------------------------------------------------------------
	 */
	
	u8 			X_A		workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
	u8 			tmpPrivteKeyBuf[MAX_ECC_KEY_BYTELEN + 4];
	u8 			tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN*2];
	u8 			outputBuf[MAX_ECC_KEY_BYTELEN * 2];
	u8 			argBuf[MH_ECC521_SPACE_LEN];
	
	MHECC_PARAM  					myEccInitParam;
	MHECC_KEY   					KeySrc;
	MH_ECC_INNER_BASE		  InBasePri,InBasePub;
	MHECC_ECDH_CALL 			EcdhCall;
	u32 		resultVal;
	u16 		bECKeyBitLength;
	u8 			bECKeyByteLength;
	u8 			secretLen;
	
	uint8_t u8Base = 0;
	bECKeyByteLength = g_ECKeyStore.b_PLength;
	bECKeyBitLength = bECKeyByteLength<<3;
	resultVal = MH_RET_ECC_FAILURE;
	
		//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);
	
	//decide which type of DH to use
//	if(ALG_EC_SVDP_DHC == bECDHType || ALG_EC_SVDP_DHC_PLAIN == bECDHType)
	{ 
		initECParams(&myEccInitParam, workSpaceParam, argBuf);

		resultVal = MHECC_ParamInit(&myEccInitParam);
		if(MH_RET_ECC_SUCCESS != resultVal)
		{
			while(1);
		}
		resultVal = MH_RET_ECC_FAILURE;
		
		ecpMem_Copy(MEMtPX8(&tmpPrivteKeyBuf[0]), g_ECKeyStore.pb_PrivateKeyS, bECKeyByteLength);
		ecpMem_Copy(MEMtPX8(&tmppublicKeyBuf[0]), g_ECKeyStore.pb_PublicKeyQxy+1, bECKeyByteLength);
		ecpMem_Copy(MEMtPX8(&tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN]), g_ECKeyStore.pb_PublicKeyQxy + 1+bECKeyByteLength, bECKeyByteLength);
		
		//set pri
		memset(&KeySrc,0,sizeof(KeySrc));
		KeySrc.pu8D = MEMtPX8(&tmpPrivteKeyBuf[0]);
		KeySrc.u16Bits = bECKeyBitLength;
		KeySrc.u32Crc = MHCRC_CalcBuff(0xffff, &KeySrc, sizeof(MHECC_KEY)-4); 
		resultVal = MHECC_SetKey(&KeySrc,&InBasePri,&u8Base,&myEccInitParam);
		if(MH_RET_ECC_SUCCESS != resultVal)
		{
			while(1);
		}
		resultVal = MH_RET_ECC_FAILURE;
			
		//set pub
		memset(&KeySrc,0,sizeof(KeySrc));
		KeySrc.pu8PubX = MEMtPX8(&tmppublicKeyBuf[0]);
		KeySrc.pu8PubY = MEMtPX8(&tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN]);
		KeySrc.u16Bits = bECKeyBitLength;
		KeySrc.u32Crc = MHCRC_CalcBuff(0xffff, &KeySrc, sizeof(MHECC_KEY)-4); 
		resultVal = MHECC_SetKey(&KeySrc,&InBasePub,&u8Base,&myEccInitParam);
		if(MH_RET_ECC_SUCCESS != resultVal)
		{
			while(1);
		}
		resultVal = MH_RET_ECC_FAILURE;
			
		//do ecdh
		EcdhCall.pParam = &myEccInitParam;
		EcdhCall.pInBaseU = &InBasePri;
		EcdhCall.pInBaseV = &InBasePub;
		EcdhCall.pu8KeyOut = MEMtPX8(outputBuf);
		EcdhCall.u32Crc = MHCRC_CalcBuff(0xffff, &EcdhCall, sizeof(MHECC_ECDH_CALL)-4);
		resultVal = MHECC_ECDH(&EcdhCall);
		if(MH_RET_ECC_SUCCESS != resultVal)
		{
			while(1);
		}
		
	}
	
		//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);
	
	
	//check the result is not the infinity(not zero)
	if(checkSrcIsAllZero(outputBuf, bECKeyByteLength) == true)
	{
		return EC_PROCESS_FAIL;
	}
	
//	if(0x00 == getRealLength(MEMtPX8(oData + bECKeyByteLength), bECKeyByteLength))
//	{
//		return EC_PROCESS_FAIL;
//	}
	
	ecpMem_Copy(g_CryptoData.pbBuffer, MEMtPX8(outputBuf), bECKeyByteLength);
		
	if(ALG_EC_SVDP_DHC == bECDHType || ALG_EC_SVDP_DH == bECDHType)
	{
		//if nessary, we can delete zeros in the front using the code below
//		secretLen = getRealLength(g_CryptoData.pbBuffer, bECKeyByteLength);	 //modify by chenpingdi@20221022: ???????,???????8??????,?????0???SHA?????:	ECC_generateSecret_01
		//compute SHA-1 value using the result from above
		HASHDoFinal((PM8)g_CryptoData.pbBuffer, bECKeyByteLength, g_CryptoData.pbBuffer, HASH_INIT, NULL, NULL, ALG_SHA_1);
		secretLen = SHA1_DIGEST_SIZE;
	}
	
	return EC_PROCESS_OK;
}

/**********************************************************************
 * Function name: ecpUniversalAMulPointXY()
 * Qxy = [A] * Pxy:
 * APoint must be 32 bytes
 **********************************************************************/
u8 ecpUniversalAMulPointXY(PM8 APoint, PM8 PointPXY, PX8 PointQXY, PX8 ECDomainParams)
{
	
	/**
     * Information of workSpace bytes:
     * -------------------------------------------------------------------------------------------
	 * | FP   				 	| paramA  				|paramB   				|  paramG  					|  paramN  				|
	 * | MAX_ECC_KEY_BYTELEN	| MAX_ECC_KEY_BYTELEN 	|MAX_ECC_KEY_BYTELEN 	| MAX_ECC_KEY_BYTELEN	* 2 |  MAX_ECC_KEY_BYTELEN  |
	 * -------------------------------------------------------------------------------------------
	 */
	
	u8 			workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
	
	 /**
     * Information of workSpace bytes:
     * -------------------------------------------------------------------------------------------
	 * | PrivareKey(priD) 	 	| publicKey 		 		| 
	 * | MAX_ECC_KEY_BYTELEN	| MAX_ECC_KEY_BYTELEN	* 2 |  
	 * -------------------------------------------------------------------------------------------
	 */
	u8 			tmpPrivteKeyBuf[MAX_ECC_KEY_BYTELEN + 4];
	u8 			tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN*2];
	u8 			outputBuf[MAX_ECC_KEY_BYTELEN * 2];
	u8			argBuf[MH_ECC521_SPACE_LEN];
	
	MHECC_PARAM   myEccInitParam;
	MHECC_KEY			myEccParam;
	MHECC_POINT_MUL_CALL		MulCall;
	
	MHECC_POINT   Q,P;
	MHBN_BIGNUM   A;
	
	u32      resultVal;
	uint8_t u8Base = 0;
	u8 tempPLength;
	
	resultVal = MH_RET_ECC_FAILURE;
	tempPLength = g_ECKeyStore.b_PLength;
	
		//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);

	initECParams(&myEccInitParam, workSpaceParam, argBuf);

	resultVal = MHECC_ParamInit(&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	resultVal = MH_RET_ECC_FAILURE;
	
	ecpMem_Copy(MEMtPX8(&tmpPrivteKeyBuf[0]), APoint, tempPLength);
	ecpMem_Copy(MEMtPX8(&tmppublicKeyBuf[0]), PointPXY, tempPLength);
	ecpMem_Copy(MEMtPX8(&tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN]), PX8_A_U16(PointPXY, tempPLength), tempPLength);

	Q.pu8X = MEMtPX8(outputBuf);
	Q.pu8Y = MEMtPX8(&outputBuf[MAX_ECC_KEY_BYTELEN]);
	P.pu8X = MEMtPX8(tmppublicKeyBuf);
	P.pu8Y = MEMtPX8(&tmppublicKeyBuf[MAX_ECC_KEY_BYTELEN]);
	LOADBIGNUM(A,&tmpPrivteKeyBuf[0],tempPLength);
//	A.pu8Data = MEMtPX8(&tmpPrivteKeyBuf[0]);
//	A.u32Len = tempPLength;
	MulCall.pPointR = &Q;
	MulCall.pPointA = &P;
	MulCall.pK = &A;
	MulCall.u32Crc = MHCRC_CalcBuff(0xffff, &MulCall, sizeof(MHECC_POINT_MUL_CALL)-4);     

	resultVal = MHECC_PointMul(&MulCall,&myEccInitParam);
	
		//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);
	
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}

	ecpMem_Copy(PointQXY, MEMtPX8(&outputBuf[0]), tempPLength);
	ecpMem_Copy(PX8_A_U16(PointQXY, tempPLength), MEMtPX8(&outputBuf[MAX_ECC_KEY_BYTELEN]), tempPLength);
//	
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
//	big A, B, POrBitCofficient, Qx, Qy, G_order;
//	epoint * Q;
//	miracl * mip;
//	u16 digitNum;

//	u8 tempBuff[MAX_ECC_KEY_BYTELEN];
////	ecpEccGfp_KeyGen_Param_t X_A ECKeyGenParams;
//	u8 tmpLen;
//	u8 bECKeyByteLength;

//	bECKeyByteLength = g_ECKeyStore.b_PLength;
//	tmpLen = g_ECKeyStore.b_RLength;

//	digitNum = tmpLen << 1;
//	mip = mirsys(digitNum, 16); // use hex internally

//	PATCH_BOOT_CRYPTO_ELLIPSECURVE_API_007();

//	A = mirvar(0);
//	B = mirvar(0);
//	POrBitCofficient = mirvar(0);
//	Qx = mirvar(0);
//	Qy = mirvar(0);
//	G_order = mirvar(0);

//	memRefresh();

////	initECParams(&ECKeyGenParams, NULL, NULL);

////	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.a, bECKeyByteLength);
////	bytes_to_big(bECKeyByteLength, tmpBuff, A);
////
////	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.b, bECKeyByteLength);
////	bytes_to_big(bECKeyByteLength, tmpBuff, B);
////
////	ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.p.pPtr, bECKeyByteLength);
////	bytes_to_big(bECKeyByteLength, tmpBuff, POrBitCofficient);
////
////    ecpMem_Copy(tmpBuff, ECKeyGenParams.domain.n.pPtr, tmpLen);
////    bytes_to_big(tmpLen, tmpBuff, G_order);

//	ecpMem_Copy(tempBuff, ECDomainParams, bECKeyByteLength);
//	bytes_to_big(bECKeyByteLength, tempBuff, POrBitCofficient);

//	ecpMem_Copy(tempBuff, ECDomainParams + bECKeyByteLength, bECKeyByteLength);
//	bytes_to_big(bECKeyByteLength, tempBuff, A);

//	ecpMem_Copy(tempBuff, ECDomainParams + bECKeyByteLength*2, bECKeyByteLength);
//	bytes_to_big(bECKeyByteLength, tempBuff, B);

//	ecpMem_Copy(tempBuff, xPointPtr, bECKeyByteLength);
//    bytes_to_big(bECKeyByteLength, tempBuff, Qx); // skip type value 0x04

//	if(g_ECKeyStore.b_PLength == 0)
//	{
//	    //Cleans up after the current instance of MIRACL, and frees all internal variables.
//	    //A subsequent call to mirsys() will re-initialise the MIRACL system
//	    //Must be called after mirsys()
//	    mirexit();
//		return EC_KEY_CHECK_FAIL;
//	}
//	else
//	{
//		// the public key is smaller than P
//		if(mr_compare(Qx, POrBitCofficient) >= 0)
//		{
//		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
//		    //A subsequent call to mirsys() will re-initialise the MIRACL system
//		    //Must be called after mirsys()
//		    mirexit();
//			return EC_KEY_CHECK_FAIL;
//		}

//		// check the public point whether is on the elliptic curve
//		ecurve_init(A, B, POrBitCofficient, MR_PROJECTIVE);
//		Q = epoint_init();
//		if(!epoint_set(Qx, Qx, 1, Q))
//		{
//		    //Cleans up after the current instance of MIRACL, and frees all internal variables.
//		    //A subsequent call to mirsys() will re-initialise the MIRACL system
//		    //Must be called after mirsys()
//		    mirexit();
//			return EC_KEY_CHECK_FAIL;
//		}

//		//output the shared yPointPtr element z = ECKEYGEN_BASE_TempQx
//		epoint_get(Q, Qx, Qy);
//		big_to_bytes(bECKeyByteLength, Qy, yPointPtr, TRUE);

//		if(mr_compare(Qy, POrBitCofficient) >= 0)
//		{
//			//Cleans up after the current instance of MIRACL, and frees all internal variables.
//			//A subsequent call to mirsys() will re-initialise the MIRACL system
//			//Must be called after mirsys()
//			mirexit();
//			return EC_KEY_CHECK_FAIL;
//		}

//	}
//    //Cleans up after the current instance of MIRACL, and frees all internal variables.
//    //A subsequent call to mirsys() will re-initialise the MIRACL system
//    //Must be called after mirsys()
//    mirexit();
	return EC_KEY_CHECK_OK;
}


/**********************************************************************
 * Function name: ECPointAdd()
 * ECPointC = ECPointA + ECPointB
 **********************************************************************/
u8 ECPointAdd(PM8 ECPointA, PM8 ECPointB, PM8 ECPointC)
{
//	u8 workSpaceParam[ECC_PARAM_LENGTH];

//	u8 workSpaceTemp[(MAX_ECC_KEY_BYTELEN * 6)];
//	
//	u32	EccParaInit[(MAX_ECC_KEY_BYTELEN * 17 + 28) >> 2];
//	
//	u32 result;

//	ECC_PARA ECC_PARA0;
//	ECC_POINT pointA, pointB, pointC;
//	
//	result = 1;
//	
//	pointA.x = (unsigned int *)(&workSpaceTemp[0]);
//	pointA.y = (unsigned int *)(&workSpaceTemp[MAX_ECC_KEY_BYTELEN]);
//	ecpMem_Copy(&workSpaceTemp[0], ECPointA, g_ECKeyStore.b_PLength);
//	ecpMem_Copy(&workSpaceTemp[MAX_ECC_KEY_BYTELEN], (ECPointA + g_ECKeyStore.b_PLength), g_ECKeyStore.b_PLength);
//	
//	pointB.x = (unsigned int *)(&workSpaceTemp[MAX_ECC_KEY_BYTELEN * 2]);
//	pointB.y = (unsigned int *)(&workSpaceTemp[MAX_ECC_KEY_BYTELEN * 3]);
//	ecpMem_Copy(&workSpaceTemp[MAX_ECC_KEY_BYTELEN * 2], ECPointB, g_ECKeyStore.b_PLength);
//	ecpMem_Copy(&workSpaceTemp[MAX_ECC_KEY_BYTELEN * 3], (ECPointB + g_ECKeyStore.b_PLength), g_ECKeyStore.b_PLength);
//	
//	pointC.x = (unsigned int *)(&workSpaceTemp[MAX_ECC_KEY_BYTELEN * 4]);
//	pointC.y = (unsigned int *)(&workSpaceTemp[MAX_ECC_KEY_BYTELEN * 5]);
//	
//	initECParams_mathX(&ECC_PARA0, MEMtPX8(workSpaceParam));
//	
//	PKE_Open();
//	
//	ECC_Init(&ECC_PARA0, (unsigned int *)EccParaInit);
//	
//	// 0, set module to p used for cal
//	ECC_SetModulus ((unsigned int *)EccParaInit, 0);
//	
//	result = ECC_PointAdd((unsigned int *)EccParaInit, &pointA, &pointB, &pointC);
//	
//	if(result != 0)
//	{
//		while(1);
//	}
//	
//  PKE_Close();
//	
//	ecpMem_Copy(ECPointC, pointC.x, g_ECKeyStore.b_PLength);
//	ecpMem_Copy(PX8_A_U16(ECPointC, g_ECKeyStore.b_PLength), pointC.y, g_ECKeyStore.b_PLength);
//	
	return CAL_SUCCESSED;
}

/**********************************************************************
 * Function name: ECPointAdd()
 * ECPointC = ECPointA - ECPointB
 **********************************************************************/
u8 ECPointSub(PM8 ECPointA, PM8 ECPointB, PM8 ECPointC)
{
	u8   tempSpace[MAX_ECC_KEY_BYTELEN * 2];
	u8 	 tECPoint[MAX_ECC_KEY_BYTELEN + MAX_ECC_KEY_BYTELEN + 4];
	PX8  tECPointb;

	ecpMem_Copy(tECPoint, ECPointB, (g_ECKeyStore.b_PLength << 1) );
	tECPointb = PX8_A_U16(tECPoint, g_ECKeyStore.b_PLength);
	
	//copy ECC_PARAM_FP
	ecpMem_Copy(tempSpace, g_ECKeyStore.pb_domainParams, g_ECKeyStore.b_PLength);
	
	ecpASubBModC(tempSpace, g_ECKeyStore.b_PLength, tECPointb, tempSpace, tECPointb);
	
	return ECPointAdd(ECPointA, tECPoint, ECPointC);
}

/*
*description: KAT(??????),ECKA-EG
*
*@@See      : ??GP??E,?EUICC??,??ECKA-EG??????("EllipticCurveCryptographyVersion.pdf" Section_4.3.1)
*
*@Prerequisite:	ECCPara????????,????????????????
*
*@function    :calcSHS
*
*@param       :BYTE * publicKey
*@param       :BYTE publicKeylen
*@param       :BYTE keyRef
*@param       :BYTE * privateKey
*@param       :BYTE privateKeyLen   //?????????
*@param       :BYTE * shsBuf   :???????ECC 256 ,??shsBuf ???128???
*
*@return      :shsLen
*/
u8 calcSHS(u8 *publicKey, u8 publicKeyLen, u8 keyRef, u8 *privateKey, u8 privateKeyLen, u8 *shsBuf)
{	
	u8 bECKeyByteLength;
	setECDomainParams(keyRef);
	ecpUniversalAMulPointXY(privateKey,publicKey,shsBuf,g_ECKeyStore.pb_domainParams);
	
	return g_ECKeyStore.b_PLength;
}

#if UICC__AUTH__5G__SUPI

// void bigsmallendianswitch(u8 *big,u8 length,u8 *small)
// {
//	 u8 i;

//	 for (i=0;i<length;i++)
//	 {
//		 small[i]=big[length-i-1];
//	 }
// }
//u8 CONST CURVES_25519_DOMAINPARAMS[] = {
///* p */	0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
//		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7,
///* a */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x6D, 0x06,
///* b */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
///* Gx*/	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
///* Gy*/	0x20, 0xAE, 0x19, 0xA1, 0xB8, 0xA0, 0x86, 0xB4, 0xE0, 0x1E, 0xDD, 0x2C, 
//		0x77, 0x48, 0xD1, 0x4C, 0x92, 0x3D, 0x4D, 0x7E, 0x6D, 0x7C, 0x61, 0xB2, 
//		0x29, 0xE9, 0xC5, 0xA2, 0x7E, 0xCE, 0xD3, 0xD9,

///* n */	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//		0x00, 0x00, 0x00, 0x00, 0x14, 0xDE, 0xF9, 0xDE, 0xA2, 0xF7, 0x9C, 0xD6, 
//		0x58, 0x12, 0x63, 0x1A, 0x5C, 0xF5, 0xD3, 0xED
//};
//#define CURVES_25519_BYTE_LENGTH 32
//void HED_X25519 (unsigned char *shared_secret, const unsigned char *private_key, const unsigned char *public_key)
//{
//	return;

//}

void ECCKeyPair_ProfileA(u8 * pkBuf_smallendian, u8 * skBuf_smallendian)
{
	//u8 l_pri[32] = {0x77,0x07,0x6D,0x0A,0x73,0x18,0xA5,0x7D,0x3C,0x16,0xC1,0x72,0x51,0xB2,0x66,0x45,0xDF,0x4C,0x2F,0x87,0xEB,0xC0,0x99,0x2A,0xB1,0x77,0xFB,0xA5,0x1D,0xB9,0x2C,0x2A};
	//u8 l_pub[32];
	
//	u8 _9[32] = {0x09};
//	Random_number_generator(skBuf_smallendian, 32,true);	
//	X25519(pkBuf_smallendian, skBuf_smallendian,_9);
	
	u8	Pub[32],pri[32];
	MH25519_KEY 	XKey;
	
	u32 	 	resultVal;

	resultVal = MH_RET_ECC_FAILURE;
	
			//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);
	
	XKey.pu8D = pri;
    XKey.pu8Pub = Pub;
    XKey.u32Crc = MHCRC_CalcBuff(0xffff, &XKey, sizeof(MH25519_KEY)-4);
    resultVal = MHX25519_GenKey(&XKey);
	
		//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);
	
    if(MH_RET_25519_SUCCESS != resultVal)
    {
        while(1);
    }
	
	ecpMem_Copy(skBuf_smallendian, pri, 32);
	ecpMem_Copy(pkBuf_smallendian, Pub, 32);

	return;
}

void ECCKeyAgreet_ProfileA(u8 * HNPublicKey_smallendian, u8 * USIMSKey_smallendian, u8 * shareKey_smallendian)
{
	//X25519(shareKey_smallendian, USIMSKey_smallendian,HNPublicKey_smallendian);
	u8	Pub[32],pri[32],shs[32];
	MH25519_INNER_BASE InBaseSelf, InbaseOther;
	MHX25519_DH_CALL XdhCall;
	u32 	 	resultVal;

	resultVal = MH_RET_ECC_FAILURE;
	
				//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);
	
	ecpMem_Copy(pri, USIMSKey_smallendian, 32);
	ecpMem_Copy(Pub, HNPublicKey_smallendian, 32);
	
	resultVal = MHX25519_SetKey(pri, Pub, &InBaseSelf, &InbaseOther);
	
			//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);
	
	if(MH_RET_25519_SUCCESS != resultVal)
    {
        while(1);
    }
	
	resultVal = MH_RET_ECC_FAILURE;
	XdhCall.pInBaseSelf = &InBaseSelf;
    XdhCall.pInBaseOther = &InbaseOther;
    XdhCall.pu8OutK = shs;
    XdhCall.u32Crc = MHCRC_CalcBuff(0xffff, &XdhCall, sizeof(MHX25519_DH_CALL)-4);
    resultVal = MHX25519_DH(&XdhCall);
	if(MH_RET_25519_SUCCESS != resultVal)
    {
        while(1);
    }
	
	ecpMem_Copy(shareKey_smallendian, shs, 32);
	
	return;	
}

//??
//u8 CONST HedEcc256Para[]={
//	/*p*/	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//	/*a*/	0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,
//	/*b*/	0x5A, 0xC6, 0x35, 0xD8, 0xAA, 0x3A, 0x93, 0xE7, 0xB3, 0xEB, 0xBD, 0x55, 
//			0x76, 0x98, 0x86, 0xBC, 0x65, 0x1D, 0x06, 0xB0, 0xCC, 0x53, 0xB0, 0xF6, 
//			0x3B, 0xCE, 0x3C, 0x3E, 0x27, 0xD2, 0x60, 0x4B,
//	/*g*/	0x04, 0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8, 0xBC, 0xE6, 
//			0xE5, 0x63, 0xA4, 0x40, 0xF2, 0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 
//			0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96, 0x4F, 0xE3, 0x42, 
//			0xE2, 0xFE, 0x1A, 0x7F, 0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 
//			0x16, 0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE, 0xCB, 0xB6, 0x40, 
//			0x68, 0x37, 0xBF, 0x51, 0xF5,
//	/*r*/	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 
//			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 
//			0x84, 0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
//};

//sec1-v2.pdf 3.6.1 ANS X9.63 Key Derivation Function
u8 ANS_X963_KDF_Hal(u8* Z,u8 ZLen, u8 keydatalen,u8* sharedInfo, u8 shardInfoLen, u8* K)
{	
	//SHA_INFO xdata ShaInfo;
	u8 hashlen=32; //SHA256_DIGEST_SIZE
	u8 hashsrc[69];
	u8 times;
	
	u8 i;
	u8 counter[4];
	times=(keydatalen+hashlen-1)/hashlen;//
		
	counter[0]=0x0;
	counter[1]=0x0;
	counter[2]=0x0;
	counter[3]=0x1;
	
	for(i=1;i<=times;i++)
	{
		memcpy(hashsrc,Z,ZLen);
		memcpy(hashsrc+32,counter,4);
		memcpy(hashsrc+36,sharedInfo,shardInfoLen);
		
		SHA256DoFinalSingleCall(K+(i-1)*hashlen,hashsrc,ZLen+4+shardInfoLen,0,NULL);
		counter[3]=counter[3]+1;
	}
	

	return 0;
}
void AESEncrypt_Hal(u8* Key, u8* aIn,u8* aOut)
{
	SymCipherDofinal(aIn, 16, aOut, Key, LENGTH_AES_128, NULL, NULL, ALG_AES_BLOCK_128_ECB_NOPAD, (MODE_ENCRYPT  | SYM_ECB_MODE), SBC_INIT, ALG_AES);
}

void ECCPointDecompress(PX8 HNPublicKey)
{
	u8 		workSpaceParam[(MAX_ECC_KEY_BYTELEN * 6)];
	u8		argBuf[MH_ECC521_SPACE_LEN];
	u8		tHNPublicKey[0x41];
	
	MHECC_PARAM   myEccInitParam;
	MHECC_KEY			myEccParam;
	MHECC_POINT PointA;
	
	u32 	 	resultVal;
	
	resultVal = MH_RET_ECC_FAILURE;
	
				//open crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,ENABLE);
	
	setECDomainParams(ECCCure_P_256_KeyParaRef);
	
	initECParams(&myEccInitParam, workSpaceParam, argBuf);
	
	resultVal = MHECC_ParamInit(&myEccInitParam);
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	resultVal = MH_RET_ECC_FAILURE;

	//resultVal = PointDecompress(tHNPublicKey, HNPublicKey,workSpaceParam);
	
	PointA.pu8X = MEMtPX8(tHNPublicKey);   
	PointA.pu8Y = MEMtPX8(&tHNPublicKey[0x20]); 
	resultVal = MHECC_PointUncompress(&PointA, HNPublicKey, &myEccInitParam);
	
		//close crypto clock
	SYSCTRL_ClockEnable(SYSCTRL_Periph_ASYMC,DISABLE);
	
	if(MH_RET_ECC_SUCCESS != resultVal)
	{
		while(1);
	}
	
	HNPublicKey[0] = 0x04;
	ecpMem_Copy(&HNPublicKey[1], tHNPublicKey, 0x40);

//	unsigned int tHNPublicKey[0x41];
//	HED_ECCPointDecompress(tHNPublicKey, HNPublicKey, MEMtPM32(Hed_EC_nistp256));
//	HNPublicKey[0] = 0x04;
//	memcpy(&HNPublicKey[1],tHNPublicKey,64);
	return;
	
}

void ECCKeyPair_ProfileB(u8 * pkBuf, u8 * skBuf)
{
	 u8 pkdata[65];
	
	 GenECCKeyPair(pkdata, skBuf, ECCCure_P_256_KeyParaRef);
	
	 ecpMem_Copy(pkBuf, &pkdata[1], 64);
	
	return;
}

void ECCKeyAgreet_ProfileB(u8 * HNPublicKey, u8 * USIMSKey, u8 * shareKey)
{
	u8 		share[64];
	calcSHS(&HNPublicKey[0], 64, ECCCure_P_256_KeyParaRef, &USIMSKey[0], 32, &share[0]);
	ecpMem_Copy(shareKey, share, 32);
	return;
}

#endif //UICC__AUTH__5G__SUPI

/*********************************************************************
 * Function name: SM2_ECP_Getw()
 *
 * w = log2 inv CPoint / 2 - 1
 *********************************************************************/
u8 SM2_ECP_Getw(PX8 CPoint, u16 byteLen)
{
//	big cPoint;
	u32 w;
//	miracl *mip;
//	u16 digitNum;

//	digitNum = byteLen << 1;
//	mip = mirsys(digitNum, 16); // use hex internally

//	cPoint = mirvar(0);

//	mip->IOBASE = 16;

//	bytes_to_big(byteLen, CPoint, cPoint);

//	w = logb2(cPoint);

//	w = (w/2)*2 + ((w%2 == 0)?0:1); //top integer of w

//	w = (w/2) + ((w%2 == 0)?0:1); //top integer of w

//	w -= 1;

//	//Cleans up after the current instance of MIRACL, and frees all internal variables.
//	//A subsequent call to mirsys() will re-initialise the MIRACL system
//	//Must be called after mirsys()
//	mirexit();

	return w;
}
/*********************************************************************
 * Function name: SM2_ECP_TransX_toInteger()
 *
 * ponitX1 = 2^w + (pointX & (2^w ?1))
 *********************************************************************/
u32 SM2_ECP_TransX_toX1(u32 w, PX8 pointX, u16 byteLen, PX8 pointX1)
{
//	big pointx, pointx1, bigE;
//	big tmp;
//	u16 index;
	u32 num;

//	u8 eValue[1];

//	eValue[0] = 1;

//	miracl *mip;
//	u16 digitNum;

//	digitNum = byteLen << 1;
//	mip = mirsys(digitNum, 16); // use hex internally

//	pointx = mirvar(0);
//	pointx1 = mirvar(0);
//	bigE = mirvar(0);

//	mip->IOBASE = 16;
//	bytes_to_big((u32)byteLen, pointX, pointx);
//	bytes_to_big(sizeof(eValue), eValue, bigE);

//	expb2(w, tmp);// tmp = 2^w

//	subtract(tmp, bigE, pointx1); //pointx1 =  2^w -1

//	big_to_bytes(byteLen, pointx1, pointX1, true);

//	big_to_bytes(byteLen, pointx, pointX, true);

//	for(index = 0; index < byteLen; index++)
//	{
//		pointX1[index] &= pointX[index];
//	}

//	bytes_to_big((u32)byteLen, pointX1, pointx1);

//	add(tmp,pointx1,pointx1); // pointx1 = tmp + pointx1

//	num = big_to_bytes(byteLen, pointx1, pointX1, true);

//    mirexit();

    return num;

}
/*********************************************************************
 * Function name: ecpCalT()
 * Six big numbers x,y,z,w,q and r. On exit q=(x.y+z)/w and r contains the
 * remainder. If w and q are not distinct variables then only the remainder
 * is returned; if q and r are not distinct then only the quotient is returned.
 * The addition of z is not done if x and z (or y and z) are the same
 *********************************************************************/
void SM2_ECP_GetT(PX8 X, PX8 Y, PX8 Z, PX8 W, PX8 Q, PX8 R, u16 byteLen)
{
//	big bigX, bigY, bigZ, bigW, bigQ, bigR;
//	miracl *mip;
//	u16 digitNum;
//	u8 		tempBuf[SM2_DOMAIN_NLEN];

//	if(byteLen > SM2_DOMAIN_NLEN)
//	{
//		return;
//	}

//	digitNum = byteLen << 1;
//	mip = mirsys(digitNum, 16); // use hex internally

//	bigX = mirvar(0);
//	bigY = mirvar(0);
//	bigZ = mirvar(0);
//	bigW = mirvar(0);
//	bigQ = mirvar(0);
//	bigR = mirvar(0);

//	mip->IOBASE = 16;
//	ecpMem_Copy(tempBuf, X, byteLen);
//	bytes_to_big(byteLen, tempBuf, bigX);

//	ecpMem_Copy(tempBuf, Y, byteLen);
//	bytes_to_big(byteLen, tempBuf, bigY);

//	ecpMem_Copy(tempBuf, Z, byteLen);
//	bytes_to_big(byteLen, tempBuf, bigZ);

//	ecpMem_Copy(tempBuf, W, byteLen);
//	bytes_to_big(byteLen, tempBuf, bigW);

//	mad(bigX,bigY,bigZ,bigW,bigQ,bigR); // q=(x.y+z)/w and r contains the remainder

//	big_to_bytes(byteLen, bigR, tempBuf, true);
//	ecpMem_Copy(R, tempBuf, byteLen);

//	mirexit();
	return;

}
// [h*x](PointXY)
void SM2_ECP_GetU(PX8 h, PX8 X, PX8 pointXY, u16 byteLen)
{
//	big bigH;
//	big bigX;
//	big bigZ;
//	big bigW;
//	big bigR;
//	big bigQ;
//	miracl *mip;
//	u16 digitNum;
//	u8  X_A     z[1];
//	u8  X_A     w[1];
//	u8  X_A     q[byteLen];

//	z[0] = 0;
//	w[0] = 1;

//	digitNum = byteLen << 1;
//	mip = mirsys(digitNum, 16); // use hex internally

//	bigX 				= mirvar(0);
//	bigH 				= mirvar(0);
//	bigZ                = mirvar(0);
//	bigW                = mirvar(0);
//	bigQ                = mirvar(0);
//	bigR                = mirvar(0);

//	mip->IOBASE = 16;
//	bytes_to_big(byteLen, h, bigH);
//	bytes_to_big(byteLen, X, bigX);
//	bytes_to_big(byteLen, z, bigZ);
//	bytes_to_big(byteLen, w, bigW);

//	mad(bigX,bigH,bigZ,bigW,bigQ,bigR); // q=(x.y+z)/w and r contains the remainder

//	big_to_bytes(SM2_DOMAIN_NLEN, bigQ, q, true);

//	mirexit();

//	ecpAMulPointXY(q, pointXY, pointXY);

	return;
}

//typedef struct
//{
//    uint8_t *pbSymKey;
//    uint8_t *pbThisVerifyData;
//    uint8_t *pbOtherVerifyData;
//}SM2KeyExchangeOut_t;
/* SM2 end */
#if WITH_SM2
void SM2_KeyExchange(PM8 Pa,PM8 Pb,PM8 tmpPubOfA,PM8 tmpPubOfB,PM8 priOfA,PM8 tmpPriOfA,PM8 IDa,PM8 IDb,u8 IdBytesLen_A,u8 IdBytesLen_B,PM8 outBuffer,u8 exchangeKeyLen,u8 bMode)
{
//	u8 ZA[33];
//	u8 ZB[33];
//	u8 PA[64];
//	u8 PB[64];
//	u8 RA[64];
//	u8 RB[64];
//	u8 selfDe[32];
//	u8 selfTempDe[32];
//	
////	u8 s1[32];
////  u8 sA[32];
//	u8 SM2_oData[256];

//  SM2_KeyExchange_Parameters * SM2KeyExch_Param;
//  SM2_KeyExchange_Parameters SM2KeyExch_ParamVar;
//	
//	memCopyToRam(PA, Pa, 64); 
//	memCopyToRam(PB, Pb, 64);
//	ecpMem_Set(RA, 0, 64);
//	memCopyToRam(RA, tmpPubOfA, 64);
//	memCopyToRam(RB, tmpPubOfB, 64);
//	memCopyToRam(selfDe, priOfA, 32);
//	memCopyToRam(selfTempDe, tmpPriOfA, 32);
//	
//	//use AID compute ZA
//  SM2_ECP_GetZa(IDa, IdBytesLen_A, Pa, ZA);
//	
//	//use BID compute ZA
//	SM2_ECP_GetZa(IDb, IdBytesLen_B, Pb, ZB);
//		
//	SM2KeyExch_Param = &SM2KeyExch_ParamVar;
//	
//	if(bMode == GMSM2KeyExchange_SEND_MODE)
//	{
//		SM2KeyExch_Param->ZA = (unsigned int *)ZA;
//		SM2KeyExch_Param->ZB = (unsigned int *)ZB;
//		SM2KeyExch_Param->PA = (unsigned int *)PA;
//		SM2KeyExch_Param->PB = (unsigned int *)PB;
//		SM2KeyExch_Param->RA = (unsigned int *)RA;
//		SM2KeyExch_Param->RB = (unsigned int *)RB;
//		SM2KeyExch_Param->selfDe = (unsigned int *)selfDe;
//		SM2KeyExch_Param->selfTempDe = (unsigned int *)selfTempDe;
//	}
//	else if(bMode == GMSM2KeyExchange_RECIVE_MODE)
//	{
//		SM2KeyExch_Param->ZA = (unsigned int *)ZB;
//		SM2KeyExch_Param->ZB = (unsigned int *)ZA;
//		SM2KeyExch_Param->PA = (unsigned int *)PB;
//		SM2KeyExch_Param->PB = (unsigned int *)PA;
//		SM2KeyExch_Param->RA = (unsigned int *)RB;
//		SM2KeyExch_Param->RB = (unsigned int *)RA;
//		SM2KeyExch_Param->selfDe = (unsigned int *)selfDe;
//		SM2KeyExch_Param->selfTempDe = (unsigned int *)selfTempDe;
//	}
//	else
//	{
//		while(1);
//	}
//	
//	SM2KeyExch_Param->argPtr = NULL;
//	SM2KeyExch_Param->type = bMode;
//	SM2KeyExch_Param->optMode = 0;
//	SM2KeyExch_Param->keyByteLen = exchangeKeyLen;
//	SM2KeyExch_Param->key = SM2_oData;
//	SM2KeyExch_Param->S1 = NULL;
//	SM2KeyExch_Param->SA = NULL;
//	
//	if (HED_SM2KeyExchangeNew(SM2KeyExch_Param) != 0)
//	{
//			while(1);
//	} 	

//	memCopyNoAtomic(outBuffer, SM2_oData, exchangeKeyLen);
}

#endif
#endif
