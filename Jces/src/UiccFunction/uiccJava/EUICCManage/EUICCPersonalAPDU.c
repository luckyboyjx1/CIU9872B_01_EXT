/*
*@author     : xuyz
*
*@copyright  :eastcompeace
*
*@description:该文件主要存放EUICC的个人化命令
*
*@history     :V1.0 :create @ 20181106  
*/
#include "EUICCCustomerCertificate.h"
#include "EUICCPersonalAPDU.h"
#include "EUICCCustomer.h"
#include "ASN1Operation.h"
#include "Interpreter.h"
#include "T0ApduImpl.h"
#include "EUICCMain.h"
#include "stkhead.h"
#include "EUICCCustomerPublic.h"
#include "ProfileGPCommand.h"
#include "EUICCProfile.h"
//#include "Sha1.h"
#include "EUICCInterface.h"
#include "cardManager.h"
#include "FunctionSwitch.h"
#include "EUICCMain.h"
#include "platform_api.h"
#include "VMAccessEeprom.h"
#include "object.h"
#include "ob_manager.h"
#include "nativemethod_impl.h"
#include "EUICCCustomerCertificate.h"
#include "Interface_LTEFromPlatform.h"
#include "Crypto_Constant.h"
#include "euiccConfig.h" //warning JcvmConfig_GetSEState
#include "ProfileInterpreter.h" //warning interpreterPE
#include "SEEUICCInterface.h" //warning SEUICC_BackISDPAppletTable
#include "hard_api.h" //warning TransactionBegin();
#if GP__AMENDMENT__B__FOR_MASK
#include "HttpProcess.h"
#endif

#if (EUICC_ONLY||SE_AND_EUICC)

WORD xdata G_CAWrite;  //当前正在个人化的证书已经写入的数据长度
WORD xdata G_CALength; //当前正在个人化的证书的总长度


BYTE xdata G_PersonalProfileState;
BYTE xdata G_PersonalProfileIndex;

/*
*@function   :EUICCPersonalCACIData
*
*@description:个人化CI数据, 该函数同时支持RSP版和M2M版的证书个人化。证书的个人化格式为：证书总长度 + 证书数据
*             比如：0261 3082025D30820203A003020102020900DC9E5713D3A88FD8300A06082A8648CE3D0403023062310B300906035504061302434E310B300906035504080C02424A310B300906035504070C02424A310D300B060355040A0C04434D4343310D300B060355040B0C04434D5249311B301906035504030C12434D434320534D44502B20526F6F74204341301E170D3137303631353130303130365A170D3138303631353130303130365A3062310B300906035504061302434E310B300906035504080C02424A310B300906035504070C02424A310D300B060355040A0C04434D4343310D300B060355040B0C04434D5249311B301906035504030C12434D434320534D44502B20526F6F742043413059301306072A8648CE3D020106082A8648CE3D03010703420004D33CFE7CFC3335A9BA826D9271DFA5AD9A973A31D5F8F417F66A3E51B01D78FFFCCD718DEE61B43EFD81106C0A739EE667BEBEC0DDA517EFCE1469E92D5BCE53A381A130819E301D0603551D0E04160414595BC6C25193CEF8E5804BDAFEF4ED063F906B1E30150603551D11040E300C880A2B0601040181DD456401300F0603551D130101FF040530030101FF30170603551D200101FF040D300B3009060767811201020100302C0603551D1F042530233021A01FA01D861B687474703A2F2F3139322E3136382E302E312F746573742E63726C300E0603551D0F0101FF040403020106300A06082A8648CE3D0403020348003045022100992D583700D23BD73D31A008A89FBC7621B6053ABFCAC56931B71CD9E9A71FC702201E6B71E1ADBEAEDABE032E1E0A15BD67CC05A84282F338184E74D5AA820F235A
*
*@param      :void
*
*@return     :void
*
*@author     :xuxiaojun@20181207
*
*@test       :测试通过
*CI证书个人化指令：
00A4040000
8020A200FF 02613082025D30820203A003020102020900DC9E5713D3A88FD8300A06082A8648CE3D0403023062310B300906035504061302434E310B300906035504080C02424A310B300906035504070C02424A310D300B060355040A0C04434D4343310D300B060355040B0C04434D5249311B301906035504030C12434D434320534D44502B20526F6F74204341301E170D3137303631353130303130365A170D3138303631353130303130365A3062310B300906035504061302434E310B300906035504080C02424A310B300906035504070C02424A310D300B060355040A0C04434D4343310D300B060355040B0C04434D5249311B301906035504030C12434D43
8020A200FF 4320534D44502B20526F6F742043413059301306072A8648CE3D020106082A8648CE3D03010703420004D33CFE7CFC3335A9BA826D9271DFA5AD9A973A31D5F8F417F66A3E51B01D78FFFCCD718DEE61B43EFD81106C0A739EE667BEBEC0DDA517EFCE1469E92D5BCE53A381A130819E301D0603551D0E04160414595BC6C25193CEF8E5804BDAFEF4ED063F906B1E30150603551D11040E300C880A2B0601040181DD456401300F0603551D130101FF040530030101FF30170603551D200101FF040D300B3009060767811201020100302C0603551D1F042530233021A01FA01D861B687474703A2F2F3139322E3136382E302E312F746573742E63726C30
8020A20065 0E0603551D0F0101FF040403020106300A06082A8648CE3D0403020348003045022100992D583700D23BD73D31A008A89FBC7621B6053ABFCAC56931B71CD9E9A71FC702201E6B71E1ADBEAEDABE032E1E0A15BD67CC05A84282F338184E74D5AA820F235A
*
*@history    : modified by xuxiaojun@20210414 _newarrayObject内已经存在异常判断
*注意        :在个人化证书过程中不允许插入任何其他的指令，包括个人化另外一个证书，且证书的第一条指令的数据长度必须大于2。 且必须保证证书传入长度的正确性，包括证书总长度必须和证书结构中的长度相符合
*            :按照目前的证书个人化实现方案，只有个人化失败，都需要重新做卡，因为证书第一条指令中会分配引用，并写入。而如果后续发生错误时，并没有将相关数据清除。
*            :该函数中开启事务只是为了分配空间
*/
void EUICCPersonalCACIData(void)
{
	UWORD xdata writeAddr,curCACIAddr, nextCACIAddr, lastCACIAddr, lastCACIDataAddr, lastCACIParaAddr;
	WORD idata curCACIRef, lastCACIRef, lastCACIDataRef,pkidRef;
#if EUICC__FUNCTION__GUOMI_eSIM
	BYTE idata CertType;

	CertType = Cert_ECC;
#endif
	curCACIAddr = GetAddrAndIndexByIndexAddr(EUICCCARef_Addr,&curCACIRef);

	if(curCACIRef == REF_NULL)
	{
		writeAddr = EUICCCARef_Addr;//当前个人化的证书链引用需要存储的地方
	}
	else
	{
		while(1)
		{
			writeAddr = CACIListGet_nextCIListRefAddr(curCACIAddr);

			nextCACIAddr = GetAddrByIndexAddr(writeAddr);

			if(nextCACIAddr == ADDR_IS_NULL)
			{
				break;
			}

			curCACIAddr = nextCACIAddr;
		}
	}

	lastCACIRef = REF_NULL;

	lastCACIAddr = ADDR_IS_NULL;

	if(G_CAWrite == 0) //当前收到的是个人化CI证书的第1条指令，需要分配空间
	{
		beginTransaction(0);

		lastCACIRef = newObject(CACILIST_CLASS_OFFSET, ECASD_PKG_ID, BIT_NEED_REFRESH);// 先分配CACIList结构

		lastCACIAddr = locateObjectInMemory(lastCACIRef);//lastCACIAddr = VMCOSHeapAlloc(sizeof(CACIList), &lastCACIRef, CardMemory_Mode);// 先分配CACIList结构

		memWriteU16Atomic(MEMtPM8(writeAddr),lastCACIRef);//VMWriteBuf(curCACIAddr, (BYTE*)&lastCACIRef, 0x02); //把CACIList的引用链接到表的末尾，如果考虑到多条指令的话，这个引用应该是最后一条的时候才写入，需要将其做成全局变量

		G_CALength = BuildWord(RecData);

		lastCACIDataRef = _newarrayObject(AType_Byte, G_CALength+2, BIT_NEED_REFRESH);// 再分配证书实际大小 + 2字节证书总长度

		lastCACIDataAddr = get_array_data((PF8)locateObjectInMemory(lastCACIDataRef));//VMCOSHeapAlloc(G_CALength + 2, &lastCACIDataRef, CardMemory_Mode);// 再分配证书实际大小

		memWriteU16Atomic(MEMtPM8(CACIListGet_curCertCIArrayAddr(lastCACIAddr)),lastCACIDataRef);//把证书引用写入CACIList.curCertCIRef

		TransactionCommit();
	}
	else
	{
		lastCACIDataAddr = CACIListGet_curCertCIArrayValueAddr(curCACIAddr);//GetAddrAndIndexByIndexAddr(CACIListGet_curCertCIArrayAddr(curCACIAddr), &lastCACIDataRef);
	}

	VMWriteBuf(lastCACIDataAddr + G_CAWrite, RecData, P3); //把证书数据写入证书的实际存储地址

	G_CAWrite += P3;                         //修改G_CAWrite的长度，用于表示当前已经存储的证书的数据长度

	if(G_CAWrite > (G_CALength + 2))         //证书个人化指令有问题，需要重新做卡
	{
		ThrowError(SW_WRONG_DATA);
	}

	if(G_CAWrite == (G_CALength + 2))  //收到最后一条指令
	{
#if EUICC__FUNCTION__GUOMI_eSIM
		TBSCertificate_GetField((BYTE *)(lastCACIDataAddr + 2), G_CALength, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm, &tempBuf[0]); //CERT.DPpb.SM2SIG  取出曲线参数 1字节  LV结构 tempBuf[1]存曲线参数

		if((sizeof(SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI) == tempBuf[0]) && (EQUAL == ecpMem_Compare(&SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI[0], &tempBuf[1], tempBuf[0])))
		{
			CertType = Cert_GUOMI;
		}
#endif
		if(RESPONSE_NoError != TBSCertificate_GetField((BYTE *)(lastCACIDataAddr + 2), G_CALength, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_subjectPublicKey, tempBuf)) //查找pkCI, 公钥长度为64字节，tempBuf中的数据是LV结构
		{
			ThrowError(SW_WRONG_DATA);
		}

		beginTransaction(0);

		pkidRef = _newarrayObject(AType_Byte, 64, BIT_NEED_REFRESH);//PK value ref

		memWriteU16Atomic(MEMtPM8(CACIListGet_pkCIArrayAddr(curCACIAddr)),pkidRef);

		VMWriteBuf(get_array_data((PF8)locateObjectInMemory(pkidRef)), &tempBuf[1], tempBuf[0]);//VMWriteBuf(curCACIAddr + CACIList_PkCIValue_Offset, &tempBuf[1], tempBuf[0]); //直接存储64字节的public key

#if EUICC__FUNCTION__GUOMI_eSIM
		if(CertType != Cert_GUOMI) //国密证书的算法参数是缺省的
#endif
		{
			if(RESPONSE_NoError != TBSCertificate_GetField((BYTE *)(lastCACIDataAddr + 2), G_CALength, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters, tempBuf)) //查找algorithm.parameter
			{
				ThrowError(SW_WRONG_DATA);
			}
		}

		memWriteU8Atomic(MEMtPM8(CACIListGet_parameterCIAddr(curCACIAddr)),tempBuf[1]);//VMSetBuf(curCACIAddr + CACIList_ParameterCI_Offset, tempBuf[1], tempBuf[0]); //把algorithm parameters对应的ECC参数引用写入到链表结构中

		if(FALSE == TBSCertificateExtensions_GetField((BYTE *)(lastCACIDataAddr + 2), G_CALength, id_ce_subjectKeyIdentifier, tempBuf,NULL)) //查找Subject Key Identifier
		{
			ThrowError(SW_WRONG_DATA);
		}

		if(tempBuf[0] > CACIList_PkIdCI_MAX_Len)
		{
			ThrowError(SW_WRONG_DATA);
		}

		pkidRef = _newarrayObject(AType_Byte, tempBuf[0]+1, BIT_NEED_REFRESH);//PKID

		lastCACIParaAddr = get_array_data((PF8)locateObjectInMemory(pkidRef));//lastCACIParaAddr = VMCOSHeapAlloc(tempBuf[0]+1, &lastCACIParaRef, CardMemory_Mode);

		VMWriteBuf(lastCACIParaAddr, &tempBuf[0], tempBuf[0] + 1); //先存储Subject Key Identifier

		memWriteU16Atomic(MEMtPM8(CACIListGet_pkIdCIArrayAddr(curCACIAddr)),pkidRef);//VMWriteBuf(curCACIAddr + CACIList_PkIdCIRef_Offset, (BYTE*)&lastCACIParaRef, 0x02); //把Subject Key Identifier的引用写入到链表结构中

		TransactionCommit();

		G_CAWrite = 0;

		G_CALength = 0;
		//个人化时COS不检测证书的有效性，由个人化人员保证
	}
}


