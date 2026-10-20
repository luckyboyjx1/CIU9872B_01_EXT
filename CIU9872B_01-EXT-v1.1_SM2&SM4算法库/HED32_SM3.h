/**
 * @file HED32_SM3.h
 * @brief SM3算法接口头文件
 * @author Yuzhh
 * @version 1.0
 * @date 2021-05-06
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
 *  2021-05-06    | V1.0         | Firmware Team| initial version
 *
 */

#ifndef  HED32_SM3_API_INC_
#define  HED32_SM3_API_INC_

#define SM3_SUCCESS                 0x0            /*  */
#define SM3_ERROR                   0x1            /*  */

#define SM3_IV_BUF_WLEN             9
/**
 * @addtogroup HASH_module
 * @{ */

/**
 * @defgroup SM3_module SM3算法
 * @par 概述
 *      -# 用于SM3 HASH运算
 * @par 功能
 *      -# 哈希运算中用到的参数和数据，均为大端模式存储
 *      -# 算法使用流程：使用算法时必须按照如下顺序进行函数的调用：运算初始化-> [Hash运算非最后一块]->Hash运算最后一块
 * @attention
 *      -# 非最后一块运算的数据长度必须是64字节，最后一块数据长度为0-63字节。最后一块运算时进行message padding操作
 *      -# 不支持数据总长度超过4G Bytes的运算
 * @{ */

/* ********************************************************/
/**
 * @brief  运算初始化
 * @note
 * @arg 函数将参数V指向地址的36字节长的数据进行初始化，用于后续哈希运算，前32字节是哈希计算初始值，后4字节用于记录message总长度，
 * @arg 不支持数据总长度超过4G Bytes的运算
 * @param[out] V  长度为9的无符号32位整型数组（36字节），哈希计算初始值数据起始地址
 */
/* ********************************************************/
void HED_SM3Initial (unsigned int *V);

/* ********************************************************/
/**
 * @brief  块运算
 * @note
 * @arg 第一次调用此函数前应执行SHA-256运算初始化
 * @arg 当参与Hash运算的数据超过64个字节时，需要将数据按照64字节长度截取为多个数据块，最后一个数据块可以不足64个字节，然后顺序将每个数据块作为输入调用本函数进行运算
 * @arg 如果用户的数据长度为64字节的整数倍，则最后需要以数据长度为零，调用一次该函数，才能获得最后的哈希值
 * @arg 只有最后一块的长度才可以为0-63，非最后一块的长度必须为64字节
 * @arg 不支持数据总长度超过4G Bytes的运算
 * @param [in, out] V         输入前一轮运算结果，输出本轮运算结果，总长度36字节
 * @param [in] DataValue      输入数据的指针
 * @param DataLen             输入数据长度，取值范围0-64
 * @return
 *        - 0                运算完成
 *        - 1                输入数据长度不正确
 */
/* ********************************************************/
unsigned int HED_SM3Block( unsigned int *V, const unsigned int *DataValue, unsigned int DataLen);

/**  @} */

/**  @} */

#endif   /* ----- #ifndef HED32_SM3_API_INC_  ----- */
