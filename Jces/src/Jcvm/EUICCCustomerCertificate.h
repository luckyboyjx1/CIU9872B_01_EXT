#ifndef    _EUICCCUSTOMERCERTIFICATE_H   
#define	   _EUICCCUSTOMERCERTIFICATE_H 
#include "ecpDefine.h"
/**********************certificate define*********************************/
/*
证书结构参考规范:  RFC 5280 4.1.  Basic Certificate Fields

Certificate  ::=  SEQUENCE  
{
        tbsCertificate       TBSCertificate,
        signatureAlgorithm   AlgorithmIdentifier,
        signatureValue       BIT STRING  
}

TBSCertificate  ::=  SEQUENCE  {
        version         [0]  EXPLICIT Version DEFAULT v1,
        serialNumber         CertificateSerialNumber,
        signature            AlgorithmIdentifier,
        issuer               Name,
        validity             Validity,
        subject              Name,
        subjectPublicKeyInfo SubjectPublicKeyInfo,
        issuerUniqueID  [1]  IMPLICIT UniqueIdentifier OPTIONAL,
                             -- If present, version MUST be v2 or v3
		subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL,
                             -- If present, version MUST be v2 or v3
        extensions      [3]  EXPLICIT Extensions OPTIONAL
                             -- If present, version MUST be v3
}
		
Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }

CertificateSerialNumber  ::=  INTEGER

Validity ::= SEQUENCE {
        notBefore      Time,
        notAfter       Time }
Time ::= CHOICE {
        utcTime        UTCTime,
        generalTime    GeneralizedTime }
		
UniqueIdentifier  ::=  BIT STRING

SubjectPublicKeyInfo  ::=  SEQUENCE  {
        algorithm            AlgorithmIdentifier,
        subjectPublicKey     BIT STRING  }
		
Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension

Extension  ::=  SEQUENCE  {
        extnID      OBJECT IDENTIFIER,
        critical    BOOLEAN DEFAULT FALSE,
        extnValue   OCTET STRING
                    -- contains the DER encoding of an ASN.1 value
                    -- corresponding to the extension type identified
                    -- by extnID
        }
		
4.1.1.2.  signatureAlgorithm

AlgorithmIdentifier  ::=  SEQUENCE  
{
        algorithm               OBJECT IDENTIFIER,
        parameters              ANY DEFINED BY algorithm OPTIONAL  
}

4.1.1.3.  signatureValue


Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
Extension  ::=  SEQUENCE  {
     extnID      OBJECT IDENTIFIER,
     critical    BOOLEAN DEFAULT FALSE,
     extnValue   OCTET STRING
                 -- contains the DER encoding of an ASN.1 value
                 -- corresponding to the extension type identified
                 -- by extnID
     }

*/

#define  GetCertDataItemAddr                     GetObjectItemAddr
#define  GetCertData_ArrayItemAddr               GetObject_ArrayItemAddr

#define CACIListGet_nextCIListRefAddr(certListAddr)           GetCertDataItemAddr(certListAddr,nextCIListRef_INDEX)
#define CACIListGet_curCertCIArrayAddr(certListAddr)		  GetCertDataItemAddr(certListAddr,curCertCIArray_INDEX)
#define CACIListGet_pkIdCIArrayAddr(certListAddr)             GetCertDataItemAddr(certListAddr,pkIdCIArray_INDEX)
#define CACIListGet_parameterCIAddr(certListAddr)             GetCertDataItemAddr(certListAddr,parameterCI_INDEX)
#define CACIListGet_pkCIArrayAddr(certListAddr)               GetCertDataItemAddr(certListAddr,pkCIArray_INDEX)

#define CACIListGet_curCertCIArrayValueAddr(certListAddr)	  GetObject_ArrayItemAddr(certListAddr,curCertCIArray_INDEX)
#define CACIListGet_pkIdCIArrayValueAddr(certListAddr)        GetObject_ArrayItemAddr(certListAddr,pkIdCIArray_INDEX)
#define CACIListGet_pkCIArrayValueAddr(certListAddr)          GetObject_ArrayItemAddr(certListAddr,pkCIArray_INDEX)


