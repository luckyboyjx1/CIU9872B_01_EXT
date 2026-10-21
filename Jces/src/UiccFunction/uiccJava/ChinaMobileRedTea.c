/*
移动红茶对store data 指令格式，以 tag 为 BF7F 的标识为核心数据进行解密写入
 */ 
#include "hard_api.h" 
#include "ChinaMobileRedTea.h"
#include "AES.h"
//#include "CMAC.h"
#include "typedef.h"
#include "Interface_LTEFromPlatform.h"
//#include  "main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "gsmpublic.h"
//#include "NativeMethod.h"
#include "T0ApduImpl.h"
#include "profileApi.h"
#include "ASN1Operation.h"
#include "EUICCCustomer.h"

#if RSP_EUICC

#if  EUICC__FUNCTION_RedTea_Mobile

#define RB_128BITS     0x87       //AES,b=128。TDEA,b=64
//BYTE code RootKeyValue[16] = {0x3D,0x95,0x00,0x38,0x0E,0x02,0x28,0x00,0x00,0x86,0x60,0xBE,0xA0,0x00,0x00,0x00};
/*
*function    : GetEuiccAESKeyValue
*
*@description:  用根密钥对EID 进行AES CBC得到EUICC密钥
				
*
*@param      :void

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/
void GetEuiccAESKeyValue(BYTE *rootKeyValue,BYTE keylength,BYTE *outputKeyValue)
{
	BYTE idata eid[0x11];
	
	IF_Get_EUICCEID(eid);
	
	if(CheckAesKeyLength(keylength)  == FALSE)
	{
		//return FALSE;          //外层控制  不会走这里
	}
	
	KeyExpansion(rootKeyValue,&tempBuf[270]);     //用后半部分存扩展密钥
	
	AESEncryptCore(&eid[1], outputKeyValue,&tempBuf[270]);
}
/*
*function    : GetEuiccAESCMACValue
*
*@description:  计算AES CMAC,采取强制补位 1-16字节,传入数据为AES_BLOCKSIZE块整数倍则补位16字节的800000.....
				
*
*@param      :input 输入 RAM  

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/
void GetEuiccAESCMACValue(BYTE *input,BYTE inputlength,BYTE *output,BYTE *keyContext)
{
	BYTE idata i,k,n,lastLgth;
	BYTE xdata keyValue[0x10];				//缓存K  目前长度为0x10
	BYTE xdata SubkeyValue[0x10];				//缓存Sub Key(K1/K2公用)
	BYTE xdata aesBuf[16];
	BYTE xdata cpResult[0x10];
	
	n = inputlength / AES_BLOCKSIZE;
	
	n++;     //强制补位
	
	lastLgth =inputlength % AES_BLOCKSIZE;
	
	ecpMem_Set(aesBuf,0x00,sizeof(aesBuf));       //暂时用作计算L的初始buf
	
	AESEncryptCore(aesBuf, aesBuf,keyContext);
	
	moveOneBitLeft(aesBuf,0x10,SubkeyValue);    // if MSB1(L) = 0,then K1=L<<1;else,Else K1 = (L << 1) ⊕ Rb,Rb=R128=0(120)10000111=0x(0000......87)
	
	if((aesBuf[0]&0x80) != 0x00)       //计算K1，不管Mlen是不是B的整数倍，都是要先计算K1，Mlen不是B的整数倍的时候，要通过K1计算出K2
	{
		SubkeyValue[AES_BLOCKSIZE-1] ^= RB_128BITS;
	}
	
	if(lastLgth != 0x00)       //If Mn* is a complete block,let Mn = K1 ⊕ Mn*; else, let Mn = K2 ⊕ (Mn*||10j),where j = nb-Mlen-1.
	{
		i = SubkeyValue[0] & 0x80;        //暂用I
		
		ecpMem_Copy(aesBuf,SubkeyValue,0x10);
		
		moveOneBitLeft(aesBuf,0x10,SubkeyValue);  //If MSB1(K1) = 0, then K2 = K1 << 1;Else K2 = (K1 << 1) ⊕ Rb.
		
		if(i != 0x00)
		{
			SubkeyValue[AES_BLOCKSIZE-1] ^= RB_128BITS;
		}
	}
	
	ecpMem_Set(cpResult,0x00,sizeof(cpResult));
	
	for(i = 0x01;i<=n;i++)
	{
		if(i != n)
		{
			 ecpMem_Move(&aesBuf[0],&input[(i-1)*AES_BLOCKSIZE],AES_BLOCKSIZE);
		}
		else
		{
			ecpMem_Set(keyValue,0x00,0x10);
							
			keyValue[0] |= 0x80;
			
			ecpMem_Move(&aesBuf[0],&input[(i-1)*AES_BLOCKSIZE],lastLgth); //读取最后的数据  lastLgth可能为0x00
			
			for(k=0x00;k<0x10;k++)  //最后一块需要与K1/K2异或，这里K1/K2同一个buf
			{
				cpResult[k] ^= SubkeyValue[k];
			}
			
			ecpMem_Copy(&aesBuf[lastLgth],keyValue,0x10-lastLgth); //拷贝补位数据
		}
		
		for(k=0x00;k<0x10;k++)
		{
			aesBuf[k] ^= cpResult[k];
		}
		
		AESEncryptCore(aesBuf, cpResult , keyContext );  // AES-ECB	
	}
	
	ecpMem_Copy(output,cpResult,0x08);    //这里拷贝计算结果  取多长看函数调用

	return ;
}
/*
*function    : WriteplmnTFBuf子函数WriteCoreTFBuf
*
*@description:  更新plmn参数
				
*

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/
BYTE WriteCoreTFBuf(WORD DFName, WORD EFName,BYTE Offse, BYTE *input, BYTE len)
{
	BYTE idata result;
	UWORD xdata coreAddr;
	
	coreAddr = SelectTFFile(DFName,EFName); // 
	
	if(coreAddr == FALSE)
	{
		return FALSE;
	}
	
	result = WriteTFBuf(coreAddr, Offse, input, len);
	
	return result;
}
/*
*function    : WriteplmnTFBuf
*
*@description:  更新plmn参数
				
*

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/
WORD WriteplmnTFBuf(UWORD srcAddr,WORD dfName,WORD efName,BYTE plmnLen)
{
	BYTE idata lLen,result;
	WORD idata desfilesize,requestTotalLen;
	UWORD xdata desAddr;
	
	lLen = ASN1_GetBERTlvLen(srcAddr, (UWORD*)&requestTotalLen);  //tag的数据长度

	if(requestTotalLen % plmnLen)
	{
		return FALSE;
	}
	
	if(requestTotalLen>sizeof(tempBuf))
	{
		desAddr = SelectTFFile(dfName,efName); // 
		if(desAddr == FALSE)
		{
			return FALSE;
		}
		
		desfilesize = GetTFSize(desAddr);
			
		result = VMCopyDataToAddr(desAddr+sizeof(TranFileHead), desfilesize, (srcAddr+lLen+1), requestTotalLen);
		
		if(result == FALSE)
		{
			return FALSE;
		}
	}
	else
	{					
	
		VMReadBuf(tempBuf, (srcAddr+lLen),requestTotalLen);//读取V
		
		result = WriteCoreTFBuf(dfName, efName,0x00, tempBuf, requestTotalLen);
	 
		if(result == FALSE)
		{
			return FALSE;
		}
	}
	
	return (lLen+requestTotalLen+1);
	
}
/*
*function    : InitplmnTFBuf
*
*@description:  初始化plmn参数,除了rpmn
				
*

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/
BYTE code initplmnfile[] = {0xFF,0xFF,0xFF,0x00,0x00};
void InitplmnTFBuf(WORD dfName,WORD efName,BYTE plmnLen)
{
	WORD idata desfilesize,i,offset,len;
	UWORD xdata desAddr;
	BYTE idata init[5];

	desAddr = SelectTFFile(dfName,efName); //

	desfilesize = GetTFSize(desAddr);
	
	offset = 0x00;
	
	ecpMem_Copy(&init[0],&initplmnfile[0],sizeof(initplmnfile));
	
	while(desfilesize>0x00)
	{
		len = (desfilesize>(100*plmnLen)?(100*plmnLen):desfilesize);
		
		if(plmnLen == 0x03)
		{
			ecpMem_Set(tempBuf,0xFF,len);
		}
		else
		{
			for(i=0x00;i<len;i += 0x05)
			{
				ecpMem_Copy(&tempBuf[i],&init[0],plmnLen);
			}
		}
		
		WriteTFBuf(desAddr, offset, tempBuf, len);
		
		offset += len;
		
		desfilesize -= len;
	}
}
/*
*function    : InitoneplmnTFBuf
*
*@description:  初始化rpml参数
				
*

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/
void InitoneplmnTFBuf(void)
{
	BYTE idata i,offset;
	WORD idata usimadf,efName;
	UWORD xdata desAddr;
	BYTE idata init[5];
	
	usimadf = GetADFFID((BYTE *)&USIMPartial[0]);
	
	ecpMem_Copy(&init[0],&initplmnfile[0],sizeof(initplmnfile));
	
	for(i = 0x00;i < 0x03;i++)
	{
		if(i == 0x00)
		{
			efName = 0x6F7E;
			offset = 0x04;
		}
		else if(i == 0x01)
		{
			efName = 0x6F73;
			offset = 0x07;
		}
		else
		{
			efName = 0x6FE3;
			offset = 12;
		}

	    desAddr = SelectTFFile(usimadf,efName); //

	    if(desAddr != FALSE)
		{
			WriteTFBuf(desAddr, offset, &init[0], sizeof(init));
		}
	}		
}
/*
*function    : sqnFlagconfig
*
*@description:  更新sqnFlag参数
				
*

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/

BYTE sqnFlagconfig(BYTE sqnOption)
{
	BYTE  idata tmpData,AlgorithmID;	
	WORD idata usimadf;
	UWORD xdata desAddr;
	
	usimadf = GetADFFID((BYTE *)&USIMPartial[0]);
	
	if(usimadf ==FALSE)
	{
		return FALSE;
	}
	
	desAddr = SearchFile(E_FILE_START,usimadf); //
	
	AlgorithmID = getAlgorithmID(desAddr);
	
	desAddr = SelectFile(desAddr,EF_PRIVATE_AUTHCFG);
	
	if(desAddr == FALSE)
	{
		return FALSE;
	}
	//SQN Delta (1: not used, 0: used) ,此时把2E00里面的deltaC字节设置成FF(不使用delta)
	if(sqnOption&0x04)
	{
		tmpData = 0xFF;
		
		if(AlgorithmID == 1)//milenage
		{

			WriteTFBuf(desAddr,OFFSET_AUTHCFG_N,&tmpData,0x01);
		}
		else if(AlgorithmID == 2)//TUAK
		{

			WriteTFBuf(desAddr,OFFSET_TUAK_AUTHCFG_N,&tmpData,0x01);
		}
		else
		{
			
		}					   
	}

	if((sqnOption&0x08) == 0x08) //SQN Age Limit (1: not used, 0: used),将配置文件中的L设置为0
	{
		ecpMem_Set(&tempBuf[10],0,6);
		//setSqnAgeLimit(&tempBuf[0], 6);
		if(AlgorithmID == 1)//milenage
		{		
			WriteTFBuf(desAddr,OFFSET_AUTHCFG_L,&tempBuf[10],6);
		}
		else if(AlgorithmID == 2)//TUAK
		{
			 WriteTFBuf(desAddr,OFFSET_TUAK_AUTHCFG_L,&tempBuf[10],6);
		}
		else
		{
			
		}	
	}
	//bit1:Anonymity Key (AK) (1: not used, 0: used)
	//bit2:SQN wrap around (1: not allowed, 0: allowed)
	//In case SQN wrap around is allowed it means that SQN verification will be disabled if the respective SEQ value has reached the maximum value 07FFFFFFFFFF
	switch(sqnOption&0x03)
	{
		case 0:
			tmpData = 5;
			break;
		case 1:
			tmpData = 4;
			break;
		case 2:
			tmpData = 1;
			break;
		//case 3:
		default:
			tmpData = 0;
			break;
	}	
	WriteTFBuf(desAddr,OFFSET_AUTHCFG_OPCFG,&tmpData,0x01);
	
	return TRUE;
}
/*
*function    : CheckEnableProfileISDP
*
*@description:  判断当前激活的Profile ISD-P AID是不是红茶定义的RedTea_ISDP_AID
				
*

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/
BYTE code RedTea_ISDP_AID[0x11] = {0x10,0xA0,0x00,0x00,0x05,0x59,0x10,0x10,0xFF,0xFF,0xFF,0xFF,0x89,0x00,0x00,0x11,0x00};     //
BYTE CheckEnableProfileISDP(void)
{

	BYTE xdata AID[17];
	
	if(G_CurEnabledProfileIndex != ILLEGAL_PROFILEINDEX)
	{
		//profileAppTableAddr = GetAddrByIndexAddr(PROFILE_APPTable_REF + G_CurEnabledProfileIndex*sizeof(WORD));  //ISD-P一定位于SE_APP_INSTALLED_MAX的这个索引处
		AID[0] = GetProfileAIDByProfileIndex(G_CurEnabledProfileIndex, &AID[1]);  
		
//		profileAppTableAddr += (PROFILE_AppletAIDListTable_Offset + SE_APP_INSTALLED_MAX *List_LenInTable) ;  //point to the applist 
//		
//		len = VMReadByte(profileAppTableAddr);
		
		if(EQUAL == NvmCmp((BYTE *)&RedTea_ISDP_AID,&AID[0], sizeof(AID)))
		{
			return TRUE;
		}
	}
	
	return FALSE;
}
/*
*function    : CheckStoreDataTagBF7F
*
*@description:  检查计算红茶移动 Store Data tag FF7F
				
*
*@param      :输入数据Recdata

*
*@return     :
*
*@history    :V1.0 by LCZ@20190529
*/
//WORD code CoreFile[8]={DF_PHASE,EF_IMSI,DF_MF,EF_ICCID,DF_PROFILECFG,EF_KI,DF_PROFILECFG,EF_PRIVATE_AUTHCFG};  //DF/Ef格式   其中KI OPC写到当前UISM ADF下 
WORD code RplmnFile[3] ={EF_LOCI,0x6F73,0x6FE3};
WORD code foheplmnFile[5] ={EF_LOCI,0x6F7B,0x6F62,0x6FD9,0x6F61}; //第一个EF_LOCI 没有用 为了方便  
BYTE code foheplmnlen[5]  ={0x03,0x03,0x05,0x03,0x05}; //第一个0x03  没有用  为了方便
void CheckStoreDataTagFF7F(void)
{
	BYTE idata tmp,result,tag,transActioinFlag,tag87to8B,i;
	WORD idata requestTotalLen,tagLen,offset,tagExsit,tag1offset,usimadf;
	UWORD xdata dataAddr;
	BYTE xdata rootkey[0x21];
	BYTE xdata sha256result[0x20];
	
	if(!RedTea_Mobile)
	{
		return;
	}

	if(G_CustomerStoreDataApdu.storeDataLen <= 0xFF)
	{
		dataAddr = (UWORD)RecData;
	}
	else
	{
		dataAddr = EUICCTempDataBuf;
	}
	
	result = ResponseOk;
	
	tmp = ASN1_GetBERTlvLen(dataAddr + 2, (UWORD*)&requestTotalLen);  //FF7F的数据长度
	
	offset = 0x02 + tmp;  //dataOffset[0] 表示the first tag
	
	tag1offset = offset;
	
	VMReadBuf(&tag, (dataAddr + offset),0x01);//读取tag值
	
	if(tag != 0x30) //tbhRequest TBHRequest 0x30
	{
		result = dataFormatError;
	}
	
	tmp = ASN1_GetBERTlvLen(dataAddr + offset+1, (UWORD*)&tagLen);  //tag的数据长度
	
	offset += (1 + tmp + tagLen); //下一个tag偏移
	
	VMReadBuf(&tag, (dataAddr + offset),0x01);//读取tag值
	
	if(tag != 0x41)     //hashCode [APPLICATION 1] OCTET STRING
	{
		result = dataFormatError;
	}
	
	VMReadBuf(&tag, (dataAddr + offset+1),0x01);//读取tag值  长度
	
	if(tag != 0x20)
	{
		result = dataFormatError;
	}
	
	if(CheckEnableProfileISDP() != TRUE)
	{
		if(result == ResponseOk)
		{
			result = isdpnotsupported;
		}
	}
	
	VMReadBuf(&rootkey[0x00], (dataAddr + offset+2),0x20);//读取sha256值
	
	SHA256DoFinalSingleCall_Ex((dataAddr+tag1offset),(tagLen+tmp+1), &sha256result[0]);
	
	if(ecpMem_Compare(&sha256result[0],&rootkey[0x00],0x20)!=EQUAL) //
	{
		if(result == ResponseOk)
		{
			result = checkhashError;
		}
	}
	
	usimadf = GetADFFID((BYTE *)&USIMPartial[0]);
	
	if(usimadf == FALSE)
	{
		if(result == ResponseOk)
		{
			result = dataFormatError;
		}
	}
	
	if(requestTotalLen !=(1 + tmp + tagLen + 2 +0x20))
	{
		if(result == ResponseOk)
		{
			result = dataFormatError;
		}
	}		
	
	tagExsit = 0x00;
	
	tag87to8B = 0x00; //如果对应的plmn参数有值则更新对应文件内容,无值(tag不存在)则需要将对于的文件内容更新为默认值FFFFFF或者FFFFFF0000
	
	tag1offset +=(1 + tmp);     //
	
	if(result == ResponseOk)
	{
		ReadEeprom(rootkey,CARDCFG_RedTeaAesKey,sizeof(rootkey));    //读取根密钥  
		
		if((rootkey[0] != 0x10)&&(rootkey[0] != 0x18)&&(rootkey[0] != 0x20))
		{
			result = undefinedError;
		}
		else
		{
			CheckAesKeyLength(rootkey[0]);
			//GetEuiccAESKeyValue(&rootkey[1],rootkey[0],keyvalue); //获取 key

			KeyExpansion(&rootkey[1],&tempBuf[270]);     //用后半部分存扩展密钥
		}
	}
	
	transActioinFlag = FALSE;

    if(result == ResponseOk)
	{		
	   transActioinFlag = IF_BeginTransaction();  //TODO:该处若采用事物写，则需要判断当前是否已开启了事物，如果没有开启，则开启一个新的事物
	}
	
	while((tagLen>0x00)&&(result ==ResponseOk))
	{
		VMReadBuf(&tag, (dataAddr + tag1offset),0x01);//读取tag值
		
		switch(tag)
		{
			case 0x80://iccid
				
				 VMReadBuf(&tempBuf[0x00], (dataAddr + tag1offset+1),0x20);//读取LV
			
				 IF_UpdateFileToMetaData(tempBuf[0x00], &tempBuf[0x01],0,EF_ICCID); //MetaData ICCID同步更新
				
				 tag = WriteCoreTFBuf(DF_MF, EF_ICCID,0x00, &tempBuf[0x01], tempBuf[0x00]);
			
				 if(tag == FALSE )
				 {
					result = writeResultError;
				 }
			     else
			     {
					tagLen -=(tempBuf[0x00] + 2);
					tag1offset +=(tempBuf[0x00] + 2);
			     }
				 tagExsit++;
			     
				break;
			case 0x81: //imsi
				
				 VMReadBuf(&tempBuf[0x00], (dataAddr + tag1offset+1),0x20);//读取LV
				
				 tag = WriteCoreTFBuf(usimadf, EF_IMSI,0x00, &tempBuf[0x01], tempBuf[0x00]);
			
				 if(tag == FALSE )
				 {
					result = writeResultError;
				 }
			     else
			     {
					tagLen -=(tempBuf[0x00] + 2);
					tag1offset +=(tempBuf[0x00] + 2);
			     }
				 tagExsit++;
				 
				break;
			case 0x82: //ki
				 VMReadBuf(&tempBuf[0x00], (dataAddr + tag1offset+1),0x20);//读取LV
			
				 AESDecryptCore(&tempBuf[0x01], &tempBuf[0x01], &tempBuf[270]); //解密ki  这里前提是 ki  16字节 整数倍
				
				 tag = WriteCoreTFBuf(usimadf, EF_KI,0x00, &tempBuf[0x01], tempBuf[0x00]);
			
				 if(tag == FALSE )
				 {
					result = writeResultError;
				 }
			     else
			     {
					tagLen -=(tempBuf[0x00] + 2);
					tag1offset +=(tempBuf[0x00] + 2);
			     }
				 tagExsit++;
				 
				 break;
			case 0x83: //opc
				 VMReadBuf(&tempBuf[0x00], (dataAddr + tag1offset+1),0x20);//读取LV
			
				 AESDecryptCore(&tempBuf[0x01], &tempBuf[0x01], &tempBuf[270]); //解密opc 这里前提是opc  16字节  整数倍
				
				 tag = WriteCoreTFBuf(usimadf, EF_PRIVATE_AUTHCFG,0x01, &tempBuf[0x01], tempBuf[0x00]);
			
				 if(tag == FALSE )
				 {
					result = writeResultError;
				 }
			     else
			     {
					tagLen -=(tempBuf[0x00] + 2);
					tag1offset +=(tempBuf[0x00] + 2);
			     }
				 tagExsit++;
				 
				 break;
			case 0x84: //rotation OCTET STRING OPTIONAL, -- optional,4000204060
				 VMReadBuf(&tempBuf[0x00], (dataAddr + tag1offset+1),0x06);//读取LV
			
				 if(tempBuf[0x00] != 0x05)
				 {
					 result = dataFormatError;
				 }
				 
				 tag = WriteCoreTFBuf(usimadf, EF_PRIVATE_AUTHCFG,OFFSET_AUTHCFG_R, &tempBuf[0x01], tempBuf[0x00]);
			
				 if(tag == FALSE )
				 {
					 if(result ==ResponseOk)
					 {
						result = writeResultError;
					 }
				 }
			     else
			     {
					tagLen -=(tempBuf[0x00] + 2);
					tag1offset +=(tempBuf[0x00] + 2);
			     }				 
				
				break;
			case 0x85: //xoring OCTET STRING OPTIONAL
				 VMReadBuf(&tempBuf[0x00], (dataAddr + tag1offset+1),0x51);//读取LV  数据长度为80字节
			
				 if(tempBuf[0x00] != 0x50)
				 {
					 result = dataFormatError;
				 }
				 
				 tag = WriteCoreTFBuf(usimadf, EF_PRIVATE_AUTHCFG,OFFSET_AUTHCFG_C, &tempBuf[0x01], tempBuf[0x00]);
			
				 if(tag == FALSE )
				 {
					 if(result ==ResponseOk)
					 {
						result = writeResultError;
					 }
				 }
			     else
			     {
					tagLen -=(tempBuf[0x00] + 2);
					tag1offset +=(tempBuf[0x00] + 2);
			     }				 
				
				break;
			case 0x86: //sqnFlag BOOLEAN OPTIONAL, -- optional
				
				 VMReadBuf(&tempBuf[0x00], (dataAddr + tag1offset+1),0x02);//读取LV  数据长度一字节
			
				 if(tempBuf[0x00] != 0x01)
				 {
					 result = dataFormatError;
				 }
				 else
				 {
				
			       tag = sqnFlagconfig(tempBuf[0x01]);
					 
				   if(tag == FALSE )
				   {
					   result = writeResultError;
				   }
				   
					tagLen -=(tempBuf[0x00] + 2);
					tag1offset +=(tempBuf[0x00] + 2);
			   }
				
				break;
			
			case 0x87: //rplmn OCTET STRING OPTIONAL, -- optional, 存储格式64F010,默认FFFFFF  只带一个plmn
				
				 VMReadBuf(&tempBuf[0x00], (dataAddr + tag1offset+1),0x04);//读取LV 

				 if(tempBuf[0x00] != 0x03)
				 {
					 result = dataFormatError;
				 }
				 
				 tempBuf[0x04] = 0x12;
				 tempBuf[0x05] = 0x34;
				 tempBuf[0x06] = 0xFF; //对于LAI则是下一个字节RFU,对于TAI则是最后一个字节
				 tempBuf[0x07] = 0x01;
				 
				 requestTotalLen = 0x07;
				 
				 for(i = 0x00;i<0x03;i++)
				 {
					 if(i == 0x00)  //0x6F7E
					 {
						 offset = 0x04;
					 }
					 else if(i == 0x01) //6F73
					 {
						 offset = 0x07;
					 }
					 else              //0x6FE3  没有RFU字段
					 {
						 offset = 12;
						 tempBuf[0x06] = 0x01;
						 requestTotalLen = 0x06;
					 }
					 
					 tag = WriteCoreTFBuf(usimadf, RplmnFile[i],offset, &tempBuf[0x01], requestTotalLen);
					 
					 if(tag == FALSE )
					 {
						result = writeResultError;
					 }				 
				 }		
				tagLen -=(tempBuf[0x00] + 2);
				tag1offset +=(tempBuf[0x00] + 2);
				 
				SetBit(tag87to8B,0x00);
				break;
			case 0x88: //fplmn OCTET STRING OPTIONAL, -- optional, 格式同0x87      0x6F7B
				
				//VMReadBuf((BYTE *)&requestTotalLen, (dataAddr + tag1offset+1),0x01);//读取L
			
				offset =WriteplmnTFBuf(dataAddr + tag1offset+1,usimadf,EF_FPLMN,0x03);
			
				if(offset == FALSE )
				{
				  result = writeResultError;
				}
				tagLen -= offset;
				
				tag1offset += offset;	
				
			
				SetBit(tag87to8B,0x01);
				break;
			
			case 0x89: //hplmn OCTET STRING OPTIONAL, -- optional,多个，格式64F0100000 64F000000,默认FFFFFF0000   0x6F62
				
				offset =WriteplmnTFBuf(dataAddr + tag1offset+1,usimadf,0x6F62,0x05);
			
				if(offset == FALSE )
				{
				  result = writeResultError;
				}
				tagLen -= offset;
				
				tag1offset += offset;
				
				SetBit(tag87to8B,0x02);
				break;
			
			case 0x8A://eplmn OCTET STRING OPTIONAL -- optional,多个,同case 0x89   0x6F62
				offset =WriteplmnTFBuf(dataAddr + tag1offset+1,usimadf,0x6FD9,0x03);
			
				if(offset == FALSE )
				{
				  result = writeResultError;
				}
				tagLen -= offset;
				
				tag1offset += offset;
				
				SetBit(tag87to8B,0x03);
				break;
			
			case 0x8B://oplmn OCTET STRING OPTIONAL, -- optional,多个,同case 0x89  0x6FD9
				offset =WriteplmnTFBuf(dataAddr + tag1offset+1,usimadf,0x6F61,0x05);
			
				if(offset == FALSE )
				{
				  result = writeResultError;
				}
				tagLen -= offset;
				
				tag1offset += offset;				
			
				SetBit(tag87to8B,0x04);
				break;
			default: //白盒测试添加
				result = dataFormatError;
				break;
		}
	}
	
	if(tagExsit != 0x04) //80-84 tag必须存在
	{
		if(result == ResponseOk)
		{
			result = dataFormatError;
		}
	}
			
	if(result != ResponseOk)  //执行失败  此函数开启的事物 则回滚
	{
		IF_AbortTransaction(transActioinFlag);
		
		transActioinFlag = FALSE;
	}

	IF_CommitTransaction(transActioinFlag);
	
	
	if(result == ResponseOk) //由于EPX事物能力不够 调整位置,初始化oplm等文件不加在事物中
	{
		for(i = 0x00;i < 0x05;i++)
		{
			if(ValueBit(tag87to8B,i) == FALSE) //tag不存在的话  要对plmn文件做初始化
			{
				if(i == 0x00)
				{
					InitoneplmnTFBuf();
				}
				else
				{
					InitplmnTFBuf(usimadf,foheplmnFile[i],foheplmnlen[i]);
				}
			}
		}
	}
	
	RecData[0] = 0xFF;
	RecData[1] = 0x7F;
	RecData[2] = 0x03; //len
	RecData[3] = 0x80;
	RecData[4] = 0x01; // Len
	
	RecData[5] = result;//V  固定一字节
	
	SetOutgoingAndSend(0x05,(short)(6));
	
	if(result == ResponseOk)
	{
		IF_SendRefreshCommandOfUICCResetMode();
	}

}
#endif
#endif
