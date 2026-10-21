/************************ Copyright (c) 2021-2022, All rights reserved.   ******************
 * @file                : mh_eccpoint.h
 * @author              :
 * @version             : V2.1
 * @date                : 2021-12-10
 * @brief               : This file contains all the definition for the ecc algorithm
 *********************************************************************************************************/

#ifndef __MH_ECCPOINT_H
#define __MH_ECCPOINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mh_bignum.h"

#define     MH_ECC192_SPACE_LEN     (156)
#define     MH_ECC224_SPACE_LEN     (168)
#define     MH_ECC256_SPACE_LEN     (180)
#define     MH_ECC384_SPACE_LEN     (228)
#define     MH_ECC521_SPACE_LEN     (282)

typedef struct
{
    uint16_t u16BitLen;			  
    uint8_t *pu8A;                
    uint8_t *pu8B;                                           
    uint8_t *pu8Gx;               
    uint8_t *pu8Gy;                                               
    uint8_t *pu8N;                                              
    uint8_t *pu8P;               
    uint8_t *pu8Space;            //for inner space
    uint32_t u32SpaceLen;         //inner space length
    uint32_t u32Crc;                            
} MHECC_PARAM;   
    
  
typedef struct
{
    uint16_t u16Bits;
    uint8_t *pu8PubX;
    uint8_t *pu8PubY;   
    uint8_t *pu8D;
    uint32_t u32Crc;
} MHECC_KEY;

typedef struct
{
    uint8_t *pu8X;
    uint8_t *pu8Y;
}MHECC_POINT;

typedef struct
{
    MHECC_POINT *pPointR;
    MHECC_POINT *pPointA;
    MHBN_BIGNUM *pK;
    uint32_t u32Crc;
}MHECC_POINT_MUL_CALL;

typedef struct
{
    MHECC_POINT *pPointR;
    MHECC_POINT *pPointA;
    MHECC_POINT *pPointB;
    uint32_t u32Crc;
}MHECC_POINT_ADD_CALL;

#define MH_RET_ECCPOINT_SUCCESS                   (0x53C2DAAA)
#define MH_RET_ECCPOINT_INIT                      (MH_RET_ECCPOINT_SUCCESS - 0x100)  
#define MH_RET_ECCPOINT_PARAM_ERR                 (MH_RET_ECCPOINT_INIT + 0x01)
#define MH_RET_ECCPOINT_DFA_LOCK                  (MH_RET_ECCPOINT_INIT + 0x02)
#define MH_RET_ECCPOINT_TRNG_ERR                  (MH_RET_ECCPOINT_INIT + 0x03)
#define MH_RET_ECCPOINT_ERR                       (MH_RET_ECCPOINT_INIT + 0x04)
#define MH_RET_ECCPOINT_VERIFY_FAILURE            (MH_RET_ECCPOINT_INIT + 0x05)

uint32_t MHECC_ParamInit(MHECC_PARAM *pParam);

uint32_t MHECC_PointMul(MHECC_POINT_MUL_CALL *pMulCall, MHECC_PARAM *pParam);

uint32_t MHECC_PointAdd(MHECC_POINT_ADD_CALL *pAddCall, MHECC_PARAM *pParam);

#ifdef __cplusplus
}
#endif

#endif
