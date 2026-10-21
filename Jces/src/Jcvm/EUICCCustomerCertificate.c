/*
*@author     : xuyz
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放EUICC CUSTOMER 证书相关的操作函数，如验证证书的合法性、验证签名等处理流程
*
*@history     :V1.0 :create @ 20181106  
*/
#include "EUICCCustomerCertificate.h"
#include "ASN1Operation.h"
#include "EUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可
#if (EUICC_ONLY||SE_AND_EUICC)

//以下OID字段来自于SGP22_v2.2
BYTE code ID_RSPROLE[] = {
	0x67,0x81,0x12,0x01,0x02,0x01,0x04,   //2.23.143.1.2.1.4  id-rspRole-dp-auth OBJECT IDENTIFIER ::= {id-rspRole 4} 
	0x67,0x81,0x12,0x01,0x02,0x01,0x05,   //2.23.143.1.2.1.5  id-rspRole-dp-pb OBJECT IDENTIFIER ::=   {id-rspRole 5} 
	0x67,0x81,0x12,0x01,0x02,0x01,0x07,   //2.23.143.1.2.1.7  id-rspRole-ds-auth OBJECT IDENTIFIER ::= {id-rspRole 7} 
};

BYTE code SUBJECTPublicKeyInfo_Alg_Algorithm[] = {0x2A,0x86,0x48,0xCE,0x3D,0x02,0x01};   //1.2.840.10045.2.1
BYTE code SUBJECTPublicKeyInfo_Alg_Parameters_BrainpoolP256r1[] = {0x2B,0x24,0x03,0x03,0x02,0x08,0x01,0x01,0x07}; //1 3 36 3 3 2 8 1 1 7
BYTE code SUBJECTPublicKeyInfo_Alg_Parameters_NISTP256[] = {0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x07}; //1 2 840 10045 3 1 7
BYTE code SUBJECTPublicKeyInfo_Alg_Parameters_FRP256V1[] = {0x2A,0x81,0x7A,0x01}; //1 2 250 1
BYTE code SIGNATURE_Alg_Algorithm[] = {0x2A,0x86,0x48,0xCE,0x3D,0x04,0x03,0x02};   //1.2.840.10045.4.3.2
BYTE code SIGNATURE_Alg_Algorithm_GUOMI[] = {0x2A,0x81,0x1C,0xCF,0x55,0x01,0x83,0x75};
BYTE code SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI[] = {0x2A,0x81,0x1C,0xCF,0x55,0x01,0x82,0x2D};



/*********************************************************************************************************/

//如果想获取证书里的值没有在这两个数组中，则需要应用开发者手动添加这两个数组

/*
*TBSCertificateFieldDepthAndTagValue数组定义说明：
每一行的定义：第一个字节表示该行后续字节数；后续字节分别是T1 T2 T3 C1 C2 C3, 其中T表示TBSCertificate中的某个域，C表示该域在其所处层次的TAG中，与其相同的TAG出现的次数
比如：TAG_Certificate_tbsCertificate_signature 的tag是0x30，与其同层次的TAG依次为0xA0, 0x02, 0x30, 0x30, 0x30, 0x30, 0x30, 0xA3. TAG_Certificate_tbsCertificate_signature是第1个0x30，所以C1的值为0x01
*/

BYTE code TBSCertificateFieldDepthAndTagValue[] =
{
	0x04,TAG_Certificate_tbsCertificate_signature,TAG_Certificate_tbsCertificate_signature_algorithm, 0x01, 0x01,
	0x06,TAG_Certificate_tbsCertificate_subjectPublicKeyInfo, TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm, TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm,5,1,1,
	0x06,TAG_Certificate_tbsCertificate_subjectPublicKeyInfo, TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm, TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters,5,1,2,
	0x04,TAG_Certificate_tbsCertificate_subjectPublicKeyInfo, TAG_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey,5,1,
};

/*
TBSCertificateFieldDepthAndTagValue_Offset数组定义说明：
TBSCertificateFieldDepthAndTagValue数组有多少行，则TBSCertificateFieldDepthAndTagValue_Offset有多少个元素， 
第一个元素的值对应TBSCertificateFieldDepthAndTagValue第一行的起始偏移；第二个元素的值对应TBSCertificateFieldDepthAndTagValue第二行的起始偏移；以此类推...
*/
BYTE code TBSCertificateFieldDepthAndTagValue_Offset[]=
{
	0x00,	
	0x05,
	0x0C,
	0x13,
};


