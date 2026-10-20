##
## Auto Generated makefile by CDK
## Do not modify this file, and any manual changes will be erased!!!   
##
## BuildSet
ProjectName            :=CIU9872B_01_V1.1_7816_DemoCOS_Base
ConfigurationName      :=BuildSet
WorkspacePath          :=./
ProjectPath            :=./
IntermediateDirectory  :=Obj
OutDir                 :=$(IntermediateDirectory)
User                   :=zenghao
Date                   :=19/03/2026
CDKPath                :=D:/C-Sky/CDK
ToolchainPath          :=D:/C-Sky/CDKRepo/Toolchain/CKV2ElfMinilib/V3.10.29/R/
LinkerName             :=csky-elfabiv2-gcc
LinkerNameoption       :=
SIZE                   :=csky-elfabiv2-size
READELF                :=csky-elfabiv2-readelf
CHECKSUM               :=crc32
SharedObjectLinkerName :=
ObjectSuffix           :=.o
DependSuffix           :=.d
PreprocessSuffix       :=.i
DisassemSuffix         :=.asm
IHexSuffix             :=.ihex
BinSuffix              :=.bin
ExeSuffix              :=.elf
LibSuffix              :=.a
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
ElfInfoSwitch          :=-hlS
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
UnPreprocessorSwitch   :=-U
SourceSwitch           :=-c 
ObjdumpSwitch          :=-S
ObjcopySwitch          :=-O ihex
ObjcopyBinSwitch       :=-O binary
OutputFile             :=$(ProjectName)
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
PreprocessOnlyDisableLineSwitch   :=-P
ObjectsFileList        :=CIU9872B_01_V1.1_7816_DemoCOS_Base.txt
MakeDirCommand         :=mkdir
LinkOptions            := -mcpu=e802   -mbig-endian    -nostartfiles -Wl,--gc-sections -T$(ProjectPath)/Obj/CIU9872B_01_V1.1_7816_DemoCOS_Base.ld
LinkOtherFlagsOption   := -Wl,--ckmap=$(ProjectPath)/Lst/$(OutputFile).map 
IncludePackagePath     :=
IncludeCPath           := $(IncludeSwitch). $(IncludeSwitch)../../../../Drivers/Standard_Peripheral/inc $(IncludeSwitch)../../../../Drivers/Standard_Peripheral/inc/cdkinc $(IncludeSwitch)../../../../Middlewares/7816/inc $(IncludeSwitch)../../../../Middlewares/rf/inc $(IncludeSwitch)../../user/inc $(IncludeSwitch)../../../../Drivers/Standard_Peripheral/inc/ $(IncludeSwitch)../../../../Drivers/Standard_Peripheral/inc/cdkinc/ $(IncludeSwitch)../../../../Middlewares/rf/inc/ $(IncludeSwitch)../../user/inc/ $(IncludeSwitch)../../../../Middlewares/7816/inc/  
IncludeAPath           := $(IncludeSwitch). $(IncludeSwitch)../../../../Drivers/Standard_Peripheral/inc $(IncludeSwitch)../../../../Drivers/Standard_Peripheral/inc/cdkinc $(IncludeSwitch)../../../../Middlewares/7816/inc $(IncludeSwitch)../../../../Middlewares/rf/inc $(IncludeSwitch)../../user/inc  
Libs                   :=
ArLibs                 := 
PackagesLibPath        :=
LibPath                := $(PackagesLibPath) 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       :=csky-elfabiv2-ar rcu
CXX      :=csky-elfabiv2-g++
CC       :=csky-elfabiv2-gcc
AS       :=csky-elfabiv2-gcc
OBJDUMP  :=csky-elfabiv2-objdump
OBJCOPY  :=csky-elfabiv2-objcopy
CXXFLAGS := -mcpu=e802  -melrw  -mbig-endian   $(PreprocessorSwitch)KIT_DEBUG   -O0  -g3  -Wall  
CFLAGS   := -mcpu=e802  -melrw  -mbig-endian   $(PreprocessorSwitch)KIT_DEBUG   -O0  -g3  -Wall  
ASFLAGS  := -mcpu=e802  -melrw  -mbig-endian     -Wa,-gdwarf-2    
PreprocessFlags  := -mcpu=e802  -melrw  -mbig-endian   $(PreprocessorSwitch)KIT_DEBUG   -O0     -Wall  


