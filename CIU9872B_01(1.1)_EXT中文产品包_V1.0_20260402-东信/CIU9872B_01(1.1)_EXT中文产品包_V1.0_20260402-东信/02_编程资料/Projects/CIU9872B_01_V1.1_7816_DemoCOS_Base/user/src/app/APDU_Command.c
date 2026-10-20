/************************************************************************************************/
/**
* @file               APDU_Command.c
* @author             Firmware Team
* @brief              APDU application-layer processing
* @version            V1.0
* @date               2021-03-12
*
**************************************************************************************************
* @attention
*
* <h2><center>CEC Huada Electronic Design Co.,Ltd</center></h2>
*
* The copyright of this manual is reserved by CEC Huada Electronic Design Co., Ltd.
* Without prior permission, any duplication, printing or publication and distribution
* of the manual will constitute a violation of copyright towards CEC Huada Electronic
* Design Co., Ltd. CEC Huada Electronic Design Co., Ltd. reserves the right to bring
* them to court. \n
* The right to change the manual without notice to the user is reserved by CEC Huada
* Electronic Design Co., Ltd. Though we have checked content of this manual, errors
* are unavoidable. So, we will regularly check the content and make necessary
* modifications in the next version. We suggest you obtain the latest version of this
* manual from CEC Huada Electronic Design Co., Ltd. before making final design.
*
* @par Modify log
* <table>
* <tr><th>Date        <th>Version     <th>Author          <th>Description     </tr>
* <tr><td>2021/03/12  <td>V1.0        <td>Firmware Team   <td>First release   </tr>
* </table>
*
**************************************************************************************************
*/

/*------------------------------------------includes-----------------------------------------------*/
#include "Declare.h"

/************************************************************************************************/

#define DemoNVM_START_ADDR           0x19000// just take 100k for an example
#define DemoNVM_END_ADDR             0x64000// just take 400k for an example 

#define Card_Status_ADDR         0x1FFF8000
#define HEFlash_Addr             0x0011C000//base on the condition that MAX user FLASH is 0x11BFFF

#define ECB_ENCRYPT             0x00
#define ECB_DECRYPT             0x40
#define CBC_ENCRYPT             0x20
#define CBC_DECRYPT             0x60
#define MAC_EXTPARA             0x84
#define AES128_CBC_ENC          0x20

uint8_t LoopBackData[512] = {0x00};
static int16_t Sendlen = 0x00;
/** @addtogroup APP_Demonstrations
  * @{
  */

/** @addtogroup APDU_Command
  * @{
  */