/*
*@function    :TBSCertificate_GetField
*
*@description :获取X509 证书的TBSCertificate域中除extensions以外的字段(包括子字段)，该函数不获取extention中的子字段
*
*@param       :BYTE *certificateAddr  :存储证书的逻辑地址，由于证书的数据量过大，此处传入的地址暂时为E2，传入的证书格式为X509 格式
*@param       :WORD  certificateLen   :证书长度
*@param       :BYTE  filedMode        :指示待获取的字段，该数据会转换成证书的深度和位置进行证书的遍历查找，
*                                      传入的是TBSCertificateFieldDepthAndTagValue_Offset的下标，查找的是对应行的最后一个TAG
*@param       :BYTE *dataBuf          :LV格式的输出结果
*
*@return      :RESPONSE_NoError表示对应的字段存在且合法/others表示其他错误
*
*@author      :lixin
*
*@history     :modify by xuxiaojun@20181205增加filedMode越界处理
*/
BYTE TBSCertificate_GetField(BYTE *certificateAddr,WORD certificateLen, BYTE filedMode, BYTE *dataBuf)
{
	WORD idata i,depth;
	BYTE idata tag;
	WORD idata depthOff, occurOff;
	ASN1Info xdata caTlv;
	
	ASN1_InitTLVInfo(&caTlv, (UWORD)(certificateAddr), certificateLen);					 				
	
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate, FALSE, 1)) 													
	{
		return ErrorCode_invalidCertificate;
	}
	
	ASN1_SetNextDepthToFindTLV(&caTlv);  
	
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate, FALSE, 1))
	{
		return ErrorCode_invalidCertificate;
	}

	ASN1_InitTLVInfo(&caTlv, (UWORD)(caTlv.bufferAddr + caTlv.curValueOff), caTlv.curValueLen);	  //将 TAG_Certificate_tbsCertificate 作为第一级的tag 处理
	
	if(filedMode == FIELD_Certificate_tbsCertificate_subject_serialNumber)  //单独处理该tag,由于该tag中存在 set of类型，不固定位置
	{
		if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate_subject, FALSE, 4) )
		{
			return ErrorCode_invalidCertificate;
		}
		
		ASN1_SetNextDepthToFindTLV(&caTlv);		
		
		while(1)			
		{
			if(TRUE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_SETOF, FALSE, 1))
			{
				ASN1_SetNextDepthToFindTLV(&caTlv);
				
				if(TRUE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_SEQUENCE, FALSE, 1))			
				{
					ASN1_SetNextDepthToFindTLV(&caTlv);
					
					if(TRUE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_OBJECTIDENTIFIER, FALSE, 1))		//0x06
					{
						tag = VMReadByte((UWORD)(caTlv.bufferAddr+caTlv.curValueOff+2));
					
						if (tag == 0x05)    //id-at OBJECT IDENTIFIER ::= { joint-iso-ccitt(2) ds(5) 4 }  id-at-serialNumber      AttributeType ::= { id-at 5 }
						{
							if(TRUE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_PrintableString, FALSE, 1))				//找到eid的tag
							{
								break;
							}
							else
							{
								return ErrorCode_invalidCertificate;
							}
						}
					}
					
					ASN1_ReturnFromDepthToFindTLV(&caTlv);
				}						
				
				ASN1_ReturnFromDepthToFindTLV(&caTlv);
			}
			else
			{
				return ErrorCode_invalidCertificate;
			}
		}
	}
	else
	{
	
		if(filedMode > (sizeof(TBSCertificateFieldDepthAndTagValue_Offset) - 1))  //filedMode越界
		{
			return ErrorCode_invalidCertificate;
		}		

		depth = (TBSCertificateFieldDepthAndTagValue[ TBSCertificateFieldDepthAndTagValue_Offset[filedMode] ]) / 2;   //待查找 tag 在 TAG_Certificate_tbsCertificate 中的深度
		
		depthOff = TBSCertificateFieldDepthAndTagValue_Offset[filedMode] + 1;
		
		occurOff = depthOff + depth;
		
		for(i =0 ; i < depth ;  i++ )
		{
			if(FALSE == ASN1_FindTLV(&caTlv, TBSCertificateFieldDepthAndTagValue[i+depthOff], FALSE, TBSCertificateFieldDepthAndTagValue[i+occurOff]) )													//第1个30， subjectPublicKeyInfo
			{
				return ErrorCode_invalidCertificate;
			}
			if(i == (depth-1))
			{
				break;
			}
			ASN1_SetNextDepthToFindTLV(&caTlv);		
		}
	}
	
	if(dataBuf != NULL)
	{
		dataBuf[0] = caTlv.curValueLen;
				
		VMReadBuf(&dataBuf[1], (UWORD)(caTlv.bufferAddr + caTlv.curValueOff), caTlv.curValueLen);
		
		if(filedMode == FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey) 
		{
			if((dataBuf[0] != 0x42) || (dataBuf[1] != 0x00)|| (dataBuf[2] != 0x04)) //这里需要考虑获取subjectPublicKey时，目前支持的算法的public key的长度为64字节，如果第一个字节为04则表示是不压缩的算法，目前卡片只支持不压缩
			{
				return ErrorCode_unsupportedCurve;
			}
			
			dataBuf[0] = caTlv.curValueLen - 2;
			
			ecpMem_Move(&dataBuf[1], &dataBuf[3], dataBuf[0]);
		}

		if(filedMode == FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters) //将算法参数的OID转换成ECC参数的引用，并且校验算法参数的OID的有效性
		{
			if((sizeof(SUBJECTPublicKeyInfo_Alg_Parameters_NISTP256) == dataBuf[0]) && (EQUAL == ecpMem_Compare(&SUBJECTPublicKeyInfo_Alg_Parameters_NISTP256[0], &dataBuf[1], dataBuf[0])))
			{
				dataBuf[0] = 0x01;
				
				dataBuf[1] = ECCCure_P_256_KeyParaRef;
			}
			else if((sizeof(SUBJECTPublicKeyInfo_Alg_Parameters_BrainpoolP256r1) == dataBuf[0]) && (EQUAL == ecpMem_Compare(&SUBJECTPublicKeyInfo_Alg_Parameters_BrainpoolP256r1[0], &dataBuf[1], dataBuf[0])))
			{
				dataBuf[0] = 0x01;
				
				dataBuf[1] = ECCCure_brainpoolP256r1_KeyParaRef;
			}
			else if((sizeof(SUBJECTPublicKeyInfo_Alg_Parameters_FRP256V1) == dataBuf[0]) && (EQUAL == ecpMem_Compare(&SUBJECTPublicKeyInfo_Alg_Parameters_FRP256V1[0], &dataBuf[1], dataBuf[0])))
			{
				dataBuf[0] = 0x01;
				
				dataBuf[1] = ECCCure_RP256V1_KeyParaRefF;
			}
			else
			{
				return ErrorCode_unsupportedCurve; //算法参数错误
			}
		}
	}

	return RESPONSE_NoError;	
}
/*
*@function    :TBSCertificateExtensions_GetField
*
*@description :从X509证书的 TBSCertificate Extensions字段中获取指定extnID 的数据
*
*@param       :BYTE *certificateAddr  :存储证书的逻辑地址，由于证书的数据量过大，此处传入的地址暂时为E2，传入的证书格式为X509 格式
*@param       :WORD  certificateLen   :证书长度
*@param       :BYTE  ice_id
*@parap       :BYTE  *dataBuf:当指针不为空时，表示要读取对应字段的长度到缓存中，格式为LV格式 
*@parap       :BYTE* Len  当*dataBuf指针为空时，返回需要查找的数据的地址（指向value） ，用该变量取查询数据的长度
*@return      :Addr:对应的字段存在时 返回字段的value处的地址/FALSE:对应的字段不存在
*
*@author      :xuyz
*
*@history     :V1.1 modify by lgy 由于上层调用该函数时使用tempBuf作为参数dataBuf输入 会引起数据被冲掉的问题  将该函数新加一个参数Len， ，当缓存不够时 BYTE *dataBuf传入NULL ，返回数据地址 + len
*/
u32 TBSCertificateExtensions_GetField(u8 *certificateAddr,u16 certificateLen, u8 ice_id, u8 *dataBuf,u8* Len)
{
	BYTE idata tmpId;
	ASN1Info xdata caTlv;
	
	ASN1_InitTLVInfo(&caTlv, (UWORD)(certificateAddr), certificateLen);					 				
	
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate, FALSE, 1)) 													
	{
		return FALSE;
	}
	
	ASN1_SetNextDepthToFindTLV(&caTlv);  
	
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate, FALSE, 1))
	{
		return FALSE;
	}
	
	ASN1_SetNextDepthToFindTLV(&caTlv);  
	
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate_extensions, FALSE, 1))
	{
		return FALSE;
	}
	
	ASN1_SetNextDepthToFindTLV(&caTlv);  
	
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate_extensions_sequenceOf, FALSE, 1))
	{
		return FALSE;
	}
	
	ASN1_InitTLVInfo(&caTlv, (UWORD)(caTlv.bufferAddr + caTlv.curValueOff), caTlv.curValueLen);	//将 TAG_Certificate_tbsCertificate_extensions_sequenceOf 作为第一级的tag 处理
	
	while(1)
	{	
		if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate_extensions_sequence, FALSE, 1)) 		
		{
			return FALSE;
		}
		
		ASN1_SetNextDepthToFindTLV(&caTlv);
		
		if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate_extensions_sequence_extnID, FALSE, 1)) 	
		{
			return FALSE;
		}
		
		tmpId = VMReadByte((UWORD)(caTlv.bufferAddr+caTlv.curValueOff+2));
		
		if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate_extensions_sequence_extnValue,  FALSE, 1))
		{
			return FALSE;
		}
		
		if(tmpId == ice_id)
		{			
			ASN1_SetNextDepthToFindTLV(&caTlv); //执行此函数后会跳到子tag，因此放在上面的话，如果if(tmpId == ice_id)不满足就无法跳至上一级tag继续查找。所以放在这里。
			
			switch (ice_id)
			{
				case id_ce_authorityKeyIdentifier:	
                    /*
				AuthorityKeyIdentifier ::= SEQUENCE {
							keyIdentifier             [0] KeyIdentifier           OPTIONAL,
							authorityCertIssuer       [1] GeneralNames            OPTIONAL,
							authorityCertSerialNumber [2] CertificateSerialNumber OPTIONAL  
                    */				
					
					if(FALSE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_SEQUENCE, FALSE, 1)) 
					{
						return FALSE;
					}
					
					ASN1_SetNextDepthToFindTLV(&caTlv);
					
					if(FALSE == ASN1_FindTLV(&caTlv, 0x80, FALSE, 1))  //查找 keyIdentifier
					{
						return FALSE;
					}			
				
					break;
				
				case id_ce_subjectKeyIdentifier:	

                    // SubjectKeyIdentifier ::= KeyIdentifier	 KeyIdentifier ::= OCTET STRING			
					
					if(FALSE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_OCTETSTRING,  FALSE, 1))
					{
						return FALSE;
					}

					break;
							
				case id_ce_subjectAltName:	
					/*
				SubjectAltName ::= GeneralNames
						GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
						GeneralName ::= CHOICE {
						otherName                       [0]     OtherName,
						rfc822Name                      [1]     IA5String,
						dNSName                         [2]     IA5String,
						x400Address                     [3]     ORAddress,
						directoryName                   [4]     Name,
						ediPartyName                    [5]     EDIPartyName,
						uniformResourceIdentifier       [6]     IA5String,
						iPAddress                       [7]     OCTET STRING,
						registeredID                    [8]     OBJECT IDENTIFIER 
				    */
					
					if(FALSE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_SEQUENCEOF,FALSE, 1)) 		
					{
						return FALSE;
					}
					
					ASN1_SetNextDepthToFindTLV(&caTlv);

					if(FALSE == ASN1_FindTLV(&caTlv, 0x88, FALSE, 1)) 	 //registeredID		
					{
						return FALSE;
					}
				
					break;
				case id_ce_certificatePolicies: 
					/*
						certificatePolicies ::= SEQUENCE SIZE (1..MAX) OF PolicyInformation
						PolicyInformation ::= SEQUENCE {
							policyIdentifier   CertPolicyId,
							policyQualifiers   SEQUENCE SIZE (1..MAX) OF PolicyQualifierInfo OPTIONAL }
						CertPolicyId ::= OBJECT IDENTIFIER
				    */
				
					if(FALSE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_SEQUENCEOF, FALSE, 1)) 
					{
						return FALSE;
					}
					
					ASN1_SetNextDepthToFindTLV(&caTlv);
					
					if(FALSE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_SEQUENCE, FALSE, 1)) 
					{
						return FALSE;
					}
					
					ASN1_SetNextDepthToFindTLV(&caTlv);
					
					if(FALSE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_OBJECTIDENTIFIER, FALSE, 1)) 
					{
						return FALSE;
					}	
					break;
				case id_ce_keyUsage:
					/*
					KeyUsage ::= BIT STRING {
					   digitalSignature        (0),
					   nonRepudiation          (1), -- recent editions of X.509 have
											-- renamed this bit to contentCommitment
					   keyEncipherment         (2),
					   dataEncipherment        (3),
					   keyAgreement            (4),
					   keyCertSign             (5),
					   cRLSign                 (6),
					   encipherOnly            (7),
					   decipherOnly            (8) }
				    */
					if(FALSE == ASN1_FindTLV(&caTlv, ASN1_UNIVERCLASS_BITSTRING,  FALSE, 1))
					{
						return FALSE;
					}
				
					break;
				
				default:
					return FALSE;
			}
			
			if(dataBuf != NULL)
			{
				dataBuf[0] = caTlv.curValueLen;
						
				VMReadBuf(&dataBuf[1], (UWORD)(caTlv.bufferAddr + caTlv.curValueOff), caTlv.curValueLen);		
			}
			else
			{
				Len[0] = caTlv.curValueLen;
			}
			
			return (caTlv.bufferAddr + caTlv.curValueOff);			
		}

		ASN1_ReturnFromDepthToFindTLV(&caTlv);
	}	

