/*
 * RELEASE: 2019-01-08

 * AUTHER:	郑冬梅

 * HISTROY:	V1.0 创建源文件,非共享文件相关处理文件。
 						
*/
#include <uicc_error.h>
#include  "typedef.h"
#include  "uicc_main.h"
#include  "ceph.h"
//#include  "main.h"
#include  "fileSystem.h"
#include  "fileAction.h"
#include  "string.h"
//#include  "resume.h"
#include  "gsmpublic.h"
#include  "stkhead.h"
#include "Crypto_Constant.h"
#include  "ceph.h"
#include  "interface.h"
//#include  "stkalert.h"
#include  "uicc_interface.h"
//#include  "T0ApduImpl.h"
//#include  "JCREMain.h"
//#include  "GPKeyManager.h"
//#include  "GPApplet.h"
//#include  "ChannelManagement.h"
//#include  "GPCommand.h"
//#include  "STKToolkitInterface.h"
//#include  "RFMFile.h"
//#include "BIPModuleProcess.h"
//#include "TimerModuleProces.h"
#include  "FunctionSwitch.h"
#include "stack.h"
#include "SCPInterface.h"
#include "keySet.h"
#if UICC__FUNCTION__STKAPI  //支持Toollkit功能
#include "ToolkitRFM.h"
#include "STKRemoteAppManagement.h"
#include "STKCATRuntimeEnviorment.h"
#include "STKFileView.h"
#include "ToolkitAPIPublic.h"
#include "casdStoreKey.h"

#include "Interface_LTEFromPlatform.h"
#endif

#include "cardManager.h"

#if UICC__FUNCTION__UICCAPI
#include "UICCToolkitAPI.h"
#endif

#if GP__AMENDMENT__B__FOR_MASK
#include "Interface_AnnexB.h"
#include "HttpProcess.h"
#endif

#include 	"Authentication.h"