#define CAEUMAndEUICCListGet_nextEUMAndEUICCListRefAddr(certListAddr)       GetObjectItemAddr(certListAddr,nextEUMAndEUICCListRef_INDEX)
#define CAEUMAndEUICCListGet_EUMAndEUICCCertIndexAddr(certListAddr)         GetObjectItemAddr(certListAddr,EUMAndEUICCCertIndex_INDEX)
#define CAEUMAndEUICCListGet_curCertEUMArrayAddr(certListAddr)              GetObjectItemAddr(certListAddr,curCertEUMArray_INDEX)
#define CAEUMAndEUICCListGet_curCertEUICCArrayAddr(certListAddr)            GetObjectItemAddr(certListAddr,curCertEUICCArray_INDEX)
#define CAEUMAndEUICCListGet_skEUICCArraysAddr(certListAddr)                GetObjectItemAddr(certListAddr,skEUICCArrays_INDEX)
#define CAEUMAndEUICCListGet_parameterEUICCAddr(certListAddr)               GetObjectItemAddr(certListAddr,parameterEUICC_INDEX)

#define CAEUMAndEUICCListGet_curCertEUMArrayValueAddr(certListAddr)         GetObject_ArrayItemAddr(certListAddr,curCertEUMArray_INDEX)
#define CAEUMAndEUICCListGet_curCertEUICCArrayValueAddr(certListAddr)       GetObject_ArrayItemAddr(certListAddr,curCertEUICCArray_INDEX)
#define CAEUMAndEUICCListGet_skEUICCArraysValueAddr(certListAddr)           GetObject_ArrayItemAddr(certListAddr,skEUICCArrays_INDEX)

#define M2MCAEUICCListGet_curCertEUICCArrayAddr(certListAddr)               GetObjectItemAddr(certListAddr,M2MCurCertEUICCArray_INDEX)
#define M2MCAEUICCListGet_skEUICCArrayAddr(certListAddr)                    GetObjectItemAddr(certListAddr,M2MSkEUICCArray_INDEX)
#define M2MCAEUICCListGet_parameterEUICCAddr(certListAddr)                  GetObjectItemAddr(certListAddr,M2MParameterEUICC_INDEX)

#define M2MCAEUICCListGet_curCertEUICCArrayValueAddr(certListAddr)          GetObject_ArrayItemAddr(certListAddr,M2MCurCertEUICCArray_INDEX)
#define M2MCAEUICCListGet_skEUICCArrayValueAddr(certListAddr)               GetObject_ArrayItemAddr(certListAddr,M2MSkEUICCArray_INDEX)

/**********************证书结构以及内部偏移宏定义***********************/
//typedef __packed struct
//{
//	u16 nextListRef;    //下一个CI证书链的引用
//	u16 curCertCIRef;   //当前CI证书的引用，证书的实际存储格式是 2字节证书总长度 + 完整的证书
//	u16 pkIdCIRef;      //PKID的引用， Extensions.Subject Key Identifier  用于AuthenticateServer时的证书查找
//	u8 parameterCI; //算法参数, subjectPublicKeyInfo.algorithm.parameters
//	u8 pkCIValue[64];  //PK Value的值，由于现在只支持ECC256，所以这里public key的长度是固定的
//}CACIList;

//#define CACIList_NextListRef_Offset       0x00
//#define CACIList_CurCertCIRef_Offset      0x02
//#define CACIList_PkIdCIRef_Offset         0x04
//#define CACIList_ParameterCI_Offset    	  0x06
//#define CACIList_PkCIValue_Offset         0x07