//	return FALSE;
}
/*
*@fucnction  : Certificate_GetCERTRole
*
*@description: 通过证书中的Certificate Policies 字段判断当前证书的角色
*
*@param      :BYTE *dataBuf: LV结构
*
@return      :Role
*
*@auth       :xuyz
*
*@history    :
*/
BYTE Certificate_GetCERTRole(BYTE * dataBuf)
{
	BYTE idata i;
	
	for(i = 1;  i< sizeof(ID_RSPROLE); i += 7)
	{
		if((ecpMem_Compare((BYTE *)&ID_RSPROLE[i-1], &dataBuf[1], dataBuf[0]) == EQUAL) &&(dataBuf[0] == 7))   //每个oid 固定为7字节
		{
			return i;  //表示  RSPROLE_DP_AUTH  or RSPROLE_DP_PB or RSPROLE_DS_AUTH
		}
	} 
	
	return RSPROLE_ERROR;
}
/*
*@function    :Certificate_VerifyValidityOfSignatureAlgorithm
*
*@description :验证证书中 signatureAlgorithm 字段的有效性
*
*@see         :<SGP.22_v2.2.pdf> 4.5.2.1.1 Algorithm Identifiers and Parameters
*
*@param       :BYTE *certificateAddr  :存储证书的地址，由于证书的数据量过大，此处传入的地址暂时为E2，传入的证书格式为X509 格式
*@param       :WORD certificateLen    :证书长度
*
*@return      :TRUE/FALSE
*
*@test        :
*
*@author      :xuyz
*
*@history     :modify by lx @20181130
*             :modify by xuxiaojun@20190628  规范书写方式
*/
BYTE Certificate_VerifyValidityOfSignatureAlgorithm(BYTE *certificateAddr,WORD certificateLen)
{
	BYTE xdata dataBuf[sizeof(SIGNATURE_Alg_Algorithm)];
	ASN1Info xdata caTlv;
	
	ASN1_InitTLVInfo(&caTlv, (UWORD)(certificateAddr), certificateLen);					 				
	
 	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate, FALSE, 1)) 													
	{
		return FALSE;
	}
	
	ASN1_SetNextDepthToFindTLV(&caTlv);  
		
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_signatureAlgorithm, FALSE, 2)) 													
	{
		return FALSE;
	}
	
	ASN1_SetNextDepthToFindTLV(&caTlv); 
	
	if(FALSE == ASN1_FindTLV(&caTlv, 0x06, FALSE, 1)) 
	{
		return FALSE;
	}

#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
			if(caTlv.curValueLen != sizeof(SIGNATURE_Alg_Algorithm_GUOMI))
			{
				return FALSE; //长度错误，抛错
			}
		}
		else
