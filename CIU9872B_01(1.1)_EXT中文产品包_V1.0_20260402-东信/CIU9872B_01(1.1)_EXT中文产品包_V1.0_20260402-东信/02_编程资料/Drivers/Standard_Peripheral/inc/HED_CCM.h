/**
  ******************************************************************************
  * @file    HED_CCM.h
  * @author  Firmware Team
  * @brief   Alg lib Header File
  * @date    2021-12-20
  * @version V1.0
  ******************************************************************************
  * @attention HED_CCM.h
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
  * @par 修改日志
  * <table>
  * <tr><th>Date        <th>Version     <th>Author          <th>Description   </tr>
  * <tr><td>2021/04/08  <td>V1.0        <td>Firmware Team   <td>创建            </tr>
  * <tr><td>2021/04/08  <td>V1.1        <td>Firmware Team   <td>修改XXXX        </tr>
  * </table>
  ******************************************************************************
  */
/** @defgroup ALG
  * @brief ALG core code
  * @{
  */
/** @defgroup MODE
  * @brief HED_CCM.h
  * @{
  */
#ifndef HED_CCM_H__
#define HED_CCM_H__
/************************************************************************************/
#include <stdint.h>
#include <string.h>
/************************************************************************************/
#define HED_CCM_DECRYPT       1
#define HED_CCM_ENCRYPT       0

/** Bad input parameters to the function. */
#define HED_ERR_CCM_BAD_INPUT             0x000D
/** Authenticated decryption failed. */
#define HED_ERR_CCM_AUTH_FAILED           0x000F
/** This is a bug in the library */
#define HED_ERR_ERROR_CORRUPTION_DETECTED 0x006E

/**
 * \brief    The CCM context-type definition. The CCM context is passed
 *           to the APIs called.
 */
typedef struct CCM_Context_Key
{
    uint32_t Nr;
    uint8_t rk[240];
    void (* pKeyExpansionFunc)(uint8_t *, uint8_t *, uint16_t);
}
CCM_Context_Key;

typedef struct CCM_Context
{
    uint8_t y[16];    /*!< The Y working buffer */
    uint8_t ctr[16];  /*!< The counter buffer */
    //HED_cipher_context_t HED_PRIVATE(cipher_ctx);    /*!< The cipher context used. */
    size_t plaintext_len;   /*!< Total plaintext length */
    size_t add_len;         /*!< Total authentication data length */
    size_t tag_len;         /*!< Total tag length */
    size_t processed;       /*!< Track how many bytes of input data
                                 were processed (chunked input).
                                 Used independently for both auth data
                                 and plaintext/ciphertext.
                                 This variable is set to zero after
                                 auth data input is finished. */
    uint8_t q;              /*!< The Q working value */
    uint8_t mode;           /*!< The operation to perform:
                                 #HED_CCM_ENCRYPT or
                                 #HED_CCM_DECRYPT or
                                 #HED_CCM_STAR_ENCRYPT or
                                 #HED_CCM_STAR_DECRYPT. */
    int32_t state;          /*!< Working value holding context's
                                 state. Used for chunked data
                                 input */
    // starts fun para
    int32_t mode_;
    const uint8_t *iv_;
    size_t iv_len_;
    // set lengths fun para
    size_t total_ad_len_;
    size_t plaintext_len_;
    size_t tag_len_;
    // updata ad fun para
    const uint8_t *ad_;
    size_t ad_len_;
    // updata fun para
    const uint8_t *input_;
    size_t input_len_;
    uint8_t *output_;
    size_t output_size_;
    size_t *output_len_;
    // finish fun para
    uint8_t *tag_;
    //size_t tag_len_;
    void (* pAlgEcbFunc)(uint8_t *, uint8_t *, uint8_t);
}
CCM_Context;


/**
 * \brief           This function initializes the specified CCM context,
 *                  to make references valid, and prepare the context
 *                  for HED_CCMSetkey().
 *
 * \param ctx       The CCM context to initialize. This must not be \c NULL.
 */
void HED_CCMInit(CCM_Context *ctx);

/**
 * \brief           This function initializes the CCM context set in the
 *                  \p ctx parameter and sets the encryption key.
 *
 * \param ctx       The CCM context to initialize. This must be an initialized
 *                  context.
 * \param key       The encryption key. This must not be \c NULL.
 * \param keybits   The key size in bits. This must be acceptable by the cipher.
 *
 * \return          \c 0 on success.
 * \return          A CCM or cipher-specific error code on failure.
 */