#define CACIList_PkIdCI_MAX_Len           0x14 // <rfc5280 4.2.1.2.  Subject Key Identifier> The keyIdentifier is composed of the 160-bit SHA-1 hash of the  value of the BIT STRING subjectPublicKey (excluding the tag, length, and number of unused bits)
                                               //                                            The keyIdentifier is composed of a four-bit type field with the value 0100 followed by the least significant 60 bits of the SHA-1 hash of the value of the BIT STRING subjectPublicKey (excluding the tag, length, and number of unused bits)
//typedef __packed struct
//{
//	u16 nextListRef;     	//下一个EUM和EUICC证书链的引用
//	u8 index;	          	//EUM和EUICC证书链的索引
//	u16 curCertEUMRef;   	//当前EUM证书的引用，证书的实际存储格式是 2字节证书总长度 + 完整的证书
//	u16 curCertEUICCRef; 	//当前EUICC证书的引用，证书的实际存储格式是 2字节证书总长度 + 完整的证书
//	u8 skEUICCValue[33];	//当前EUICC证书SK的值，由于现在只支持ECC256，所以这里SK的长度是固定的，结构为LV，L为一个字节
//	u8 parameterEUICC; //EUICC证书算法参数
//}CAEUMAndEUICCList;

//#define   CAEUMAndEUICCList_nextListRef_Offset           0x00
//#define   CAEUMAndEUICCList_index_Offset                 0x02
//#define   CAEUMAndEUICCList_curCertEUMRef_Offset         0x03
//#define   CAEUMAndEUICCList_curCertEUICCRef_Offset       0x05
//#define   CAEUMAndEUICCList_skEUICCValueOffset           0x07
//#define   CAEUMAndEUICCList_parameterEUICCRefOffset      0x28

//typedef __packed struct
//{
//	u16 curCertEUICCRef; 	//当前EUICC证书的引用，证书的实际存储格式是 2字节证书总长度 + 完整的证书
//	u8 skEUICCValue[33];  //当前EUICC证书SK的值，由于现在只支持ECC256，所以这里SK的长度是固定的，结构为LV，L为一个字节
//	u8 parameterEUICC; //EUICC证书算法参数
//}M2MCAEUICCList;

//#define M2MCAEUICCList_curCertEUICCRef_Offset                     0x00
//#define M2MCAEUICCList_skEUICCValue_Offset                        0x02
//#define M2MCAEUICCList_parameterEUICC_Offset                      0x23

#define TAG_Certificate                                            0x30            

#define TAG_Certificate_tbsCertificate                             0x30      
#define TAG_Certificate_signatureAlgorithm                         0x30
#define TAG_Certificate_signatureValue                             0x03

#define TAG_Certificate_tbsCertificate_version                     0xA0      
#define TAG_Certificate_tbsCertificate_serialNumber                0x02
#define TAG_Certificate_tbsCertificate_signature                   0x30
        #define TAG_Certificate_tbsCertificate_signature_algorithm               0x06    
		#define TAG_Certificate_tbsCertificate_signature_parameters              0x06
#define TAG_Certificate_tbsCertificate_issuer                       0x30
#define TAG_Certificate_tbsCertificate_validity                     0x30
#define TAG_Certificate_tbsCertificate_subject                      0x30
#define TAG_Certificate_tbsCertificate_subjectPublicKeyInfo         0x30
        #define TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm            0x30
		         #define TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm   0x06
				 #define TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters  0x06
		#define TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey     0x03
#define TAG_Certificate_tbsCertificate_issuerUniqueID               0xA1
#define TAG_Certificate_tbsCertificate_subjectUniqueID              0xA2
#define TAG_Certificate_tbsCertificate_extensions                   0xA3
        #define TAG_Certificate_tbsCertificate_extensions_sequenceOf              0x30
		         #define TAG_Certificate_tbsCertificate_extensions_sequence              0x30
				         #define  TAG_Certificate_tbsCertificate_extensions_sequence_extnID     0x06
						 #define  TAG_Certificate_tbsCertificate_extensions_sequence_critical   0x01
						 #define  TAG_Certificate_tbsCertificate_extensions_sequence_extnValue  0x04