#endif
		{
			if(caTlv.curValueLen != sizeof(SIGNATURE_Alg_Algorithm))
			{
				return FALSE; //长度错误，抛错
			}
		}
			
	VMReadBuf(&dataBuf[0], (UWORD)(caTlv.bufferAddr + caTlv.curValueOff), caTlv.curValueLen);		

#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
			if(ecpMem_Compare(&SIGNATURE_Alg_Algorithm_GUOMI[0], &dataBuf[0], caTlv.curValueLen) == EQUAL)
			{
				return TRUE;
			}
		}
		else
#endif
		{
			if(ecpMem_Compare(&SIGNATURE_Alg_Algorithm[0], &dataBuf[0], caTlv.curValueLen) == EQUAL)
			{
				return TRUE;
			}
		}
	
	return FALSE;
}
/*
*@function    :Certificate_VerifyValidityOfSignatureValue
*
*@description :验证证书中 SignatureValue 字段的有效性
*
*@see         :<RFC5280-X.509.pdf> 4.1.1.3. signatureValue
*              <rfc3279.txt> 2.2.3 ECDSA Signature Algorithm
*
*@param       :BYTE *certificateAddr  :存储证书的地址，由于证书的数据量过大，此处传入的地址暂时为E2，传入的证书格式为X509 格式
*@param       :WORD certificateLen    :证书长度
*
*@return      :TRUE/FALSE
*
*@author      :xuyz
*
*@history     :modify by lx @20181130
*             :modify by xuxiaojun@20190621 优化代码
*
*@note        :tempBuf的前192字节用来缓存验签所需要的数据
*/
BYTE Certificate_VerifyValidityOfSignatureValue(BYTE *certificateAddr,WORD certificateLen)
{
	BYTE idata i,curOff;
	ASN1Info xdata caTlv;
 	WORD idata tbsCertificateLen;
 	UWORD xdata tbsCertificateAddr, caAddr;

	if(FALSE == TBSCertificateExtensions_GetField(certificateAddr, certificateLen, id_ce_authorityKeyIdentifier,tempBuf,NULL)) // Extension for Authority Key Identifier: To identify the PK.CI.ECDSA that has to be used to verify this Certificate
	{
		return FALSE;
	}
	
	caAddr = Certificate_GetCICertAddrByPKID(&tempBuf[1],tempBuf[0]); //根据authorityKeyIdentifier查找CI证书的PKID，取得对应CI证书的数据
	
	if(caAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}
		
	ASN1_InitTLVInfo(&caTlv, (UWORD)(certificateAddr), certificateLen);					 				
	
 	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate, FALSE, 1)) 													
	{
		return FALSE;
	}
	
	tbsCertificateAddr = caTlv.bufferAddr+caTlv.curValueOff; //缓存tbsCertificate的起始地址，之后算哈希值的时候用到. <rfc5280> The ASN.1 DER encoded tbsCertificate is used as the input to the signature function
	
	ASN1_SetNextDepthToFindTLV(&caTlv);  
	
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_tbsCertificate, FALSE, 1)) //tbsCertificate											
	{
		return FALSE;
	}
	
	tbsCertificateLen = caTlv.curTLVLen; //缓存tbsCertificate的长度，之后算哈希值的时候用到	
			
	if(FALSE == ASN1_FindTLV(&caTlv, TAG_Certificate_signatureValue, FALSE, 1)) //SignatureValue										
	{
		return FALSE;
	}
					
	if(VMReadByte(caTlv.bufferAddr + caTlv.curValueOff) != 0x00) //BIT STRING 补位不为0，返回错误 . BIT STRING 补位，有效位的长度刚好是8的整数倍，所以尾部无效0的bit数为0，根据ASN.1的编码规范，前面需要补一个00
	{
		return FALSE;
	}
	
	SHA256DoFinalSingleCall_Ex(tbsCertificateAddr, tbsCertificateLen, &tempBuf[129]);  //计算证书tbsCertificate部分的哈希值， 该函数的调用提前是因为此函数内部会使用到tempBuf[0],避免数据被冲
	
	caTlv.curValueLen -= 1; //减去00的1字节长度
	
	caTlv.curValueOff += 1; //偏移到tag 30	
	
	ASN1_InitTLVInfo(&caTlv, (UWORD)(caTlv.bufferAddr + caTlv.curValueOff), caTlv.curValueLen);	//将 30 作为第一级的tag 处理
	
	if(FALSE == ASN1_FindTLV(&caTlv, 0x30, FALSE, 1)) 													
	{
		return FALSE;
	}
		
	ASN1_SetNextDepthToFindTLV(&caTlv);
	
	for(i=0;i<2;i++)
	{				
		if(FALSE == ASN1_FindTLV(&caTlv, 0x02, FALSE, 1)) //02分别为签名值前、后部分的tag											
		{
			return FALSE;
		}
		
		if(caTlv.curValueLen > 0x21)
		{
			return FALSE; 
		}
		
		curOff = VMReadByte(caTlv.bufferAddr + caTlv.curValueOff);
		
		if(curOff > 0x7F)  //第一个字节一定不能大于7F
		{
			return FALSE;
		}
		
		curOff = 64+ i* 0x20;
		
		ecpMem_Fill(&tempBuf[curOff], 0x00, 0x20);  //先清空r s签名的初始值
		
		if(caTlv.curValueLen == 0x21)
		{
			VMReadBuf(&tempBuf[curOff], (UWORD)(caTlv.bufferAddr + caTlv.curValueOff), 0x21);  //读取 前2字节的数据进行检查 
			
			if(tempBuf[curOff] != 0x00)
			{
				return FALSE;
			}
			else
			{
				if(tempBuf[curOff+ 1] < 0x80)  //当填充为0时，integer的编码第2字节应该是 > 7F 
				{
					return FALSE;
				}
			}
			
			ecpMem_Move(&tempBuf[curOff], &tempBuf[curOff +1], 0x20);  //有效数据只是后32字节
		}
		else
		{
			VMReadBuf(&tempBuf[curOff + 0x20-caTlv.curValueLen], (UWORD)(caTlv.bufferAddr + caTlv.curValueOff), caTlv.curValueLen); 
		}
	
	}
	
	VMReadBuf(&tempBuf[0], CACIListGet_pkCIArrayValueAddr(caAddr), 64); //ReadEEPromCache(&tempBuf[0], caAddr+CACIList_PkCIValue_Offset, 64); //读CI的PK value

#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		if(TRUE == VerifySignForSM2(&tempBuf[0], (BYTE *)tbsCertificateAddr, tbsCertificateLen, &tempBuf[64])) //SM2验签
		{
			return TRUE; //签名值正确
		}
	}
	else