#if RSP_EUICC
/*
*@function   :EUICCPersonalCAEUMAndEUICCData
*
*@description:个人化EUM,EUICC数据和EUICC的SK, 该函数支持RSP版的证书个人化。证书的个人化格式为：证书总长度 + 证书数据
*             比如EUM证书：  02B4 308202B030820255A003020102020138300A06082A8648CE3D0403023062310B300906035504061302434E310B300906035504080C02424A310B300906035504070C02424A310D300B060355040A0C04434D4343310D300B060355040B0C04434D5249311B301906035504030C12434D434320534D44502B20526F6F74204341301E170D3137303732353031353130355A170D3138303732353031353130355A304D310B300906035504061302434E310B300906035504080C02424A3110300E060355040A0C0765617374636F6D311F301D06035504030C16534D44502B2045554D20287072696D653235367631293059301306072A8648CE3D020106082A8648CE3D030107034200042B4383A02551CE38D19E16D04EE80542BBA5871DC7EA327B0E5B340BBE5A16D3A2225D0FDAD55015498278C7C03D1738EA4E44915BAE622FB13068A41FCF6839A382010F3082010B301F0603551D23041830168014595BC6C25193CEF8E5804BDAFEF4ED063F906B1E301D0603551D0E04160414FAE0C678C64F35D7A0752BFA1D8875D08E27E41530120603551D130101FF040830060101FF02010030470603551D1E0101FF043D303BA0393037A4353033311E301C060355040A0C154769657365636B6520616E64204465767269656E743111300F06035504051308383930343930333230150603551D11040E300C880A2B0601040181DD45020430170603551D200101FF040D300B3009060767811201020102302C0603551D1F042530233021A01FA01D861B687474703A2F2F3139322E3136382E302E312F746573742E63726C300E0603551D0F0101FF040403020106300A06082A8648CE3D0403020349003046022100CCC9FC9A4962653EA00E786DFCC96923D0B49DD05E1B842087AA335BCF94508502210086CD9A630994FBCAD919F1CFA6BE014B756AF75BACE620601DD7BA91BFEE11E2
*				  EUICC证书：0217 30820213308201B9A003020102020116300A06082A8648CE3D040302304D310B300906035504061302434E310B300906035504080C02424A3110300E060355040A0C0765617374636F6D311F301D06035504030C16534D44502B2045554D20287072696D65323536763129301E170D3137303631353130303633385A170D3138303631353130303633385A306C310B300906035504061302434E312930270603550405132038393034393033323132333435313233343531323334353637383930313333323110300E060355040A130765617374636F6D3120301E06035504031317534D44502B205549434320287072696D653235367631293059301306072A8648CE3D020106082A8648CE3D03010703420004662D8AE22CD2547E463E7FA06CEFD0E10C83A4BC4B2E4F7DE06A778BC1A9B9B9157F3E8945F686FBDC4BFB19E2663C49A955B0F8CA8250592F9060272CEB146BA36B3069301F0603551D23041830168014FAE0C678C64F35D7A0752BFA1D8875D08E27E415301D0603551D0E041604143DF74C402E0E6460C3B949F61F6D02DB7CB9255C300E0603551D0F0101FF04040302078030170603551D200101FF040D300B3009060767811201020101300A06082A8648CE3D0403020348003045022100F5851CDDDE1E914420735F605A22E9BE3C9B6778EEAAA63F9AFDB46B0C4038EA02203A288C057F75B6357CF60B59A06AE2E9554946F1142AAFB4438796454CB821F6
				  SK:        0020 D3B6DCBE5BF54EC2ACEBE8B7B0933A11148B10D09AFD3A41EC3A30C76EFF71B4
*
*@param      :void
*
*@return     :void
*
*@author     :lx @20181207
*
*@test       :
*
*EUM证书个人化指令：
00A4040000
8020A300FF 02B4308202B030820255A003020102020138300A06082A8648CE3D0403023062310B300906035504061302434E310B300906035504080C02424A310B300906035504070C02424A310D300B060355040A0C04434D4343310D300B060355040B0C04434D5249311B301906035504030C12434D434320534D44502B20526F6F74204341301E170D3137303732353031353130355A170D3138303732353031353130355A304D310B300906035504061302434E310B300906035504080C02424A3110300E060355040A0C0765617374636F6D311F301D06035504030C16534D44502B2045554D20287072696D653235367631293059301306072A8648CE3D020106
8020A300FF 082A8648CE3D030107034200042B4383A02551CE38D19E16D04EE80542BBA5871DC7EA327B0E5B340BBE5A16D3A2225D0FDAD55015498278C7C03D1738EA4E44915BAE622FB13068A41FCF6839A382010F3082010B301F0603551D23041830168014595BC6C25193CEF8E5804BDAFEF4ED063F906B1E301D0603551D0E04160414FAE0C678C64F35D7A0752BFA1D8875D08E27E41530120603551D130101FF040830060101FF02010030470603551D1E0101FF043D303BA0393037A4353033311E301C060355040A0C154769657365636B6520616E64204465767269656E743111300F06035504051308383930343930333230150603551D11040E300C880A
8020A300B8 2B0601040181DD45020430170603551D200101FF040D300B3009060767811201020102302C0603551D1F042530233021A01FA01D861B687474703A2F2F3139322E3136382E302E312F746573742E63726C300E0603551D0F0101FF040403020106300A06082A8648CE3D0403020349003046022100CCC9FC9A4962653EA00E786DFCC96923D0B49DD05E1B842087AA335BCF94508502210086CD9A630994FBCAD919F1CFA6BE014B756AF75BACE620601DD7BA91BFEE11E2
*
*EUICC证书个人化指令：
00A4040000
8020A400FF 021730820213308201B9A003020102020116300A06082A8648CE3D040302304D310B300906035504061302434E310B300906035504080C02424A3110300E060355040A0C0765617374636F6D311F301D06035504030C16534D44502B2045554D20287072696D65323536763129301E170D3137303631353130303633385A170D3138303631353130303633385A306C310B300906035504061302434E312930270603550405132038393034393033323132333435313233343531323334353637383930313333323110300E060355040A130765617374636F6D3120301E06035504031317534D44502B205549434320287072696D6532353676312930593013
8020A400FF 06072A8648CE3D020106082A8648CE3D03010703420004662D8AE22CD2547E463E7FA06CEFD0E10C83A4BC4B2E4F7DE06A778BC1A9B9B9157F3E8945F686FBDC4BFB19E2663C49A955B0F8CA8250592F9060272CEB146BA36B3069301F0603551D23041830168014FAE0C678C64F35D7A0752BFA1D8875D08E27E415301D0603551D0E041604143DF74C402E0E6460C3B949F61F6D02DB7CB9255C300E0603551D0F0101FF04040302078030170603551D200101FF040D300B3009060767811201020101300A06082A8648CE3D0403020348003045022100F5851CDDDE1E914420735F605A22E9BE3C9B6778EEAAA63F9AFDB46B0C4038EA02203A288C057F
8020A4001B 75B6357CF60B59A06AE2E9554946F1142AAFB4438796454CB821F6
*
*SK个人化指令：
00A4040000
8020A50022 0020D3B6DCBE5BF54EC2ACEBE8B7B0933A11148B10D09AFD3A41EC3A30C76EFF71B4
*
*@history    :modified by xuxiaojun@20210414 newObject 和 _newarrayObject内已经存在异常判断
*注意        :在个人化证书过程中不允许插入任何其他的指令，包括个人化另外一个证书，且证书的第一条指令的数据长度必须大于2。 且必须保证证书传入长度的正确性，包括证书总长度必须和证书结构中的长度相符合
*            :按照目前的证书个人化实现方案，只有个人化失败，都需要重新做卡，因为证书第一条指令中会分配引用，并写入。而如果后续发生错误时，并没有将相关数据清除。
*            :该函数中开启事务只是为了分配空间
*/
void EUICCPersonalCAEUMAndEUICCData(void)
{
	//CAEUMAndEUICCList xdata caEumEuiccList;
	UWORD xdata caAddr, caAddrLast, caAddrPre;
	WORD idata caRef, caLen,nextListRef,curCertEUMRef,curCertEUICCRef,skEuiccRef;
	BYTE idata eidNum[0x10], i, index;

#if EUICC__FUNCTION__GUOMI_eSIM
	BYTE idata CertType;
	CertType = Cert_ECC;
#endif
	caAddr = ADDR_IS_NULL;

	if((P1 != PERSONALTAG_CERTEUM) && (P1 != PERSONALTAG_CERTEUICC) && (P1 != PERSONALTAG_SKEUICC))
	{
		ThrowError(SW_INCORRECT_P1P2); //不是EUM证书、EUICC证书或EUICC证书的SK，抛错
	}

	skEuiccRef = REF_NULL;
	curCertEUMRef = REF_NULL;
	curCertEUICCRef = REF_NULL;
	nextListRef = REF_NULL;
	index = 0;

	caRef = memReadU16(MEMtPM8(EUICCEUMUICCCARef_Addr));//ReadEEPromCache((BYTE *)&caRef, EUICCEUMUICCCARef_Addr, 0x02); //取EUICCEUMUICCCARef_Addr的值，复用caRef

	beginTransaction(0);

	if(caRef == REF_NULL) //证书链为空
	{
		caRef = newObject(CAEUMANDEUICCLIST_CLASS_OFFSET, ECASD_PKG_ID, BIT_NEED_REFRESH);

		caAddrLast = locateObjectInMemory(caRef);//VMCOSHeapAlloc(sizeof(caEumEuiccList), &caRef, CardMemory_Mode);

		memWriteU16Atomic(MEMtPM8(EUICCEUMUICCCARef_Addr),caRef);//VMWriteBuf(EUICCEUMUICCCARef_Addr, (BYTE *)&caRef, sizeof(caRef)); //存储第一个CA证书的引用

		memWriteU8Atomic(MEMtPM8(CAEUMAndEUICCListGet_EUMAndEUICCCertIndexAddr(caAddrLast)),P2);//caEumEuiccList.index = P2;
	}
	else //证书链不为空
	{
		caAddrPre = GetAddrByIndexAddr(EUICCEUMUICCCARef_Addr);

		//ReadEEPromCache((BYTE *)&caEumEuiccList, caAddrPre, sizeof(caEumEuiccList));
		nextListRef = memReadU16(MEMtPM8(CAEUMAndEUICCListGet_nextEUMAndEUICCListRefAddr(caAddrPre)));
		index = memReadU8(MEMtPM8(CAEUMAndEUICCListGet_EUMAndEUICCCertIndexAddr(caAddrPre)));

		while((nextListRef != REF_NULL) && (index != P2)) //EUM证书,EUICC证书,SK的index不等于P2且不是最后一个证书链
		{
			caAddrPre = locateObjectInMemory(nextListRef);//G_AddrIndexTablePointer[caEumEuiccList.nextListRef-1].dataAddr;

			//ReadEEPromCache((BYTE *)&caEumEuiccList, caAddrPre, sizeof(caEumEuiccList));
			nextListRef = memReadU16(MEMtPM8(CAEUMAndEUICCListGet_nextEUMAndEUICCListRefAddr(caAddrPre)));
			index = memReadU8(MEMtPM8(CAEUMAndEUICCListGet_EUMAndEUICCCertIndexAddr(caAddrPre)));
		}

		if(index != P2)//需在证书链创建新的证书链
		{
			caRef = newObject(CAEUMANDEUICCLIST_CLASS_OFFSET, ECASD_PKG_ID, BIT_NEED_REFRESH);

			caAddrLast = locateObjectInMemory(caRef);//caAddrLast = VMCOSHeapAlloc(sizeof(caEumEuiccList), &caRef, CardMemory_Mode);

			memWriteU16Atomic(MEMtPM8(CAEUMAndEUICCListGet_nextEUMAndEUICCListRefAddr(caAddrPre)),caRef);//VMWriteBuf(caAddrPre+CAEUMAndEUICCList_nextListRef_Offset, (BYTE *)&caRef, sizeof(WORD)); //将新证书链加入证书链

			memWriteU8Atomic(MEMtPM8(CAEUMAndEUICCListGet_EUMAndEUICCCertIndexAddr(caAddrLast)),P2);//caEumEuiccList.index = P2;

		}
		else //不需创建新的证书链
		{
			skEuiccRef = memReadU16(MEMtPM8(CAEUMAndEUICCListGet_skEUICCArraysAddr(caAddrPre)));
			curCertEUMRef = memReadU16(MEMtPM8(CAEUMAndEUICCListGet_curCertEUMArrayAddr(caAddrPre)));
			curCertEUICCRef = memReadU16(MEMtPM8(CAEUMAndEUICCListGet_curCertEUICCArrayAddr(caAddrPre)));;

			if(G_CAWrite == 0x00) //第一条指令，判断证书是否已存在
			{

				if((P1 == PERSONALTAG_CERTEUM) && (curCertEUMRef != REF_NULL)) //CERT_EUICC
				{
					ThrowError(SW_SECURITY_STATUS_NOT_SATISFIED); //EUICC证书已存在，抛错
				}
				else if((P1 == PERSONALTAG_CERTEUICC) && (curCertEUICCRef != REF_NULL)) //CERT_EUICC
				{
					ThrowError(SW_SECURITY_STATUS_NOT_SATISFIED); //EUICC证书已存在，抛错
				}
				else if((P1 == PERSONALTAG_SKEUICC) && (skEuiccRef != REF_NULL)) //SK_EUICC
				{
					ThrowError(SW_SECURITY_STATUS_NOT_SATISFIED); //SK已存在，抛错
				}
			}

			caAddrLast = caAddrPre;
		}
	}

	if(G_CAWrite == 0x00) //CERT_EUM、CERT_EUICC、SK_EUICC的第一条put certificate的处理
	{
		if(P3 < 0x02) //第一条长度的值必须保证完整
		{
			ThrowError(SW_WRONG_DATA);
		}

		G_CALength = BuildWord(&RecData[0]);

		if(P1 == PERSONALTAG_CERTEUM)
		{
			caRef = _newarrayObject(AType_Byte, G_CALength+2, BIT_NEED_REFRESH);
			memWriteU16Atomic(MEMtPM8(CAEUMAndEUICCListGet_curCertEUMArrayAddr(caAddrLast)),caRef);
		}
		else if(P1 == PERSONALTAG_CERTEUICC) //
		{
			caRef = _newarrayObject(AType_Byte, G_CALength+2, BIT_NEED_REFRESH);
			memWriteU16Atomic(MEMtPM8(CAEUMAndEUICCListGet_curCertEUICCArrayAddr(caAddrLast)),caRef);
		}
		else//PERSONALTAG_SKEUICC   SK == L(1byte) V
		{
			caRef = _newarrayObject(AType_Byte, G_CALength+1, BIT_NEED_REFRESH);

			memWriteU16Atomic(MEMtPM8(CAEUMAndEUICCListGet_skEUICCArraysAddr(caAddrLast)),caRef);
		}

		caAddr = get_array_data((PF8)locateObjectInMemory(caRef));
	}
	else
	{
		//if(P1 != PERSONALTAG_SKEUICC)
		//{
			if(P1 == PERSONALTAG_CERTEUM)
			{
				caRef = curCertEUMRef;
			}
			else if(P1 == PERSONALTAG_CERTEUICC)
			{
				caRef = curCertEUICCRef;
			}
			else//PERSONALTAG_SKEUICC
			{
				caRef = skEuiccRef;
			}

			caAddr = get_array_data((PF8)locateObjectInMemory(caRef));//G_AddrIndexTablePointer[caRef-1].dataAddr;
		//}

	}

	if(P1 != PERSONALTAG_SKEUICC) //写入数据，不包括SK
	{
		VMWriteBuf(caAddr+G_CAWrite, &RecData[0], P3); //把证书数据写入caAddr
	}

	G_CAWrite += P3; //更新已收数据长度

	if(G_CAWrite > (G_CALength + 0x02)) //证书个人化指令有问题，需要重新做卡
	{
		ThrowError(0x6A80);
	}

	if(P1 == PERSONALTAG_CERTEUM) //处理CERT_EUM
	{
		//caEumEuiccList.curCertEUMRef = caRef;

		if(G_CAWrite == (G_CALength + 0x02)) //CERT_EUM收完
		{
#if EUICC__FUNCTION__GUOMI_eSIM
			TBSCertificate_GetField((BYTE *)(caAddr+2), G_CALength, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm, &tempBuf[0]); //CERT.DPpb.SM2SIG  取出曲线参数 1字节  LV结构 tempBuf[1]存曲线参数

			if((sizeof(SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI) == tempBuf[0]) && (EQUAL == ecpMem_Compare(&SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI[0], &tempBuf[1], tempBuf[0])))
			{
				CertType = Cert_GUOMI;
			}
#endif
			G_CAWrite = 0x00;

			G_CALength = 0x00; //清空长度
		}
	}
	else if(P1 == PERSONALTAG_CERTEUICC) //处理CERT_EUICC
	{
		//caEumEuiccList.curCertEUICCRef = caRef;

		if(G_CAWrite == (G_CALength+0x02))//CERT_EUICC收完，开始校验EID，取算法参数
		{
#if EUICC__FUNCTION__GUOMI_eSIM
			TBSCertificate_GetField((BYTE *)(caAddr+2), G_CALength, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_algorithm, &tempBuf[0]); //CERT.DPpb.SM2SIG  取出曲线参数 1字节  LV结构 tempBuf[1]存曲线参数

			if((sizeof(SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI) == tempBuf[0]) && (EQUAL == ecpMem_Compare(&SUBJECTPublicKeyInfo_Alg_Algorithm_GUOMI[0], &tempBuf[1], tempBuf[0])))
			{
				CertType = Cert_GUOMI;
			}
#endif
			caLen = G_CALength;

			G_CAWrite = 0x00;

			G_CALength = 0x00; //清空长度

			if(RESPONSE_NoError == TBSCertificate_GetField((BYTE *)(caAddr+2), caLen, FIELD_Certificate_tbsCertificate_subject_serialNumber, tempBuf))
			{
				if(tempBuf[0] != 0x20)
				{
					ThrowError(SW_WRONG_DATA);//证书中EID ASCII的长度错误
				}

				for(i=0;i<tempBuf[0];i++) //将ascii码的EID转换成hex，通过CharToInt转换后的数据需要按顺序将前一个字节的低4位作为高4位和后一个字节的低4位组成一个字节（如第1个字节和第2个字节，第3个字节和第4个字节）
				{
					if(i%2 == 0) //第i+1个字节，i为偶数，需向左移位
					{
						eidNum[i/2] = CharToInt(tempBuf[1+i],ASCiitoHex);

						eidNum[i/2] = (eidNum[i/2] << 4) & 0xF0;
					}
					else //i为奇数，不需要移位
					{
						eidNum[i/2] |= (CharToInt(tempBuf[1+i],ASCiitoHex) & 0x0F);
					}
				}

				if (VMReadByte(EUICCEIDInfo_Addr) == 0x10) //卡片已更新EID
				{
					if(EQUAL != NvmCmp((BYTE *)(EUICCEIDInfo_Addr+1),(BYTE *)&eidNum[0], 0x10)) //EID不匹配，释放heap空间，抛异常
					{
//						VMCOSHeapFree(caEumEuiccList.curCertEUICCRef, caLen+2, CardMemory_Mode);
//
//						VMSetBuf(caAddrLast+CAEUMAndEUICCList_curCertEUICCRef_Offset, REF_NULL, sizeof(WORD)); //更新证书链中EUICC证书的ref

						memWriteU16NoAtomic(MEMtPM8(CAEUMAndEUICCListGet_curCertEUICCArrayAddr(caAddrLast)),REF_NULL);
						bRamBuffer[GC_Requested] = 1;//打垃圾回收标记
						ThrowError(SW_WRONG_DATA);
					}
				}
				else //卡片未更新EID，需更新卡片EID
				{
					VMSetBuf(EUICCEIDInfo_Addr, 0x10, 0x01);

					VMWriteBuf(EUICCEIDInfo_Addr+1, &eidNum[0], 0x10);
				}
			}
			else
			{
				ThrowError(SW_WRONG_DATA);//没有EID字段，抛错
			}

#if EUICC__FUNCTION__GUOMI_eSIM
			if(CertType != Cert_GUOMI) //国密证书的算法参数是缺省的
#endif
			{
				if(RESPONSE_NoError == TBSCertificate_GetField((BYTE *)(caAddr+2), caLen, FIELD_Certificate_tbsCertificate_subjectPublicKeyInfo_algorithm_parameters, tempBuf)) //查找algorithm.parameter
				{
					memWriteU8Atomic(MEMtPM8(CAEUMAndEUICCListGet_parameterEUICCAddr(caAddrLast)),tempBuf[1]);//caEumEuiccList.parameterEUICC = tempBuf[1]; //把algorithm parameters对应的ECC参数引用更新到链表结构中
				}
				else
				{
					ThrowError(SW_WRONG_DATA);
				}
			}
		}
	}
	else //处理SK_EUICC
	{
		if(G_CALength != 0x20) //由于现在只支持ECC256，所以这里SK的长度是固定32字节
		{
			ThrowError(SW_WRONG_DATA);
		}

		if(G_CAWrite == P3) //SK的第一条，RecData中包含长度
		{
			VMWriteBuf(caAddr,&RecData[1], P3-1);//ecpMem_Copy(&caEumEuiccList.skEUICCValue[G_CAWrite-P3], &RecData[1], P3-1);
		}
		else
		{
			VMWriteBuf(caAddr+G_CAWrite-P3-1,&RecData[1], P3);//ecpMem_Copy(&caEumEuiccList.skEUICCValue[G_CAWrite-P3-1], &RecData[0], P3);
		}

		if(G_CAWrite == (G_CALength+0x02)) //SK_EUICC收完
		{
			G_CAWrite = 0x00;

			G_CALength = 0x00; //清空长度
		}
	}

	//VMWriteBuf(caAddrLast, (BYTE *)&caEumEuiccList, sizeof(caEumEuiccList)); //更新证书链

	TransactionCommit();
}
#endif


