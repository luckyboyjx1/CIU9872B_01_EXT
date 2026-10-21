#ifndef _SECURITY_INTERFACE_H_
#define _SECURITY_INTERFACE_H_

/*密钥算法标识*/
#define  THREE_DES      			0x01
#define  AES_128   			0x02
#define  AES_256  			0x03
#define  SM4       			0x04
#define  HMAC_SHA256		0x21
#define  SHA_256			0x31
#define  SM3				0x32

/*物联网安全服务平台   密钥类型 */
#define  OPER_DE_ENCRYPT    0x01
#define  OPER_MAC_KEY       0x02
#define  FIGURE_DE_ENCRYPT  0x03
#define  FIGURE_MAC_KEY     0x04
#define  DATA_DE_ENCRYPT    0x05
#define  DATA_MAC_KEY       0x06

/*IoT-SP平台  密钥类型 */
#define  SP_OPER_DE_ENCRYPT       0x21
#define  SP_OPER_MAC_KEY          0x22
#define  STANDARD_KEY			  0x23
#define  SP_DATA_DE_ENCRYPT       0x25
#define  SP_DATA_MAC_KEY          0x26

#define  SP_SENSE_DATA_DE_ENCRYPT 0x27
#define  SP_SENSE_DATA_MAC_KEY    0x28

#define  SP_CURRENCY_DE_ENCRYPT   0x29
#define  SP_CURRENCY_MAC_KEY      0x2A

extern void security(void);

extern void SP_accredit(void);

extern void SP_self(void);

extern void get_random(void);

extern void key_arrange(void);

#endif