static uint32_t AES_Gcm_ALG(void)
{
    GCM_Context gtxPtr;
    GCM_Context_Key gtx_keyPtr;
    uint8_t tag;
    size_t output_length;

    uint32_t ret = 0;
    uint8_t key_128[16] = {0xFE, 0xFF, 0xE9, 0x92, 0x86, 0x65, 0x73, 0x1C, 0x6D, 0x6A, 0x8F, 0x94, 0x67, 0x30, 0x83, 0x08};
    uint8_t iv_128[12] = {0xCA, 0xFE, 0xBA, 0xBE, 0xFA, 0xCE, 0xDB, 0xAD, 0xDE, 0xCA, 0xF8, 0x88};
    uint8_t add_128[20] = {0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED, 0xFA, 0xCE, 0xDE, 0xAD, 0xBE, 0xEF, 0xAB, 0xAD, 0xDA, 0xD2};
    uint8_t inputdata_128[60] = {0xD9, 0x31, 0x32, 0x25, 0xF8, 0x84, 0x06, 0xE5, 0xA5, 0x59, 0x09, 0xC5, 0xAF, 0xF5, 0x26, 0x9A, 0x86, 0xA7, 0xA9, 0x53, 0x15, 0x34, 0xF7, 0xDA, 0x2E, 0x4C, 0x30, 0x3D, 0x8A, 0x31, 0x8A, 0x72, 0x1C, 0x3C, 0x0C, 0x95, 0x95, 0x68, 0x09, 0x53, 0x2F, 0xCF, 0x0E, 0x24, 0x49, 0xA6, 0xB5, 0x25, 0xB1, 0x6A, 0xED, 0xF5, 0xAA, 0x0D, 0xE6, 0x57, 0xBA, 0x63, 0x7B, 0x39};
    uint8_t g_outputdata_128[60] = {0x42, 0x83, 0x1E, 0xC2, 0x21, 0x77, 0x74, 0x24, 0x4B, 0x72, 0x21, 0xB7, 0x84, 0xD0, 0xD4, 0x9C, 0xE3, 0xAA, 0x21, 0x2F, 0x2C, 0x02, 0xA4, 0xE0, 0x35, 0xC1, 0x7E, 0x23, 0x29, 0xAC, 0xA1, 0x2E, 0x21, 0xD5, 0x14, 0xB2, 0x54, 0x66, 0x93, 0x1C, 0x7D, 0x8F, 0x6A, 0x5A, 0xAC, 0x84, 0xAA, 0x05, 0x1B, 0xA3, 0x0B, 0x39, 0x6A, 0x0A, 0xAC, 0x97, 0x3D, 0x58, 0xE0, 0x91};
    uint8_t outputdata_128[60];


    //< ---------AES GCM 128-------------
    //< HED_GCMInit
    gtxPtr.pAlgEcbFunc = Rijndael_Encrypt;
    HED_GCMInit(&gtxPtr);
    //< HED_GCMSetkey
    gtx_keyPtr.pKeyExpansionFunc = Rijndael_KeyExpansion;
    ret |= HED_GCMSetkey(&gtxPtr, &gtx_keyPtr, key_128, 128);
    //< HED_GCMStarts
    gtxPtr.mode_ = HED_GCM_ENCRYPT;
    gtxPtr.iv_ = iv_128;
    gtxPtr.iv_len_ = (size_t)sizeof(iv_128);;
    ret |= HED_GCMStarts(&gtxPtr, &gtx_keyPtr);
    //< HED_GCMUpdataAd
    gtxPtr.ad_ = add_128;
    gtxPtr.ad_len_ = (size_t)sizeof(add_128);
    ret |= HED_GCMUpdataAd(&gtxPtr);
    //< HED_GCMUpdata
    gtxPtr.input_ = inputdata_128;
    gtxPtr.input_length_ = sizeof(inputdata_128);
    gtxPtr.output_ = outputdata_128;
    gtxPtr.output_size_ = sizeof(outputdata_128);
    gtxPtr.output_length_ = (void *)&output_length;
    ret |= HED_GCMUpdata(&gtxPtr, &gtx_keyPtr);
    //< HED_GCMFinish
    gtxPtr.tag_ = (uint8_t *)&tag;
    gtxPtr.tag_len_ = (size_t)16;
    ret |= HED_GCMFinish(&gtxPtr);

    ret |= memcmp(outputdata_128, g_outputdata_128, sizeof(g_outputdata_128));
    return ret;
}
static uint32_t AES_Ccm_ALG(void)
{
    CCM_Context ctxPtr;
    CCM_Context_Key ctx_keyPtr;
    uint8_t tag;
    size_t output_length;

    uint32_t ret = 0;
    uint8_t key_128[16] = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F};
    uint8_t iv_128[12] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B};
    uint8_t add_128[20] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13};
    uint8_t inputdata_128[24] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
    uint8_t g_outputdata_128[24] = {0xE3, 0xB2, 0x01, 0xA9, 0xF5, 0xB7, 0x1A, 0x7A, 0x9B, 0x1C, 0xEA, 0xEC, 0xCD, 0x97, 0xE7, 0x0B, 0x61, 0x76, 0xAA, 0xD9, 0xA4, 0x42, 0x8A, 0xA5};
    uint8_t outputdata_128[24];

    //< ---------AES GCM 128-------------
    //< HED_CCMInit
    ctxPtr.pAlgEcbFunc = Rijndael_Encrypt;
    HED_CCMInit(&ctxPtr);
    //< HED_CCMSetkey
    ctx_keyPtr.pKeyExpansionFunc = Rijndael_KeyExpansion;
    ret |= HED_CCMSetkey(&ctx_keyPtr, key_128, 128);
    //< HED_CCMStarts
    ctxPtr.mode_ = HED_GCM_ENCRYPT;
    ctxPtr.iv_ = iv_128;
    ctxPtr.iv_len_ = 12;
    ret |= HED_CCMStarts(&ctxPtr, &ctx_keyPtr);
    ///< HED_CCMSetLengths
    ctxPtr.total_ad_len_ = (size_t)sizeof(add_128);
    ctxPtr.plaintext_len_ = (size_t)sizeof(inputdata_128);
    ctxPtr.tag_len_ = (size_t)8;
    ret = HED_CCMSetLengths(&ctxPtr, &ctx_keyPtr);
    //< HED_CCMUpdataAd
    ctxPtr.ad_ = add_128;
    ctxPtr.ad_len_ = 20;
    ret |= HED_CCMUpdataAd(&ctxPtr, &ctx_keyPtr);
    //< HED_CCMUpdata
    ctxPtr.input_ = inputdata_128;
    ctxPtr.input_len_ = sizeof(inputdata_128);
    ctxPtr.output_ = outputdata_128;
    ctxPtr.output_size_ = sizeof(outputdata_128);
    ctxPtr.output_len_ = (void *)&output_length;
    ret |= HED_CCMUpdata(&ctxPtr, &ctx_keyPtr);
    //< HED_CCMFinish
    ctxPtr.tag_ = (uint8_t *)&tag;
    ret |= HED_CCMFinish(&ctxPtr, &ctx_keyPtr);

    ret |= memcmp(outputdata_128, g_outputdata_128, sizeof(g_outputdata_128));
    return ret;
}
/**
  * @brief  -# Demonstrate symmetric algorithm API call method
  * -# APDU command format:BFF0000000
  * @note   The Macro RUN_IN_DEBUG_TOOL definition out when SM1 is running on the
            chip, if SM1 is running on the development, the macro definition must
            be enable.
  * @param  none
  * @retval none
  */