/***********************certificate macro********************************/

#define FIELD_Certificate_tbsCertificate_signature_algorithm                        0x00   //查找 tbsCertificate 中的 TAG_Certificate_tbsCertificate_signature_algorithm 所对应的字段是否存在
#define FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm   0x01   //查找 tbsCertificate 中的 TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm 所对应的字段是否存在
#define FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters  0x02   //查找 tbsCertificate 中的 TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters 所对应的字段是否存在
#define FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey      0x03   //查找 tbsCertificate 中的 TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey 所对应的字段是否存在
#define FIELD_Certificate_tbsCertificate_subject_serialNumber                       0x04   //查找 tbsCertificate 中的 TAG_Certificate_tbsCertificate_subject 中 serialNumber 字段    


extern u8 const SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI[8];

/******************ice in extension****************************************/
//参考规范RFC5280
#define  id_ce_authorityKeyIdentifier    0x23
#define  id_ce_subjectKeyIdentifier      0x0E
#define  id_ce_keyUsage                  0x0F
#define  id_ce_certificatePolicies       0x20
#define  id_ce_subjectAltName            0x11
#define  id_ce_cRLDistributionPoints     0x1F
#define  id_ce_cRLNumber                 0x14
#define  id_ce_basicConstraints          0x13
#define  id_ce_nameConstraints           0x1E

/***********************macro define**************************************/
#define  RSPROLE_ERROR                   0x00
#define  RSPROLE_DP_AUTH                 0x01   //该字段的值参与运算，等于ID_RSPROLE 中对应偏移 + 1
#define  RSPROLE_DP_PB                   0x08
#define  RSPROLE_DS_AUTH                 0x0F


#define  EUM_Certificate_Mode            0x00
#define  EUICC_Certificate_Mode          0x01
#define  SK_EUICC_Mode                   0x02

/**********************prepare download中的DownloadErrorCode***********************/
#define ErrorCode_invalidCertificate                                  0x01
#define ErrorCode_invalidSignature                                    0x02
#define ErrorCode_unsupportedCurve                                    0x03


#define RESPONSE_NoError                                              0x00

/*****************《rfc5480.pdf》 2.2节:	ecc公钥的编码格式，分为压缩和非压缩两种*****************/
#define GPCERT_PublicKey_UnCompressed       0x04

/***********************global variable define****************************/
/***********************functioin define**********************************/
extern u8 TBSCertificate_GetField(u8 *certificateAddr,u16 certificateLen, u8 filedMode, u8 *dataBuf);
extern u32 TBSCertificateExtensions_GetField(u8 *certificateAddr,u16 certificateLen, u8 ice_id, u8 *dataBuf,u8* Len);
extern u8 Certificate_GetCERTRole(u8 * dataBuf);
extern u8 Certificate_VerifyValidity(u8 *certificateAddr,u16 certificateLen, u8 mode );
extern u8 Certificate_VerifyValidityOfSignatureAlgorithm(u8 *certificateAddr,u16 certificateLen);
extern u8 Certificate_VerifyValidityOfSignatureValue(u8 *certificateAddr,u16 certificateLen);
extern u8 Certificate_VerifyValidityOfTbsCertificate(u8 *certificateAddr,u16 certificateLen);


extern u32 Certificate_SearchEUICCCerficateByCIPKID(u32 dataAddr, u8 pkIdLen, u16* ref);
extern u32 Certificate_GetEUMEUICCCerInfoByListAddr(u32 listAddr, u8 mode);
extern u32 Certificate_GetCICertAddrByPKID(u8* pkidBuf, u8 pkidLen);
extern u8 GPCertificate_VerifySMSRCert(u8* certBuf, u16 totleLen, u8 keySetType);
extern u8 GPCertificate_Check7F49Valid(u8* Buf);

#endif