#if UICC__AUTH__5G__SUSPEND
#include "SuspendUicc.h"
#endif
#include "Arithmatic.h"
/*
*@description:开机时遍历当前非共享文件，并且设置相应的状态
*
*@param      :void
*
*@return     :void
*
*@author     :create by xuyz
*
*@history    :V1.0:create by xuyz@20140211
			  v1.2 modify by xuyz@20160506 扩展非共享文件属性，其中通道号扩展至64个通道，与6F80关联的记录号扩展到255个
*/
void IF_InitNoShareableFile(void)
{
#if UICC__FUNCTION__STKAPI
	UWORD xdata shareableAddr;
	BYTE idata i,fileNum;
	BYTE xdata filePath[NOShareableFile_Length];
	
	ReadEeprom(&fileNum,E_FILE_NoShareableRec,1);		

	if(fileNum == 0x00)
	{
		return;
	}
	
	IF_ProcessNoShareableFileListAfterDeleteFile(); //整理当前的非共享文件,防止删除文件时断电

	shareableAddr = E_FILE_NoShareableRec +1 ;

	for(i = 0x00;i<fileNum ;i++)
	{
		ReadEeprom(&filePath[0],shareableAddr+ i*sizeof(filePath), NOShareableFile_Length) ;
		
		if(filePath[0] == 0xFF)
		{
			continue;   //该条记录为空记录
		}
		if((filePath[NOShareableFile_Length-3]>>6) != 0xFF)   //读取第 11模式字节,即当前是SIM API 模式下选中的非共享文件,初始化时要清除
		{
			SetEepromEX(shareableAddr+ i*sizeof(filePath), 0xFF, NOShareableFile_Length) ; 
		}
		else    //当前是UICC 或UICC API 选中的非共享文件，那么只保留fileView上下文为永久型的非共享文件
		{
			if((filePath[NOShareableFile_Length-1]>>6) != 0x00)   //当前不为永久对象 ，则释放整个记录
			{
				SetEepromEX(shareableAddr+ i*sizeof(filePath), 0xFF, NOShareableFile_Length) ;
			}
		}
	}
#endif
}
/*
*@description:隐式选择时候清空非共享记录,比如00040800047F106F44 执行失败时候 会回滚ResumeFilePath,但是非共享文件没有清,所以添加此函数,在ResumeFilePath前调用
*
*@param      :UWORD fileAddr  EF地址
*@param      :WORD fileName   EF name
*
*@history    :V1.0:create by LCZ@2019-9-4
*/
BYTE IF_CleanShareableFile(UWORD fileAddr,WORD fileName)
{
	BYTE idata filePathLgth,i,fileNum;
	BYTE  xdata filePath[NOShareableFile_Length],tmp[NOShareableFile_Length];
	UWORD xdata shareableAddr;

	//在setChannel之前判断上一次的ef文件是否为非共享文件，如果是，则要从当前文件中删除
	
	fileNum = ReadEECacheByte(E_FILE_NoShareableRec);

	shareableAddr = E_FILE_NoShareableRec + 1;

    ecpMem_Set(filePath, 0x00, sizeof(filePath));
	
	filePathLgth = FALSE;

	filePathLgth = GetNoShareableFilePath(fileAddr,fileName,G_CurrentDfAddr[G_ChannelNum],G_CurrentDfName[G_ChannelNum],&filePath[0]);

	if(filePathLgth != 0x00)  //上一次操作的是非共享文件(已经被记录在非共享文件列表中)，将其退选
	{
		for(i =0 ; i<fileNum ; i++)
		{
			ReadEEPromCache(tmp,shareableAddr+i*sizeof(filePath), NOShareableFile_Length);
	
			if(ecpMem_Compare(tmp,filePath,0x0A) == EQUAL) //退选文件
			{													
				VMSetBuf(shareableAddr+i*sizeof(filePath), 0xFF,NOShareableFile_Length);

				break;
			}
		}
	
	}
	
	return TRUE;
	
}
/*
*@description:创建文件时，如果当前文件创建成功，则判断该文件是否为非共享文件，如果是，则将其非共享文件路径保存在3F00/EF_ToolkitNoShareableFile
*
*@param      :UWORD fileAddr
*@param      :WORD fileName
*@param      :BYTE mode  :0x00:通过APDU(INS为0xE0)模式创建或者选择文件；
*                         0x01:通过UICC/SIM API模式创建或选择文件；
*                         0x02:通过APDU删除(INS为0xE4)文件或者调整文件大小(INS为0xD4);
*                         0x03:通过UICC API删除或调整文件大小
*
*@return     :void
*
*@author     :create by xuyz@20140208
*
*@history    :V1.0:create by xuyz@20140208
*/
BYTE IF_ProcessNoShareableFile(UWORD fileAddr,WORD fileName,BYTE mode,BYTE apiMode)
{
#if UICC__FUNCTION__STKAPI
	BYTE idata /*fileType,*/tag;
	BYTE idata filePathLgth,i,fileNum;
	BYTE  xdata filePath[NOShareableFile_Length],tmp[NOShareableFile_Length];
	UWORD xdata shareableAddr;

	//在setChannel之前判断上一次的ef文件是否为非共享文件，如果是，则要从当前文件中删除
	
	ReadEeprom(&fileNum,E_FILE_NoShareableRec,1);

	shareableAddr = E_FILE_NoShareableRec + 1;

    ecpMem_Set(filePath, 0x00, sizeof(filePath));
	
	filePathLgth = FALSE;

	if((mode&0x01) == 0x00)
	{
		filePathLgth = GetNoShareableFilePath(G_CurrentEfAddr[G_ChannelNum],G_CurrentEfName[G_ChannelNum],G_CurrentDfAddr[G_ChannelNum],G_CurrentDfName[G_ChannelNum],&filePath[0]);
	}
	else
	{
		filePathLgth = GetNoShareableFilePath(G_ToolkitCurrentEfAddr,G_ToolkitCurrentEfName,G_ToolkitCurrentDfAddr,G_ToolkitCurrentDfName,&filePath[0]);	
	}

	if(filePathLgth != 0x00)  //上一次操作的是非共享文件(已经被记录在非共享文件列表中)，将其退选
	{
		for(i =0 ; i<fileNum ; i++)
		{
			ReadEeprom(tmp,shareableAddr+i*sizeof(filePath), NOShareableFile_Length);
	
			if(ecpMem_Compare(tmp,filePath,0x0A) == EQUAL) //退选文件
			{													
				SetEepromEX(shareableAddr+i*sizeof(filePath), 0xFF,NOShareableFile_Length);
				break;
			}
		}
	
	}
 	tag = NoShareableFileIsValidAndSetBit(fileAddr,fileName,mode, apiMode);

	if(tag == FALSE) 	// 当前文件为非共享文件，且该非共享文件已经被其他对象所占用
	{
		return FALSE;		
	}

	return TRUE;
    #else
	return TRUE;
    #endif
}
/*
*@description:判断指定路径的文件是否存在
*
*@function   :checkPathFileExist
*
*@param      :buf
*@param      :pathLen
*
*@return     TRUE:文件存在/false:文件不存在
*
*@history
*/
BYTE checkPathFileExist(BYTE *pathBuf, BYTE pathLen)
{
	BYTE idata j;
	WORD idata fileName;
	UWORD xdata fileAddr;

	fileAddr = E_FILE_START;

	for(j=0x00;j<pathLen;j=j+2)
	{
		 if(((pathBuf[j] == 0x00)&&(pathBuf[j+1] == 0x00)) ||((pathBuf[j] == 0xFF)&&(pathBuf[j+1] == 0xFF)))  // 0000 或FFFF被认为是无效记录
		 {
			break;
		 }
		 else
		 {
         	fileName = ((WORD)(pathBuf[j]<<8)&0xFF00) + pathBuf[j+1];
		
		 	fileAddr = SelectFile(fileAddr,fileName);

			if(fileAddr == FALSE)//找到被删除文件的目录
			{
				return FALSE;
			}
		}

	}

	if(j == 0)
	{
		return FALSE;  //此判断条件程序正常执行的时候不会走进来.
	}

	return TRUE;		
}
/*
*@description:遍历当前EF_ToolkitNoShareableFile,删除当前不存在的路径文件
*
*@param      :void
*
*@return     : void
*
*@author     :Create by xuyz@20140211
*
*@history   :V1.0:create by xuyz@20140211
			 v1.1:modify by xuyz@20160506修改函数功能实现错误的问题,为整理非共享文件碎片问题，对于非共享文件，前后同步遍历进行空闲块的合并
             v1.2:modfify by xuyz@20160629 由于白俄罗斯项目，对方案进行了调整,非共享文件的最大支持数目固定，则在删除文件后，仅把该区域清成全FF即可,中国移动也采用该方案
			 V1.3:由于非共享文件数目固定，更新E2的过程不需要采用事务机制@20170619 by xuyz
*/
void IF_ProcessNoShareableFileListAfterDeleteFile(void)
{
#if UICC__FUNCTION__STKAPI
	UWORD xdata shareableAddr;
	BYTE idata i,fileNum;
	BYTE xdata filePath[NOShareableFile_Length];

	ReadEeprom(&fileNum,E_FILE_NoShareableRec,1);  //fileNum指示当前已经存在过的最多次非共享文件记录

	if(fileNum == 0x00)
	{
		return;
	}

	shareableAddr = E_FILE_NoShareableRec + 1;//相对E_File_Start的地址
	

	for(i = 0x00;i<fileNum ;i++)
	{
		ReadEeprom(&filePath[0],shareableAddr+ i*sizeof(filePath), NOShareableFile_Length) ;  //正向遍历非共享文件
		
		if(filePath[0] == 0xFF)
		{
			continue;   //空记录
		}

		if(checkPathFileExist(filePath, 0x0A) == FALSE)
		{
			SetEepromEX(shareableAddr+ i*sizeof(filePath) ,0xFF,NOShareableFile_Length);  //清空无效的非共享记录为全FF
		}
	}
    #endif
}
/*
*@description: 在toolkit应用触发完毕后，判断当前是否占用了非共享文件信息，并是否需要清除
*@			   对于UICC 模式触发的应用来说，因为由每个fileView自己保存上下文信息，而上下文信息又分为DTR RTR和永久对象，根据其上下文属性，这些都不需要在退出触发应用时清除
×@             对于SIM 模式触发的应用来说 ，如果SIM 模式占用了非共享文件信息，则在退出触发时，清除当前event占用的非共享文件信息
*
*@function   :ProcessNoShariableFileForToolkit
*
*@param       :void
*
*@return      :void
*
*@history      :V1.0 
*/

