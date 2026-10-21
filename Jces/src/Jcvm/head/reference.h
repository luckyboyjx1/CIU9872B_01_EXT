/* Copyright ?2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * Patents pending.
 */

#ifndef   __REFERENCE_H__
#define   __REFERENCE_H__

#include "grt.h"
#include "ob_manager.h"
#include "memory.h"

extern	memref  resolveReferenceAddress(jref referenceID, u8 pkgID, u8 comp_type);
extern	u8 		resolveReferencePackage(jref referenceID, u8 pkgID);

#endif /* __REFERENCE_H__ */

