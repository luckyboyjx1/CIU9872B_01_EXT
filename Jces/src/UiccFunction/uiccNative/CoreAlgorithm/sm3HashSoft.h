/**
 * \file sm3.h
 * SM3 standards:http://www.oscca.gov.cn/News/201012/News_1199.htm
 */
#ifndef SM3HashSoft_H
#define SM3HashSoft_H


/**
 * \brief          SM3 context structure
 */
typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[8];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */

}sm3_context;


/**
 * \brief          SM3 context setup
 *
 * \param ctx      context to be initialized
 */
extern void sm3_starts( sm3_context *ctx );

/**
 * \brief          SM3 process buffer
 *
 * \param ctx      SM3 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
extern void sm3_update( sm3_context *ctx, unsigned char *input, int ilen );
/*
*/
extern  void sm3_process( sm3_context *ctx, unsigned char vdata[64] );//data¸ÄÎªvdata
/**
 * \brief          SM3 final digest
 *
 * \param ctx      SM3 context
 */
extern void sm3_finish( sm3_context *ctx, unsigned char output[32] );

/**
 * \brief          Output = SM3( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SM3 checksum result
 */
extern void sm3( unsigned char *input, int ilen,unsigned char output[32]);


/**
 * \brief          SM3 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
extern void sm3_hmac_starts( sm3_context *ctx, unsigned char *key, int keylen);

/**
 * \brief          SM3 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
extern void sm3_hmac_update( sm3_context *ctx, unsigned char *input, int ilen );

/**
 * \brief          SM3 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SM3 HMAC checksum result
 */
extern void sm3_hmac_finish( sm3_context *ctx, unsigned char output[32] );

/**
 * \brief          Output = HMAC-SM3( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SM3 result
 */
extern void sm3_hmac( unsigned char *key, int keylen,unsigned char *input, int ilen,unsigned char output[32] );



#endif /* sm3.h */
