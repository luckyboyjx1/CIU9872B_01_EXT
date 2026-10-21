#ifndef    _variable_tag_h_
#define    _variable_tag_h_
//#include "typedef.h"

/*
*文件功能：为防止不同的应用使到相同的variable_tag,降低应用开发速度 
*          本文件主要用于记录COS中所用到的各种variable tag,COS开发人员根据功能将所用到的variable tag在此处列出，应用开发人员应该规避掉此文件中已经定义的tag
           
*          01- 0F 供OTA bytcoe使用，请不要定义此区间的tag

*@Version :V1.0 : 创建源文件

*/

#define Tag_BakSupiData    	          0xB8
#define Tag_BakOddData    	          0xB9
#define Tag_BakRecData    	          0xBA
#define Tag_ToolkitRFMResponse  	  0xBB	  //缓存 deliver response
#define Tag_PorResendData  	          0xBC
#define Tag_BakPostData  	          0xBD
#define Tag_BakSecondSmsData          0xBE
#define Tag_BakIncreaseResponseData   0xBF   //最大长度为0x7F，在支持UICC API的平台，因为Increase指令可能触发Toolkit应用，从而冲掉RecData中的数据，所以要将响应数据先备份起来
#endif