void cmd_Symmetry_ALG(void)
{
//#define RUN_IN_DEBUG_TOOL
    uint32_t ret;

    DES_Parameters *Demo_DES_Param;
    DES_Parameters  Demo_DES_ParamVar;
    TDES_Parameters  Demo_3DES_Param;
    AES_Parameters  Demo_AES_Param;

    /*************************************************
        DES/3DES/SSF33 standard data
    *************************************************/
    uint32_t  std_DES_Key[6] = {0x53D6A7A9, 0x0D38BC86, \
                                0x13B88389, 0x963E20CB, 0x0DCDD706, 0x01E3F300
                               };
    uint32_t  std_DES_IV[2] = {0x3DCD8D1E, 0x76F1233B};
    uint32_t  std_DES_InData[8] = {0xF0E5AC7F, 0x803571B0, 0xA7408E0C, 0x378C05AE, \
                                   0xE58732C7, 0xCC28A34E, 0x0B52EE7A, 0x8214C162
                                  };

    uint32_t  std_DES_ECB_EncData[8] = {0x1D1A9D5A, 0x7BE7C2F0, 0xDD3BFC0A, 0x74DBA3C3, 0x5DAFFB8C, 0x87286F17, 0xE36194F3, 0x307DFAA3};
    uint32_t  std_2Key3DES_CBC_EncData[8] = {0x8E507B1D, 0x81EF13C6, 0x56BCF427, 0x8A35434D, 0xA5DB5C40, 0x04B9FE12, 0x5A81FED3, 0x82AC8DF7};
//    uint32_t  std_3Key3DES_CBC_EncData[8] = {0x789CE568, 0x88BF3C43, 0x1EE20937, 0x3F6ECA98, 0x84745AF9, 0x3D2E8300, 0x8BC69E0F, 0x54CBC0A8};


    uint32_t  std_AES128_Key[4] = {0x2B7E1516, 0x28AED2A6, 0xABF71588, 0x09CF4F3C};
    uint32_t  std_AES128_IV[4] = {0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F};
    uint32_t  std_AES128_InData[8] = {0x6BC1BEE2, 0x2E409F96, 0xE93D7E11, 0x7393172A, 0xAE2D8A57, 0x1E03AC9C, 0x9EB76FAC, 0x45AF8E51};
    uint32_t  std_AES128_CBC_EncData[8] = {0x7649ABAC, 0x8119B246, 0xCEE98E9B, 0x12E9197D, 0x5086CB9B, 0x507219EE, 0x95DB113A, 0x917678B2};


    /*1. run DES ECB_ENC */
    //1-1.config DES struct para
    Demo_DES_Param = &Demo_DES_ParamVar;
    Demo_DES_Param->iDataPtr = std_DES_InData;
    Demo_DES_Param->oDataPtr = G_output;
    Demo_DES_Param->DataLen = sizeof(std_DES_InData);
    Demo_DES_Param->IVPtr = std_DES_IV;
    Demo_DES_Param->iKeyPtr = std_DES_Key;
    Demo_DES_Param->Type = ECB_ENCRYPT;
    //    Demo_DES_Param->paraChkSum = G_paraCheckSum;

    //1-2.run DES
    ret = HED_DESBlock(Demo_DES_Param);

    if((ret != 0xB7C8D9EA) ||
            (Demo_DES_Param->paraChkSum != (uint32_t)std_DES_InData + ((uint32_t)G_output) + sizeof(std_DES_InData) + (uint32_t)Demo_DES_Param->iKeyPtr + Demo_DES_Param->Type))
    {
        SW1SW2 = 0x6F01;
        return;
    }

    //1-3.check DES data
    ret = memcmp(Demo_DES_Param->oDataPtr, std_DES_ECB_EncData, sizeof(std_DES_ECB_EncData));

    if(ret != 0)
    {
        SW1SW2 = 0x6F02;
        return;
    }

    /*2. run 3DES 2KEY CBC ENC /DEC */
    //2-1.config 3DES struct para
    Demo_3DES_Param.iDataPtr = std_DES_InData;
    Demo_3DES_Param.oDataPtr = G_output;
    Demo_3DES_Param.DataLen = sizeof(std_DES_InData);
    Demo_3DES_Param.IVPtr = std_DES_IV;
    Demo_3DES_Param.iKeyPtr = std_DES_Key;
    Demo_3DES_Param.Type = CBC_ENCRYPT;
    //    Demo_3DES_Param.paraChkSum = G_paraCheckSum;

    //2-2.run 3DES
    ret = HED_TDESBlock_2Key(&Demo_3DES_Param);

    if((ret != 0xB7C8D9EA) ||
            (Demo_3DES_Param.paraChkSum != (uint32_t)std_DES_InData + ((uint32_t)G_output) + sizeof(std_DES_InData) + (uint32_t)Demo_3DES_Param.iKeyPtr + Demo_3DES_Param.Type))
    {
        SW1SW2 = 0x6F03;
        return;
    }

    //2-3.check 3DES data
    ret = memcmp(Demo_3DES_Param.oDataPtr, std_2Key3DES_CBC_EncData, sizeof(std_2Key3DES_CBC_EncData));

    if(ret != 0)
    {
        SW1SW2 = 0x6F04;
        return;
    }

    //2-4.check 3DES data Dec
//    Demo_3DES_Param.iDataPtr = std_DES_InData;
//    Demo_3DES_Param.oDataPtr = G_output;
//    Demo_3DES_Param.DataLen = sizeof(std_DES_InData);
//    Demo_3DES_Param.IVPtr = std_DES_IV;
//    Demo_3DES_Param.iKeyPtr = std_DES_Key;
//    Demo_3DES_Param.Type = 0x20;
//
//    HED_TDESBlock_3Key(&Demo_3DES_Param);
//    ret = memcmp(Demo_3DES_Param.oDataPtr, std_3Key3DES_CBC_EncData, sizeof(std_3Key3DES_CBC_EncData));
//
//    if(ret != 0)
//    {
//        SW1SW2 = 0x6F05;
//        return;
//    }

    /*3. run AES ECB, other AES operations is same */
    Demo_AES_Param.iDataPtr = std_AES128_InData;
    Demo_AES_Param.oDataPtr = G_output;
    Demo_AES_Param.DataLen = sizeof(std_AES128_InData);
    Demo_AES_Param.iKeyPtr = std_AES128_Key;
    Demo_AES_Param.IVPtr = std_AES128_IV;
    Demo_AES_Param.Type = AES128_CBC_ENC;
    //  Demo_AES_Param.paraChkSum = G_paraCheckSum;

    ret = HED_AESBlock(&Demo_AES_Param);

    if((ret != 0xB7C8D9EA) ||
            (Demo_AES_Param.paraChkSum != (uint32_t)std_AES128_InData + ((uint32_t)G_output) + sizeof(std_AES128_InData) + (uint32_t)Demo_AES_Param.iKeyPtr + Demo_AES_Param.Type))
    {
        SW1SW2 = 0x6F0B;
        return;
    }

    ret = memcmp(Demo_AES_Param.oDataPtr, std_AES128_CBC_EncData, sizeof(std_AES128_CBC_EncData));

    if(ret != 0)
    {
        SW1SW2 = 0x6F0C;
        return;
    }

    ret = AES_Gcm_ALG();

    if(ret != 0)
    {
        SW1SW2 = 0x6F0D;
        return;
    }

    ret = AES_Ccm_ALG();

    if(ret != 0)
    {
        SW1SW2 = 0x6F0E;
        return;
    }

    //return
    SW1SW2 = 0x9000;
}

