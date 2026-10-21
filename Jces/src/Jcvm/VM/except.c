/*****************************************************************************
 * File:  except.c
 * Version 1.9

 wang hui
 eastcompeace Technology Corporation

 History:
 2007,7,3	.modify

 ****************************************************************************/
#include "except.h"
#include "stack.h"
#include "object.h"
#include "mask.h"
#include "memory.h"
#include "context.h"
#include "anomaly.h"
#include "ob_manager.h"
#include "packageloc.h"
#include "reference.h"
#include "global_variable.h"
#include "NvmMap.h"
#include "SEEUICCInterface.h"
#include "uicc.h"// modify by jiangqingbin@20220416该头文件内部用于包括所有的与平台层无关的头文件包含，后续如果需要包含其他都文件时，在uicc.h中增加即可 

/**
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 *                          ExceptionTableList中mask中下载的包存放的是方法组建的引用；后下载的包存放的是方法组建相对于pkg_info起始地址的偏移.
 */
void handleException(jref objref, PF8 objectAddr)		SMALL_MODE
{
	u1			B_A		stopbit_caught;
	u1			B_A		StopBit;
	u16			D_A		i;
	u8			D_A		currentPackage;
	PF8			D_A		methodCompBase;
	PF8			D_A		currenttable;
	PF8			D_A		ExceptionTableList;
	PF8			D_A		exceptiontable;
	u8			D_A		exceptiontablesize;
	PF8			D_A		Start;
	u16			D_A		StartRef;
	u16			D_A		Length;
	u16			D_A		Handler;
	jref		D_A		Catch_Type_Index;
	PF8			D_A		Catch_Type_Index_Addr;
	u16			D_A		temp_index;
	PF8			D_A		exceptionEntry;
	u8			D_A		catchTypePkg;
	
	PATCH_JCVM_VM_EXCEPT_001();
	
	Catch_Type_Index_Addr	= MEMtPF8(0);
	//catchTypePkg			= 0;

	while (true)
	{
		/* Iterate through the chain of ExceptionTable Lists, until the exception is 'matched'
		 with its handler, or it is determined that there is an error */
	//	ExceptionTableList =  MapTable_GetEntityContent((u16)loadAddress(CURRENT_EXCEPTION_LIST));
		temp_index			= U32t16(JcvmConfig_GetCurrentExceptionList());
		ExceptionTableList	= MEMtPF8(PFtMEM(MapTable_GetEntityContent(temp_index)));


		/* initialize stopbit_caught to false for each time we search within a frame's context */
		stopbit_caught = false;

		while (true)
		{
			/* Point to the first ExceptionTable List to process through */
//			currenttable = loadAddress((memref)(ExceptionTableList + EL_pExceptionTable)); 
			currentPackage			= PF_R_U8(PF8_A_CONST(ExceptionTableList, EL_pkgId));

		    if(isInRom(ExceptionTableList))	  //if in mask code area
			{
				currenttable		= MEMtPF8(PF8_A_CONST(ExceptionTableList, ExceptionTableNodeSize));
			}
			else
			{
		        temp_index	= (PF_R_U16(PF8_A_U16(MEMtPF8(G_CurExecuEnvironment.pkgLocationTable + SIZE_U16), U8_M_U8(currentPackage, SIZE_MEMREF))));
		        currenttable	= MEMtPF8(MapTable_GetEntityContent(temp_index));
				currenttable 	= PF8_A_U32(currenttable,PF_R_U32(PF8_A_U8(currenttable, U8_M_U8(COMPONENT_METHOD, (SIZE_MEMREF + SIZE_U16)))));
			}



			/* The next line sets us up for iteration through the next Table
			 if we fail to find our handler in the current table */
//			ExceptionTableList = loadAddress((memref)(ExceptionTableList + EL_pNextExceptionTable));
      /*      if(((u16)loadAddress((u8 *)(ExceptionTableList + EL_pNextExceptionTable))) == 0)
			{
				ExceptionTableList	= 0;
			}
			else
			{	  */
		        temp_index			= U32t16(nvmReadU32(PF8_A_CONST(ExceptionTableList, EL_pNextExceptionTable)));
		        ExceptionTableList	= MEMtPF8(MapTable_GetEntityContent(temp_index));
	//		}

			// added by wanghui 2007.7.4 ,if the PCpkgID != currentPackage,don't need search this package
			//if this need

			if (PCpkgID != currentPackage)
			{
				if (PFtMEM(ExceptionTableList) == NULL_ExceptionList)
				{
					break;
				}
				continue;
			}

			exceptiontablesize		= PF_R_U8(PF8_A_CONST(currenttable, EL_ExceptionTableLength));
			methodCompBase			= MEMtPF8(getPackageComponentLocation(currentPackage, COMPONENT_METHOD));
			exceptiontable			= PF8_A_CONST(currenttable, EL_ExceptionTable);


			// end of modify

			/* Iterate through the table looking for the exception handler code */
			for (i = 0; i < exceptiontablesize; U16_INC(i))
			{
				exceptionEntry		= PF8_A_CONST(exceptiontable, U8_M_U16(ExceptionEntrySize, i));
				StartRef			= nvmReadU16(PF8_A_CONST(exceptionEntry, ET_start));
				Start				= MEMtPF8(PF8_A_U16(methodCompBase, StartRef));
				Length				= nvmReadU16(PF8_A_CONST(exceptionEntry, ET_length));


				/* mask off for the stopbit portion */
				StopBit				= (U16_AND_U16(Length, ExceptionTable_STOPBIT_MASK) == ExceptionTable_STOPBIT_MASK);
				/* mask off for the real length portion */
				Length				= U16_AND_U16(Length, ExceptionTable_ACTIVE_LENGTH_MASK);

				Handler				= nvmReadU16(PF8_A_CONST(exceptionEntry, ET_handler));
				Catch_Type_Index	= nvmReadU16(PF8_A_CONST(exceptionEntry, ET_class));


				/*
				 Check the exceptiontable entry's range and its 'Catch' Type index
				 to see if this is the desired 'catch' segment
				 */
				/* IMPORTANT: PC points to the NEXT instruction to execute.
				 So, (PC-1) must be within interval [Start, Start + Length[ */
				if ((PFtMEM(JPC) > PFtMEM(Start)) && (PFtMEM(JPC) <= PFtMEM(PF8_A_U16(Start, Length))))
				{
					
					if (Catch_Type_Index != 0xFFFF)
					{
						catchTypePkg			= resolveReferencePackage(Catch_Type_Index, currentPackage);
						Catch_Type_Index_Addr	= MEMtPF8(resolveReferenceAddress(Catch_Type_Index, catchTypePkg, COMPONENT_CLASS));
					}
					/* PC is within the range.  The exception is caught if
					 Catch_Type_Index == 0xFFFF or
					 if object is an instance of that class */
					if (Catch_Type_Index == 0xFFFF || isinstanceof(objectAddr, AType_Object, Catch_Type_Index_Addr))
					{
						/* Match!  We've found the 'catch' segment... */
						popOperands();
						pushReference(objref);
						JPC						= MEMtPF8(PF8_A_U16(methodCompBase, Handler));
						return;
					}
					else if (StopBit)
					{
						stopbit_caught			= true;
						break;
					}
				}
			}
			/* If no more exception table to search through, try in the caller */
			if ((PFtMEM(ExceptionTableList) == NULL_ExceptionList) || (stopbit_caught))
			{
				break;
			}
		}


		/* popFrame and restart search if FP >=0  */
		if (isStackNotEmpty())
		{
			popFrame();
		}
		else
		{
			/* uncaught exception */
			fatal_error(UNCAUGHT_EXCEPTION_ERROR);
		}
	}
}
/**
 * modifed by xxj@20210328: mask中包对象的个数不变，后下载的包在下载过程中创建的对象由5个(class_component / method_component / staticfield_component / export_component / pkg_info)减少为1个(pkg_info)
 *                          pkg_info中的信息，mask中包对象保持不变，存储的分别是4个组建引用(4字节) + 组建大小(2字节) ; 后下载的包对象发生变化，存储的分别是4个组建地址相对于pkg_info起始地址的偏移(4字节) + 组建大小(2字节)
 *                          ExceptionTableList中mask中下载的包存放的是方法组建的引用；后下载的包存放的是方法组建相对于pkg_info起始地址的偏移.
 */