/*
*@function   :M2M_EUICCPersonalEUICCData
*
*@description:个人化EUM,EUICC数据和EUICC的SK, 该函数支持M2M版的证书个人化。证书的个人化格式为：证书总长度 + 证书数据
*
*@param      :void
*
*@return     :void
*
*@history     :v1.0 creat by lx @20181207
*			   v1.1 modify by jiangqb @20191031	-增加euicc证书写入时对eid的值判断
*
*@test       :
*
*EUICC证书个人化指令：
00A4040000
8020A400FF 01057F21820100931054DF673A0B0E20D4DC23B9A76F1BC661421047544F47544F000000000012340000005F201089001012012341234012345678901224950200805F2504201410095F240420240101450BBBBBBBBBBBBBBBBBBBBBBB731CC000C100C200C91435581BA3AA64EAF79DB8235AC713510E2E2F66B67F4946B041043F157A6242EC54888EB50967BD84D1A885E51D66B2F6CD135354724C91D790EDB48B015F6B272DF50E49EAB2E1383BF0EEB0E9848543B971397D274D88E8EA77F001005F3740BC1E5C5BD97A98706A57FB8AE52DA043E7A5627E404F0346DF575D9B1E4E97B032B0AE7B47A0898EE8EF82D1EC11548D4F6EAA510C5B2BE9
8020A40008 4BEA4F24832B7353
*
*SK个人化指令：
00A4040000
8020A50022 00204ECBE3647D24F4C330D5C137671321F45EBB9CBE6D6C1D804019AAB456632156
*
*@history    : modified by xuxiaojun@20210414 newObject和_newarrayObject内已经存在异常判断
*注意        :在个人化证书过程中不允许插入任何其他的指令，包括个人化另外一个证书，且证书的第一条指令的数据长度必须大于2。 且必须保证证书传入长度的正确性，包括证书总长度必须和证书结构中的长度相符合
*            :按照目前的证书个人化实现方案，只有个人化失败，都需要重新做卡，因为证书第一条指令中会分配引用，并写入。而如果后续发生错误时，并没有将相关数据清除。
*            :该函数中开启事务只是为了分配空间
*/
#if M2M_EUICC
void M2M_EUICCPersonalEUICCData(void)
{
//	M2MCAEUICCList xdata caEuiccList;
	WORD idata caRef, caOffset, tagOffset;
	UWORD xdata caAddrList, caAddr, caLen;
	BYTE idata eidLen, caTemp[2];
	WORD xdata   curCertEUICCRef,skEuiccRef;
	
	caAddr = ADDR_IS_NULL;
	
	if((P1 != PERSONALTAG_CERTEUICC) && (P1 != PERSONALTAG_SKEUICC))
	{
		ThrowError(SW_INCORRECT_P1P2); //不是EUICC证书或EUICC证书的SK，抛错
	}
	
	caRef =  memReadU16(MEMtPM8(M2M_EUICCEUMUICCCARef_Addr));//ReadEEPromCache((BYTE *)&caRef, EUICCEUMUICCCARef_Addr, 0x02); //取EUICCEUMUICCCARef_Addr的值，复用caRef
	
	beginTransaction(0);
	
	if(caRef == REF_NULL) //证书链为空
	{
		caRef = newObject(M2MCAEUICCLIST_CLASS_OFFSET, ECASD_PKG_ID, BIT_NEED_REFRESH);

		caAddrList = locateObjectInMemory(caRef);
		
		memWriteU16Atomic(MEMtPM8(M2M_EUICCEUMUICCCARef_Addr),caRef);//存储CA证书的引用
		
		//ecpMem_Set(&caEuiccList, 0x00, sizeof(caEuiccList)); //caEuiccList设为新证书链
	}
	else //证书链不为空
	{
		caAddrList = GetAddrByIndexAddr(M2M_EUICCEUMUICCCARef_Addr);
							
		curCertEUICCRef = memReadU16(MEMtPM8(M2MCAEUICCListGet_curCertEUICCArrayAddr(caAddrList)));//ReadEEPromCache((BYTE *)&caEuiccList, caAddrList, sizeof(caEuiccList));

		skEuiccRef = memReadU16(MEMtPM8(M2MCAEUICCListGet_skEUICCArrayAddr(caAddrList)));

		if(G_CAWrite == 0x00) //第一条指令
		{
			if((P1 == PERSONALTAG_CERTEUICC) && (curCertEUICCRef != REF_NULL)) //CERT_EUICC
			{
				ThrowError(SW_SECURITY_STATUS_NOT_SATISFIED); //EUICC证书已存在，抛错
			}
			else if((P1 == PERSONALTAG_SKEUICC) && (skEuiccRef != REF_NULL)) //SK_EUICC
			{
				ThrowError(SW_SECURITY_STATUS_NOT_SATISFIED); //SK已存在，抛错
			}
		}
	}
	
	if(G_CAWrite == 0x00) //第一条put certificate的处理
	{
		if(P3 < 0x02) //第一条长度的值必须保证完整
		{
			ThrowError(SW_WRONG_DATA);
		}
			
		G_CALength = BuildWord(&RecData[0]);
		
		if(P1 != PERSONALTAG_SKEUICC) //SK的值存在证书链里，不需要重新分配空间
		{
			caRef = _newarrayObject(AType_Byte, G_CALength+2, BIT_NEED_REFRESH);

			memWriteU16Atomic(MEMtPM8(M2MCAEUICCListGet_curCertEUICCArrayAddr(caAddrList)),caRef);
		}
		else //SK == L(1byte) V
		{
			caRef = _newarrayObject(AType_Byte, G_CALength+1, BIT_NEED_REFRESH);

			memWriteU16Atomic(MEMtPM8(M2MCAEUICCListGet_skEUICCArrayAddr(caAddrList)),caRef);
		}
	}
	else
	{	
		if(P1 == PERSONALTAG_CERTEUICC) //CERT_EUICC
		{
			caRef = memReadU16(MEMtPM8(M2MCAEUICCListGet_curCertEUICCArrayAddr(caAddrList)));//caEuiccList.curCertEUICCRef;
		}
		else
		{
			caRef = memReadU16(MEMtPM8(M2MCAEUICCListGet_skEUICCArrayAddr(caAddrList)));
		}
	}
	
	caAddr = get_array_data((PF8)locateObjectInMemory(caRef));

	if(P1 != PERSONALTAG_SKEUICC) //写入数据，不包括SK
	{		
		VMWriteBuf(caAddr+G_CAWrite, &RecData[0], P3); //把证书数据写入caAddr
	}
	
	G_CAWrite += P3; //更新已收数据长度

	if(G_CAWrite > (G_CALength + 0x02)) //证书个人化指令有问题，需要重新做卡
	{
		ThrowError(0x6A80);
	}
			
	if(P1 == PERSONALTAG_CERTEUICC) //处理CERT_EUICC
	{
		//caEuiccList.curCertEUICCRef = caRef;
				
		if(G_CAWrite == (G_CALength+0x02))//CERT_EUICC收完，开始校验EID，取算法参数
		{			
			caOffset = 4; //跳过证书的L和tag'7F21'				
			
			caOffset += ASN1_GetBERTlvLen(caAddr+caOffset, (UWORD*)&caLen); //取证书的长度，并偏移到长度后的第一个字节
			
			tagOffset = ASN1_FindTagTLV(caAddr+caOffset, caLen, 0x5F20); //5F20是EID的tag，tagOffset表示0x5F20相对于caAddr+caOffset的偏移量
			
			caLen = G_CALength;
			
			G_CAWrite = 0x00;
				
			G_CALength = 0x00; //清空长度
			
			if(tagOffset == TAG_NOT_FOUND)
			{
				ThrowError(SW_WRONG_DATA); //找不到EID，抛错
			}
			
			caOffset += (tagOffset+2); //偏移到5F20后的第一个字节
			
			eidLen = ReadEECacheByte(caAddr+caOffset); //取EUICC证书中EID的长度
			
			if(eidLen != 0x10)
			{
				ThrowError(SW_WRONG_DATA); //EID长度不等于0x10，抛错。《SGP.02_v3.2_updated》4.1.1.5 eUICC Capability Audit规定EID长度为0x10
			}	

			VMReadBuf(&tempBuf[0], caAddr+caOffset, 0x11);
			
			if (VMReadByte(EUICCEIDInfo_Addr) == 0x10) //卡片已更新EID
			{					
				if(EQUAL != NvmCmp((BYTE *)(EUICCEIDInfo_Addr),(BYTE *)&tempBuf[0], 0x11)) //EID不匹配，释放heap空间，抛异常
				{		
					ThrowError(SW_WRONG_DATA);
				}
			}
			else 															//卡片未更新EID，需更新卡片EID
			{	
				VMWriteBuf(EUICCEIDInfo_Addr, &tempBuf[0] , 0x11); //将EUICC证书中的EID写入EUICCEIDInfo_Addr，LV结构	
			}
	
			caOffset = 4; //跳过证书的L和tag'7F21'，开始找Key Parameter Reference
			
			caOffset += ASN1_GetBERTlvLen(caAddr+caOffset, (UWORD*)&caLen); //取证书的长度，并偏移到长度后的第一个字节
			
			tagOffset = ASN1_FindTagTLV(caAddr+caOffset, caLen, 0x7F49); //7F49是Public Key的tag，tagOffset表示0x7F49相对于caAddr+caOffset的偏移量
			
			if(tagOffset == TAG_NOT_FOUND)
			{
				ThrowError(SW_WRONG_DATA); //找不到Public Key，抛错
			}
			
			caOffset += (tagOffset+2); //偏移到7F49后的第一个字节
			
			caOffset += ASN1_GetBERTlvLen(caAddr+caOffset, (UWORD*)&caLen); //取7F49的长度，并偏移到长度后的第一个字节
			
			tagOffset = ASN1_FindTagTLV(caAddr+caOffset, caLen, 0xF0); //F0是Key Parameter Reference的tag，tagOffset表示0xF0相对于caAddr+caOffset的偏移量
			
			if(tagOffset == TAG_NOT_FOUND)
			{
				ThrowError(SW_WRONG_DATA); //找不到Key Parameter Reference，抛错
			}
			
			caOffset += (tagOffset+1); //偏移到F0后的第一个字节
			
			VMReadBuf(&caTemp[0], caAddr+caOffset, 0x02);
			
			if(caTemp[0] == 0x01)
			{				
				memWriteU8Atomic(MEMtPM8(M2MCAEUICCListGet_parameterEUICCAddr(caAddrList)),caTemp[1]);//caEuiccList.parameterEUICC = caTemp[1]; //把Key Parameter Reference更新到链表结构中
			}
			else
			{
				ThrowError(SW_WRONG_DATA); //Key Parameter Reference长度错误，抛错
			}
		}
	}
	else //处理SK_EUICC
	{
		if(G_CALength != 0x20) //由于现在只支持ECC256，所以这里SK的长度是固定32字节
		{
			ThrowError(SW_WRONG_DATA);
		}
		
		if(G_CAWrite == P3) //SK的第一条，RecData中包含长度
		{
			VMWriteBuf(caAddr,&RecData[1], P3-1);//ecpMem_Copy(&caEuiccList.skEUICCValue[G_CAWrite-P3], &RecData[1], P3-1);
		}
		else
		{
			VMWriteBuf(caAddr+G_CAWrite-P3-1,&RecData[1], P3);//ecpMem_Copy(&caEuiccList.skEUICCValue[G_CAWrite-P3-1], &RecData[0], P3);
		}
		
		if(G_CAWrite == (G_CALength+0x02)) //SK_EUICC收完
		{
			G_CAWrite = 0x00;
			
			G_CALength = 0x00; //清空长度
		}
	}
	
	//VMWriteBuf(caAddrList, (BYTE *)&caEuiccList, sizeof(caEuiccList)); //更新证书链
	
	TransactionCommit();
}
#endif
/*
*@fucntion   : EUICCCalCERTCheckSum
*
*@description: 分别计算CA CERT(20字节的sha1值)/EUM CERT(20字节的sha1值)/EUICC CERTEUICC SK(20字节的sha1值)
*
*@param      : void
*
*@return     :CA checksum + EUM checksum + euicc checksum  sk chencksum
*
*@author     :xuyz
*
*@history    :
*/
void EUICCCalCERTCheckSum(void)
{
//	BYTE idata tmp[2];
	WORD idata caSize;
	UWORD xdata caListAddr,curCertAddr,nextAddr;
	BYTE  xdata HASHmdBuff[SHA1_ICV_SIZE+2];
	BYTE  xdata HASHtmpBuff[SHA1_BLOCK_SIZE+1];
#if RSP_EUICC
	BYTE  xdata HASHmdBuff2[SHA1_ICV_SIZE+2];
	BYTE  xdata HASHtmpBuff2[SHA1_BLOCK_SIZE+1];
#endif
	if(P3 != 0x00)
	{
		ThrowError(0x6700); 
	}
	
	ecpMem_Set(tempBuf, 0x00, sizeof(tempBuf));  //先清空缓存结果的变量
	
	caListAddr = GetAddrByIndexAddr(EUICCCARef_Addr);  //判读CA是否存在

	//SHA1Init(&G_Sha1Context);
	HASHUpdate(NULL, 0, HASH_INIT, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
	while(caListAddr != ADDR_IS_NULL)
	{
		curCertAddr = CACIListGet_curCertCIArrayValueAddr(caListAddr);//GetAddrByIndexAddr(caListAddr + CACIList_CurCertCIRef_Offset);
		
		if(curCertAddr != ADDR_IS_NULL) //计算CERT 证书的sha1值
		{
			//VMReadBuf(tmp, curCertAddr, 0x02);  //读取两个字节的CASize
			
			caSize = memReadU16(MEMtPM8(curCertAddr));//BuildWord(tmp);
			
			//SHA1Update(&G_Sha1Context,0,caSize,0x01,curCertAddr+2);
			HASHUpdate((PM8)(curCertAddr+2), caSize, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
		}
		
#if M2M_EUICC
		curCertAddr = CACIListGet_pkIdCIArrayValueAddr(caListAddr);//GetAddrByIndexAddr(caListAddr + CACIList_PkIdCIRef_Offset);  // 计算PKID 的sha1值
		
		if(curCertAddr != ADDR_IS_NULL)
		{
			caSize = VMReadByte(curCertAddr);//读取1字节的pkid len
			
			//SHA1Update(&G_Sha1Context,0,caSize,0x01,curCertAddr+1);
			HASHUpdate((PM8)(curCertAddr+1), caSize, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
		}
		
		//SHA1Update(&G_Sha1Context,0,64,0x01,CACIListGet_pkCIArrayValueAddr(caListAddr));//SHA1Update(&G_Sha1Context,0,64,0x01,caListAddr+CACIList_PkCIValue_Offset);
		HASHUpdate((PM8)(CACIListGet_pkCIArrayValueAddr(caListAddr)), 64, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
#endif		

		nextAddr = GetAddrByIndexAddr(CACIListGet_nextCIListRefAddr(caListAddr));////GetAddrByIndexAddr(caListAddr + CACIList_NextListRef_Offset);
		
		if(nextAddr == ADDR_IS_NULL)   //说明当前为最后一条CA 数据
		{ 
			//sha1Caculate(&G_Sha1Context , &tempBuf[0]);   //tempBuf[0] - tempBuf[19] 保存CA证书中CERT 、PKID、PublicKey Value的值
			HASHDoFinal(NULL, 0, &tempBuf[0], HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
		}	
		caListAddr = nextAddr;		
	}
	
#if RSP_EUICC
	caListAddr = GetAddrByIndexAddr(EUICCEUMUICCCARef_Addr);// GetAddrByIndexAddr(EUICCEUMUICCCARef_Addr);  //计算EUM/EUICC证书的sha1
	
	//SHA1Init(&G_LoadSha1Context);      /*初始化结构体*/
	
	//SHA1Init(&G_Sha1Context);   //借用两个结构体变量，分别保存EUM CERT的sha1 和EUICC 的hasj值
	HASHUpdate(NULL, 0, HASH_INIT, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
	HASHUpdate(NULL, 0, HASH_INIT, HASHmdBuff2, HASHtmpBuff2, ALG_SHA_1);
	while(caListAddr != ADDR_IS_NULL)
	{
		nextAddr = GetAddrByIndexAddr(CAEUMAndEUICCListGet_nextEUMAndEUICCListRefAddr(caListAddr));//GetAddrByIndexAddr(caListAddr + CACIList_NextListRef_Offset);这里应该是CAEUMAndEUICCList_nextListRef_Offset
		
		curCertAddr = CAEUMAndEUICCListGet_curCertEUMArrayValueAddr(caListAddr);//GetAddrByIndexAddr(caListAddr + CAEUMAndEUICCList_curCertEUMRef_Offset);
		
		if(curCertAddr != ADDR_IS_NULL)
		{
			//VMReadBuf(tmp, curCertAddr, 0x02);  //读取两个字节的CASize
			
			caSize = memReadU16(MEMtPM8(curCertAddr));
			
			//SHA1Update(&G_LoadSha1Context,0,caSize,0x01,curCertAddr+2);
			if(nextAddr != ADDR_IS_NULL)
			{
				HASHUpdate((PM8)(curCertAddr+2), caSize, HASH_UPDATE, HASHmdBuff2, HASHtmpBuff2, ALG_SHA_1);
			}
			else
			{
				//ha1Caculate(&G_LoadSha1Context , &tempBuf[20]);  
				HASHDoFinal((PM8)(curCertAddr+2), caSize, &tempBuf[20], HASH_UPDATE, HASHmdBuff2, HASHtmpBuff2, ALG_SHA_1);
			}
		}
		
		curCertAddr = CAEUMAndEUICCListGet_curCertEUICCArrayValueAddr(caListAddr);//GetAddrByIndexAddr(caListAddr + CAEUMAndEUICCList_curCertEUICCRef_Offset);
		
		if(curCertAddr != ADDR_IS_NULL)
		{
			//VMReadBuf(tmp, curCertAddr, 0x02);  //读取两个字节的CASize
			
			caSize = memReadU16(MEMtPM8(curCertAddr));//BuildWord(tmp);
			
			//SHA1Update(&G_Sha1Context,0,caSize,0x01,curCertAddr+2);
			HASHUpdate((PM8)(curCertAddr+2), caSize, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
		}
		
		//SHA1Update(&G_Sha1Context,0,32,0x01,CAEUMAndEUICCListGet_skEUICCArraysValueAddr(caListAddr)+1);//SHA1Update(&G_Sha1Context,0,32,0x01,caListAddr+CAEUMAndEUICCList_skEUICCValueOffset+1);
		if(nextAddr != ADDR_IS_NULL)
		{
			HASHUpdate((PM8)(CAEUMAndEUICCListGet_skEUICCArraysValueAddr(caListAddr)+1), 32, HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
		}
		else
		{
			//sha1Caculate(&G_Sha1Context , &tempBuf[40]);  
			HASHDoFinal((PM8)(CAEUMAndEUICCListGet_skEUICCArraysValueAddr(caListAddr)+1), 32, &tempBuf[40], HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
		}
	
		caListAddr = nextAddr;				
	}
#else
	//tempBuf[20] -tempBuf[39]为EUM 的证书的sha1 ，设备版由于没有EUM 证书，此处为0
	
	caListAddr = GetAddrByIndexAddr(M2M_EUICCEUMUICCCARef_Addr);  //计算EUICC证书的sha1
	
	//SHA1Init(&G_Sha1Context);   // EUICC 的hash值
	
	if(caListAddr != ADDR_IS_NULL)
	{
		curCertAddr = M2MCAEUICCListGet_curCertEUICCArrayValueAddr(caListAddr);// GetAddrByIndexAddr(caListAddr + M2MCAEUICCList_curCertEUICCRef_Offset);
		
		if(curCertAddr != ADDR_IS_NULL)
		{
			//VMReadBuf(tmp, curCertAddr, 0x02);  //读取两个字节的CASize
			
			caSize = memReadU16(MEMtPM8(curCertAddr));//BuildWord(tmp);
			
			//SHA1Update(&G_Sha1Context,0,caSize,0x01,curCertAddr+2);
			HASHUpdate(MEMtPM8(curCertAddr+2), caSize, HASH_INIT, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
			//SHA1Update(&G_Sha1Context,0,32,0x01,M2MCAEUICCListGet_skEUICCArrayValueAddr(caListAddr)+1);//SHA1Update(&G_Sha1Context,0,32,0x01,caListAddr+M2MCAEUICCList_skEUICCValue_Offset+1);
			HASHDoFinal((PM8)(M2MCAEUICCListGet_skEUICCArrayValueAddr(caListAddr)+1), 32, &tempBuf[40], HASH_UPDATE, HASHmdBuff, HASHtmpBuff, ALG_SHA_1);
			//sha1Caculate(&G_Sha1Context , &tempBuf[40]); 
		}
	}

#endif
	
	ecpMem_Copy(RecData, tempBuf, 0x3C);
	
	SetOutgoingAndSend(0x05,0x3C);
}
/*
*@function   :EUICCPersonalPKIDData
*
*@description:更新CI 的 publicKey或 PKID 信息
*
*@param      :void
*
*@return      :void
*
*@history     :V1.0 :create @20190311
*@history     :V1.1 :modified by xuxiaojun@20210414 newObject和_newarrayObject内已经存在异常判断
*/
#if M2M_EUICC
void M2M_EUICCPersonalPKIDData(void)
{
	WORD idata caRef,pkIdRef,pkCIArrayRef;
	UWORD xdata caAddr,nextcaAddr,writeAddr;
	
	caAddr = GetAddrByIndexAddr(EUICCCARef_Addr);
	
	writeAddr = EUICCCARef_Addr;
	
	nextcaAddr = caAddr;
	
	while(nextcaAddr != ADDR_IS_NULL)
	{
		nextcaAddr = GetAddrByIndexAddr(CACIListGet_nextCIListRefAddr(caAddr));//GetAddrByIndexAddr(caAddr+CACIList_NextListRef_Offset);
		
		if(nextcaAddr == ADDR_IS_NULL)
		{
			writeAddr = CACIListGet_nextCIListRefAddr(caAddr);//caAddr + CACIList_NextListRef_Offset;
			break;
		}
			
		caAddr = nextcaAddr;
	}
	
	if(G_CAWrite == 0) //当前收到的是个人化CI证书的第1条指令，需要分配空间
	{
		if(P3 <76)  //第一条命令的长度必须大于76字节，76= L总长度 2 字节 + A0 L(2字节) + B0(1字节)+ 41(1字节) + 04 (1字节) +Pkvalue(64字节) + F0 01 XX(3字节) + A1 L
		{
			ThrowError(0x6A80);
		}
		
		if((RecData[2] != PERSONALTAG_PKCI)||(RecData[4] != 0xB0) ||(RecData[5] != 0x41) ||(RecData[6] != 0x04) ||(RecData[71] != 0xF0)||(RecData[72] != 0x01)||(RecData[74] != 0xA1) )
		{
			ThrowError(0x6A80);
		}
		
		G_CALength = BuildWord(RecData);
		
		beginTransaction(0);

		caRef = newObject(CACILIST_CLASS_OFFSET, ECASD_PKG_ID, BIT_NEED_REFRESH);

		caAddr = locateObjectInMemory(caRef);//VMCOSHeapAlloc(sizeof(CACIList), &caRef, CardMemory_Mode);// 先分配CACIList结构

		memWriteU8Atomic(MEMtPM8(CACIListGet_parameterCIAddr(caAddr)),RecData[73]);

		pkCIArrayRef = _newarrayObject(AType_Byte, 64, BIT_NEED_NOTHING);

		nextcaAddr = get_array_data(MEMtPF8(locateObjectInMemory(pkCIArrayRef))); // 修改编译错误

		VMWriteBuf(nextcaAddr,&RecData[7], 64);

		pkIdRef = _newarrayObject(AType_Byte, RecData[75]+1, BIT_NEED_REFRESH);//nextcaAddr = VMCOSHeapAlloc(RecData[75]+1, &pkIdRef, CardMemory_Mode);
		
		nextcaAddr = get_array_data(MEMtPF8(locateObjectInMemory(pkIdRef)));

		VMWriteBuf(nextcaAddr,  &RecData[75],   P3 - 75); 
		
		G_CAWrite =  P3 - 75;
		
		memWriteU16Atomic(MEMtPM8(CACIListGet_pkIdCIArrayAddr(caAddr)), pkIdRef);//VMWriteBuf(caAddr + CACIList_PkIdCIRef_Offset, (BYTE *)&pkIdRef, sizeof(WORD));
		
		memWriteU16Atomic(MEMtPM8(CACIListGet_pkCIArrayAddr(caAddr)), pkCIArrayRef);

		memWriteU16Atomic(MEMtPM8(writeAddr),caRef);//VMWriteBuf(writeAddr, (BYTE*)&caRef, 0x02); //把CACIList的引用链接到表的末尾，如果考虑到多条指令的话，这个引用应该是最后一条的时候才写入，需要将其做成全局变量
	
		TransactionCommit();
	}
	else
	{
		if(caAddr == ADDR_IS_NULL)
		{
			ThrowError(0x6A80);
		}
		
		writeAddr = CACIListGet_pkIdCIArrayValueAddr(caAddr);//GetAddrByIndexAddr(caAddr+CACIList_PkIdCIRef_Offset);
		
		if(writeAddr == ADDR_IS_NULL)
		{
			ThrowError(0x6A80);
		}
		
		VMWriteBuf(writeAddr + G_CAWrite, RecData, P3); //把证书数据写入证书的实际存储地址
	
		G_CAWrite += P3;                         //修改G_CAWrite的长度，用于表示当前已经存储的证书的数据长度
		
		if((G_CAWrite+ 73) > G_CALength)         //证书个人化指令有问题，需要重新做卡
		{
			ThrowError(0x6A80);
		}
	}
	
	if((G_CAWrite + 73) == G_CALength)
	{
		G_CAWrite = 0;
		
		G_CALength =0;
		
	}
}
#endif
/*
*@function   :EUICCPersonalEUICCInfo
*
*@description:设置EUICC INFO
*
*@param      :BYTE *buffer   1字节的L + V结构：CC 8103020100 8203020200 8303040200 840D 00000000000000000000000000 850405 580200 8603 120000 8703 020300 88020490 A92C 0414 F54172BDF98A95D65CBEB88A38A1C11D800A85C3 0414 C0BC70BA36929D43B467FF57570530E57AB8FCD8 AA2C 0414 F54172BDF98A95D65CBEB88A38A1C11D800A85C3 0414 C0BC70BA36929D43B467FF57570530E57AB8FCD8 8B0101 99020640 0403000101 0C1653415341636372656469746174696F6E4E756D626572 AC18800D506C6174666F726D4C6162656C8107446C6F6155726C
*
*@return     :void
*
*@author     :xuxiaojun@20181112
*
*@test       :C0C8040011 CC 8103020100 8203020200 8303040200 840D 00000000000000000000000000 850405 580200 8603 120000 8703 020300 88020490 A92C 0414 F54172BDF98A95D65CBEB88A38A1C11D800A85C3 0414 C0BC70BA36929D43B467FF57570530E57AB8FCD8 AA2C 0414 F54172BDF98A95D65CBEB88A38A1C11D800A85C3 0414 C0BC70BA36929D43B467FF57570530E57AB8FCD8 8B0101 99020640 0403000101 0C1653415341636372656469746174696F6E4E756D626572 AC18800D506C6174666F726D4C6162656C8107446C6F6155726C(9000)  ;set EUICC info
*
*@history    :created by xuxiaojun@20181112
*说明        :由于该指令是私有指令，本函数中不考虑EUICC INFO的合法性，由个人化要员保证其正确性、合法性。
*/
//void EUICCPersonalEUICCInfo2()
//{
//	UWORD xdata addr, e2Addr;
//	WORD idata ref;
//
//	e2Addr = ADDR_IS_NULL;
//
//	switch(P2)
//	{
//		case 0x01: // 个人化EUICCINFO2_SECTION1:  802005010F  810302010082030202008303040200
//		{
//			if(P3 != EUICCINFO2_SECTION1_LEN)
//			{
//				ThrowError(0x6A80);
//			}
//
//			WriteEeprom(EUICCINFO2_SECTION1, RecData, P3);  // 发卡之前不需要考虑断电
//
//			return;
//		}
//		case 0x02:// 个人化EUICCINFO2_SECTION2: 8020050213 8503017E3A86030902008703020300880204D0
//		{
//			e2Addr = EUICCINFO2_SECTION2;
//
//			if(P3 > EUICCINFO2_SECTION2_MAX_LEN)
//			{
//				ThrowError(0x6A80);
//			}
//			break;
//		}
//		case 0x03:
//		{
//			e2Addr = EUICCINFO2_CATEGORY;
//
//			if(P3 != EUICCINFO2_CATEGORY_LEN)
//			{
//				ThrowError(0x6A80);
//			}
//			break;
//		}
//		case 0x04:
//		{
//			e2Addr = EUICCINFO2_SECTION3;
//			break;
//		}
//		default:
//		{
//			ThrowError(0x6A80);
//		}
//	}
//	addr = GetAddrAndIndexByIndexAddr(e2Addr,&ref);
//
//	if(addr != ADDR_IS_NULL)//个人化不支持更新，要更新就重新做卡
//	{
//		ThrowError(0x6985);
//	}
//
//	beginTransaction(0);
//
//	addr = VMCOSHeapAlloc(P3 + 1, &ref, CardMemory_Mode);
//
//	if(addr == ADDR_IS_NULL)//个人化时空间就不够了, 此处开事务仅仅是为了分配空间，其实在个人化时没有必要做事务保护
//	{
//		ThrowError(0x6A84);
//	}
//
//	WriteEeprom(addr, (BYTE*)&G_ApduBuffer[4], P3 + 1);
//
//	WriteEeprom(e2Addr, (BYTE*)&ref, 0x02);
//
//	TransactionCommit();
//}
/*
*@function   :NM_EUICCPersonal
*
*@description:用于选择ISD-R之后执行个人化指令，P1表示TAG ， 当P1为0x01时表示, INS为0x20
*
*@param      :void
*
*@return     :void
*
*@test       :
*
*@history    :created by xuxiaojun@20181107
*/
//void NM_EUICCPersonal(void)
//{
//	if(JcvmConfig_GetCardState() == CARD_OP_READY)// 个人化ISD 密钥时，可以直接使用putkey指令，当卡片状态为Ready时，不要进行checkvalue检测
//	{
		//G_AddrIndexTablePointer = (AddrIndexTableStruct*)(ADDR_INDEX_TABLE + EEPROM_BASE + 2);
		//G_AddrIndexTablePointer = (AddrIndexTableStruct*)(0);
//
//		if(P1 == PERSONALTAG_EUICCINFO2)
//		{
//			EUICCPersonalEUICCInfo2();
//		}
//		else if(P1 == PERSONALTAG_RAT)  //表示当前个人化RAT, P3表示一条PPAR的长度，同样的PPR 规则下，最先更新的PPAR 优先级最高
//		{
//			EUICCPersonalRATData();
//		}
//		else if(P1 == PERSONALTAG_DEFAULTADDR)  //表示当前个人化SMDPDefaultAddress, P3表示address的长度
//		{
//			EUICCPersonalSMDPDefaultAddress(&G_ApduBuffer[4], P2);  // P2为1表示个人化Default SM-DP+ address / P2为其他值表示个人化root smds
//		}
//		else
//		{
//			ThrowError(0x6A86);
//		}
//	}
//	else  //卡片发卡后不支持20指令
//	{
//		ThrowError(0x6D00);
//	}
//
		
//}
/*
*@function   :NM_ECASDPersonal
*
*@description:用于选择ECASD之后执行个人化指令，P1表示TAG ， 当P1为0x01时表示, INS为0x20
*
*@param      :void
*
*@return     :void
*
*@test       :
*
*@history    :created by xuxiaojun@20190509
*/
void NM_ECASDPersonal(void)
{
	if(JcvmConfig_GetSEState() == CARD_OP_READY)
	{
		if(P1 == PERSONALTAG_CALCERTCC)  //分别计算CI CERT/EUM CRT /EUICC CERT 和EUICC SK 的证书值
		{
			EUICCCalCERTCheckSum();
			return;
		}
		
		if(P1 == PERSONALTAG_CERTCI)
		{
			EUICCPersonalCACIData();
			return;
		}		
				
#if RSP_EUICC
		if((P1 == PERSONALTAG_CERTEUM) || ((P1 == PERSONALTAG_CERTEUICC)) || (P1 == PERSONALTAG_SKEUICC))
		{
			EUICCPersonalCAEUMAndEUICCData();
			return;
		}
#else
		if(P1 == PERSONALTAG_PKCI)
		{
			M2M_EUICCPersonalPKIDData();
			return;
		}
		
		if(((P1 == PERSONALTAG_CERTEUICC)) || (P1 == PERSONALTAG_SKEUICC))
		{
			M2M_EUICCPersonalEUICCData();
			return;
		}
#endif
		
		ThrowError(0x6A86);

	}
	else  //卡片发卡后不支持20指令
	{
		ThrowError(0x6D00);
	}
	
		
}
#if M2M_EUICC
void NM_M2M_ValidIns(void)
{
	switch(INS)
	{
		case 0xE6:  //预置安装ISD - P
		case 0xD8:
		case 0xE2:
		case 0xFE:  //激活profile
			PushOperandStackRef(TRUE);
			break;

		default:
			PushOperandStackRef(FALSE);
			return;
	}
}
/*
*@fucntion   :NM_PersonalProfileCommand

*@description:执行预置profile操作
*
*@paran       :void
*
*@return      :void
*
*@author      :
*
*@history     :V1.0 :create @20190225
               NOTE :要考虑预置profile的过程中出错，是否要进行数据的回滚，按道理应该处理，目前未处理
*              V1.1 :modify by xuxiaojun@20191021 增加需求实现：预置多个profile，后个人化时，需要分别激活profile再写个性化数据
*/
void NM_PersonalProfileCommand(void)
{
	BYTE idata lifeCycle,index,pkgId;
	BYTE idata dataOff;
	WORD idata sw;
	BYTE idata targetProfileIndex;
	
	sw = 0x9000;

	//lifeCycle = JcvmConfig_GetSEState() ;

	//if(lifeCycle == CARD_OP_READY)
	{
		switch(INS)
		{
			case 0xA4: 
				if(G_PersonalProfileState != PersonalProfile_State_NULL)
				{
					PushOperandStackRef(TRUE);  //为兼容ISDR 下选择ISDP不返回异常，暂时在发卡前支持A4的处理
				}
				else
				{
					PushOperandStackRef(FALSE); 
				}
				return;
			
			case 0xE6:  //安装ISD - P
				if(P1 == P1_INSTALLFORINSTALLANDSELECTABLE)
				{
					pkgId = findPkgID(&RecData[1],0,RecData[0],0); 	// （不需要检查版本号）
		
					if(pkgId == ISDP_PkgId)  //初始化ISD P
					{
						if(G_PersonalProfileState != PersonalProfile_State_NULL)
						{
							ThrowError(SW_CONDITIONS_NOT_SATISFIED); 
						}
						
						index = GetIdleProfileIndex();
		
						if(index == ILLEGAL_PROFILEINDEX)
						{
							ThrowError(SW_FILE_FULL); 
						}
//#if RSP_EUICC
//						G_RSPSessionContext.downloadProfileIndex = index;  //借用G_RSPSessionContext.downloadProfileIndex保存当前待下载的profileindex ，后续在PEExcuteGPCommand 中会使用到该变量
//#elif M2M_EUICC
						G_DownLoadingProfileIndex = index;  //PE解析中会使用到变量G_DownLoadingProfileIndex
//#endif
						
						bRamBuffer[LOADING_PROFILE_INDEX] = index;

						sw = CreateProfileEntry();

						if(sw != 0x9000)//创建ProfileEntry
						{
							ThrowError(sw);

							return;//profile已满
						}

						SEUICC_BackISDPAppletTable(bRamBuffer[LOADING_PROFILE_INDEX]);

						sw = CUSTOMERGP_InstallforInstallISDP(g_bSDId[selected_channel]);
						
						SEUICC_ResumeISDPAppletTable();

						if(sw != 0x9000)
						{
							ThrowError(sw); 
						}

						G_PersonalProfileState = PersonalProfile_State_ISDP;
						
						G_PersonalProfileIndex = index;
						
						break;
					}
					else
					{
						PushOperandStackRef(FALSE);
						return;
					}
					
				}
				else
				{
					PushOperandStackRef(FALSE);
					return;
				}
			case 0xD8:
				
				if((G_PersonalProfileState != PersonalProfile_State_ISDP)&&(G_PersonalProfileState != PersonalProfile_State_Personal))  //支持重复putKey操作
				{
					PushOperandStackRef(FALSE);  //ISDR 本身也支持 普通Key操作
					return;
					
				}
				
				PEGPGlobalVariableInit();
				
				G_profileGPState.peInstallAppId = ISDP_APPID; // 更新ISD -P 的密钥
				
				G_profileGPState.keyCounter[0] = 0x00;
				
				SEUICC_BackISDPAppletTable(bRamBuffer[LOADING_PROFILE_INDEX]);

				sw = PEExcuteGPCommand(G_profileGPState.peInstallAppId);
				
				SEUICC_ResumeISDPAppletTable();

				if(sw != 0x9000)
				{
					ThrowError(sw); 
				}
				
				G_PersonalProfileState = PersonalProfile_State_Personal;		
				
				break;
				
			case 0xE2:
				if((G_PersonalProfileState != PersonalProfile_State_Personal)&&(G_PersonalProfileState != PersonalProfile_State_Enable))
				{
					PushOperandStackRef(FALSE);  //ISDR 本身也支持 storeData操作
					return;
				}
				
				if(RecData[0] == 0x84)
				{				
					break;
				}
				if(RecData[0] == 0x86)  //Process Profile
				{
					dataOff = FALSE;
					
					if(RecData[1] < 0x80)
					{
						dataOff = 2;
						
						lifeCycle = RecData[1]; //暂时用变量 lifeCycle  缓存后续数据长度
					}
					else if(RecData[1] == 0x81)
					{
						dataOff = 3;
						
						lifeCycle = RecData[2];  //暂时用变量 lifeCycle 缓存后续数据长度
					}
					else    //不支持超过82字节的86的处理
					{
						ThrowError(SW_WRONG_DATA); //6A80
					}
					
					VMWriteBuf(EUICCTempDataBuf, &RecData[dataOff], lifeCycle); // 先将RecData中的数据写到EUICCTempDataBuf， 避免profile解析时造成RecData数据被冲掉
					
					SEUICC_BackISDPAppletTable(bRamBuffer[LOADING_PROFILE_INDEX]);

					interpreterPE(EUICCTempDataBuf, lifeCycle,ResData);  //错误数据存放在ResData中,ResData[0]存放的是长度，目前不支持长度超过0xFF的情况.Notification_GenerateProfileInstallResult中会直接使用ResData中的结果进行拼装
	
					SEUICC_ResumeISDPAppletTable();

					lifeCycle = IF_GetPfoileExcuteState();
					
					if(lifeCycle == FALSE)  //profile执行过程中出错
					{
						G_PersonalProfileState = PersonalProfile_State_NULL;
						
						ThrowError(SW_WRONG_DATA); //6A80
					}
					
					G_PersonalProfileState = PersonalProfile_State_Enable;
				}
				
				break;
			case 0xFE:  //激活profile
				if(P3 == 0x00)   //激活当前的profile
				{
					if(G_PersonalProfileState != PersonalProfile_State_Enable)
					{
						ThrowError(SW_CONDITIONS_NOT_SATISFIED); 
					}							
					
					JcvmConfig_SetProfileEnableIndex(G_PersonalProfileIndex);
					
					JcvmConfig_SetM2MFallBackProfileIndex(G_PersonalProfileIndex);
				}
				else if(P3 == 0x10) //激活指定 ISDP AID的profile
				{
					targetProfileIndex = GetProfileIndexByAID(&RecData[0], 0x10);
					
					if(targetProfileIndex == ILLEGAL_PROFILEINDEX)
					{
						G_PersonalProfileState = PersonalProfile_State_NULL;
						
						ThrowError(SW_WRONG_DATA); //6A80
					}
					else
					{
						JcvmConfig_SetProfileEnableIndex(targetProfileIndex);
					
						JcvmConfig_SetM2MFallBackProfileIndex(targetProfileIndex);
					}
				}
				else
				{
					
				}
				
				G_PersonalProfileState = PersonalProfile_State_NULL;
				
				G_PersonalProfileIndex = 0xFF;
				
				break;
				
			default: 
				PushOperandStackRef(FALSE); 
				return;
		}
		
		PushOperandStackRef(TRUE);
	}
//	else
//	{
//		G_PersonalProfileState = PersonalProfile_State_NULL;
//
//		PushOperandStackRef(FALSE);
//	}
}
#endif  //end if M2M_EUICC
#endif

