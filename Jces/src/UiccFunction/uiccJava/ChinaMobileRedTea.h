#ifndef    _ChinaMobileRedTea_h_
#define    _ChinaMobileRedTea_h_

#define   ResponseOk                  0x00
#define   alreadyWriteSuccess         0x01
#define   dataFormatError             0x02
#define   checkhashError              0x03
#define   writeResultError            0x04
#define   isdpnotsupported            0x05
#define   undefinedError              0x7F

extern BYTE code RootKeyValue[16];

extern void CheckStoreDataTagFF7F(void);

extern void GetEuiccAESCMACValue(BYTE *input,BYTE inputlength,BYTE *output,BYTE *keyContext);
#endif
