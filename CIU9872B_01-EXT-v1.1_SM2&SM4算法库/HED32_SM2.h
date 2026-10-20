/**
 * @file HED32_SM2.h
 * @brief SM2算法接口头文件
 * @author Yuzhh
 * @version 1.0
 * @date 2021-04-28
 * @attention
 *
 * <h2><center>CEC Huada Electronic Design Co.,Ltd</center></h2>
 *
 * The copyright of this manual is reserved by CEC Huada Electronic Design Co., Ltd.
 * Without prior permission, any duplication, printing or publication and distribution
 * of the manual will constitute a violation of copyright towards CEC Huada Electronic
 * Design Co., Ltd. CEC Huada Electronic Design Co., Ltd. reserves the right to bring
 * them to court.
 * The right to change the manual without notice to the user is reserved by CEC Huada
 * Electronic Design Co., Ltd. Though we have checked content of this manual, errors
 * are unavoidable. So, we will regularly check the content and make necessary
 * modifications in the next version. We suggest you obtain the latest version of this
 * manual from CEC Huada Electronic Design Co., Ltd. before making final design.
 *
 * @par History
 *  Date          | Version      | Author       | Description
 *  ------------- | -------------| -------------| ---------------
 *  2021-04-28    | V1.0         | Firmware Team| initial version
 *
 *
 */

#ifndef  HED32_SM2_INC_
#define  HED32_SM2_INC_
/**
 * @defgroup SM2_module SM2算法
 * @brief    SM2函数及定义
 * @par 概述
 * -# 提供国密SM2算法
 * @par 功能
 * -# 主要提供密钥产生、签名、验签、加密、解密、密钥交换、公私钥校验、Z值计算、点压缩、点解压缩
 * @if LITTLE_ENDIAN
 * -# 算法库数据为小端模式，即低字节存放低地址
 * @else
 * -# 算法库数据为大端模式，即高字节存放低地址
 * @endif
 * @{ */
/**
 * @defgroup sm2_return_value 返回值定义
 * @{ */
#ifndef   SM2_NORMAL_VAL
#define     SM2_SUCCESS             0x80A5 //!< SM2运算成功
#define     SM2_CURVE_INVALID       0x91B7 //!< SM2曲线参数无效
#define     SM2_FAILURE             0x92C9 //!< SM2运算失败
#define     SM2_RANDOM_FAILURE      0x93DB //!< SM2运算随机数失败

#else
#define     SM2_SUCCESS             0x0    //!< SM2运算成功
#define     SM2_CURVE_INVALID       0x1    //!< SM2曲线参数无效
#define     SM2_FAILURE             0x2    //!< SM2运算失败
#define     SM2_RANDOM_FAILURE      0x3    //!< SM2运算随机数失败
#endif   /* ----- #if (defined SM2_NORMAL_VAL) ----- */

/**  @} */
/**
 * @defgroup SM2_Parameters  结构体定义
 * @{ */

/* ********************************************************/
/**
 * @brief  SM2参数结构体定义
 */
/* ********************************************************/

#ifndef  SM2_Parameters
typedef struct
{
    const unsigned char   *iDataPtr;              //!<[in]       SM2运算输入数据
    unsigned char         *oDataPtr;              //!<[out]      SM2运算输出数据
    const unsigned int    *rndPtr;                //!<[in]       SM2算法中使用的随机数，8个字，该参数只在外部输入随机数时有效
    unsigned int          *priKeyPtr;             //!<[in, out]  SM2运算私钥，长度为8个字
    unsigned int          *pubKeyPtr;             //!<[in, out]  SM2运算公钥，长度为16个字，包含两部分（Px,Py）
    const unsigned int    *argPtr;                //!<[in]       SM2曲线参数，目前保留
    unsigned int           paraChkSum;            //!<[out]      安全函数输出运算参数校验值
} SM2_Parameters;

#endif   /* ----- #ifndef SM2_Parameters  ----- */
/* ********************************************************/
/**
 * @brief  SM2密钥交换参数结构体定义
 */