void ProcessNoShariableFileForToolkit(jshort trigerMode)
{
#if UICC__FUNCTION__STKAPI
	BYTE idata fileNum,i;
	UWORD xdata shareableAddr;
	BYTE xdata tmp[NOShareableFile_Length];
	
	//if(G_ToolkitContext.CurrentTrigeMode[G_CurSelectedChannel] == SIMAPI_Mode)  // 对于SIM 模式触发的应用来说 ，如果SIM 模式占用了非共享文件信息，则在退出触发时，清除当前event占用的非共享文件信息
	if(trigerMode == SIMAPI_Mode)
	{
		ReadEeprom(&fileNum,E_FILE_NoShareableRec,1);                           //读取当前已有的非共享文件个数，最大为0x10个

		shareableAddr = E_FILE_NoShareableRec+1;
		
		for(i =0 ; i<fileNum ; i++)		
		{
			ReadEeprom(tmp,shareableAddr+i*sizeof(tmp), NOShareableFile_Length);
			
			if(tmp[0] == 0xFF)  //该记录为空记录
			{
				continue;
			}
			
			if((tmp[NOShareableFile_Length - 3] == 0x01)&&(G_ToolkitContext.CurrentEvent[G_CurSelectedChannel] == tmp[NOShareableFile_Length - 1]))
			{
				SetEepromEX(shareableAddr+ i*sizeof(tmp),0xFF,NOShareableFile_Length);  //当前event中占用的非共享文件信息
			}
		}
	}
    #endif
}