#endif
	{
		if(TRUE == VerifySignForECDSA(&tempBuf[0], 64, VMReadByte(CACIListGet_parameterCIAddr(caAddr)), (BYTE *)tbsCertificateAddr, tbsCertificateLen, &tempBuf[64], &tempBuf[96], 0x40, ALG_ECDSA_SHA_256)) //验签
		{
			return TRUE; //签名值正确
		}
	}
	
	return FALSE;
}
/*
*@function    :Certificate_VerifyValidityOfTbsCertificate
*
*@description : 验证证书中 tbsCertificate 字段的有效性
*
*@param       :BYTE *certificateAddr  :存储证书的地址，由于证书的数据量过大，此处传入的地址暂时为E2，传入的证书格式为X509 格式
*@param       :WORD certificateLen    :证书长度
*
*@return      :RESPONSE_NoError表示证书验证通过; others表示其他错误
*
*@author      :xuyz
*
*@history     :modify by lx @20181130
*/
BYTE Certificate_VerifyValidityOfTbsCertificate(BYTE *certificateAddr,WORD certificateLen)
{
	BYTE idata result;
	
	if(TBSCertificateExtensions_GetField(certificateAddr, certificateLen, id_ce_keyUsage, tempBuf,NULL)) 
	{
		if((tempBuf[2] & 0x80) == 0x00)      // The extension 'Key usage' SHALL be set with the value 'digitalSignature'. 
		{
			return ErrorCode_invalidCertificate; //出错返回
		}		
	}
	else
	{
		return ErrorCode_invalidCertificate;
	}
	
	result = TBSCertificate_GetField(certificateAddr, certificateLen,FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey,tempBuf);  //检查证书里面的公钥字段是否存在
	
	if(RESPONSE_NoError != result)
	{
		return result;
	}
		
	result = TBSCertificate_GetField(certificateAddr, certificateLen,FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm, tempBuf);//4.5.2.1.1 For section 'subjectPublicKeyInfo' the following settings SHALL apply: 'AlgorithmIdentifier.algorithm' field SHALL be set to: "iso(1) member-body(2) us(840) ansi-X9-62(10045) keyType(2) ecPublicKey(1)" as defined in RFC 5480. 
	
	if(RESPONSE_NoError == result) 
	{
#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
			if((tempBuf[0] != sizeof(SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI)) || (ecpMem_Compare(&tempBuf[1],&SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI[0],tempBuf[0]) != EQUAL)) //AlgorithmIdentifier.algorithm
			{
				return ErrorCode_invalidCertificate;
			}
		}
		else
#endif
		{
			if((tempBuf[0] != sizeof(SUBJECTPublicKeyInfo_Alg_Algorithm)) || (ecpMem_Compare(&tempBuf[1],&SUBJECTPublicKeyInfo_Alg_Algorithm[0],tempBuf[0]) != EQUAL)) //AlgorithmIdentifier.algorithm
			{
				return ErrorCode_invalidCertificate;
			}
		}
	}
	else
	{
		return result;
	}