/* ********************************************************/
typedef struct
{
    const unsigned int   *ZA;                   //!<[in] 发送方身份标识，长度8个字
    const unsigned int   *ZB;                   //!<[in] 接收方身份标识，长度8个字
    const unsigned int   *PA;                   //!<[in] 发送方公钥，长度16个字
    const unsigned int   *PB;                   //!<[in] 接收方公钥，长度16个字
    const unsigned int   *RA;                   //!<[in] 发送方临时公钥，长度16个字
    const unsigned int   *RB;                   //!<[in] 接收方临时公钥，长度16个字
    const unsigned int   *selfDe;               //!<[in] 己方私钥，长度8个字
    const unsigned int   *selfTempDe;           //!<[in] 己方临时私钥，长度8个字
    const unsigned int   *argPtr;               //!<[in] 曲线参数，保留
    unsigned int          type;                 //!<[in] 己方身份： 0，接收方；1，发送方
    unsigned int          optMode;              //!<[in] 可选项模式：
    //! -   bit0：0，不输出S1; 1，输出S1
    //! -   bit1：0，不输出SA; 1，输出SA
    unsigned int          keyByteLen;           //!<[out] 输出交换密钥字节长度
    unsigned char        *key;                  //!<[out] 输出密钥的存储空间，长度为keyByteLen字节
    unsigned char        *S1;                   //!<[out] 输出可选值S1（或SB）的存储空间，32字节
    unsigned char        *SA;                   //!<[out] 输出可选值S2（或SA）的存储空间，32字节
    unsigned int          paraChkSum;           //!<[out] 安全函数输出运算参数校验值

} SM2_KeyExchange_Parameters;                /* ----------  end of struct SM2_KeyExchange_Parameters ---------- */

/**  @} */

/**
 * @defgroup sm2_function 接口函数
 * @{ */

/*-----------------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------------*/
/* ********************************************************/
/**
 * @brief  SM2私钥验证函数
 * @note
 * @arg 栈使用约为48字节
 * @arg 输入为8个字的私钥
 * @arg 算法API执行前后，保持本算法IP的门控时钟状态不变
 * @param SM2_Param
 *        @arg[in] SM2_Parameters#priKeyPtr    待验证的SM2私钥
 * @return
 *        - #SM2_SUCCESS ，密钥验证通过
 *        - @a 其它， 密钥验证失败
 */
/* ********************************************************/
unsigned int HED_SM2IsLegalPriKey(SM2_Parameters *SM2_Param);

/* ********************************************************/
/**
 * @brief  SM2公钥验证函数
 * @note
 * @arg 栈使用约为144字节
 * @arg 输入为16个字的公钥
 * @arg 算法API执行前后，保持本算法IP的门控时钟状态不变
 *
 * @param SM2_Param
 *        @arg[in] SM2_Parameters#pubKeyPtr     待验证的SM2公钥
 * @return
 *         - #SM2_SUCCESS ，密钥验证通过
 *         - @a 其它， 密钥验证失败
 */
/* ********************************************************/
unsigned int HED_SM2IsLegalPubKey(SM2_Parameters *SM2_Param);

/* ********************************************************/
/**
 * @brief  SM2密钥生成函数
 * @note
 * @arg 栈使用约为472字节
 * @param SM2_Param
 *         @arg[out] SM2_Parameters#priKeyPtr    生成的SM2私钥
 *         @arg[out] SM2_Parameters#pubKeyPtr    生成的SM2公钥
 * @return
 *          - #SM2_SUCCESS ，密钥生成成功
 *          - @a 其它， 密钥生成失败
 */
/* ********************************************************/
unsigned int HED_SM2GenKey(SM2_Parameters *SM2_Param);

/* ********************************************************/
/**
 * @brief  SM2签名函数
 * @note
 * @arg 栈使用约为544字节
 * @arg 输入输出数据地址不可以重合
 * @arg 待签名的消息为32字节，签名结果长度为64字节
 * @param SM2_Param
 *        @arg[in]  SM2_Parameters#iDataPtr                指向待签名的消息，长度为4*8字节，消息已写入
 *        @arg[in]  SM2_Parameters#priKeyPtr               指向用户私钥，长度为8字，私钥已经写入
 *        @arg[out] SM2_Parameters#oDataPtr                指向分配好的的一块内存，内存大小等于64字节
 * @return
 *         - #SM2_SUCCESS ，签名成功
 *         - @a 其它， 签名失败
 */