/*
*@description :判断当前DF下的某个文件是否为非共享文件，如果是非共享文件，则判断其是否已被某个通道或fileView对象所选中
*
*@param       :fileAddr
*@param       :fileName
*@param       :BYTE mode: 0x00:通过APDU(INS为0xE0)模式创建或者选择文件；
*                         0x01:通过UICC/SIM API模式创建或选择文件；
*                         0x02:通过APDU删除(INS为0xE4)文件或者调整文件大小(INS为0xD4);
*                         0x03:通过UICC API删除或调整文件大小
*
*@return      :TRUE：该文件不是非共享文件
*              TRUE: 该文件是非共享文件，但是当前没有被通道或其他fileView对象选中,返回为路径长度
*              FALSE：该文件是非共享文件，且该文件已经被其他通道或fileView对象占用
*
*@author      :create by xuyz
*
*@history     :V1.0 :create by xuyz@20140211
*/
BYTE NoShareableFileIsValidAndSetBit(UWORD fileAddr,WORD fileName,BYTE mode,BYTE apiMode)
{
#if UICC__FUNCTION__STKAPI
	BYTE idata 	fileType,filePathLgth,fileNum,i;
	BYTE  xdata filePath[NOShareableFile_Length],tmp[NOShareableFile_Length],buf[NOShareableFile_Length];
	UWORD xdata shareableAddr;

	if(fileAddr == FALSE)
	{
		return TRUE;	
	}

	ReadEeprom(&fileType,fileAddr + F_Type_Offset,sizeof(BYTE));

	if((fileType&FILETYPE)==0x38)					//DF  ADF  MF
	{
		return TRUE;	
	}

	if((fileType&0x40) == 0x40)  //共享文件
	{
		return TRUE;
	}
	
	ReadEeprom(&fileNum,E_FILE_NoShareableRec,1);//读取当前已有的非共享文件个数，最大为0x10个

	shareableAddr = E_FILE_NoShareableRec + 1;

	ecpMem_Set(buf ,0x00, NOShareableFile_Length);  //buf第一字节存储当前如果非共享文件已经存在其记录号，其第一字节存储记录号，后续10字节存储路径

	filePathLgth = FALSE;
	
	if((mode&0x01) == 0x00)
	{
		filePathLgth = GetNoShareableFilePath(fileAddr,fileName,G_CurrentDfAddr[G_ChannelNum],G_CurrentDfName[G_ChannelNum],&buf[1]);

	}
	else
	{
		filePathLgth = GetNoShareableFilePath(fileAddr,fileName,G_ToolkitCurrentDfAddr,G_ToolkitCurrentDfName,&buf[1]);
	}

	ecpMem_Set(filePath, 0x00, sizeof(filePath));

	ecpMem_Copy(filePath,&buf[1],filePathLgth);

	for(i =0 ; i<fileNum ; i++)		//当前通道选中的非共享文件，已经被其他通道或fileView 对象所选中
	{
		ReadEeprom(tmp,shareableAddr+i*sizeof(filePath), NOShareableFile_Length);

		if(ecpMem_Compare(tmp,filePath,0x0A) == EQUAL)  // 目前的方案中，只有存在路径相同的，则一定被占用
		{
			return FALSE;		
		}
	}

	if(mode > 0x01)	  //02:APDU模式，仅判断权限delete 、resize权限是否满足 ，0x03：仅判断Toolkit操作delete resize时判断权限是否满足
	{
		return TRUE;
	}

	if(ValueBit(G_SystemStatus, B_RfmPerform) == TRUE)    //对于RFM 操作，如果当前操作非共享文件权限已经满足，由于退出RFM 时，会释放占用的非共享文件，所以此处不再登记将占用的非共享文件信息
	{
		return TRUE;                                                                                         
	}
	
	filePath[0x0A] = 0xFF;

	filePath[0x0B] = 0xFF;
	
	filePath[0x0C] = 0xFF;

	if(mode == 0x00)//APDU选中当做clear on reset处理，记录号为0
	{
		 filePath[0x0B] = 0x00;  //表示当前操作由UICC Applet发起的

		 filePath[0x0C]	= 0x00;	 //init value
		 
		 filePath[0x0C] |= 0x40;  //bit7bit6:01:当作Clear_ON_Reset对象使用
		 
		 filePath[0x0C] |= G_CurSelectedChannel;	//保存当前通道号
	}
	else //if((mode&0x01) == 0x01)  //0x00:永久对象，0x01:clear on reset,0x02:clear on deselect,0x03空闲
	{
		 //if(G_ToolkitContext.CurrentTrigeMode[G_CurSelectedChannel] == SIMAPI_Mode)  //当前是SIM API模式
		if(apiMode == SIMAPI_Mode)
		 {
			 if((U8_AND_U8(U16t8(getChannelContext(selected_channel)), 0xFF)) != UICC_APPID)	   //当前是通过process方式选中的 某个SIM API toolkit应用
			 {	
				filePath[0x0A]	 = 0x02;
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
				filePath[0x0B] = getAppIDByIndex(G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]);   //toolkit模式下创建文件，G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]一定为有效值，否则操作权限不满足，进入不到这里
#else
				filePath[0x0B] = G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel];   //toolkit模式下创建文件，G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]一定为有效值，否则操作权限不满足，进入不到这里
#endif
				filePath[0x0C]	= 0x00;	 //init value
				
				filePath[0x0C] |= 0x80;   //当作DTR对象
				
				filePath[0x0C] |= G_CurSelectedChannel;	//保存当前通道号
			 }
			 else  //当前选中的应用为UICC应用，通过processToolkit方式操作非共享文件
			 {
				filePath[0x0A] = 0x01;
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
				filePath[0x0B] = getAppIDByIndex(G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]);   //toolkit模式下创建文件，G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]一定为有效值，否则操作权限不满足，进入不到这里
#else
				filePath[0x0B] = G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel];   //toolkit模式下创建文件，G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]一定为有效值，否则操作权限不满足，进入不到这里