/**
  * @brief  -# Demonstrate CRC algorithm API call method
  * -# APDU command format:BFF2000000
  * @note   none
  * @param  none
  * @retval none
  */
void cmd_CRC_Calculate(void)
{
    uint16_t ret;
    uint8_t crcXInput[261];
    uint32_t crc32_ret;

    /*************************************************
        CRC16 standard data
    *************************************************/
    const uint8_t crcFInput[261] = {0xBA, 0x37, 0xB1, 0x5A, 0xCF, 0xF7, 0x7C, 0x7D, 0x0C, 0xFD, 0xA4, 0x06, 0xC2, 0xA6, 0x6D, 0xD8, 0x35, 0x02, 0x8D, 0x5C, 0x28, 0x4A, 0x57, 0x1B, 0xA7, 0xA0, 0x25, 0x9D, 0x16, 0x08, 0x06, 0x83, \
                                    0x79, 0xA5, 0xA7, 0xE3, 0x04, 0x37, 0x85, 0xE2, 0xE4, 0xD3, 0x52, 0x96, 0x4E, 0xE3, 0xA6, 0x1D, 0xC8, 0x60, 0xE9, 0xF7, 0xB6, 0x4C, 0x7F, 0xAB, 0x26, 0x74, 0x34, 0x98, 0xDF, 0x66, 0xE9, 0x1A, \
                                    0xA6, 0xB0, 0x7F, 0xE0, 0xD1, 0x59, 0x02, 0x8B, 0x10, 0xA3, 0x18, 0x8A, 0x7A, 0x01, 0xAA, 0x33, 0xD5, 0x54, 0xD3, 0x24, 0x29, 0x6D, 0x08, 0xDB, 0x86, 0xBE, 0x87, 0x92, 0x13, 0x61, 0x04, 0x5E, \
                                    0x59, 0x4D, 0x90, 0x8A, 0xD0, 0xD6, 0xCC, 0x30, 0xA9, 0x64, 0x4D, 0x18, 0xDC, 0x76, 0x51, 0xD3, 0x74, 0xD9, 0xA2, 0x19, 0x19, 0x23, 0xC9, 0x62, 0xDD, 0x75, 0x75, 0xC2, 0x4A, 0x70, 0x2D, 0x09, \
                                    0xAA, 0x76, 0x33, 0x19, 0x97, 0x24, 0xBA, 0x88, 0xC5, 0x0E, 0x4A, 0x79, 0x0E, 0xBC, 0x72, 0xB5, 0xBD, 0xE4, 0xAF, 0x10, 0x1E, 0xE8, 0x9A, 0xF9, 0x44, 0x91, 0x57, 0x61, 0x1C, 0x0B, 0x3C, 0xD1, \
                                    0xB1, 0x22, 0xC0, 0xC5, 0xC0, 0xBD, 0xA5, 0x4C, 0x7E, 0x9A, 0x67, 0xE4, 0xA6, 0x4B, 0xE6, 0x91, 0xC8, 0x6F, 0x51, 0x3F, 0xD0, 0x33, 0x54, 0x58, 0xD4, 0x0A, 0x85, 0xA8, 0x21, 0xAA, 0x0A, 0x6F, \
                                    0x86, 0x49, 0x8E, 0xC5, 0xE1, 0x18, 0x64, 0x34, 0xEA, 0xFF, 0xFB, 0x93, 0x3E, 0x99, 0x84, 0x1F, 0xAE, 0x71, 0xE1, 0xDF, 0xC7, 0x7C, 0xCF, 0x37, 0xA3, 0xD9, 0x56, 0xCC, 0xF1, 0xC6, 0x6F, 0x9A, \
                                    0x42, 0xE4, 0xF5, 0x52, 0x94, 0xAE, 0xD0, 0xF8, 0x23, 0x35, 0x5F, 0xBC, 0x6D, 0x20, 0x25, 0x17, 0x85, 0xE2, 0xB6, 0x27, 0x9B, 0x3C, 0xE2, 0x4E, 0xCB, 0xF5, 0x22, 0x08, 0x24, 0xD6, 0x42, 0x0C, \
                                    0xFB, 0xEA, 0x4E, 0xA5, 0x70
                                   };
    const uint8_t crcFInput2[13] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x11, 0x22, 0x33, 0x44};
    uint8_t crc32Input1[32] = {0x9C, 0xA2, 0xEA, 0xBE, 0x23, 0xCE, 0x2A, 0x2C, 0x4E, 0x5A, 0x4E, 0xC5, 0x66, 0xCB, 0x05, 0x48, 0xD2, 0xCA, 0xE2, 0x22, 0x5F, 0x70, 0x23, 0x4C, 0x37, 0x88, 0x82, 0xA7, 0xD7, 0x60, 0x7B, 0xC1};
    uint8_t crc32Input2[16] = {0xFC, 0xDF, 0xA2, 0x55, 0xFB, 0xBF, 0x81, 0x12, 0x11, 0x07, 0xE6, 0xDC, 0x52, 0xD3, 0x05, 0xCC};

