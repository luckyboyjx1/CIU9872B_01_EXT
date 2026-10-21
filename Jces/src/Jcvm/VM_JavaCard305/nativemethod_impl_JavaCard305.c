#include "nativemethod_impl_JavaCard305.h"
#include "profile.h"
#include "Algorithm_List.h"
#include "nativemethod_impl.h"
#include "stack.h"
#include "SensiTiveResult.h"
#include "Crypto_DH.h"
#include "ob_manager.h"
#include "object.h"
#include "firewall.h"
#include "utilMethod.h" //warning check_bounds_interval

#if WITH_JC305
typedef struct AlgorithmStruct
{
	u8 Algorithm;
	u8 MessageDigest;
	u8 Cipher;
	u8 Padding;
}AlgorithmStruct;

typedef struct CipherAlgority
{
	u8 Algorithm;
	u8 Cipher;
	u8 Padding;
}CipherAlgority;

static const AlgorithmStruct AlgorithmList[]=
{
#if VM_WITH_DES
	{ALG_DES_MAC4_NOPAD,      ALG_NULL,     SIG_CIPHER_DES_MAC4,   PAD_NOPAD},
	{ALG_DES_MAC8_NOPAD,      ALG_NULL,     SIG_CIPHER_DES_MAC8,   PAD_NOPAD},
	{ALG_DES_MAC4_ISO9797_M1, ALG_NULL,     SIG_CIPHER_DES_MAC4,   PAD_ISO9797_M1},
	{ALG_DES_MAC8_ISO9797_M1, ALG_NULL,     SIG_CIPHER_DES_MAC8,   PAD_ISO9797_M1},
	{ALG_DES_MAC4_ISO9797_M2, ALG_NULL,     SIG_CIPHER_DES_MAC4,   PAD_ISO9797_M2},
	{ALG_DES_MAC8_ISO9797_M2, ALG_NULL,     SIG_CIPHER_DES_MAC8,   PAD_ISO9797_M2},
	{ALG_DES_MAC4_ISO9797_1_M1_ALG3, ALG_NULL,    SIG_CIPHER_DES_MAC4,PAD_ISO9797_1_M1_ALG3},
	{ALG_DES_MAC8_ISO9797_1_M1_ALG3, ALG_NULL,    SIG_CIPHER_DES_MAC8,PAD_ISO9797_1_M1_ALG3},
	{ALG_DES_MAC4_ISO9797_1_M2_ALG3, ALG_NULL,    SIG_CIPHER_DES_MAC4,PAD_ISO9797_1_M2_ALG3},
	{ALG_DES_MAC8_ISO9797_1_M2_ALG3, ALG_NULL,    SIG_CIPHER_DES_MAC8,PAD_ISO9797_1_M2_ALG3},
	{ALG_DES_MAC4_PKCS5,      ALG_NULL,     SIG_CIPHER_DES_MAC4,   PAD_PKCS5},
	{ALG_DES_MAC8_PKCS5,      ALG_NULL,     SIG_CIPHER_DES_MAC8,   PAD_PKCS5},
	{ALG_DSA_SHA,             ALG_SHA,      SIG_CIPHER_DSA,        PAD_NULL},
#endif
#if VM_WITH_RSA
	{ALG_RSA_SHA_ISO9796,     ALG_SHA,      SIG_CIPHER_RSA,        PAD_ISO9796},
	{ALG_RSA_SHA_PKCS1,       ALG_SHA,      SIG_CIPHER_RSA,        PAD_PKCS1},
	{ALG_RSA_SHA_PKCS1_PSS,   ALG_SHA,      SIG_CIPHER_RSA,        PAD_PKCS1_PSS},
	{ALG_RSA_SHA_256_PKCS1_PSS,      ALG_SHA_256, SIG_CIPHER_RSA,  PAD_PKCS1_PSS},
	{ALG_RSA_SHA_384_PKCS1_PSS,      ALG_SHA_384, SIG_CIPHER_RSA,  PAD_PKCS1_PSS},
	{ALG_RSA_SHA_512_PKCS1_PSS,      ALG_SHA_512, SIG_CIPHER_RSA,  PAD_PKCS1_PSS},
	{ALG_RSA_MD5_PKCS1,       ALG_MD5,      SIG_CIPHER_RSA,        PAD_PKCS1},
	{ALG_RSA_MD5_PKCS1_PSS,   ALG_MD5,      SIG_CIPHER_RSA,        PAD_PKCS1_PSS},
	{ALG_RSA_MD5_RFC2409,     ALG_MD5,      SIG_CIPHER_RSA,        PAD_RFC2409},
	{ALG_RSA_RIPEMD160_ISO9796, ALG_RIPEMD160, SIG_CIPHER_RSA,     PAD_ISO9796},
	{ALG_RSA_RIPEMD160_PKCS1, ALG_RIPEMD160,   SIG_CIPHER_RSA,     PAD_PKCS1},
	{ALG_RSA_RIPEMD160_PKCS1_PSS, ALG_RIPEMD160, SIG_CIPHER_RSA,   PAD_PKCS1_PSS},
	{ALG_RSA_SHA_224_PKCS1,   ALG_SHA_224,  SIG_CIPHER_RSA,        PAD_PKCS1},
	{ALG_RSA_SHA_224_PKCS1_PSS,   ALG_SHA_224,   SIG_CIPHER_RSA,   PAD_PKCS1_PSS},
	{ALG_RSA_SHA_256_PKCS1,   ALG_SHA_256,  SIG_CIPHER_RSA,        PAD_PKCS1},
	{ALG_RSA_SHA_384_PKCS1,   ALG_SHA_384,  SIG_CIPHER_RSA,        PAD_PKCS1},
	{ALG_RSA_SHA_512_PKCS1,   ALG_SHA_512,  SIG_CIPHER_RSA,        PAD_PKCS1},
	{ALG_RSA_SHA_RFC2409,     ALG_SHA,      SIG_CIPHER_RSA,        PAD_RFC2409},
	#if WITH_SIGNMR
	{ALG_RSA_SHA_ISO9796_MR,  ALG_SHA,      SIG_CIPHER_RSA,        PAD_ISO9796_MR},
	{ALG_RSA_RIPEMD160_ISO9796_MR,   ALG_RIPEMD160, SIG_CIPHER_RSA,PAD_ISO9796_MR},
	#endif
#endif
#if VM_WITH_KOREANSEED
	{ALG_KOREAN_SEED_MAC_NOPAD, ALG_NULL,   SIG_CIPHER_KOREAN_SEED_MAC, PAD_NOPAD},
#endif
#if VM_WITH_AES
	{ALG_AES_MAC_128_NOPAD,   ALG_NULL,     SIG_CIPHER_AES_MAC128, PAD_NOPAD},
#endif
#if VM_WITH_ECDSA
	{ALG_ECDSA_SHA,           ALG_SHA,      SIG_CIPHER_ECDSA,      PAD_NULL},
	{ALG_ECDSA_SHA_224,       ALG_SHA_224,  SIG_CIPHER_ECDSA,      PAD_NULL},
	{ALG_ECDSA_SHA_256,       ALG_SHA_256,  SIG_CIPHER_ECDSA,      PAD_NULL},
	{ALG_ECDSA_SHA_384,       ALG_SHA_384,  SIG_CIPHER_ECDSA,      PAD_NULL},
	{ALG_ECDSA_SHA_512,       ALG_SHA_512,  SIG_CIPHER_ECDSA,      PAD_NULL},
#endif
#if VM_WITH_HMAC
	{ALG_HMAC_MD5,            ALG_MD5,      SIG_CIPHER_HMAC,       PAD_NULL},
	{ALG_HMAC_SHA1,           ALG_SHA,      SIG_CIPHER_HMAC,       PAD_NULL},
	{ALG_HMAC_SHA_256,        ALG_SHA_256,  SIG_CIPHER_HMAC,       PAD_NULL},
	{ALG_HMAC_SHA_384,        ALG_SHA_384,  SIG_CIPHER_HMAC,       PAD_NULL},
	{ALG_HMAC_SHA_512,        ALG_SHA_512,  SIG_CIPHER_HMAC,       PAD_NULL},
	{ALG_HMAC_RIPEMD160,      ALG_RIPEMD160,SIG_CIPHER_HMAC,       PAD_NULL},
#endif
};
static const CipherAlgority CipherAlgorityList[]=
{
	{ALG_AES_BLOCK_128_CBC_NOPAD,  CIPHER_AES_CBC,  PAD_NOPAD},
	{ALG_AES_BLOCK_128_ECB_NOPAD,  CIPHER_AES_ECB,  PAD_NOPAD},
	{ALG_AES_CBC_ISO9797_M1,       CIPHER_AES_CBC,  PAD_ISO9797_M1},
	{ALG_AES_CBC_ISO9797_M2,       CIPHER_AES_CBC,  PAD_ISO9797_M2},
	{ALG_AES_CBC_PKCS5,            CIPHER_AES_CBC,  PAD_PKCS5},
	{ALG_AES_ECB_ISO9797_M1,       CIPHER_AES_ECB,  PAD_ISO9797_M1},
	{ALG_AES_ECB_ISO9797_M2,       CIPHER_AES_ECB,  PAD_ISO9797_M2},
	{ALG_AES_ECB_PKCS5,            CIPHER_AES_ECB,  PAD_PKCS5},
	{ALG_AES_CCM,                  CIPHER_AES_CCM,  PAD_NULL},
	{ALG_AES_GCM,                  CIPHER_AES_GCM,  PAD_NULL},
	{ALG_DES_CBC_ISO9797_M1,       CIPHER_DES_CBC,  PAD_ISO9797_M1},
	{ALG_DES_CBC_ISO9797_M2,       CIPHER_DES_CBC,  PAD_ISO9797_M2},
	{ALG_DES_CBC_NOPAD,            CIPHER_DES_CBC,  PAD_NOPAD},
	{ALG_DES_CBC_PKCS5,            CIPHER_DES_CBC,  PAD_PKCS5},
	{ALG_DES_ECB_ISO9797_M1,       CIPHER_DES_ECB,  PAD_ISO9797_M1},
	{ALG_DES_ECB_ISO9797_M2,       CIPHER_DES_ECB,  PAD_ISO9797_M2},
	{ALG_DES_ECB_PKCS5,            CIPHER_DES_ECB,  PAD_PKCS5},
	{ALG_DES_ECB_NOPAD,            CIPHER_DES_ECB,  PAD_NOPAD},
	{ALG_KOREAN_SEED_CBC_NOPAD,    CIPHER_KOREAN_SEED_CBC, PAD_NOPAD},
	{ALG_KOREAN_SEED_ECB_NOPAD,    CIPHER_KOREAN_SEED_ECB, PAD_NOPAD},
	{ALG_RSA_NOPAD,                CIPHER_RSA,      PAD_NOPAD},
	{ALG_RSA_PKCS1,                CIPHER_RSA,      PAD_PKCS1},
	{ALG_RSA_PKCS1_OAEP,           CIPHER_RSA,      PAD_PKCS1_OAEP},
};

