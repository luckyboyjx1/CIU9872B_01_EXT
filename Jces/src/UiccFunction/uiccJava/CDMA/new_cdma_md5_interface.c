/**
 * @file 	cdma_md5_interface.c
 *
 * @brief 	'80 80 XX XX XX'
 * 		用于CDMA EVDO 1X RUIM鉴权，产生一个8字节（64 bits）的MD5密文
 *
 * @author 	WG TongLiang MJG
 *
 * @history 	2008-04-28 WG: 修改为 支持TLV结构密钥文件,澳门EVOD脚本测试通过
 
 				V1.2 增加宏定义MD5RAM，该宏用于将MD5下行数据缓存在RAM里，而不是放在缓存文件6E83中去，可提高算法运算速度
 
  */
#include <string.h>
#include "uicc_main.h"
#include "uicc_error.h"
#include "gsmpublic.h"
#include "fileAction.h"
#include "fileSystem.h"
//#include "md5.h"
#include "cdma.h"


#if UICC__AUTH__CDMA__MD5

BYTE xdata MIP_RRQ_Hash_Buf[0x10]; 	// 暂存MIP_RRQ指令计算结果, 16BYTE

BYTE xdata Nai_Index; 			// SS(秘密参数)编号

BYTE xdata G_P1; 			// 预期的下一条8080指令的P1
#define nextP1 G_P1
BYTE xdata G_P2[2];
#define nextP2_1 G_P2[0] 		// 预期的下一条8080指令的P2
#define nextP2_2 G_P2[1]		// 预期的下一条8080指令的P2


/**
 * @brief 	set the next P1 and P2's values for the next runMD5Algorithm
 * 		设置预期的下一条指令的P1P2
 *
 * @author 	TongLiang WG
 *
 * @history 	删除函数参数 by WG@080428 19:49
 * @history 	Add comment by TongLiang@080111 15:57
 * @history 	created by TongLiang@080110 12:57
 */
void PointerEvaluate(void)
{
	if ((P2 == 0x02)||(P2==0x03)) //single or last block
	{
		nextP1 = (P1 +0x01);
		nextP2_1 = 0x00; // first
		nextP2_2 = 0x02; // single
	}
	else
	{
		nextP1 = P1;
		nextP2_1 = 0x01; // next
		nextP2_2 = 0x03; // last
	}
}

/**
 * @brief 	update the EF_ComputerIp(0x6F83)'s bytes indicating the length
 * 		将(需要进行MD5计算的数据的)长度写到缓存文件的开始两个字节
 *
 * @param 	fileAddrRes:缓存文件地址
 * @param 	length:数据长度，WORD
 *
 * @Returns   	TRUE/FALSE
 *
 * @author 	TongLiang
 *
 * @history 	modified by WG @080428 19:40 -- 删除全局变量FileSize；参数2名称由'offset'改为'length'
 * @history 	Add comment by TongLiang@080111 15:59
 */
BYTE LengthEvaluate(UWORD fileAddrRes, WORD length)
{
	BYTE  idata  tmp[2];

	if ((length + sizeof(tmp)) > GetTFSize(fileAddrRes))
	{
		TechnicalProblem();

		return FALSE;
	}

	tmp[0] = length>>8;

	tmp[1] = (length&0xFF);

	WriteTFBuf(fileAddrRes, 0, &tmp[0], sizeof(tmp));

	return TRUE;
}

/**
 * @brief 	lookup the record which it's index is equal to the Nai_Index
 *
 * @param 	fileAddr：TF地址
 * @param 	mode: 0x00 --> 仅查找; 其他值：查找并读取到RecData[0x00]
 *
 * @Returns   	查找模式下，仅返回长度值（不等于零）；或返回零（表示没找到）
 * 		其他模式下，查找并读取，返回壹；或返回零（表示没找到）
 *
 * @author 	TongLiang WG
 *
 * @history 	modified by WG, @080428 19:24 -- 修改为从TF中根据TLV读取对应的SS
 * @history 	Add comment by TongLiang@080111 15:59
 */
