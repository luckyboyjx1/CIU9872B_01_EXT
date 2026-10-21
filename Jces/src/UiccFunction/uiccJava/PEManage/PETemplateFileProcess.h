#ifndef	 _PETemplateFileProcess_h_
#define  _PETemplateFileProcess_h_
#include   "typedef.h"
//#include   "eepromdefine.h"
#include   "tlvHeader.h"
#define MF_TEMPLATE_LAST_ID         1
#define CD_TEMPLATE_LAST_ID         2
#define TELECOM_TEMPLATE_LAST_ID    3
#define USIM_TEMPLATE_LAST_ID       4
#define OPT_USIM_TEMPLATE_LAST_ID   5
#define PHONEBOOK_TEMPLATE_LAST_ID  6
#define GSM_ACCESS_TEMPLATE_LAST_ID 7
#define ISIM_TEMPLATE_LAST_ID       8
#define OPT_ISIM_TEMPLATE_LAST_ID   9
#define CSIM_TEMPLATE_LAST_ID       10
#define OPT_CSIM_TEMPLATE_LAST_ID   11
#define EAP_TEMPLATE_LAST_ID        12
#define DF5GS_TEMPLATE_LAST_ID        13
#define SAIP_TEMPLATE_LAST_ID       14
#define PRIVATE_TEMPLATE_LAST_ID    15


#define FILE_TEMPLATE_STATE_TAG    0xB0
#define FILE_TEMPLATE_END_TAG    0xBD

typedef struct TagControl{
	WORD  filename;
	BYTE  fileType;
}TemplateFileList;


/*static const TemplateFileList MFtemplateFileList[]=
{
	{EF_ICCID,TRANTYPE}, //2FE2
	{EF_ARR,LINTYPE}, //2F06
	{0x2F00,LINTYPE}, //2F00
	{EOL,0},

};

static const TemplateFileList CDtemplateFileList[]=
{
	{0x6F01,TRANTYPE},
	{EOL,0},

};

static const TemplateFileList TELECOMtemplateFileList[]=
{
	{0x6F53,TRANTYPE},
	{0x6F54,TRANTYPE},
	{0x6FE5,TRANTYPE},
	{EOL,0},

};

static const TemplateFileList UsimtemplateFileList[]=
{
	{0x6F07,TRANTYPE},
	{0x6F38,TRANTYPE},
	{0x6F46,TRANTYPE},
	{0x6F56,TRANTYPE},
	{0x6F78,TRANTYPE},
	{0x6FB7,LINTYPE},
	{EOL,0},

};

static const TemplateFileList OptialUsimtemplateFileList[]=
{
	{0x6F3E,TRANTYPE},
	{0x6F3F,TRANTYPE},
	{0x6FB1,TRANTYPE},
	{0x6FB2,TRANTYPE},
	{0x6FB3,TRANTYPE},
	{0x6FB4,TRANTYPE},
	{0x6FB5,TRANTYPE},
	{0x6FC5,LINTYPE},
	{0x6FC6,LINTYPE},
	{0x6FC7,LINTYPE},
	{EOL,0},

};

static const TemplateFileList DF5GStemplateFileList[]=
{
	{0x4F06,TRANTYPE},
	{0x4F09,TRANTYPE},
	{EOL,0},

};

static const TemplateFileList IsimtemplateFileList[]=
{
	{0x6F02,TRANTYPE},
	{0x6F04,LINTYPE},
	{0x6F03,TRANTYPE},
	{0x6F07,TRANTYPE},
	{EOL,0},

};

static const TemplateFileList OptialIsimtemplateFileList[]=
{
	{0x6F09,LINTYPE},
	{0x6FE7,LINTYPE},
	{EOL,0},

};

static const TemplateFileList CsimtemplateFileList[]=
{
	{0x6F26,TRANTYPE},
	{0x6F27,TRANTYPE},
	{0x6F2A,TRANTYPE},
	{0x6F2C,TRANTYPE},
	{0x6F2D,TRANTYPE},
	{0x6F2F,TRANTYPE},
	{0x6F30,TRANTYPE},
	{0x6F31,TRANTYPE},
	{0x6F32,TRANTYPE},
	{0x6F35,TRANTYPE},

	{EOL,0},

};
static const TemplateFileList OptialCsimtemplateFileList[]=
{
	{0x6F2E,TRANTYPE},
	{0x6F3F,TRANTYPE},
	{0x6F41,TRANTYPE},
	{0x6F44,LINTYPE},
	{EOL,0},

};*/

extern BYTE UnknownPETag(void);
extern BYTE MFTemplateProcess(void);
extern BYTE CDTemplateProcess(void);
extern BYTE TelTemplateProcess(void);
extern BYTE UsimTemplateProcess(void);
extern BYTE OptialUsimTemplateProcess(void);
extern BYTE IsimTemplateProcess(void);
extern BYTE OptialIsimTemplateProcess(void);
extern BYTE phonebookTemplateProcess(void);
extern BYTE GSMAccemplateProcess(void);
extern BYTE CsimTemplateProcess(void);
extern BYTE OptialCsimTemplateProcess(void);
extern BYTE eapTemplateProcess(void);
extern BYTE df5gsTemplateProcess(void);
extern BYTE SaipTemplateProcess(void);
extern BYTE TemplatePEEndProcess(BYTE tempId);
extern BYTE contentRequiredCheck(void);
extern void getPEID(void);



extern BYTE TemplatePreProcess(BYTE tempId);


#endif