/*
 * function name:   sensitiveResult
 * function:        处理sensitiveresult的总函数
 */
void sensitiveResult(void)
{
	#if SENSITIVE_RESULT_SUPPORT

	u8 type;
	u16 obj;
	u16 val;

	val = popShort();
	obj = popReference();
	type = popShort();

	switch(type)
	{
		case AssertEqualsObj:
			sensitiveResultAssertEqualsObj(obj);
		  break;
		case AssertEquals:
				sensitiveResultAssertEquals(val);
			  break;
		case AssertTrue:
				sensitiveResultAssertTrue();
			  break;
		case AssertFalse:
			sensitiveResultAssertFalse();
		  break;
		case AssertGreaterThan:
			sensitiveResultAssertGreaterThan(val);
		  break;
		case AssertLessThan:
			sensitiveResultAssertLessThan(val);
		  break;
		case AssertNegative:
			sensitiveResultAssertNegative();
		  break;
		case AssertPositive:
			sensitiveResultAssertPositive();
		  break;
		case AssertZero:
		  sensitiveResultAssertZero();
		  break;
		case Reset:
			sensitiveResultReset();
		  break;
    default:
			break;
  }
	#endif
}
/*
 *function name:    arrayFill
 *function:         使用指定的值填充指定数组指定长度个单位
 */