uint32_t HED_CCMSetkey(CCM_Context_Key *ctx_key,
                       const uint8_t *key,
                       uint16_t keybits);


/**
 * \brief           This function starts a CCM encryption or decryption
 *                  operation.
 *
 *                  This function and HED_CCMSetLengths() must be called
 *                  before calling HED_CCMUpdataAd() or
 *                  HED_CCMUpdata(). This function can be called before
 *                  or after HED_CCMSetLengths().
 *
 * \note            This function is not implemented in Mbed TLS yet.
 *
 * \param ctx       The CCM context. This must be initialized.
 * \param mode      The operation to perform: #HED_CCM_ENCRYPT or
 *                  #HED_CCM_DECRYPT or #HED_CCM_STAR_ENCRYPT or
 *                  #HED_CCM_STAR_DECRYPT.
 * \param iv        The initialization vector. This must be a readable buffer
 *                  of at least \p iv_len Bytes.
 * \param iv_len    The length of the nonce in Bytes: 7, 8, 9, 10, 11, 12,
 *                  or 13. The length L of the message length field is
 *                  15 - \p iv_len.
 *
 * \return          \c 0 on success.
 * \return          #HED_ERR_CCM_BAD_INPUT on failure:
 *                  \p ctx is in an invalid state,
 *                  \p mode is invalid,
 *                  \p iv_len is invalid (lower than \c 7 or greater than
 *                  \c 13).
 */
uint32_t HED_CCMStarts(CCM_Context *ctx,
                       CCM_Context_Key *ctx_key);

/**
 * \brief           This function declares the lengths of the message
 *                  and additional data for a CCM encryption or decryption
 *                  operation.
 *
 *                  This function and HED_CCMStarts() must be called
 *                  before calling HED_CCMUpdataAd() or
 *                  HED_CCMUpdata(). This function can be called before
 *                  or after HED_CCMStarts().
 *
 * \note            This function is not implemented in Mbed TLS yet.
 *
 * \param ctx       The CCM context. This must be initialized.
 * \param total_ad_len   The total length of additional data in bytes.
 *                       This must be less than `2^16 - 2^8`.
 * \param plaintext_len  The length in bytes of the plaintext to encrypt or
 *                       result of the decryption (thus not encompassing the
 *                       additional data that are not encrypted).
 * \param tag_len   The length of the tag to generate in Bytes:
 *                  4, 6, 8, 10, 12, 14 or 16.
 *                  For CCM*, zero is also valid.
 *
 * \return          \c 0 on success.
 * \return          #HED_ERR_CCM_BAD_INPUT on failure:
 *                  \p ctx is in an invalid state,
 *                  \p total_ad_len is greater than \c 0xFF00.
 */
uint32_t HED_CCMSetLengths(CCM_Context *ctx,
                           CCM_Context_Key *ctx_key);

/**
 * \brief           This function feeds an input buffer as associated data
 *                  (authenticated but not encrypted data) in a CCM
 *                  encryption or decryption operation.
 *
 *                  You may call this function zero, one or more times
 *                  to pass successive parts of the additional data. The
 *                  lengths \p ad_len of the data parts should eventually add
 *                  up exactly to the total length of additional data
 *                  \c total_ad_len passed to HED_CCMSetLengths(). You
 *                  may not call this function after calling
 *                  HED_CCMUpdata().
 *
 * \note            This function is not implemented in Mbed TLS yet.
 *
 * \param ctx       The CCM context. This must have been started with
 *                  HED_CCMStarts(), the lengths of the message and
 *                  additional data must have been declared with
 *                  HED_CCMSetLengths() and this must not have yet
 *                  received any input with HED_CCMUpdata().
 * \param ad        The buffer holding the additional data, or \c NULL
 *                  if \p ad_len is \c 0.
 * \param ad_len    The length of the additional data. If \c 0,
 *                  \p ad may be \c NULL.
 *
 * \return          \c 0 on success.
 * \return          #HED_ERR_CCM_BAD_INPUT on failure:
 *                  \p ctx is in an invalid state,
 *                  total input length too long.
 */
uint32_t HED_CCMUpdataAd(CCM_Context *ctx,
                         CCM_Context_Key *ctx_key);