#endif
				filePath[0x0C]	= G_ToolkitContext.CurrentEvent[G_CurSelectedChannel];	 //当前event
			 }
		 }
		 else
		 {
#if UICC__FUNCTION__UICCAPI
#if UICC__STKAPI_TOOLKITINFO_IN_HEAP
			 filePath[0x0B] = getAppIDByIndex(G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]);   //toolkit模式下创建文件，G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]一定为有效值，否则操作权限不满足，进入不到这里
#else
			 filePath[0x0B] = G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel];   //toolkit模式下创建文件，G_ToolkitContext.CurrentApprecNum[G_CurSelectedChannel]一定为有效值，否则操作权限不满足，进入不到这里
#endif
			 filePath[0x0C]	= 0x00;	 //init value
			 
			 filePath[0x0C] |= G_ToolkitFileContextEvent<<6;
			 
			 filePath[0x0C] |= G_CurSelectedChannel;	//保存当前通道号
            #endif
		 }
	}

	if(fileNum < NOShareableFile_Number)  //还没有达到最大个数 ，直接往后新加
	{
		shareableAddr = shareableAddr+fileNum*NOShareableFile_Length;

		WriteEepromEX(shareableAddr,filePath,NOShareableFile_Length);

		fileNum++;

		SetEepromEX(E_FILE_NoShareableRec,fileNum,0x01);
	}
	else  //在已有的file数目中查找一个空闲记录，将当前文件更新进去
	{
		shareableAddr = E_FILE_NoShareableRec+1;
		
		for(i =0 ; i<fileNum ; i++)		//当前通道选中的非共享文件，已经被其他通道或fileView 对象所选中
		{
			ReadEeprom(tmp,shareableAddr+i*sizeof(filePath), NOShareableFile_Length);
			
			if(tmp[0] == 0xFF)  //该记录为空记录
			{
				WriteEepromEX(shareableAddr+i*sizeof(filePath),filePath,NOShareableFile_Length);  //此时不需要改变非共享文件的记录数
				
				break;
			}
		
		} //这里不判断i==fileNum时返回FALSE的情况，原因是在实际卡片使用过程，如果因为这个原因导致选择不了，则后续操作都不可以，对于用户来说，体验更差
	}
	return TRUE;
    #else
	return TRUE;
    #endif
}