/* ********************************************************/
unsigned int HED_SM2Sign(SM2_Parameters *SM2_Param);

/* ********************************************************/
/**
 * @brief  SM2验签函数
 * @note
 * @arg 栈使用约为376字节
 * @arg 明文消息长度为32字节，签名值长度为64字节
 * @param SM2_Param
 *        @arg[in]  SM2_Parameters#iDataPtr                指向明文消息，长度为32字节，消息已写入
 *        @arg[in]  SM2_Parameters#pubKeyPtr               指向用户公钥，长度为16字，公钥已经写入
 *        @arg[in]  SM2_Parameters#oDataPtr                指向签名值，长度为64字节，分为r，s两部分，签名值已写入
 * @return
 *         - #SM2_SUCCESS ，验签成功
 *         - @a 其它， 验签失败
 */
/* ********************************************************/
unsigned int HED_SM2Verify(SM2_Parameters *SM2_Param);

/* ********************************************************/
/**
 * @brief  SM2加密函数
 * @note
 * @arg 栈使用约为600字节
 * @arg 输入输出数据地址空间不可以重合
 * @param SM2_Param
 *        @arg[in]  SM2_Parameters#iDataPtr                指向待加密的消息，长度为iLen字节，消息已写入
 *        @arg[in]  SM2_Parameters#pubKeyPtr               指向用户公钥，长度为16字，公钥已经写入
 *        @arg[out] SM2_Parameters#oDataPtr                指向分配好的一块内存，长度为iLen+64+32字节
 * @param [in]  iLen                                       输入消息的字节长度
 * @return
 *         - #SM2_SUCCESS ，加密成功
 *         - @a 其它，加密失败
 */
/* ********************************************************/
unsigned int HED_SM2Encrypt(SM2_Parameters *SM2_Param, unsigned int iLen);

/* ********************************************************/
/**
 * @brief  SM2解密函数
 * @note
 * @arg 栈使用约为576字节
 * @arg 输入输出数据地址空间不可以重合
 * @param SM2_Param
 *        @arg[in]  SM2_Parameters#iDataPtr                指向密文，长度为iLen字节，密文已写入
 *        @arg[in]  SM2_Parameters#priKeyPtr               指向用户私钥，长度为N字，私钥已经写入
 *        @arg[out] SM2_Parameters#oDataPtr                指向分配好的一块内存，长度为iLen-96字节
 * @param [in] iLen                                        输入密文的字节长度
 * @return
 *         - #SM2_SUCCESS ，解密成功
 *         - @a 其它，解密失败
 */
/* ********************************************************/
unsigned int HED_SM2Decrypt(SM2_Parameters *SM2_Param, unsigned int iLen);
//need input random

/* ********************************************************/
/**
 * @brief  SM2利用输入随机数生成密钥对函数
 * @note
 * @arg 栈使用约为472字节
 * @param SM2_Param
 *         @arg[in] SM2_Parameters#rndPtr             生成SM2密钥时用到的随机数
 *         @arg[out] SM2_Parameters#priKeyPtr         生成的SM2私钥
 *         @arg[out] SM2_Parameters#pubKeyPtr         生成的SM2公钥
 * @return
 *         - #SM2_SUCCESS ，密钥生成成功
 *         - @a 其它， 密钥生成失败
 */
/* ********************************************************/
unsigned int HED_SM2GenKey2(SM2_Parameters *SM2_Param);

/* ********************************************************/
/**
 * @brief  用户Za值计算函数
 * @note
 * @arg 栈使用约为350字节
 * @param[in]  AID                                    用户ID，LV结构（第一个字节为AID长度，随后为具体值，长度最大为32个字节）
 * @param[in]  PublicKey                              指向用户公钥数据
 * @param[out] Za                                     指向用户公钥数据，长度32字节
 * @param[in]  argPtr                                 保留
 * @return
 *         - #SM2_SUCCESS ，计算成功
 *         - @a 其它，计算失败
 */
/* ********************************************************/
unsigned int HED_SM2GetZ(const unsigned char *AID, const unsigned int *PublicKey, unsigned int *Za, const unsigned int *argPtr);