#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType != Cert_GUOMI) //国密证书的算法参数是缺省的
#endif
		{
			result = TBSCertificate_GetField(certificateAddr, certificateLen,FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters,tempBuf);  //AlgorithmIdentifier.parameters' field SHALL be set to:  for BrainpoolP256r1  for NIST P-256 For FRP256V1

			if(RESPONSE_NoError != result)
			{
				return result;
			}
		}
	
	result = TBSCertificate_GetField(certificateAddr, certificateLen,FIELD_Certificate_tbsCertificate_signature_algorithm, tempBuf);  //For sections 'signature' and 'signatureAlgorithm' the following settings SHALL apply:  'AlgorithmIdentifier.algorithm' field SHALL be set to: "iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4) ecdsa-with-SHA2(3) ecdsa-with-SHA256(2)" as defined in RFC 5758 [25] and RFC 5759 [26] ?  'AlgorithmIdentifier.parameters' field SHALL be omitted as defined in RFC 5758 [25] section 3.2. 
	
	if(RESPONSE_NoError == result)
	{
#if EUICC__FUNCTION__GUOMI_eSIM
		if(G_CertType == Cert_GUOMI)
		{
			if((tempBuf[0] != sizeof(SIGNATURE_Alg_Algorithm_GUOMI)) || (ecpMem_Compare(&tempBuf[1],&SIGNATURE_Alg_Algorithm_GUOMI[0],tempBuf[0]) != EQUAL))
			{
				return ErrorCode_invalidCertificate;
			}
		}
		else
#endif
		{
			if((tempBuf[0] != sizeof(SIGNATURE_Alg_Algorithm)) || (ecpMem_Compare(&tempBuf[1],&SIGNATURE_Alg_Algorithm[0],tempBuf[0]) != EQUAL))
			{
				return ErrorCode_invalidCertificate;
			}
		}
	}
	else
	{
		return result;
	}	
	
	return RESPONSE_NoError;
}
/*
*@function   : Certificate_VerifyValidity
*
*@description: 验证x509 证书的有效性，主要 适 适用于SGPV2.2  5.7.13  Function (ES10b): AuthenticateServer 5.7.5  Function (ES10b): PrepareDownload  中
*              要求的Verify the validity of the CERT.DPpb.ECDSA (using the ECASD service) as defined in section 4.5.2.2.
*
*@see         :SGP22_V2.2  4.5.2.2  Certificate Verification  5.7.5  Function (ES10b): PrepareDownload  5.7.13  Function (ES10b): AuthenticateServer 
*
*@param       :BYTE *certificateAddr  :存储证书的地址，由于证书的数据量过大，此处传入的地址暂时为E2，传入的证书格式为X509 格式
*@param       :WORD certificateLen    :证书长度
*@param       :BYTE mode : MODE_VERIFYCERT_DPauth( 5.7.13  Function (ES10b): AuthenticateServer )  / MODE_VERIFYCERT_DPpb(5.7.5  Function (ES10b): PrepareDownload)
*
*@return      :RESPONSE_NoError表示证书验证通过; others表示其他错误
*
*@author      :xuyz
*
*@history     :modify by lx @20181130
*             :modify by xuxiaojun@20190621 优化代码
*/
BYTE Certificate_VerifyValidity(BYTE *certificateAddr,WORD certificateLen, BYTE mode )
{
	BYTE idata rspRole,tmpLen, result;
	
	if(TBSCertificateExtensions_GetField(certificateAddr, certificateLen, id_ce_certificatePolicies,tempBuf,NULL))  //4.5.2.2 The extension 'Certificate Policies' SHALL be set with one of the following OID: id-rspRole-dp-auth' to indicate an SM-DP+ Certificate for authentication with the eUICC.
	{
		rspRole = Certificate_GetCERTRole(tempBuf);
		
		if(rspRole == RSPROLE_ERROR)    //卡片收到的证书既不是DPauth  也不是DSauth  也不是DPpb
		{
			return ErrorCode_invalidCertificate;
		}
	}
	else  //Certificate Policies 字段不存在
	{
		return ErrorCode_invalidCertificate;
	}
	
	if(mode == RSPROLE_DP_PB)
	{
		if(rspRole != mode)   //在PrepareDownload 命令中，证书只能是DPpb
		{
			return ErrorCode_invalidCertificate;
		}
	}
	else   //如果证书为DSauth 或 DP auth ，则OID不能为DPpb
	{
		if(rspRole == RSPROLE_DP_PB)
		{
			return ErrorCode_invalidCertificate;
		}
	}
	
	if(rspRole == RSPROLE_DP_AUTH)   //在 5.7.13  Function (ES10b): AuthenticateServer  中检查证书的有效性，此时用于后续profile下载做准备
	{
		if(TBSCertificateExtensions_GetField(certificateAddr, certificateLen, id_ce_subjectAltName, tempBuf,NULL))  //获取SMDP OID : 4.5.2.2 The extension 'subjectAltName' SHALL be set with the SM-DP+ OID. 
		{
			VMWriteBuf(EUICC_AuthenTmpBuf + TempSIMOID_Off, tempBuf, tempBuf[0] + 1) ;  //将DP AUTH 的OID存起来，以便之后检查 DP PB 的OID 与DP AUTH的OID是否相同
		}
		else
		{
			return ErrorCode_invalidCertificate;//报错： SM DP OID 字段不存在
		}		
	}
	else if(rspRole == RSPROLE_DP_PB)
	{
		if(TBSCertificateExtensions_GetField(certificateAddr, certificateLen, id_ce_subjectAltName, tempBuf,NULL))  //Verify that CERT.DPauth.ECDSA and CERT.DPpb.ECDSA belong to the same entity (i.e. same OID in subjectAltName). 
		{		
			if(NvmCmp((BYTE *)(EUICC_AuthenTmpBuf + TempSIMOID_Off), &tempBuf[0], tempBuf[0]+1) == EQUAL)
			{
				//检查 DP PB 的OID 与DP AUth中的相同
			}
			else
			{
				return ErrorCode_invalidCertificate;
			}
		}
		else
		{
			return ErrorCode_invalidCertificate;
		}
		
		if(TBSCertificateExtensions_GetField(certificateAddr, certificateLen, id_ce_authorityKeyIdentifier,tempBuf,NULL))  //Verify that CERT.DPauth.ECDSA and CERT.DPpb.ECDSA trust chain lead to the same GSMA CI certificate
		{
			tmpLen = VMReadByte(EUICC_AuthenTmpBuf + TempCertDPAuthAuthID);  //读取AutherServer中的CI PKID 的长度
			
			if((NvmCmp((BYTE *)(EUICC_AuthenTmpBuf + TempCertDPAuthAuthID+1), &tempBuf[1], tempBuf[0]) == EQUAL) && (tmpLen == tempBuf[0]))  
			{
				//Verify that CERT.DPauth.ECDSA and CERT.DPpb.ECDSA trust chain lead to the same GSMA CI certificate
			}
			else
			{
				return ErrorCode_invalidCertificate;
			}
		}
		else
		{
			return ErrorCode_invalidCertificate;
		}		
		
	}
	
	result = Certificate_VerifyValidityOfTbsCertificate(certificateAddr, certificateLen);
	
	if(result != RESPONSE_NoError) //验证证书中 TbsCertificate 字段的有效性
	{
		return result;
	}
	
	if(Certificate_VerifyValidityOfSignatureAlgorithm(certificateAddr, certificateLen) == FALSE) //验证证书中 SignatureAlgorithm 字段的有效性
	{
		return ErrorCode_invalidCertificate;
	}
	
	if(Certificate_VerifyValidityOfSignatureValue(certificateAddr, certificateLen) == FALSE) //验证证书中 SignatureValue 字段的有效性
	{
		return ErrorCode_invalidCertificate;
	}
	if(rspRole == RSPROLE_DP_AUTH)  //保存证书中的 Extension for Authority Key Identifier 字段，用于在prepareDownload时验证 Verify that CERT.DPauth.ECDSA and CERT.DPpb.ECDSA trust chain lead to the same GSMA CI certificate
	{
		TBSCertificateExtensions_GetField(certificateAddr, certificateLen, id_ce_authorityKeyIdentifier,tempBuf,NULL);
		
		if(tempBuf[0] <= 64)  //将CI Pkid 暂时缓存在 TempCertDPAuthAuthID 字段
		{
			VMWriteBuf(EUICC_AuthenTmpBuf + TempCertDPAuthAuthID, tempBuf, tempBuf[0] +1 ) ; 
		}
		else
		{
			return ErrorCode_invalidCertificate;
		}
	}
	
	if(rspRole == RSPROLE_DP_PB)   //保存 PK.DPbp.ECDSA
	{
		TBSCertificate_GetField((BYTE *)certificateAddr, certificateLen, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey, &tempBuf[0]);  //提取证书里面的公钥值
		
		TBSCertificate_GetField(certificateAddr, certificateLen,FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters,&tempBuf[128]);   //提取证书里面的公钥曲线参数
		
		tempBuf[0] = tempBuf[129];  //保存公钥的曲线参数
		
		VMWriteBuf(EUICC_AuthenTmpBuf + TempPKDPpb_Off, tempBuf, 65) ;   //保存公钥参数 和 PK.DPbp.ECDSA
	}
	
	return RESPONSE_NoError;
}

/*
*@function   :Certificate_SearchEUICCCerficateByeCIPKID
*
*@description:通过CI PKID 查找属于该ci链上的EUICC 证书是否存在，并返回证书链的地址
×
×@param      :UWORD dataAddr---入参，CIPkIDValueAddr  opkid起始地址
*@param      :WORD pkIdLen  ---入参，opkid的长度
*@param      :WORD* ref     ---出参，查找到的EUICC证书链的引用
*
*@return     :UWORD addr    ---查找到的EUICC证书链的地址
*
*@author     :xuyz
*
*@history    :modify by lx @20181224
*            :modify by xuxiaojun@20190613  增加参数，用于获取查找到的EUICC证书链的引用
*@note       :
*/
#if RSP_EUICC
u32 Certificate_SearchEUICCCerficateByCIPKID(u32 dataAddr, u8 pkIdLen, u16* ref)
{
	UWORD xdata CIAddr, listAddr, caAddr,tempAddr;
	WORD xdata caLen;
	BYTE idata tempLen;
	
	tempLen = 0;
	
	CIAddr = Certificate_GetCICertAddrByPKID((BYTE *)dataAddr, pkIdLen); //根据PKID 找到CI证书并返回证书的首地址
	
	if(CIAddr == ADDR_IS_NULL)
	{
		return ADDR_IS_NULL;
	}
	
	CIAddr = CACIListGet_pkIdCIArrayValueAddr(CIAddr);//GetAddrByIndexAddr(CIAddr + CACIList_PkIdCIRef_Offset); //CI的PKID
	
	listAddr = GetAddrAndIndexByIndexAddr(EUICCEUMUICCCARef_Addr, ref);
	
	while(listAddr != ADDR_IS_NULL)
	{
		caAddr = CAEUMAndEUICCListGet_curCertEUMArrayValueAddr(listAddr);//GetAddrByIndexAddr(listAddr + CAEUMAndEUICCList_curCertEUMRef_Offset); //取EUM证书的地址
		
		//VMReadBuf(&caLenBuf[0], caAddr, 0x02); //取EUM证书的长度
		
		caLen = memReadU16(MEMtPM8(caAddr));//BuildWord(&caLenBuf[0]);  modify by xuxiaojun@20220521 代码分层，使用平台层函数，而BuildWord是uicc层定义的
		
		tempAddr = TBSCertificateExtensions_GetField((BYTE *)(caAddr+2),caLen, id_ce_authorityKeyIdentifier, NULL,&tempLen); //查找EUM的鉴权PKID
		
		if((tempAddr == FALSE)||(pkIdLen != tempLen) || (NvmCmp((BYTE*)(CIAddr+1), (BYTE*)tempAddr, tempLen) != EQUAL)) //CI的PKID和EUM的鉴权ID不匹配
		{
			listAddr = GetAddrAndIndexByIndexAddr(CAEUMAndEUICCListGet_nextEUMAndEUICCListRefAddr(listAddr), ref);//GetAddrAndIndexByIndexAddr(listAddr + CAEUMAndEUICCList_nextListRef_Offset, ref);
			
			continue;
		}
		else
		{
			if(ADDR_IS_NULL != CAEUMAndEUICCListGet_curCertEUICCArrayValueAddr(listAddr))//if(ADDR_IS_NULL != GetAddrByIndexAddr(listAddr + CAEUMAndEUICCList_curCertEUICCRef_Offset))
			{			
				return listAddr;
			}
			else
			{
				break;
			}
		}
	}
	
	return ADDR_IS_NULL;
}