BYTE LookupIndexRecord(UWORD fileAddr, BYTE mode)
{
	WORD idata  offset, maxOffset;
	BYTE idata  tagLen[2];

	maxOffset = GetTFSize(fileAddr) -1;

	for (offset=0; offset < maxOffset; ) 	// 查找指定的 Nai_Index
	{
		ReadTFBuf(fileAddr, offset, tagLen, sizeof(tagLen));

		if (tagLen[0] == Nai_Index) 	// 获取 Nai_Index 对应的 MN-HA-SS
		{
			if (mode == 0x00) 	// 仅查找
			{
				return tagLen[1];
			}
			else
			{
				ReadTFBuf(fileAddr, (offset+sizeof(tagLen)), &RecData[0x00], tagLen[1]);

				return TRUE;
			}
		}

		offset += (sizeof(WORD) + tagLen[1]);
	}
	
	ReferencedDataNotFound();																	//for change2010.11.19   兼容3G模式下的返回码状态										

	return FALSE;
}

/**
 * @brief 	search the given file, then get the record's length
 * 		选择指定文件，然后在该文件里查找Nai_Index, 返回对应的SS长度或返回零
 *
 * @param 	fileAddr
 * @param 	length
 * @param 	fileID
 *
 * @Returns   	6F00+FALSE 9834+FALSE TRUE
 *
 * @author 	TongLiang
 *
 * @history 	modified by WG, @080428 20:56
 * @history 	Add comment by TongLiang@080111 15:59
 */
BYTE GetSSLength(UWORD* ptrFileAddr, BYTE* ptrLength, WORD fileID)
{

	(*ptrFileAddr) = SelectTFFile(G_DF_ID_CSIM, fileID);

	if ((*ptrFileAddr) == FALSE)
	{
		TechnicalProblem();
		return FALSE;
	}

	(*ptrLength) = LookupIndexRecord((*ptrFileAddr), 0x00);
	
	return ((*ptrLength) == 0x00)? FALSE:TRUE;
}