//! @cond WITH_KEY_EXCHANGE_MACRO
#ifndef     KEY_EXCHANG_SENDER
#define     KEY_EXCHANG_SENDER                        1    //!< 密钥交换发送方
#define     KEY_EXCHANG_RECEIVER                      0    //!< 密钥交换接收方
#endif
//! @endcond
/* ********************************************************/
/**
 * @brief  SM2密钥协商函数
 *
 * @param SM2KeyExch_Para
 *         @arg[in]   SM2_KeyExchange_Parameters#ZA          发送方身份标识，8字
 *         @arg[in]   SM2_KeyExchange_Parameters#ZB          接收方身份标识，8字
 *         @arg[in]   SM2_KeyExchange_Parameters#PA          发送方公钥，16字
 *         @arg[in]   SM2_KeyExchange_Parameters#PB          接收方公钥，16字
 *         @arg[in]   SM2_KeyExchange_Parameters#RA          发送方临时公钥，16字
 *         @arg[in]   SM2_KeyExchange_Parameters#RB          接收方临时公钥，16字
 *         @arg[in]   SM2_KeyExchange_Parameters#selfDe      己方私钥，8字
 *         @arg[in]   SM2_KeyExchange_Parameters#selfTempDe  己方临时私钥，8字
 *         @arg[in]   SM2_KeyExchange_Parameters#type        己方身份：0，接收方；1，发送方
 *         @arg[in]   SM2_KeyExchange_Parameters#optMode     可选项模式：\n
 *                                                             bit0：0，不输出S1；1， 输出S1； \n
 *                                                             bit1：0，不输出SA；1，输出SA
 *         @arg[in]   SM2_KeyExchange_Parameters#keyByteLen  输出交换密钥字节长度
 *         @arg[out]  SM2_KeyExchange_Parameters#key         输出密钥的存储空间，分配长度为keyByteLen长度
 *         @arg[out]  SM2_KeyExchange_Parameters#S1          输出可选值S1（或SB）的存储空间，32字节
 *         @arg[out]  SM2_KeyExchange_Parameters#SA          输出可选值S2（或SA）的存储空间，32字节
 *
 * @return
 *         - #SM2_SUCCESS ，密钥协商成功
 *         - @a 其它，密钥协商失败
 */
/* ********************************************************/
unsigned int HED_SM2KeyExchange(SM2_KeyExchange_Parameters *SM2KeyExch_Para);

/* ********************************************************/
/**
 * @brief  点压缩函数
 * @note
 * @arg 栈使用约为48字节
 * @arg 输出字节数为33字节
 * @param[out] x_y     输出点压缩后的值，前面一字节表征y坐标的最低位值，后面为x的值,共33字节
 * @param[in]  point   指向点，长度为64字，格式为x||y
 * @param[in]  bitLen  保留
 */
/* ********************************************************/
void HED_SM2PointCompress (unsigned char *x_y, const unsigned int *point, unsigned int bitLen);

/* ********************************************************/
/**
 * @brief  点解压缩函数
 * @note
 * @arg 栈使用约为312字节
 * @arg 输出字节数为64字节
 * @param[out] point   指向解压缩后的点，长度为64字，格式为x||y
 * @param[in]  x_y     点压缩后的值，前面一字节y坐标的最低位值，后面为x值，共33字节
 * @param[in] argPtr   保留
 * @return
 *         - #SM2_SUCCESS ，执行成功
 *         - @a 其它，执行失败
 */
/* ********************************************************/
unsigned int HED_SM2PointDecompress (unsigned int *point, const unsigned char *x_y, const unsigned int *argPtr);
//! @cond WITH_ECC
#define HED_SM2PointCompress(x_y, point,  bitLen)  HED_ECCPointCompress(x_y, point,  bitLen)
#define HED_SM2PointDecompress(point, x_y, argPtr) HED_ECCPointDecompress(point, x_y, NULL)
#ifndef  HED_ECCPointCompress 
extern void HED_ECCPointCompress (unsigned char *x_y,  const unsigned int *point, unsigned int bitLen);
extern unsigned int HED_ECCPointDecompress (unsigned int *point, const unsigned char *x_y, const unsigned int *argPtr);
#endif   /* ----- #ifndef HED_ECCPointCompress ----- */
//! @endcond
/**  @} */
/**  @} */
#endif   /* ----- #ifndef HED32_SM2_INC_  ----- */