void arrayFill(void)
{
	PM8 t;
	jbyte bvalue;
    jshort length;
    jshort destOff;
    jref dest;
    PM8 destaddr;
    u16 result;

	bvalue = (jbyte)popShort();
	length = popShort();
	destOff = popShort();
	dest = popReference();

	destaddr = MEMtPM8(locateObjectInMemory(dest));

	check_null(destaddr);

#if JCVM__VM__FIRE_WALL
	checkArrayRead(destaddr);
#endif  //__FIRE_WALL__

    check_bounds_interval(destaddr, destOff, length);
#if SENSITIVE_RESULT_SUPPORT
	sensitiveResultSetTagValUnassigned();
#endif
	t	= PM8_A_U16(MEMtPM8(get_array_data(destaddr)), destOff);

	memFillAtomic(t,bvalue,length);

    result = (U32t16(U16_A_U16(destOff, length)));
#if SENSITIVE_RESULT_SUPPORT
	sensitiveResultSet(result);
#endif
	pushShort(result);
}

/*
 * function name:   GetSignatureAlgorithm
 * function:        根据输入数据获取对应的算法‘hash算法‘模式或者填充方式
 */
void GetSignatureAlgorithm(void)
{
	u8 Mode;
	u8 Algorithm;
	u8 MessageDigest;
	u8 Cipher;
	u8 Padding;
	u8 Result;
	u8 i;

	Mode = U16t8(popShort());
	Padding = U16t8(popShort());
	Cipher = U16t8(popShort());
	MessageDigest = U16t8(popShort());
	Algorithm = U16t8(popShort());
	Result = 0;

	switch(Mode)
	{
		case GET_ALGORITHM:
			for(i = 0; i < (sizeof(AlgorithmList)/4); i++)
			{
				if((MessageDigest == AlgorithmList[i].MessageDigest) && (Cipher == AlgorithmList[i].Cipher) && (Padding == AlgorithmList[i].Padding))
				{
					Result = AlgorithmList[i].Algorithm;
				}
			}
			break;
		case GET_MESSAGEDIGEST:
			for(i = 0; i < (sizeof(AlgorithmList)/4); i++)
			{
				if(Algorithm == AlgorithmList[i].Algorithm)
				{
					Result = AlgorithmList[i].MessageDigest;
				}
			}
			break;
		case GET_CIPHER:
			for(i = 0; i < (sizeof(AlgorithmList)/4); i++)
			{
				if(Algorithm == AlgorithmList[i].Algorithm)
				{
					Result = AlgorithmList[i].Cipher;
				}
			}
			break;
		case GET_PADDING:
			for(i = 0; i < (sizeof(AlgorithmList)/4); i++)
			{
				if(Algorithm == AlgorithmList[i].Algorithm)
				{
					Result = AlgorithmList[i].Padding;
				}
			}
			break;
		default:
			break;
	}
	pushShort((jshort)Result);
}
/*
 * function name:       GetCipherAlgorithm
 * function:            根据输入数据获取对应的算法‘模式或者填充方式，输入数据是算法‘模式或者填充方式
 */
