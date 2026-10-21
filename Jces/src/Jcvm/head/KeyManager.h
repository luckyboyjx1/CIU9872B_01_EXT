#ifndef  __KEY_MANAGER_H__
#define  __KEY_MANAGER_H__

#include "profile.h"
#include "DataOperation.h"

#if GP__KEY_MANAGER

#define KEY_VERSION_INIT_VALUE                         0xFF

	extern void initIsdKey(PN8 sdDataObj);
	extern u16 PutKeys(PN8 sdDataObj);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数
	extern void deleteKey(PN8 sdDataObj);
	extern u16 getKeyNM(u16 appId,u16 keyVersion,u16 keyIndex, PM8 keyBuffer);
#else
	#define initIsdKey(sdDataObj)
	#define deleteKey(sdDataObj)
	#if GP__VERSION__FOR_MINI_MASK
		extern u16 PutKeys(PN8 sdDataObj);//merged by xuxiaojun@20220519 from EPX\v3.0\DEVELOP: 去掉未使用的参数
		extern u16 getKeyNM(u16 appId,u16 keyVersion,u16 keyIndex, PM8 keyBuffer);
	#else
		#define getKeyNM(appId, keyVersion, keyIndex, keyBuffer)					0
		#define PutKeys(sdDataObj)						             				0
	#endif
#endif

#endif
