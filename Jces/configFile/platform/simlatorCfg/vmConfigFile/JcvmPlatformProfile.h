#ifndef __JCVM_PLATFORM_PROFILE_H_
#define __JCVM_PLATFORM_PROFILE_H_

#ifndef AUTO_MAKE
	#define JCVM__SIMULATOR__JAVA_DEBUG			1
#else
	#define JCVM__SIMULATOR__JAVA_DEBUG			0
#endif

#define JCVM__CONFIG_AREA						1
#define JCVM__MIFARE							0

#define JCVM__PLATFORM__DEBUG_JCADT				0
#endif