void RunMD5Algorithm(void)
{
	UWORD idata  fileAddr, fileAddrRes;
	WORD  idata  offset;
	BYTE  idata  chapsslen, MNHASSlen, MNAAAsslen;
	BYTE  idata  tmp[2];

	if (P1 > 0x04)
	{
		P1P2Error();				// send 6b 00
		return;
	}

	if ((P1 != MIP_RRQ) && (P1 != MN_HA))
	{
		if (P2 >0x00)
		{
			P1P2Error();			// send 6b 00
			return;
		}
	}
	else
	{
		if (P2 >0x03)
		{
			P1P2Error();			// send 6b 00
			return;
		}
	}
#if(!UICC__CDMA_2G_NOT_SUPPORT)
	if(G_CommandMode == Command_Mode_2G)										//for change2010.11.19   对是否在DF_CDMA目录下先进行判断
	{
		if (G_CurrentDfName[G_ChannelNum] != DF_CDMA)
		{
			FileInconsistent();		
			return;
		}
	}
	else
#endif
	{
		//"a particular USIM application or any sub-directory must be
		//selected as the Current Directory" --TS31122
		if (G_CurrentAdfName[G_ChannelNum] == FALSE)
		{
			fileAddr = FALSE;
		}
		else 
		{
			fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum],G_CurrentDfName[G_ChannelNum]);			
			
		}

		if (fileAddr == FALSE)
		{
			ConditionSatisfied();												//Send 69 85
			return;
		}
	}
	
	
	if (P1 == MN_HA)
	{
		if ((P2 != 0x00) && (P2 != 0x02) && (P2 != nextP2_1) && (P2 != nextP2_2))
		{
			CDMAError(); 			// send 98 34
			return;
		}
	}


	if (P1 == MIP_RRQ) 				// MIP-RRQ Hash (必须先进行 MN-HA Authentication)
	{
		if ((nextP1 != MIP_RRQ) || ((P2 != nextP2_1) && (P2 != nextP2_2)))
		{	
			CDMAError(); 			// send 98 34
			return;
		}
	}
	

	if (P1 == MN_AAA) 				// MN-AAA Autherticator(必须先进行 MN-HA 和 MIP_RRQ)
	{
		if (nextP1 != MN_AAA) 			// NO MN-HA Authentication && MIP-RRQ Hash
		{
			CDMAError(); 			// send 98 34
			return;
		}
	}

	if (ValueBit(G_ChvStatus, B_Chv_1_Verified)==FALSE)
	{
#if(!UICC__CDMA_2G_NOT_SUPPORT)	
		if(G_CommandMode == Command_Mode_2G)													//for change2010.11.19  区分2G和3G模式下的返回状态码  
		{
			SecurityError();
			return;
		}
		else
#endif
		{
			SecuritySatisfied();
			return;
		}
		
	}  	

	ReceiveData();
	
	fileAddrRes = SelectLFFile(G_DF_ID_CSIM, EF_ComputerIp); // TF 暂存Registration-Data

	if (fileAddrRes == FALSE)
	{
		TechnicalProblem();
		return;
	}

	if (P1 == MN_HA) 				// MN-HA Authenticator
	{
		// MN-HA Response = Auth_MD5 (MN-HA SS || Registration-Data || MN-HA SS)

		PointerEvaluate();

		if ((P2 == 0x02) || (P2 == 0x00)) 	// Single or First block
		{
			Nai_Index = RecData[0];

			if (GetSSLength(&fileAddr, &MNHASSlen, MN_HA_SS_TABLE) == FALSE)
			{
				return;
			}

			WriteTFBuf(fileAddrRes, (sizeof(WORD) + MNHASSlen), &RecData[1], P3-1);

			offset = MNHASSlen + P3 - 1; 	// write Registration-Data into EF_ComputerIp
		}
		else 					// next block or last block
		{
			if (P3 > 0xFE) 			// Maximum length of the Registration-Data is 254 octets per block.
			{
				TechnicalProblem(); 	// add by WG, 20070628
				return;
			}

			ReadTFBuf(fileAddrRes, 0, &tmp[0], sizeof(tmp));

			offset = ((tmp[0]<<8) &0xFF00) +tmp[1];

			WriteTFBuf(fileAddrRes, (offset+sizeof(WORD)), &RecData[0], P3);

			offset += P3;
		}

		if (LengthEvaluate(fileAddrRes, offset) == FALSE)
		{
			return;
		}

		if ((P2 == 0x00)||(P2 == 0x01))
		{
			CommandFinish(0x00);
			return;
		}
		else
		{
			if (GetSSLength(&fileAddr, &MNHASSlen, MN_HA_SS_TABLE) == FALSE)
			{
				return;
			}

			if (LookupIndexRecord(fileAddr, MNHASSlen) == FALSE)
			{
				return;
			}

			WriteTFBuf(fileAddrRes, sizeof(WORD), &RecData[0], MNHASSlen);

			WriteTFBuf(fileAddrRes, (sizeof(WORD)+offset), &RecData[0x00], MNHASSlen);

			offset += MNHASSlen;

			if (LengthEvaluate(fileAddrRes, offset) == FALSE)
			{
				return;
			}
		}
	}
	else if (P1 == MIP_RRQ) // MIP-RRQ HASH
	{
		PointerEvaluate();

		if ((P2 == 0x02)||(P2==0x00))		// single or first block
		{
			WriteTFBuf(fileAddrRes, sizeof(WORD), &RecData[0], P3);

			offset = (P3&0xFF);
		}
		else 					// next or last block
		{
			ReadTFBuf(fileAddrRes, 0, &tmp[0], sizeof(WORD));

			offset = ((WORD)tmp[0]<<8) + (tmp[1]&0xFF);

			WriteTFBuf(fileAddrRes, (sizeof(WORD) + offset), &RecData[0], P3);

			offset+= P3;
		}

		if (LengthEvaluate(fileAddrRes, offset) == FALSE)
		{
			return;
		}

		if ((P2 == 0x00) || (P2 == 0x01))
		{
			CommandFinish(0x00);
			return;
		}
	}
	else if (P1 == MN_AAA) // MN-AAA Authentication
	{
		nextP1 = 0;
		nextP2_1 = 0;
		nextP2_2 = 0;

		Nai_Index = RecData[0];

		if (GetSSLength(&fileAddr, &MNAAAsslen, MN_AAA_SS_TABLE) == FALSE)
		{
			return;
		}

		WriteTFBuf(fileAddrRes, sizeof(WORD), &RecData[1], sizeof(BYTE));

		WriteTFBuf(fileAddrRes, 3+MNAAAsslen, &MIP_RRQ_Hash_Buf[0], sizeof(MIP_RRQ_Hash_Buf));

		offset = (P3>238)? 237:(P3-1);

		WriteTFBuf(fileAddrRes, (0x13+MNAAAsslen), &RecData[P3-offset], offset);

		offset = MNAAAsslen+sizeof(MIP_RRQ_Hash_Buf); // 0x10:length of MIP-RRQ Hash

		offset += (P3>238) ? 238:P3;

		if (LengthEvaluate(fileAddrRes, offset) == FALSE)
		{
			return;
		}

		if (LookupIndexRecord(fileAddr, MNAAAsslen) == FALSE)
		{
			return;
		}

		WriteTFBuf(fileAddrRes, 3, &RecData[0], MNAAAsslen);
	}
	else if (P1 == HRPD)
	{
		fileAddr = SelectTFFile(G_CurrentAdfName[G_ChannelNum], EF_CHAP_SS);

		if (fileAddr == FALSE)
		{
			TechnicalProblem();
			return;
		}

		ReadTFBuf(fileAddr, 0, &chapsslen, 1); 			// get the length of CHAP-SS from EF_CHAP_SS

		WriteTFBuf(fileAddrRes, 2, &RecData[0], 1); 		// write CHAP-ID 	     into EF_ComputerIp

		WriteTFBuf(fileAddrRes, (3+chapsslen), &RecData[1], P3-1);// write CHAP-Chanllenge into EF_ComputerIp

		ReadTFBuf(fileAddr, 1, &RecData[0], chapsslen); 	// get 	 CHAP-SS content

		WriteTFBuf(fileAddrRes, 3, &RecData[0], chapsslen); 	// write CHAP-SS content into EF_ComputerIp

		offset = P3 +chapsslen; 				// 'offset' means TOTAL LENGTH of the Data to be MD5

		if (LengthEvaluate(fileAddrRes, offset) == FALSE)
		{
			return;
		}
		// HRPD end
		// Now  EF_ComputerIp is ready for MD5: LV structure, V=(CHAP-ID + CHAP-SS + CHAP-Chanllenge)
		// CHAP-Response = Auth_MD5(CHAP-ID || CHAP-SS || CHAP-Challenge);
	}
	else // CHAP
	{
		Nai_Index = RecData[1];
		
		if (GetSSLength(&fileAddr, &chapsslen, EF_CHAP_SS_TABLE) == FALSE)
		{
			return;
		}

		
		offset = P3 +chapsslen -1;

		if (LengthEvaluate(fileAddrRes, offset) == FALSE)
		{
			return;
		}

		WriteTFBuf(fileAddrRes, 2, &RecData[0], 1); 		// write CHAP-ID into EF_ComputerIp

		WriteTFBuf(fileAddrRes, (3+chapsslen), &RecData[2], P3-2);// write CHAP-Chanllenge into EF_ComputerIp

		if (LookupIndexRecord(fileAddr, chapsslen) == FALSE)
		{
			return;
		}

		WriteTFBuf(fileAddrRes, 3, &RecData[0], chapsslen); 	// write CHAP-SS content into EF_ComputerIp
	}

	//Auth_MD5(fileAddrRes, offset, (BYTE *)&RecData[0]);
	//MD5DoFinalSingleCall((BYTE *)&RecData[0], 0,offset, 0x01,fileAddrRes+sizeof(TranFileHead)+2); //偏移到数据的位置，调用MD5算法的公共函数。

	MD5DoFinalSingleCall((BYTE *)&RecData[0], (BYTE *)(fileAddrRes+sizeof(TranFileHead)+2),offset, 0x01,0);

	if (P1 == MIP_RRQ)
	{
		ecpMem_Copy(&MIP_RRQ_Hash_Buf[0], &RecData[0], 0x10);

		CommandFinish(0x00);

		return;
	}

	CommandFinish(0x10);
}
#endif