#define std_CRCOutput_6363          0x8AA0
#define std_CRCOutput_FFFF          0x2152
#define std_CRCOutput_JAVA_TCK      0x3A2F

    HAL_CKMU_AXBControl(AHB2_CLK, CRCEN, 1);; //enable CRC IP's CLK
    memcpy(crcXInput, crcFInput, sizeof(crcXInput));

    ret = HED_CRC16(0x6363, sizeof(crcXInput), crcXInput, CRC_TYPE_TYPE_A);

    if(ret != std_CRCOutput_6363)
    {
        SW1SW2 = 0x6F01;
        return;
    }

    ret = HED_CRC16(0xFFFF, sizeof(crcFInput), (uint8_t *)crcFInput, CRC_TYPE_TYPE_B);

    if((uint16_t)(ret) != std_CRCOutput_FFFF)
    {
        SW1SW2 = 0x6F02;
        return;
    }

    ret = HED_CRC16(0xFFFF, sizeof(crcFInput2), (uint8_t *)crcFInput2, CRC_TYPE_JAVATCK); //fit to JC API's requirement,can pass TCK verify

    if((uint16_t)(ret) != std_CRCOutput_JAVA_TCK)
    {
        SW1SW2 = 0x6F03;
        return;
    }

#define std_CRC32Output_FFFFFFFF        0x55DB71A5  //crc32 32bit result
#define std_CRC32Output_6363            0xBB76      //crc32 16bit result

    crc32_ret = HED_CRC32(0xFFFFFFFF, sizeof(crc32Input1), crc32Input1, CRC_TYPE_CRC32);

    if(crc32_ret != std_CRC32Output_FFFFFFFF)
    {
        SW1SW2 = 0x6F04;
        return;
    }

    crc32_ret = HED_CRC16(0x6363, sizeof(crc32Input2), crc32Input2, CRC_TYPE_TYPE_A);

    if(crc32_ret != std_CRC32Output_6363)
    {
        SW1SW2 = 0x6F05;
        return;
    }

    HAL_CKMU_AXBControl(AHB2_CLK, CRCOFF, 1);; //close CRC IP's CLK
    SW1SW2 = 0x9000;
}