/**
 * \brief           This function feeds an input buffer into an ongoing CCM
 *                  encryption or decryption operation.
 *
 *                  You may call this function zero, one or more times
 *                  to pass successive parts of the input: the plaintext to
 *                  encrypt, or the ciphertext (not including the tag) to
 *                  decrypt. After the last part of the input, call
 *                  HED_CCMFinish(). The lengths \p input_len of the
 *                  data parts should eventually add up exactly to the
 *                  plaintext length \c plaintext_len passed to
 *                  HED_CCMSetLengths().
 *
 *                  This function may produce output in one of the following
 *                  ways:
 *                  - Immediate output: the output length is always equal
 *                    to the input length.
 *                  - Buffered output: except for the last part of input data,
 *                    the output consists of a whole number of 16-byte blocks.
 *                    If the total input length so far (not including
 *                    associated data) is 16 \* *B* + *A* with *A* < 16 then
 *                    the total output length is 16 \* *B*.
 *                    For the last part of input data, the output length is
 *                    equal to the input length plus the number of bytes (*A*)
 *                    buffered in the previous call to the function (if any).
 *                    The function uses the plaintext length
 *                    \c plaintext_len passed to HED_CCMSetLengths()
 *                    to detect the last part of input data.
 *
 *                  In particular:
 *                  - It is always correct to call this function with
 *                    \p output_size >= \p input_len + 15.
 *                  - If \p input_len is a multiple of 16 for all the calls
 *                    to this function during an operation (not necessary for
 *                    the last one) then it is correct to use \p output_size
 *                    =\p input_len.
 *
 * \note            This function is not implemented in Mbed TLS yet.
 *
 * \param ctx           The CCM context. This must have been started with
 *                      HED_CCMStarts() and the lengths of the message and
 *                      additional data must have been declared with
 *                      HED_CCMSetLengths().
 * \param input         The buffer holding the input data. If \p input_len
 *                      is greater than zero, this must be a readable buffer
 *                      of at least \p input_len bytes.
 * \param input_len     The length of the input data in bytes.
 * \param output        The buffer for the output data. If \p output_size
 *                      is greater than zero, this must be a writable buffer of
 *                      at least \p output_size bytes.
 * \param output_size   The size of the output buffer in bytes.
 *                      See the function description regarding the output size.
 * \param output_len    On success, \p *output_len contains the actual
 *                      length of the output written in \p output.
 *                      On failure, the content of \p *output_len is
 *                      unspecified.
 *
 * \return         \c 0 on success.
 * \return         #HED_ERR_CCM_BAD_INPUT on failure:
 *                 \p ctx is in an invalid state,
 *                 total input length too long,
 *                 or \p output_size too small.
 */
uint32_t HED_CCMUpdata(CCM_Context *ctx,
                       CCM_Context_Key *ctx_key);

/**
 * \brief           This function finishes the CCM operation and generates
 *                  the authentication tag.
 *
 *                  It wraps up the CCM stream, and generates the
 *                  tag. The tag can have a maximum length of 16 Bytes.
 *
 * \note            This function is not implemented in Mbed TLS yet.
 *
 * \param ctx       The CCM context. This must have been started with
 *                  HED_CCMStarts() and the lengths of the message and
 *                  additional data must have been declared with
 *                  HED_CCMSetLengths().
 * \param tag       The buffer for holding the tag. If \p tag_len is greater
 *                  than zero, this must be a writable buffer of at least \p
 *                  tag_len Bytes.
 * \param tag_len   The length of the tag. Must match the tag length passed to
 *                  HED_CCMSetLengths() function.
 *
 * \return          \c 0 on success.
 * \return          #HED_ERR_CCM_BAD_INPUT on failure:
 *                  \p ctx is in an invalid state,
 *                  invalid value of \p tag_len,
 *                  the total amount of additional data passed to
 *                  HED_CCMUpdataAd() was lower than the total length of
 *                  additional data \c total_ad_len passed to
 *                  HED_CCMSetLengths(),
 *                  the total amount of input data passed to
 *                  HED_CCMUpdata() was lower than the plaintext length
 *                  \c plaintext_len passed to HED_CCMSetLengths().
 */
uint32_t HED_CCMFinish(CCM_Context *ctx,
                       CCM_Context_Key *ctx_key);

unsigned int HED_CCMAPI_Version(void);
/************************************************************************************/
#endif//HED_CCM_H__
/**
  * @}
  */
/**
  * @}
  */