void GetCipherAlgorithm(void)
{
	u8 Mode;
	u8 Padding;
	u8 Cipher;
	u8 Algorithm;
	s8 Result;
	u8 i;

	Mode = U16t8(popShort());
	Padding = U16t8(popShort());
	Cipher = U16t8(popShort());
	Algorithm = U16t8(popShort());
	Result = 0;

	switch(Mode)
	{
		case GET_ALGORITHM:
			for(i = 0; i < (sizeof(CipherAlgorityList)/3); i++)
			{
				if((Cipher == CipherAlgorityList[i].Cipher) && (Padding == CipherAlgorityList[i].Padding))
				{
					Result = CipherAlgorityList[i].Algorithm;
				}
			}
			break;
		case GET_CIPHER:
			for(i = 0; i < (sizeof(CipherAlgorityList)/3); i++)
			{
				if(Algorithm == CipherAlgorityList[i].Algorithm)
				{
					Result = CipherAlgorityList[i].Cipher;
				}
			}
			break;
		case GET_PADDING:
			for(i = 0; i < (sizeof(CipherAlgorityList)/3); i++)
			{
				if(Algorithm == CipherAlgorityList[i].Algorithm)
				{
					Result = CipherAlgorityList[i].Padding;
				}
			}
			break;
		default:
			break;
	}

	pushShort((jshort)Result);
}