/**
  * @brief  -# Demonstrate HASH algorithm API call method
  * -# APDU command format:BFF8000000
  * @note   none
  * @param  none
  * @retval none
  */
void cmd_HASH_ALG(void)
{
    uint32_t  out_hash160_v[7];
    uint32_t  out_hash256_v[10];
    uint32_t len;
    uint32_t *inPtr;
    uint8_t ret;

    /*************************************************
        SHA1\SHA256 standard data
    *************************************************/
    uint8_t  std_SHA1_inputLess64[58] __attribute__((aligned(4))) = {0x0A, 0x0F, 0xEA, 0xAA, 0x53, 0x66, 0x5D, 0x4C, 0x8C, 0xB3, 0x85, 0x66, 0xDF, 0x4E, 0xE6, 0x1F, 0x27, 0x3B, 0xB6, 0x34, 0x83, 0x42, 0x41, 0x58, 0x08, 0x17, 0x3C, 0x55, 0x95, 0x4B, 0xFE, 0x8E, \
                                                                     0xC5, 0xCB, 0xBA, 0xF6, 0x76, 0xB8, 0x02, 0xAB, 0xD8, 0x94, 0x45, 0x88, 0x4C, 0x69, 0x7F, 0x80, 0x81, 0x52, 0x1C, 0x0B, 0x4A, 0x88, 0x30, 0x0E, 0x25, 0x46
                                                                    };
    uint8_t  std_SHA1_outputLess64[20] __attribute__((aligned(4))) = {0x34, 0x8C, 0x74, 0xD1, 0xEE, 0xB3, 0x33, 0x49, 0x9B, 0x8F, 0xE2, 0xC2, 0xD0, 0x5D, 0xBF, 0xD0, 0x9B, 0xA3, 0x6D, 0x2A};

    uint8_t  std_SHA_inputMore64[132] __attribute__((aligned(4))) = {0xF4, 0x4C, 0x7D, 0x4F, 0x8D, 0xF0, 0xFD, 0x73, 0xE8, 0x15, 0x6A, 0x23, 0x26, 0x0B, 0x04, 0x05, 0x3A, 0x16, 0xF0, 0x79, 0x4E, 0xB1, 0x06, 0xE5, 0x95, 0x4E, 0x32, 0xEE, 0x48, 0xD8, 0x6C, 0x2A, \
                                                                     0xD8, 0xA2, 0x38, 0xDF, 0x6A, 0x5D, 0x07, 0xD6, 0x88, 0x13, 0xBD, 0x8A, 0xAD, 0x63, 0x55, 0xFD, 0x0A, 0x2A, 0xDA, 0x8F, 0x74, 0xB7, 0x89, 0x1A, 0xC8, 0x42, 0x24, 0x72, 0xB9, 0x90, 0xFC, 0x2A, \
                                                                     0xE8, 0x8B, 0x62, 0xC4, 0xD0, 0x0E, 0xC1, 0x89, 0x3C, 0xBD, 0x24, 0xE9, 0xCE, 0xC1, 0x6A, 0xD3, 0x59, 0xC7, 0xDB, 0x27, 0xD0, 0x49, 0x19, 0x65, 0xC8, 0x10, 0x2C, 0x87, 0xE4, 0x18, 0xCC, 0x1C, \
                                                                     0x11, 0xED, 0x99, 0xB6, 0xF9, 0x99, 0xF9, 0xEE, 0xCC, 0xBF, 0x7E, 0xE7, 0xD4, 0x3A, 0x30, 0xFA, 0xAA, 0xAE, 0xF4, 0x26, 0x38, 0xD4, 0x7D, 0xAA, 0x55, 0xB1, 0x8E, 0xC5, 0x70, 0x22, 0x34, 0xB8, \
                                                                     0x0D, 0x18, 0x68, 0x67
                                                                    };
    uint8_t  std_SHA1_input64[64] __attribute__((aligned(4))) = {0x0A, 0x0F, 0xEA, 0xAA, 0x53, 0x66, 0x5D, 0x4C, 0x8C, 0xB3, 0x85, 0x66, 0xDF, 0x4E, 0xE6, 0x1F, 0x27, 0x3B, 0xB6, 0x34, 0x83, 0x42, 0x41, 0x58, 0x08, 0x17, 0x3C, 0x55, 0x95, 0x4B, 0xFE, 0x8E, \
                                                                 0xC5, 0xCB, 0xBA, 0xF6, 0x76, 0xB8, 0x02, 0xAB, 0xD8, 0x94, 0x45, 0x88, 0x4C, 0x69, 0x7F, 0x80, 0x81, 0x52, 0x1C, 0x0B, 0x4A, 0x88, 0x30, 0x0E, 0x25, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                                                                };
    uint8_t  std_SHA1_outputMore64[20] __attribute__((aligned(4))) = {0xFB, 0x74, 0xDD, 0xEB, 0xDB, 0x40, 0x34, 0x5C, 0x10, 0xC0, 0x09, 0x8B, 0x80, 0x3D, 0x25, 0x35, 0xF1, 0xB9, 0x43, 0xD6};
    uint8_t  std_SHA256_outputMore64[32] __attribute__((aligned(4))) = {0x39, 0x21, 0xE9, 0x5E, 0xCA, 0xE2, 0x33, 0xB3, 0x2C, 0xB8, 0x48, 0xFC, 0x45, 0xF3, 0x89, 0xA7, 0x84, 0xF1, 0x78, 0x7B, 0xF2, 0x41, 0x2E, 0x9D, 0x49, 0x25, 0xDB, 0x23, 0x4A, 0x4A, 0xDE, 0xA1};

    /*1. SHA1 , data length less than 64Bytes */
    //1-1.run these sha1 fun
    HED_SHA1_Init(out_hash160_v);
    HED_SHA1_Block(out_hash160_v, (uint32_t *)std_SHA1_inputLess64, sizeof(std_SHA1_inputLess64));

    //1-2.check sha1 output data
    ret = memcmp(out_hash160_v, std_SHA1_outputLess64, 20);

    if(ret != 0)
    {
        SW1SW2 = 0x6F01;
        return;
    }

    /*2. SHA1 , data length more than 64Bytes */
    len = sizeof(std_SHA_inputMore64);
    HED_SHA1_Init(out_hash160_v);                  //init
    inPtr = (uint32_t *)std_SHA_inputMore64;

    for(; len >= 64;)
    {
        HED_SHA1_Block(out_hash160_v, inPtr, 64);       //ProcBlock
        len -= 64;
        inPtr += 16;
    }

    HED_SHA1_Block(out_hash160_v, inPtr, len);     //FinalBlock

    ret = memcmp(out_hash160_v, std_SHA1_outputMore64, 20);

    if(ret != 0)
    {
        SW1SW2 = 0x6F02;
        return;
    }

    /*3. SHA1 , data length equal to 64Bytes */
    inPtr = (uint32_t *)std_SHA1_input64;
    HED_SHA1_Init(out_hash160_v);
    HED_SHA1_Block(out_hash160_v, inPtr, 64);       //ProcBlock
    HED_SHA1_Block(out_hash160_v, inPtr + 16, 0);  //FinalBlock

    /*4. SHA256 , data length more than 64Bytes */
    len = sizeof(std_SHA_inputMore64);
    HED_SHA256_Init(out_hash256_v);                  //init
    inPtr = (uint32_t *)std_SHA_inputMore64;

    for(; len >= 64;)
    {
        HED_SHA256_Block(out_hash256_v, inPtr, 64);       //ProcBlock
        len -= 64;
        inPtr += 16;
    }

    HED_SHA256_Block(out_hash256_v, inPtr, len);     //FinalBlock

    ret = memcmp(out_hash256_v, std_SHA256_outputMore64, 32);

    if(ret != 0)
    {
        SW1SW2 = 0x6F03;
        return;
    }

    /*5. SHA256 , data length equal to 64Bytes */
    inPtr = (uint32_t *)std_SHA1_input64;
    HED_SHA256_Init(out_hash256_v);
    HED_SHA256_Block(out_hash256_v, inPtr, 64);       //ProcBlock
    HED_SHA256_Block(out_hash256_v, inPtr + 16, 0);  //FinalBlock

    SW1SW2 = 0x9000;
}