/*
*@function   : Certificate_GetEUMEUICCCerInfoByListAddr
*
*@description:通过证书链CAEUMAndEUICCList的地址获取指定的信息，如EUM Cerificate EUICC Cerfifate ,SK EUICC等
*
*@param       :UWORD listAddr --- CAEUMAndEUICCList链表地址
*@param       :BYTE mode      --- EUM_Certificate_Mode/EUICC_Certificate_Mode/SK_EUICC_Mode
*
*@return      :UWORD dataAddr --- 对应的EUM证书/EUICC证书/SK的地址
*
*@author      :lx @20181225
*
*@history     :created by lx@20181225
*/
u32 Certificate_GetEUMEUICCCerInfoByListAddr(u32 listAddr, u8 mode)
{	
	if(mode == EUM_Certificate_Mode)
	{
		//return GetAddrByIndexAddr(listAddr + CAEUMAndEUICCList_curCertEUMRef_Offset); //返回EUM证书的地址
		return CAEUMAndEUICCListGet_curCertEUMArrayValueAddr(listAddr);
	}
	else if (mode == EUICC_Certificate_Mode)
	{
		//return GetAddrByIndexAddr(listAddr + CAEUMAndEUICCList_curCertEUICCRef_Offset); //返回EUICC证书的地址
		return CAEUMAndEUICCListGet_curCertEUICCArrayValueAddr(listAddr);
	}
	else if(mode == SK_EUICC_Mode)
	{
		//return (listAddr + CAEUMAndEUICCList_skEUICCValueOffset); //返回SK的首地址，数据为LV结构，L为一个字节
		return CAEUMAndEUICCListGet_skEUICCArraysValueAddr(listAddr);
	}
	
	return FALSE;
}
#endif
/*
*@function   : Certificate_GetCICertAddrByPKID
*
*@description:根据PKID 找到CI证书并返回证书的首地址
*
*@param       :BYTE* pkidBuf
*@param       :BYTE pkIdLen
*
*@return      :UWORD CICertAddr 返回CACIList对象头地址
*
*@author      :lgy @20181218
*
*@history     :V1.0
*/
u32 Certificate_GetCICertAddrByPKID(u8* pkidBuf, u8 pkidLen)
{
	UWORD xdata CICertAddr,pkidAddr;
	
	CICertAddr = GetAddrByIndexAddr(EUICCCARef_Addr);
	
	while(CICertAddr != ADDR_IS_NULL)
	{
		pkidAddr = CACIListGet_pkIdCIArrayValueAddr(CICertAddr);//GetAddrByIndexAddr(CICertAddr + CACIList_PkIdCIRef_Offset);
		
		if((pkidLen != VMReadByte(pkidAddr)) || (NvmCmp((BYTE*)(pkidAddr+1),pkidBuf,pkidLen) != EQUAL))
		{
			CICertAddr = GetAddrByIndexAddr(CACIListGet_nextCIListRefAddr(CICertAddr));//GetAddrByIndexAddr(CICertAddr + CACIList_NextListRef_Offset);
			
			continue;
		}
		else
		{
			return CICertAddr;
		}
	}
	
	return ADDR_IS_NULL;
}
/*
*@function   : GPCertificate_VerifySMSRCert
*
*@description:验证SM-SR证书的有效性，该过程主要用于建立ISD-R/ISD-P密钥时第一个流程下发SM-SR证书的校验
*
*@param       :BYTE* certBuf   ，7F21 XX XXXXXXX 参考设备版规范4.1.1.8章节 
*@param       :WORD totleLen    输入数据总长度  包含7F21字段以及后续总长度
*
*@return      :BYTE TRUE/FALSE
*
*@author      :lgy @20181219
*
*@history     :V1.0
*/
#if M2M_EUICC
BYTE GPCertificate_VerifySMSRCert(BYTE* certBuf, WORD totleLen, BYTE keySetType)
{
	WORD idata offset ,currentCertLen,tag73Offset,value7F21Offset;
	WORD idata octerNum;
	UWORD xdata CICertAddr,certValueLen;
	
	CICertAddr = ADDR_IS_NULL;
	offset = 0;
	
	if((certBuf[offset] != 0x7F)||(certBuf[offset+1] != 0x21))//证书tag  7F21，判断合法性
	{
		return FALSE;
	}
	
	offset += 2;    // 偏移到7F21的长度起始字节
	
	octerNum = ASN1_GetBERTlvLen((UWORD)&certBuf[offset],(UWORD*)&certValueLen);// 7F21总长度
	
	if((octerNum + 2 + certValueLen) != totleLen) //7F21的总长度和3A01的L不等
	{
		return FALSE;
	}
	
	offset += octerNum; // 偏移到7F21的值域起始字节
	
	value7F21Offset = offset;
	
	if(certBuf[offset++] != 0x93)	//Certificate Serial Number
	{
		return FALSE;
	}
	
	if((certBuf[offset] < 1) || (certBuf[offset] > 16))
	{
		return FALSE;
	}
	
	offset += (certBuf[offset] + 1);
	
	if(certBuf[offset++] != 0x42)//CA Identifier
	{
		return FALSE;
	}
	
	if((certBuf[offset] < 1) || (certBuf[offset] > 16))
	{
		return FALSE;
	}
	
	offset += (certBuf[offset] + 1);
	
	if((certBuf[offset] != 0x5F) || (certBuf[offset+1] != 0x20))//Subject Identifier 5F20
	{
		return FALSE;
	}
	
	offset += 2;
	
	if((certBuf[offset] < 1) || (certBuf[offset] > 16))
	{
		return FALSE;
	}
	
	offset += (certBuf[offset] + 1);
	
	if((certBuf[offset] != 0x95) || (certBuf[offset+1] != 0x01))//Key Usage, Signature Verification
	{
		return FALSE;
	}
	
	offset += 3; //指向‘5F25’或‘5F24’		
	
	octerNum = ASN1_FindTagTLV((UWORD)&certBuf[offset],totleLen - offset,0x5F25);
	
	if(octerNum != TAG_NOT_FOUND)
	{
		if((octerNum != 0)||(certBuf[offset + 2] != 0x04))// 如果‘5F25’存在 ，偏移一定为0  ,长度一定为4
		{
			return FALSE;
		}
		
		offset += (3+4); //指向‘5F24’		
	}
	
	if((ASN1_FindTagTLV((UWORD)&certBuf[offset],totleLen - offset,0x5F24) != 0)||(certBuf[offset + 2] != 0x04))
	{
		return FALSE;
	}
	
	offset += (3+4); //指向"73"
	
	tag73Offset = offset;
	
	if(certBuf[offset++] != 0x73)
	{
		return FALSE;
	}
	
	currentCertLen = certBuf[offset];//缓存73 tag的总长度 ，后续需要处理73的子tag
	
	offset++;
	
	if((certBuf[offset] != 0xC8)||(certBuf[offset+1] != 0x01))//'C8'01‘XX’, denoting an SM-SR certificate，表示是SM-SR/SM-DP证书
	{
		return FALSE;
	}
	
	if(keySetType == ISDR_KeySet)
	{
		if(certBuf[offset+2] != 0x02)  //02’, denoting an SM-SR certificate
		{
			return FALSE;
		}
	}
	else
	{
		if(certBuf[offset+2] != 0x01)  //‘01’,  denoting an  SM-DP certificate
		{
			return FALSE;
		}
	}
	
	offset += 3;//指向"C9"
	
	if((certBuf[offset] != 0xC9)/*||(certBuf[offset+1] != 0x14)*/)
	{
		CICertAddr = GetAddrByIndexAddr(EUICCCARef_Addr);//为兼容SGP02V3.2以下版本描述的证书格式 C9不存在时暂时不抛错  M2M版只有一个CA证书 直接获取该证书地址
	}
	else
	{
		if(certBuf[offset+1] == 0x20)//HB_Offline_Test/5KeyEstablish指令异常测试/Tele_TC2_ISDP_KeyEstablishment_ERROR_01 下发的证书PKID长度为32字节 （此处代码仅针对该用例，测试部说反馈过该问题但是并没有修改）
		{
			CICertAddr = GetAddrByIndexAddr(EUICCCARef_Addr);
		}
		else
		{
			offset++; //指向Authority key identifier LEN
		
			CICertAddr = Certificate_GetCICertAddrByPKID(&certBuf[offset+1],certBuf[offset]);//根据Authority key identifier找CI 并获取CI公钥 用于验签证书中的签名值
		}

	}
	
	if(CICertAddr == ADDR_IS_NULL)
	{
		return FALSE;
	}

	
	offset = tag73Offset + currentCertLen + 2;//指向7F49, <SGP.02_v3.2_updated.pdf> Table 24: Public Key Data Object Data Field
	
	if(GPCertificate_Check7F49Valid(&certBuf[offset]) != TRUE)
	{
		return FALSE;
	}

	offset += 6;//指向密钥值
	
	VMWriteBuf(EUICC_AuthenTmpBuf + TempSMSRCertPublickey_Off,&certBuf[offset],0x40); //缓存密钥值, <Figure 23: SM-SR Change> extract and store PK.SR.ECDSA from CERT.SR.ECDSA
	
	offset += 0x40; //指向Key Parameter Reference
	
	VMWriteBuf(EUICC_AuthenTmpBuf + TempSMSRCertKeyParaRef_Off,&certBuf[offset+2],1); //缓存Key Parameter Reference, F001XX中XX的值
	
	offset += 3;  //指向‘5F37’
	
	if((certBuf[offset] != 0x5F)||(certBuf[offset+1] != 0x37)||(certBuf[offset+2] != 0x40))
	{
		return FALSE;
	}
	
	offset += 3;  //指向‘5F37’的内容
	
	if((offset + 0x40) != totleLen)//5F37是证书的最后一个字段，如果有多余字段认为证书不合法
	{
		return FALSE;
	}
	
//	SHA256DoFinalSingleCall_Ex((UWORD)(certBuf + value7F21Offset),offset - 3 - value7F21Offset ,certBuf);//计算哈希 借用certBuf缓存hash值  会把前32字节冲掉
	
//	VMReadBuf(certBuf + 32,CACIListGet_pkCIArrayValueAddr(CICertAddr),64);//VMReadBuf(certBuf + 32,CICertAddr + CACIList_PkCIValue_Offset,64);//根据CICertAddr  获取CI公钥，曲线参数 ，验证证书的签名
	VMReadBuf(tempBuf,CACIListGet_pkCIArrayValueAddr(CICertAddr),64);//VMReadBuf(certBuf + 32,CICertAddr + CACIList_PkCIValue_Offset,64);//根据CICertAddr  获取CI公钥，曲线参数 ，验证证书的签名

#if EUICC__FUNCTION__GUOMI_eSIM
	if(G_CertType == Cert_GUOMI)
	{
		if(TRUE != VerifySignForSM2(tempBuf, (certBuf + value7F21Offset), (offset - 3 - value7F21Offset), &certBuf[offset])) //SM2验签
		{
			return FALSE;
		}
	}
	else
#endif
	{
		//if(VerifySignForECDSA(certBuf + 32,64,VMReadByte(CICertAddr + CACIList_ParameterCI_Offset), certBuf, 32, &certBuf[offset],&certBuf[offset + 32],64) != TRUE) // verify CERT.SR.ECDSA using PK.CI.ECDSA
		if(VerifySignForECDSA(tempBuf,64,VMReadByte(CACIListGet_parameterCIAddr(CICertAddr)), (certBuf + value7F21Offset), (offset - 3 - value7F21Offset), &certBuf[offset],&certBuf[offset + 32],64, ALG_ECDSA_SHA_256) != TRUE) // verify CERT.SR.ECDSA using PK.CI.ECDSA
		{
			return FALSE;
		}
	}
	
	return TRUE;
}
#endif
/*
*@function   : GPCertificate_Check7F49Valid
*
*@description:验证SM-SR证书7F49字段的有效性 
*
*@param       :BYTE* Buf   ，7F49 XX XXXXXXX 参考设备版规范4.1.1.5章节 
*@see         :<SGP.02_v3.2_updated.pdf> Table 24: Public Key Data Object Data Field
*
*@return      :BYTE TRUE/FALSE
*
*@author      :lgy @20181221
*
*@history     :V1.0
*/
BYTE GPCertificate_Check7F49Valid(BYTE* Buf)
{
	BYTE idata offset;
	
	offset = 0;
	
	if((Buf[offset] != 0x7F)||(Buf[offset+1] != 0x49)||(Buf[offset+2] != 0x46)) // 7F49总长度70字节 (B0 + L + KEY ) +  (F0 + 01 + XX)
	{
		return FALSE;
	}
	
	offset += 3;//指向B0
	
	if((Buf[offset] != 0xB0)||(Buf[offset+1] != 0x41)||(Buf[offset+2] != 0x04))
	{
		return FALSE;
	}
	
	offset += 67;  //65字节密钥 + 2字节 tag + len
	
	if((Buf[offset] != 0xF0)||(Buf[offset+1] != 0x01))
	{
		return FALSE;
	}
	
	return TRUE;
}

#endif