/*
 * function name:      storeDHKeyRef
 * function:           保存DH算法各参数的地址
 */
void storeDHKeyRef(void)
{
	#if VM_WITH_DH
	u16		X_A keyType;
	u16		X_A keybitlength;
	PM8 keyAddr;

	keyType = (u8)popShort();
	keybitlength = popShort();
	keyAddr = (u8 *)getJavaObjectDataArray();
	g_DHKeyStore.wPrimeLength = keybitlength>>3;

	if (keyType == TYPE_DH_X)
	{
		g_DHKeyStore.pbParmX = keyAddr;

	}else if(keyType == TYPE_DH_P)
	{
		g_DHKeyStore.pbParmP = keyAddr;

	}else if(keyType == TYPE_DH_G)
	{
		g_DHKeyStore.pbParmG = keyAddr;
	}else if(keyType == TYPE_DH_Y)
	{
		g_DHKeyStore.pbParmY = keyAddr;
	}
	#endif
}
/*
 * function name:    setDHDefaultPAndG
 * function：                     设置DH算法默认曲线
 */
void setDHDefaultPAndG(void)
{
	#if VM_WITH_DH
	PM8 G_Arr;
	PM8 P_Arr;
	u16 keyLen;

	keyLen = popShort();
	P_Arr = (u8 *)getJavaObjectDataArray();
	G_Arr = (u8 *)getJavaObjectDataArray();

	if(keyLen == LENGTH_DH_1024)
	{
		memCopyNoAtomic(P_Arr,(PM8)DHPparam_Lenth1024,LENGTH_DH_1024);
		memCopyNoAtomic(G_Arr,(PM8)DHGparam_Lenth1024,LENGTH_DH_1024);
	}
	else if(keyLen == LENGTH_DH_2048)
	{
		memCopyNoAtomic(P_Arr,(PM8)DHPparam_Lenth2048,LENGTH_DH_2048);
		memCopyNoAtomic(G_Arr,(PM8)DHGparam_Lenth2048,LENGTH_DH_2048);
	}
	#endif
}
/*
 * function name:   DH
 * function:        生成DH算法的私钥和公钥
 */