void addexceptiontable(u8 pkgId)		SMALL_MODE
{
//	memref X_A table = E2P_alloc(ExceptionTableNodeSize);
	jref		D_A		pVirtualTable;
	PF8			D_A		table;
//	memref X_A methodCompLocation = getPackageComponentLocation(pkgId, COMPONENT_METHOD);
    PF8			D_A		methodCompLocation;

	pVirtualTable		= NvmManager_Allocate(ExceptionTableNodeSize, MAP_ENTRY_BLOCK_PCK_ID);
	table				= MEMtPF8(MapTable_GetEntityContent(pVirtualTable));
    methodCompLocation	= MEMtPF8(getmethodComponentBaseAddr_MapIndex(pkgId, COMPONENT_METHOD)); // mask中预置的此处存的是方法组建的引用；mask后下载的此处存的是相对于package的偏移

	//check if memory allocation was successful
	checkNullAddrAndManageTransaction(table, 0);


	//write the method component address in the new table entry
	nvmWriteU32NoAtomic(PF8_A_CONST(table, EL_pExceptionTable), PFtMEM(methodCompLocation));


	//set the pointer to the next link in the list
	nvmWriteU32NoAtomic(PF8_A_CONST(table, EL_pNextExceptionTable), JcvmConfig_GetCurrentExceptionList());
	//set package ID
	nvmWriteU8NoAtomic(PF8_A_CONST(table, EL_pkgId), pkgId);


	//set the first node, the newly created node.
//	nvmWriteU32Atomic((memref) CURRENT_EXCEPTION_LIST, table);
	JcvmConfig_SetCurrentExceptionList(U16t32(pVirtualTable));
}

