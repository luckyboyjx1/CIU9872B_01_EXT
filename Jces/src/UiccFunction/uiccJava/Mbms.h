#ifndef MBMS_H
#define MBMS_H

//RFC3830
#define   PAYLOAD_Last  0 
#define   PAYLOAD_KEMAC  1 
#define   PAYLOAD_PKE  2 
#define   PAYLOAD_DH  3 
#define   PAYLOAD_SIGN  4 
#define   PAYLOAD_TS  5 
#define   PAYLOAD_ID  6 
#define   PAYLOAD_CERT  7
#define   PAYLOAD_CHASH  8
#define   PAYLOAD_V  9
#define   PAYLOAD_SP  10
#define   PAYLOAD_RAND  11
#define   PAYLOAD_ERR  12
#define   PAYLOAD_Key_data  20 
#define   PAYLOAD_EXT  21

#define ENCR_KEY 1
#define AUTH_KEY 2
#define SALT_KEY 3



//TS33.220
//NAF_ID = FQDN || UaSPI
#define MBMSUaSPI_LEN 5


/* EF_MSK
KEYDOM_ID(3) | KEYNUM(1) | MSK_ID_1(4) | TS_1(4) | MSK_ID_2(4) | TS_2(4)
*/
//offset of each field
#define KEYDOM_ID   0
#define KEYNUM      3
#define MSK_ID_1    4
#define TS_1        8
#define MSK_ID_2    12
#define TS_2        16

//length of each filed
#define KEYDOM_LEN   3
#define MSKID_LEN    4
#define TS_LEN       4
#define EFMSK_RECLEN 20


#define MSK1       0
#define VF1        (MSK1 + MSKLEN)
#define VT1        (VF1 + KVLEN)
#define CNTER1     (VT1 + KVLEN)
#define MSK2       (CNTER1 + CNTERLEN)
#define VF2        (MSK2 + MSKLEN)
#define VT2        (VF2 + KVLEN)
#define CNTER2     (VT2 + KVLEN)
#define Ef_MSK_KV_RECLEN   (CNTER2 + CNTERLEN)  //=50

#define MSKLEN 16
#define KVLEN 4
#define CNTERLEN 1


extern BYTE xdata  G_MUK_IDX;

extern BYTE MBMSAuth(void);




#endif