void cmd_LOOP_BACK(uint32_t interfaceFlag)
{
    uint16_t p3;

    if(interfaceFlag == INTERFACE_7816)
    {
        SendINS(INS);//send INS
        p3 = (uint16_t)(P3 == 0x00) ? ((EXTENDLC_H << 8)  | EXTENDLC_L) : (P3 & 0xFF);
        HED_T0_ReceiveNBytesRam(&SCIAPDUBuf[5], p3);
        memcpy(&LoopBackData[0], &SCIAPDUBuf[5], p3);
        Sendlen = p3;
        SW1SW2 = (0x61 << 8) | (P3 & 0xFF);
    }
}

#define   RNG_Exe_Success    0xBC2679B3
#define   RNG_Exe_Fail       0x927C45A5
#define   Retry_MAX_NUM      3
/**
  * @brief  -# Demonstrate to obtain true or false random number, obtain factory
               code and unique serial number of chip API call method
  * -# APDU command format:obtain 8 bytes truly random number:BFC6010008
                           obtain 8 bytes pseudo-radom nunmber:BFC6020008
                           obtain 32 bytes factory code:BFC6030020
                           obtain 7~8 bytes unique serial number:BFC6040007~BFC6040008
  * @note   none
  * @param  none
  * @retval none
  */
