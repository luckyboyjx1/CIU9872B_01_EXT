#ifndef    _TUAK_H
#define    _TUAK_H
#include  "typedef.h"



/*******File define***********************/
//#define EF_TUAKCFG   0x2D00    //存放TUAK算法的鉴权数据  ，TF，大小为39字节

//#define EF_TUAKKI    0x6F1D    //存放TUAK算法的ki   32字节长，根据2D00中的 KI SIZE，判断当前KI是32字节还是16字节 ,TF类型的文件，大小为32字节


/*******3E00 filedefine *************/
//#define TUAK_Mode_Off       0x00
//#define TUAK_TOPC_Off       0x01
//#define TUAK_KeySize_Off    0x21
//#define TUAK_RESSize_Off    0x22
//#define TUAK_CKSize_Off     0x23
//#define TUAK_IKSize_Off     0x24
//#define TUAK_MACSize_Off    0x25
//#define TUAK_KecCaliTE      0x26


#define D_ANONY_Mode        0x01

#define	TUAK_KI		   (&RecData[100])		//16字节或32字节


#define	TUAK_MODE	     (RecData[132])		//OP字段的模式（Bit2）和是否SQN需要AK进行异或（Bit1）（1字节）
#define	TUAK_TOPc	     (&RecData[133])	//32字节
#define TUAK_KeySize     (RecData[165])		//1字节
#define TUAK_RESSize     (RecData[166])		//1字节
#define TUAK_CKSize      (RecData[167])		//1字节
#define TUAK_IKSize      (RecData[168])		//1字节
#define TUAK_MACSize     (RecData[169])		//1字节
#define TUAK_KeIte       (RecData[170])		//1字节//TUAK迭代次数

#define	SQN_AK	(&RecData[227])		//保存SQN异或AK的结果    （6字节）



#if RUN_IN_SIMULATOR
extern  BYTE xdata INOUTBUF[200];  // bug 510
#else
extern __attribute__((aligned(4))) BYTE xdata INOUTBUF[200];  // bug 510
#endif
/*********FUNCTION define***************/
extern void PULL_DATA(BYTE* tmpdata,u8 length,u8 location);
extern void PUSH_DATA(const BYTE * data,u8 length,u8 location);
extern void TUAK_ComputeTOPC(BYTE * key,BYTE * TOPC);
extern BYTE TUAK2G(void);
extern BYTE TUAK(void);
extern BYTE CheckSQN(void);
//extern void UsimAuthenticationTuak(void);
#endif