/*
*@description:删除toolkitapp时，释放被其占用的非共享文件
*@param      :apprec toolkitapp对应的6F80记录号
*
*@return     :void
*
*@author     :create by lj
*
*@history    :V1.0:create by lj@20140313
*@history    modify by lgy@20220719 UICC__STKAPI_TOOLKITINFO_IN_HEAP传参为appid
*/
void ProcessNoShareableFileAfterDeleteApplet(BYTE apprec)
{
#if UICC__FUNCTION__STKAPI
	UWORD xdata shareableAddr;
	BYTE idata i,fileNum;
	BYTE xdata filePath[NOShareableFile_Length];

	ReadEeprom(&fileNum,E_FILE_NoShareableRec,1);//当前已有的非共享文件个数
	
	if(fileNum == 0x00)
	{
		return;
	}

	shareableAddr = E_FILE_NoShareableRec + 1;

	for(i = 0x00;i<fileNum ;i++)
	{
		ReadEeprom(&filePath[0],shareableAddr+ i*sizeof(filePath), NOShareableFile_Length) ;
		
		if(filePath[0] == 0xFF)
		{
			continue;  //此记录无效
		}

		if(filePath[NOShareableFile_Length-2] == apprec)//app 匹配，则释放其在所有通道上的非共享文件
		{
			SetEepromEX(shareableAddr+ i*sizeof(filePath),0xFF,NOShareableFile_Length);
		}	
	}
    #endif
}