void cmd_GET_INFO(uint32_t interfaceFlag)
{
    uint8_t p3;
    uint32_t ret;

    /*no check P3 ,user can add it */
    p3 = P3;

    /* according to P1, do*/
    switch(P1)
    {
        case 0x01:

            ret = HED_GetRandom_Enhanced(G_SendBuf, 8, &G_paraCheckSum);

            if((ret == RNG_Exe_Success) && (G_paraCheckSum == (uint32_t)(G_SendBuf + 8)))
            {
                SW1SW2 = 0x9000;
            }
            else
            {
                SW1SW2 = 0x6F00;
                return;
            }

            break;

        case 0x02:

            ret = HED_GetRandom(G_SendBuf, 8, &G_paraCheckSum);

            if((ret == RNG_Exe_Success) && (G_paraCheckSum == (uint32_t)(G_SendBuf + 8)))
            {
                SW1SW2 = 0x9000;
            }
            else
            {
                SW1SW2 = 0x6F00;
                return;
            }

            break;

        case 0x03:
            HED_GetFactoryCode(G_SendBuf);
            SW1SW2 = 0x9000;
            break;

        case 0x04:
            ret = HED_GetChipId(G_SendBuf, p3);

            if(ret == 0x00)
            {
                SW1SW2 = 0x9000;
            }
            else
            {
                SW1SW2 = 0x6F00;
                return;
            }

            break;

        default:
            SW1SW2 = 0x6F00;
            return;
    }
	if(interfaceFlag == INTERFACE_7816)
	{
		SendINS(INS);//send INS
	}
    G_APDU_SxLen = (p3 == 0 ? 256 : p3);
}


#define   Card_Status_ADDR    0x1FFF8000
#define   FlashPageSize       0x200
/**
  * @brief  -# Demonstrate HEFlash EW API call method
  * @brief  -# Demonstrate back to download state through APDU.
  * -# APDU command format：BFD4000000
  * @note   none
  * @param  none
  * @retval none
  */
void cmd_back2Loader(void)
{
    uint32_t  DataToNVM[128];
    uint32_t  ret = 0;

    memcpy((uint8_t *)DataToNVM, (uint8_t *)Card_Status_ADDR, FlashPageSize);
    DataToNVM[0] = 0xB639A527;//this value is refer to NVR user para sector

    ret = HED_UserPara_EWPage(Card_Status_ADDR, DataToNVM);

    if(ret == 0)
    {
        SW1SW2 = 0x9000;
    }
    else
    {
        SW1SW2 = 0x6A80;
    }
}

/**
  * @brief  -# Demonstrate SPI/I2C/7816 interface read data method(used with cmd_selectFile);
  * -# APDU command format:BFC00000P3
  * @note   reads the data written to RAM by the cmd_selectFile command
  * @param  none
  * @retval none
  */
void cmd_getResponse(uint32_t interfaceFlag)
{
    uint16_t t_Sentlen;
    t_Sentlen = P3;

    if(interfaceFlag == INTERFACE_7816)
    {
        if(t_Sentlen == 0x00)
        {
            t_Sentlen = 256;
        }

        if(t_Sentlen > Sendlen)
        {
            SW1SW2 = 0x6F00 | (Sendlen & 0xFF);
            return;
        }
        else
        {
            SendINS(INS);
            memcpy(G_SendBuf, &LoopBackData[0], t_Sentlen);
            G_APDU_SxLen = t_Sentlen;
            SW1SW2 = 0x9000;
            Sendlen -= t_Sentlen;

            if(Sendlen > 0x00)
            {
                memcpy(&LoopBackData[0], &LoopBackData[t_Sentlen], Sendlen);
            }
        }
    }
}

/**
  * @brief  -# Demonstrate SPI/I2C/7816 interface write data method（used with cmd_getResponse);
  * -# APDU command format:BFA40000P3 XX……XX
  * @note   无
  * @param  无
  * @retval 无
  */
void cmd_selectFile(void)
{
    if(SCIAPDUBuf[4] == 0x02)
    {
        SW1SW2 = 0x6115;
    }
    else
    {
        SW1SW2 = 0x9000;
    }
}

/**
  * @}
  */

/**
  * @}
  */