void DH(void)
{
	#if VM_WITH_DH
	u16 					rtn;

	u16 pubOffset;
	u16 gArrOffset;
	u8* pubKey;
	u16 keyLeng;
  u8* P_Arr;
	u8* privKey;
	u8* G_Arr;

	PATCH_HAL_CRYPTO_RSA_HAL_002();

	pubOffset = popShort();
	pubKey = (u8 *)getJavaObjectDataArray();
	keyLeng = popShort();
	gArrOffset = popShort();
	G_Arr = (u8 *)getJavaObjectDataArray();

	G_Arr = G_Arr + gArrOffset;
	pubKey = pubKey + pubOffset;
	privKey = g_DHKeyStore.pbParmX;
	P_Arr = g_DHKeyStore.pbParmP;

	rtn = ALG_ERROR;

	rtn = genDHPubKey(privKey,pubKey,G_Arr,keyLeng,P_Arr);
//	Random_number_generator(privKey,keyLeng,ALG_SECURE_RANDOM);

//  rtn = DHOperation(privKey,G_Arr,keyLeng,P_Arr,pubKey);

	if(ALG_OK == rtn)
	{
		pushShort(keyLeng);
	}
	else
	{
		while(1);
	}
	#endif
}
/*
 * function name:     genDHPriKeyX
 * function:          获取DH算法的key x
 */
void genDHPriKeyX()
{
	#if VM_WITH_DH
	u16 keyLeng;
	u8 tempBuf[256];
	keyLeng = popShort();

	Random_number_generator(tempBuf,keyLeng,ALG_SECURE_RANDOM);
	memCopyNoAtomic(g_DHKeyStore.pbParmX,tempBuf,keyLeng);
	#endif
}
/*
 * function name:         setSensitiveArryObj
 * function:              将数组设置为sensitivearray
 */
void setSensitiveArryObj(void)
{
	jref arrayref;
	memref arrayrefaddr;
	u8 objType;

	arrayref = popReference();
	arrayrefaddr = locateObjectInMemory(arrayref);

	objType = get_object_header_type(arrayrefaddr);
	memWriteU8Atomic((u8 *)arrayrefaddr, U8_OR_U8(objType, HEADER_SENSITIVEARRY_Yes));

	memRefresh();
}
/*
 * function name:    isSensitiveArryObj
 * function:         判断是否为sensitivearray
 */
void isSensitiveArryObj(void)
{
	jref arrayref;
	memref arrayrefaddr;

	arrayref = popReference();

	arrayrefaddr = locateObjectInMemory(arrayref);
	check_null((u8 *)arrayrefaddr);

	if(IS_SENSITIVEARRY_OBJ(arrayrefaddr) )
	{
		pushShort(true);
	}
	else
	{
		pushShort(false);
	}
}
/*
 * function name:    clearArray
 * function:         将数组内容清0
 */
void clearArray()
{
	PN8		D_A		arrayaddr;
	u16 	D_A		arrayLength;
	u8 		D_A		arrayType;
	u8 		D_A		objectSize;

	arrayaddr	= 	getJavaObjectDataRef();	// pop arrayref
#if JCVM__VM__NULLOBJ_CHECK
	check_null(arrayaddr);
#endif

#if JCVM__VM__FIRE_WALL
	checkArrayRead(arrayaddr);
#endif
	arrayType 	=	get_object_header_type(arrayaddr);
	arrayLength = get_array_length(arrayaddr);
	arrayaddr		= MEMtPN8(get_array_data(arrayaddr));
	arrayType	= 	U8_AND_U8(arrayType, OBJ_HDR_OBJECT_TYPE_MASK);
	switch(arrayType)
	{
		case Header_ArrayType_Byte:
		case Header_ArrayType_Boolean:
			objectSize = 1;
			break;
		case Header_ArrayType_Short:
		case Header_ArrayType_Reference:
			objectSize = 2;
			break;
		case Header_ArrayType_Int:
			objectSize	= SIZE_INT;
			break;
		default:
			faultDetected();
		break;
	}
	memFillNoAtomic(arrayaddr, 0, U16_M_U32(objectSize, arrayLength));
	pushShort(arrayLength);
}
#endif /*WITH_JC305*/