/*
*@function    : GetNoShareableFilePath
*
*@description :若当前非共享ef存在，则获取当前ef路径信息
*
*@param       :efAddr
*@param       :efName
*@param       :dfAddr
*@param       :efAddr
*@param       :buf：缓存当前路径
*
*@return      :返回当前路径长度
*
*@author
*/
#if UICC__FUNCTION__STKAPI
BYTE GetNoShareableFilePath(UWORD efAddr,WORD efName,UWORD dfAddr,WORD dfName,BYTE *buf)
{
	BYTE idata filePathLgth,fileType;
	BYTE xdata filePath[0x0A];
	WORD idata parentName,secondeParentName;
	
	if(efAddr == FALSE)
	{
		return FALSE;
	}

   	ReadEeprom((BYTE idata *)&fileType,efAddr + F_Type_Offset,sizeof(BYTE));

	if((fileType&0x40) == 0x40)  //共享文件
	{
		return FALSE;
	}

	filePathLgth = 0x00;

	filePath[0x0A-filePathLgth-2] = (efName>>0x08)&0xFF;
	filePath[0x0A-filePathLgth-1] = efName&0xFF;

	filePathLgth += 0x02;


	filePath[0x0A-filePathLgth-2] = (dfName>>0x08)&0xFF;
	filePath[0x0A-filePathLgth-1] = dfName&0xFF;

	filePathLgth += 0x02;

	if(dfName != 0x3F00)   //有上级DF
	{
		secondeParentName = GetFileHeadWord(dfAddr, F_SecondParentName_Offset); 
	
		filePath[0x0A-filePathLgth-2] = (secondeParentName>>0x08)&0xFF;
		filePath[0x0A-filePathLgth-1] = secondeParentName&0xFF;
	
		filePathLgth += 0x02;
	
		if(secondeParentName != 0x3F00)	   //假设目前最大支持4级DF
		{
			parentName = GetFileHeadWord(dfAddr, F_ParentName_Offset);	 
			
			filePath[0x0A-filePathLgth-2] = (parentName>>0x08)&0xFF;
			filePath[0x0A-filePathLgth-1] = parentName&0xFF;
		
			filePathLgth += 0x02; 

			if(parentName != 0x3F00)
			{
				filePath[0x0A-filePathLgth-2] = 0x3F;
				filePath[0x0A-filePathLgth-1] = 0x00;
			
				filePathLgth += 0x02; 
			}
		}
	}

	ecpMem_Copy(&buf[0], &filePath[0x0A-filePathLgth],filePathLgth);

	return filePathLgth;
}
#endif
/*
*@description:选择toolkitapp时，释放被其占用的非共享文件
*@param      :void
*
*@return     :void
*
*@author     :create by lj
*
*@history    :V1.0:create by lj@20140313
						  V1.1 ：增加当INS = 70时，退出该函数，原因：对于指令70而言，进入的该函数必须当前为打开新的通道且新的通道上有应用选中，而此时G_CurSelectedChannel上的应用未发生任何变化，所以无需检查其非共享对象 by xuyz@20161210
                     增加当INS =A4时，若当前是A4上新打开的通道，也无需判断非共享对象，
*/
void IF_NoShareableFileAfterDeSelectApplet(BYTE channel)
{
#if UICC__FUNCTION__STKAPI
	UWORD xdata shareableAddr;
	BYTE idata i,fileNum;
	BYTE xdata filePath[NOShareableFile_Length];
	
	ReadEeprom(&fileNum,E_FILE_NoShareableRec,1);//读取已经存在过的非共享文件个数
	
	if(fileNum == 0x00)
	{
		return;
	}

	shareableAddr =E_FILE_NoShareableRec +1 ;

	for(i = 0x00;i<fileNum ;i++)
	{
		ReadEeprom(&filePath[0],shareableAddr+ i*sizeof(filePath), NOShareableFile_Length) ;
		
		if(filePath[0] == 0xFF)
		{
			continue;   //该记录无效
		}

		if(filePath[NOShareableFile_Length-3] != 0x01)  //当前不是SIM API 触发模式,对于SIM API 触发模式，会在退出SIM API触发时，清除对应占用的toolkit信息。只有STK中选择应用时才会进入这里
		{
			if((filePath[NOShareableFile_Length-1]&0x3F) == channel)
			{
				if(filePath[NOShareableFile_Length-2] == 0x00)   //当前是APDU 的模式，如果函数进入这个判断条件，说明之前选中的是UICC应用，当前切换到其他应用上了，所以要清除UICC当前通道占用的非共享文件信息
				{
					SetEepromEX(shareableAddr+ i*sizeof(filePath),0xFF, NOShareableFile_Length) ;
				}
				else
				{
					if((filePath[NOShareableFile_Length-1]>>6) == 0x02)//对于非共享文件文件，则释放clear on deselect 占用的非共享文件
					{
						SetEepromEX(shareableAddr+ i*sizeof(filePath),0xFF, NOShareableFile_Length) ;
					}
				}
			}
		}
	}
    #endif
}

