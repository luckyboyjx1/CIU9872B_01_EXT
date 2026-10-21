/*

 *
 *  Created on: 2014Äê9ÔÂ29ÈÕ
 *      Author: jiangxiaohui
 */

#ifndef CRYPTOPROFILE_H_
#define CRYPTOPROFILE_H_

#include "profile.h"
#include "appHeader.h"

#if	JCVM__CRYPTO__DES_SINGLE_MAC		
#define		WITH_DES_SINGLE_MAC					  1
#else
#define		WITH_DES_SINGLE_MAC					  0
#endif

#if	JCVM__CRYPTO__DES_TRIPLE_CIPHER		
#define		WITH_DES_TRIPLE_CIPHER				      1
#else
#define		WITH_DES_TRIPLE_CIPHER					  0
#endif

#if	JCVM__CRYPTO__DES_SINGLE_CIPHER		
#define		WITH_DES_SINGLE_CIPHER					  1
#else
#define		WITH_DES_SINGLE_CIPHER					  0
#endif

#if LTE__CRYPTO__SYM_SIMPLEDES__FOR_C
		#define  WITH_ONLY_SIMPLEDES			   1
#else
		#define  WITH_ONLY_SIMPLEDES			   0
#endif
/*****Des Control Start******/
#if VM_WITH_DES
	#define  WITH_DES_PRIVATED    0
#elif ((LTE__CRYPTO__DES_CIPHER__FOR_JAVA || LTE__CRYPTO__DES_MAC__FOR_JAVA))
	#define  WITH_DES_PRIVATED    1
#else
	#define  WITH_DES_PRIVATED    0
#endif


/******Des Control End*******/

/******Aes Control Start*******/
#if VM_WITH_AES
	#define  WITH_AES_PRIVATED  0
#elif ((LTE__CRYPTO__AES_CIPHER__FOR_JAVA || LTE__CRYPTO__AES_CMAC__FOR_JAVA))
	#define  WITH_AES    		 0
	#define  WITH_AES_PRIVATED  1
#else
	#define  WITH_AES    		 0
	#define  WITH_AES_PRIVATED  0
#endif

#if LTE__CRYPTO__AES_CIPHER_SIMPLE__FOR_C
#define  WITH_AES_PRIVATED_AUTH       1
#else
#define  WITH_AES_PRIVATED_AUTH       0
#endif


#if (LTE__CRYPTO__GENERALHMAC_MD5__FOR_JAVA || LTE__CRYPTO__GENERALHMAC_SHA__FOR_JAVA || LTE__CRYPTO__GENERALHMAC_SHA__FOR_C || LTE__CRYPTO__GENERALHMAC_SHA256__FOR_C)
	#define WITH_HMAC_GENERALHMAC     1
#else
	#define WITH_HMAC_GENERALHMAC     0
#endif


#if LTE__CRYPTO__HASH_SHA_PADDING0__FOR_C
	#define WITH_HASH_DOFINAL_SHA    1
#else
	#define WITH_HASH_DOFINAL_SHA    0
#endif

#endif