Objects0=$(IntermediateDirectory)/crt0$(ObjectSuffix) $(IntermediateDirectory)/chip_name$(ObjectSuffix) $(IntermediateDirectory)/app_APDU_Command$(ObjectSuffix) $(IntermediateDirectory)/app_Protocol_Process$(ObjectSuffix) $(IntermediateDirectory)/app_APDU_ECC_ALG$(ObjectSuffix) $(IntermediateDirectory)/app_APDU_NVM$(ObjectSuffix) $(IntermediateDirectory)/core_Global_Var$(ObjectSuffix) $(IntermediateDirectory)/core_main$(ObjectSuffix) $(IntermediateDirectory)/core_PowerDownRestore$(ObjectSuffix) $(IntermediateDirectory)/core_SubFunc$(ObjectSuffix) \
	$(IntermediateDirectory)/core_Interrupt$(ObjectSuffix) $(IntermediateDirectory)/target_SCI_Config$(ObjectSuffix) $(IntermediateDirectory)/src_SCI_Protocol$(ObjectSuffix) $(IntermediateDirectory)/src_hal_ckmu$(ObjectSuffix) $(IntermediateDirectory)/src_hal_crc$(ObjectSuffix) $(IntermediateDirectory)/src_hal_dma$(ObjectSuffix) $(IntermediateDirectory)/src_hal_factorycode$(ObjectSuffix) $(IntermediateDirectory)/src_hal_gpio$(ObjectSuffix) $(IntermediateDirectory)/src_hal_io$(ObjectSuffix) $(IntermediateDirectory)/src_hal_pmu$(ObjectSuffix) \
	$(IntermediateDirectory)/src_hal_sci$(ObjectSuffix) $(IntermediateDirectory)/src_hal_timer$(ObjectSuffix) $(IntermediateDirectory)/src_hal_ramInit$(ObjectSuffix) $(IntermediateDirectory)/__rt_entry$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all
all: $(IntermediateDirectory)/$(OutputFile)

$(IntermediateDirectory)/$(OutputFile):  $(Objects) Always_Link 
	$(LinkerName) $(OutputSwitch) $(IntermediateDirectory)/$(OutputFile)$(ExeSuffix) $(LinkerNameoption) -Wl,--ckmap=$(ProjectPath)/Lst/$(OutputFile).map  @$(ObjectsFileList) $(LibraryPathSwitch)F:/SMIC40F79/Drivers/Standard_Peripheral/src $(LibraryPathSwitch)F:/SMIC40F79/Drivers/Standard_Peripheral/src $(LibraryPathSwitch)F:/SMIC40F79/Drivers/Standard_Peripheral/src $(LibraryPathSwitch)F:/SMIC40F79/Drivers/Standard_Peripheral/src $(LibraryPathSwitch)F:/SMIC40F79/Drivers/Standard_Peripheral/src $(LibraryPathSwitch)F:/SMIC40F79/Drivers/Standard_Peripheral/src  -Wl,--whole-archive $(LibrarySwitch)HED_API_ChipID  $(LibrarySwitch)HED_CCM  $(LibrarySwitch)HED_GCM  $(LibrarySwitch)CIU9872B_01_V1.1_AES  $(LibrarySwitch)CIU9872B_01_V1.1_API_NVM_Switch  $(LibrarySwitch)CIU9872B_01_V1.1_API_RNG_EXT   -Wl,--no-whole-archive  $(LinkOptions) $(LibPath) $(Libs) $(LinkOtherFlagsOption)
	-@mv $(ProjectPath)/Lst/$(OutputFile).map $(ProjectPath)/Lst/$(OutputFile).temp && $(READELF) $(ElfInfoSwitch) $(ProjectPath)/Obj/$(OutputFile)$(ExeSuffix) > $(ProjectPath)/Lst/$(OutputFile).map && echo ====================================================================== >> $(ProjectPath)/Lst/$(OutputFile).map && cat $(ProjectPath)/Lst/$(OutputFile).temp >> $(ProjectPath)/Lst/$(OutputFile).map && rm -rf $(ProjectPath)/Lst/$(OutputFile).temp
	$(OBJCOPY) $(ObjcopySwitch) $(ProjectPath)/$(IntermediateDirectory)/$(OutputFile)$(ExeSuffix)  $(ProjectPath)/Obj/$(OutputFile)$(IHexSuffix) 
	$(OBJDUMP) $(ObjdumpSwitch) $(ProjectPath)/$(IntermediateDirectory)/$(OutputFile)$(ExeSuffix)  > $(ProjectPath)/Lst/$(OutputFile)$(DisassemSuffix) 
	@echo size of target:
	@$(SIZE) $(ProjectPath)$(IntermediateDirectory)/$(OutputFile)$(ExeSuffix) 
	@echo -n checksum value of target:  
	@$(CHECKSUM) $(ProjectPath)/$(IntermediateDirectory)/$(OutputFile)$(ExeSuffix) 
	@CIU9872B_01_V1.1_7816_DemoCOS_Base.modify.bat $(IntermediateDirectory) $(OutputFile)$(ExeSuffix) 