void remove_exception_table(u8 pkgId)	SMALL_MODE
{
	u8			D_A		txn_in_progress;
	PF8			D_A		nextTable;
	PF8			D_A		prevTable;
	jref		D_A		vNextTable;
	jref		D_A		vCurrentExceptionTable;
	PF8			D_A		currentExceptionTable;  //(lzm modified according [MISA])
	
	PATCH_JCVM_VM_EXCEPT_003();
	
	txn_in_progress			= 0;
	vCurrentExceptionTable	= U32t16(JcvmConfig_GetCurrentExceptionList());
	currentExceptionTable	= MEMtPF8(MapTable_GetEntityContent(vCurrentExceptionTable));  //(lzm modified according [MISA])
	prevTable				= MEMtPF8(currentExceptionTable);
	vNextTable				= U32t16(nvmReadU32(PF8_A_CONST(currentExceptionTable, EL_pNextExceptionTable)));
	nextTable				= MEMtPF8(MapTable_GetEntityContent(vNextTable));


	//go through the entire table to find the exception table entry corresponding to the
	//pkgID
	while (PFtMEM(nextTable) != NULL_ExceptionList)
	{
		if (pkgId == nvmReadU8(PF8_A_CONST(currentExceptionTable, EL_pkgId)))
		{
			/* start a transaction if it hasn't already been started */
			if (isInProgress())
			{
				/* Transaction in progress ! */
				txn_in_progress = 1;
			}
			else
			{
				begin_transaction(SINGLE_APDU_PROCESS);
			}

			//if we are in the middle of the linked list
			if (PFtMEM(prevTable) != PFtMEM(currentExceptionTable))
			{
				nvmWriteU32Atomic(PF8_A_CONST(prevTable, EL_pNextExceptionTable), vNextTable);
			}
			else
			{
				//we are at the head of the table: just change the head
				JcvmConfig_SetCurrentExceptionList(U16t32(vNextTable));
			}
			//reclaim the memory used by the node
			NvmManager_Free(vCurrentExceptionTable, MAP_ENTRY_BLOCK_PCK_ID);

			if (txn_in_progress == 0)
			{
				commit_transaction();
			}
			break;
		}
		else
		{
			//advance to the next node
			prevTable				= MEMtPF8(currentExceptionTable);
			vCurrentExceptionTable	= vNextTable;
			currentExceptionTable	= MEMtPF8(nextTable);
			vNextTable = U32t16(nvmReadU32(PF8_A_CONST(currentExceptionTable, EL_pNextExceptionTable)));
			if(vNextTable != 0)
			{
				nextTable = MEMtPF8(MapTable_GetEntityContent(vNextTable));
			}
			else
			{
				nextTable = MEMtPF8(0);
			}
		}
	}
}
