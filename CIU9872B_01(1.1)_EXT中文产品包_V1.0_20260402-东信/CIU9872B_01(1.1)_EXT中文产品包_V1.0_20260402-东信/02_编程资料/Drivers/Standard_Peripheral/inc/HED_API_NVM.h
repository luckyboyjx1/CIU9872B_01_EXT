/*****************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.

 File name: HED_API_NVM.h
 Author:   Yangnd
 Version:
 Date:
 Description:  SE FTL  API  and NVM_Swtich API¡£
 History:   2019.3.18   First version
            2021.1.8    Add HED_HEFlash_WriteNWordAddr
******************************************************/

#ifndef __HED_API_NVM_H_
#define __HED_API_NVM_H_

// for  HED User
uint32_t HED_NVMAPI_Version(void);
uint32_t HED_HEFlash_Init(void);
uint32_t HED_HEFlash_EWPage(uint32_t FlashAddr, uint8_t *DataBuf, uint32_t len);
uint32_t HED_CommonFlash_EWPage(uint32_t FlashAddr, uint32_t *DataBuf);
uint32_t HED_CommonFlash_ErasePage(uint32_t FlashAddr, uint32_t FlashAddrINV);
uint32_t HED_CommonFlash_WriteNWord(uint32_t FlashAddr, uint32_t *DataBuf, uint32_t wordLen);
uint32_t HED_UserPara_EWPage(uint32_t FlashAddr, uint32_t *DataBuf);
uint32_t HED_HEFlash_ErasePage(uint32_t FlashAddr, uint32_t FlashAddrINV);
uint32_t HED_HEFlash_WriteWord(uint32_t FlashAddr, uint32_t value, uint32_t checkWord);
uint32_t HED_HEFlash_WriteNWord(uint32_t FlashAddr, uint32_t *DataBuf, uint32_t wordLen);

/*-----------------------------------------------------------------------------
 * define of API entry
 *-----------------------------------------------------------------------------*/
#define API_ENTRY                                             0x1FF40000
#define HED_NVMAPI_VersionAddr                                (API_ENTRY + 0x10*0)
#define HED_HEFlash_InitAddr                                  (API_ENTRY + 0x10*1)
#define HED_HEFlash_EWPageAddr                                (API_ENTRY + 0x10*2)
#define HED_CommonFlash_EWPageAddr                            (API_ENTRY + 0x10*3)
#define HED_CommonFlash_ErasePageAddr                         (API_ENTRY + 0x10*4)
#define HED_CommonFlash_WriteNWordAddr                        (API_ENTRY + 0x10*5)
#define HED_UserPara_EWPageAddr                               (API_ENTRY + 0x10*6)
#define HED_HEFlash_ErasePageAddr                             (API_ENTRY + 0x10*10)
#define HED_HEFlash_WriteWordAddr                             (API_ENTRY + 0x10*11)
#define HED_HEFlash_WriteNWordAddr                            (API_ENTRY + 0x10*12)

#define HED_NVMAPI_Version                                    (*((uint32_t (*)(void))HED_NVMAPI_VersionAddr))
#define HED_HEFlash_Init                                      (*((uint32_t (*)(void))HED_HEFlash_InitAddr))
#define HED_HEFlash_EWPage(FlashAddr,DataBuf,len)             (*((uint32_t (*)(uint32_t, uint8_t *, uint32_t))HED_HEFlash_EWPageAddr))(FlashAddr,DataBuf,len)
#define HED_CommonFlash_EWPage(FlashAddr,DataBuf)             (*((uint32_t (*)(uint32_t, uint32_t *))HED_CommonFlash_EWPageAddr))(FlashAddr,DataBuf)
#define HED_CommonFlash_ErasePage(FlashAddr,FlashAddrINV)     (*((uint32_t (*)(uint32_t, uint32_t))HED_CommonFlash_ErasePageAddr))(FlashAddr,FlashAddrINV)
#define HED_CommonFlash_WriteNWord(FlashAddr,DataBuf,wordLen) (*((uint32_t (*)(uint32_t, uint32_t *, uint32_t))HED_CommonFlash_WriteNWordAddr))(FlashAddr,DataBuf,wordLen)
#define HED_UserPara_EWPage(FlashAddr,DataBuf)                (*((uint32_t (*)(uint32_t, uint32_t *))HED_UserPara_EWPageAddr))(FlashAddr,DataBuf)
#define HED_HEFlash_ErasePage(FlashAddr,FlashAddrINV)         (*((uint32_t (*)(uint32_t, uint32_t))HED_HEFlash_ErasePageAddr))(FlashAddr,FlashAddrINV)
#define HED_HEFlash_WriteWord(FlashAddr,value,checkWord)      (*((uint32_t (*)(uint32_t, uint32_t, uint32_t))HED_HEFlash_WriteWordAddr))(FlashAddr,value,checkWord)
#define HED_HEFlash_WriteNWord(FlashAddr,DataBuf,wordLen)     (*((uint32_t (*)(uint32_t, uint32_t *, uint32_t))HED_HEFlash_WriteNWordAddr))(FlashAddr,DataBuf,wordLen)


#endif