Always_Link:


##
## Objects
##
$(IntermediateDirectory)/crt0$(ObjectSuffix): crt0.S  
	$(AS) $(SourceSwitch) crt0.S $(ASFLAGS) -MMD -MP -MT$(IntermediateDirectory)/crt0$(ObjectSuffix) -MF$(IntermediateDirectory)/crt0$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/crt0$(ObjectSuffix) $(IncludeAPath) $(IncludePackagePath)
Lst/crt0$(PreprocessSuffix): crt0.S
	$(CC) $(CFLAGS)$(IncludeAPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/crt0$(PreprocessSuffix) crt0.S

$(IntermediateDirectory)/chip_name$(ObjectSuffix): chip_name.c  
	$(CC) $(SourceSwitch) chip_name.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/chip_name$(ObjectSuffix) -MF$(IntermediateDirectory)/chip_name$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/chip_name$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/chip_name$(PreprocessSuffix): chip_name.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/chip_name$(PreprocessSuffix) chip_name.c

$(IntermediateDirectory)/app_APDU_Command$(ObjectSuffix): ../../user/src/app/APDU_Command.c  
	$(CC) $(SourceSwitch) ../../user/src/app/APDU_Command.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/app_APDU_Command$(ObjectSuffix) -MF$(IntermediateDirectory)/app_APDU_Command$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/app_APDU_Command$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/app_APDU_Command$(PreprocessSuffix): ../../user/src/app/APDU_Command.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/app_APDU_Command$(PreprocessSuffix) ../../user/src/app/APDU_Command.c

$(IntermediateDirectory)/app_Protocol_Process$(ObjectSuffix): ../../user/src/app/Protocol_Process.c  
	$(CC) $(SourceSwitch) ../../user/src/app/Protocol_Process.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/app_Protocol_Process$(ObjectSuffix) -MF$(IntermediateDirectory)/app_Protocol_Process$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/app_Protocol_Process$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/app_Protocol_Process$(PreprocessSuffix): ../../user/src/app/Protocol_Process.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/app_Protocol_Process$(PreprocessSuffix) ../../user/src/app/Protocol_Process.c

$(IntermediateDirectory)/app_APDU_ECC_ALG$(ObjectSuffix): ../../user/src/app/APDU_ECC_ALG.c  
	$(CC) $(SourceSwitch) ../../user/src/app/APDU_ECC_ALG.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/app_APDU_ECC_ALG$(ObjectSuffix) -MF$(IntermediateDirectory)/app_APDU_ECC_ALG$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/app_APDU_ECC_ALG$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/app_APDU_ECC_ALG$(PreprocessSuffix): ../../user/src/app/APDU_ECC_ALG.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/app_APDU_ECC_ALG$(PreprocessSuffix) ../../user/src/app/APDU_ECC_ALG.c

$(IntermediateDirectory)/app_APDU_NVM$(ObjectSuffix): ../../user/src/app/APDU_NVM.c  
	$(CC) $(SourceSwitch) ../../user/src/app/APDU_NVM.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/app_APDU_NVM$(ObjectSuffix) -MF$(IntermediateDirectory)/app_APDU_NVM$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/app_APDU_NVM$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/app_APDU_NVM$(PreprocessSuffix): ../../user/src/app/APDU_NVM.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/app_APDU_NVM$(PreprocessSuffix) ../../user/src/app/APDU_NVM.c

$(IntermediateDirectory)/core_Global_Var$(ObjectSuffix): ../../user/src/core/Global_Var.c  
	$(CC) $(SourceSwitch) ../../user/src/core/Global_Var.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/core_Global_Var$(ObjectSuffix) -MF$(IntermediateDirectory)/core_Global_Var$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/core_Global_Var$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/core_Global_Var$(PreprocessSuffix): ../../user/src/core/Global_Var.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/core_Global_Var$(PreprocessSuffix) ../../user/src/core/Global_Var.c

$(IntermediateDirectory)/core_main$(ObjectSuffix): ../../user/src/core/main.c  
	$(CC) $(SourceSwitch) ../../user/src/core/main.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/core_main$(ObjectSuffix) -MF$(IntermediateDirectory)/core_main$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/core_main$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/core_main$(PreprocessSuffix): ../../user/src/core/main.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/core_main$(PreprocessSuffix) ../../user/src/core/main.c

$(IntermediateDirectory)/core_PowerDownRestore$(ObjectSuffix): ../../user/src/core/PowerDownRestore.c  
	$(CC) $(SourceSwitch) ../../user/src/core/PowerDownRestore.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/core_PowerDownRestore$(ObjectSuffix) -MF$(IntermediateDirectory)/core_PowerDownRestore$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/core_PowerDownRestore$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/core_PowerDownRestore$(PreprocessSuffix): ../../user/src/core/PowerDownRestore.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/core_PowerDownRestore$(PreprocessSuffix) ../../user/src/core/PowerDownRestore.c

$(IntermediateDirectory)/core_SubFunc$(ObjectSuffix): ../../user/src/core/SubFunc.c  
	$(CC) $(SourceSwitch) ../../user/src/core/SubFunc.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/core_SubFunc$(ObjectSuffix) -MF$(IntermediateDirectory)/core_SubFunc$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/core_SubFunc$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/core_SubFunc$(PreprocessSuffix): ../../user/src/core/SubFunc.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/core_SubFunc$(PreprocessSuffix) ../../user/src/core/SubFunc.c

$(IntermediateDirectory)/core_Interrupt$(ObjectSuffix): ../../user/src/core/Interrupt.c  
	$(CC) $(SourceSwitch) ../../user/src/core/Interrupt.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/core_Interrupt$(ObjectSuffix) -MF$(IntermediateDirectory)/core_Interrupt$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/core_Interrupt$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/core_Interrupt$(PreprocessSuffix): ../../user/src/core/Interrupt.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/core_Interrupt$(PreprocessSuffix) ../../user/src/core/Interrupt.c

$(IntermediateDirectory)/target_SCI_Config$(ObjectSuffix): ../../user/src/target/SCI_Config.c  
	$(CC) $(SourceSwitch) ../../user/src/target/SCI_Config.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/target_SCI_Config$(ObjectSuffix) -MF$(IntermediateDirectory)/target_SCI_Config$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/target_SCI_Config$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/target_SCI_Config$(PreprocessSuffix): ../../user/src/target/SCI_Config.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/target_SCI_Config$(PreprocessSuffix) ../../user/src/target/SCI_Config.c

$(IntermediateDirectory)/src_SCI_Protocol$(ObjectSuffix): ../../../../Middlewares/7816/src/SCI_Protocol.c  
	$(CC) $(SourceSwitch) ../../../../Middlewares/7816/src/SCI_Protocol.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_SCI_Protocol$(ObjectSuffix) -MF$(IntermediateDirectory)/src_SCI_Protocol$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_SCI_Protocol$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_SCI_Protocol$(PreprocessSuffix): ../../../../Middlewares/7816/src/SCI_Protocol.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_SCI_Protocol$(PreprocessSuffix) ../../../../Middlewares/7816/src/SCI_Protocol.c

$(IntermediateDirectory)/src_hal_ckmu$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_ckmu.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_ckmu.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_ckmu$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_ckmu$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_ckmu$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_ckmu$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_ckmu.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_ckmu$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_ckmu.c

$(IntermediateDirectory)/src_hal_crc$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_crc.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_crc.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_crc$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_crc$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_crc$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_crc$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_crc.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_crc$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_crc.c

$(IntermediateDirectory)/src_hal_dma$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_dma.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_dma.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_dma$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_dma$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_dma$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_dma$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_dma.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_dma$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_dma.c

$(IntermediateDirectory)/src_hal_factorycode$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_factorycode.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_factorycode.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_factorycode$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_factorycode$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_factorycode$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_factorycode$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_factorycode.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_factorycode$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_factorycode.c

$(IntermediateDirectory)/src_hal_gpio$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_gpio.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_gpio.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_gpio$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_gpio$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_gpio$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_gpio$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_gpio.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_gpio$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_gpio.c

$(IntermediateDirectory)/src_hal_io$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_io.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_io.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_io$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_io$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_io$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_io$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_io.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_io$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_io.c

$(IntermediateDirectory)/src_hal_pmu$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_pmu.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_pmu.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_pmu$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_pmu$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_pmu$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_pmu$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_pmu.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_pmu$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_pmu.c

$(IntermediateDirectory)/src_hal_sci$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_sci.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_sci.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_sci$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_sci$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_sci$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_sci$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_sci.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_sci$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_sci.c

$(IntermediateDirectory)/src_hal_timer$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_timer.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_timer.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_timer$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_timer$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_timer$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_timer$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_timer.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_timer$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_timer.c

$(IntermediateDirectory)/src_hal_ramInit$(ObjectSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_ramInit.c  
	$(CC) $(SourceSwitch) ../../../../Drivers/Standard_Peripheral/src/hal_ramInit.c $(CFLAGS) -MMD -MP -MT$(IntermediateDirectory)/src_hal_ramInit$(ObjectSuffix) -MF$(IntermediateDirectory)/src_hal_ramInit$(DependSuffix) $(ObjectSwitch)$(IntermediateDirectory)/src_hal_ramInit$(ObjectSuffix) $(IncludeCPath) $(IncludePackagePath)
Lst/src_hal_ramInit$(PreprocessSuffix): ../../../../Drivers/Standard_Peripheral/src/hal_ramInit.c
	$(CC) $(CFLAGS)$(IncludeCPath) $(PreprocessOnlySwitch) $(OutputSwitch) Lst/src_hal_ramInit$(PreprocessSuffix) ../../../../Drivers/Standard_Peripheral/src/hal_ramInit.c


$(IntermediateDirectory)/__rt_entry$(ObjectSuffix): $(IntermediateDirectory)/__rt_entry$(DependSuffix)
	@$(AS) $(SourceSwitch) $(ProjectPath)/$(IntermediateDirectory)/__rt_entry.S $(ASFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/__rt_entry$(ObjectSuffix) $(IncludeAPath)
$(IntermediateDirectory)/__rt_entry$(DependSuffix):
	@$(CC) $(CFLAGS) $(IncludeAPath) -MG -MP -MT$(IntermediateDirectory)/__rt_entry$(ObjectSuffix) -MF$(IntermediateDirectory)/__rt_entry$(DependSuffix) -MM $(ProjectPath)/$(IntermediateDirectory)/__rt_entry.S

-include $(IntermediateDirectory)/*$(DependSuffix)
