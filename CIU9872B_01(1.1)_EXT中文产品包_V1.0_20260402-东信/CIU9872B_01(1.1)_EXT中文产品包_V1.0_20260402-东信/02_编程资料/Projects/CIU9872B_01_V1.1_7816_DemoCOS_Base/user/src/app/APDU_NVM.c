
/*****************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.

 File name:     DEMO_NVM.c
 Author:
 Version:       V1.0
 Date:          2018-12-01
 Description:
 History:
                V1.0    2018-12-01  initial version
******************************************************/
#include  "declare.h"

/*************************************************
  Function:      cmd_HEFLASH_Operation
  Description:
  Input:
  Output:
  Return:
  Others:
*************************************************/
//#define PowerDown
void cmd_HEFLASH_Operation(void)
{
#define G_SW1SW2   SW1SW2
#define HEFLASH_FAIL            (uint32_t)0xFFFFFFFF
#define HEFLASH_FAILALREADY     (uint32_t)0xFFBBAADD
#define HEFLASH_CONDITIONERR    (uint32_t)0xFFFF55AA
#define PAGESIZE                (uint32_t)0x200
#define HEFLASH_TEST_ADDR       (uint32_t)0x7D000
#define CommonFLASH_TEST_ADDR   (uint32_t)(0x50000 - 0x200)
//#define SECTORSIZE              (102*1024)

    uint32_t rand[512 / 4];
    uint32_t buf[PAGESIZE / 4];
    uint32_t ret;
    uint32_t objValue, checkWord;
    uint32_t flashAddr;

    /* 1. get rand */
    HED_GetRandom((uint8_t *)rand, 512, &G_paraCheckSum);
    /* 2. show-how:write one byte to CommonFlash Page(0x50000 - 0x200)*/
    flashAddr = CommonFLASH_TEST_ADDR;
    ret = HED_CommonFlash_EWPage(flashAddr, (uint32_t *)rand);

    if(ret != 0)
    {
        G_SW1SW2 = 0x6A80;
        return;
    }
    else
    {
        if(memcmp(rand, (uint8_t *)CommonFLASH_TEST_ADDR, 512))
        {
            G_SW1SW2 = 0x6A80;
            return;
        }
    }

    #ifndef PowerDown
    /* 1. get rand */
    //HED_GetRandom((uint8_t*)rand,260);
    HED_GetRandom((uint8_t *)rand, 260, &G_paraCheckSum);

    /* 2. show-how:write one byte to HEFlash Page(0x0007D011)*/
    flashAddr = HEFLASH_TEST_ADDR + 0x11;

    ret = HED_HEFlash_EWPage(flashAddr, (uint8_t *)rand, 1);

    if(ret != 0)
    {
        /* !!!NOTE:
           User need to judge ret in detail */
        if(ret == HEFLASH_FAIL)
        {
            //Maybe it means HEFlash become invalid . Restart IC agagin will be best method.
            G_SW1SW2 = 0x6A80;
            return;
        }

        // User can debug error cause by judge the two return value.
        if(ret == HEFLASH_CONDITIONERR)
        {
            G_SW1SW2 = 0x6F00;
            return;
        }
        else if(ret == HEFLASH_FAILALREADY)
        {
            // it means HEFLASH_FAIL had already appeared.
            G_SW1SW2 = 0x6F00;
            return;
        }
    }

    //HED_HEFlash_EWPage have the data compare.The memcmp is option.
    if(memcmp(rand, (uint8_t *)flashAddr, 1) != 0)
    {
        //Maybe it means HEFlash become invalid . Restart IC agagin will be the best method.
        G_SW1SW2 = 0x6A80;
        return;
    }


    /* 3. show-how:write 260 byte to HEFlash Page(0x0007D033)*/
    flashAddr = HEFLASH_TEST_ADDR + 0x33;
    ret = HED_HEFlash_EWPage(flashAddr, (uint8_t *)rand, 260);

    if(ret != 0)
    {
        /* !!!NOTE:
           User need to judge ret in detail */
        if(ret == HEFLASH_FAIL)
        {
            //Maybe it means HEFlash become invalid . Restart IC agagin will be best method.
        }

        SW1SW2 = 0x6A80;
        return;
    }

    if(memcmp(rand, (uint8_t *)flashAddr, 260) != 0)
    {
        //Maybe it means HEFlash become invalid . Restart IC agagin will be best method.
        G_SW1SW2 = 0x6A80;
        return;
    }

    #endif
    /* 3. show-how:olny erase HEFlash Page(0x0007D000)*/
    memset(buf, 0xFF, PAGESIZE);
    flashAddr = HEFLASH_TEST_ADDR;
    ret = HED_HEFlash_ErasePage(flashAddr, ~flashAddr);

    if(ret != 0)
    {
        /* !!!NOTE:
           User need to judge ret in detail */
        if(ret == HEFLASH_FAIL)
        {
            //Maybe it means HEFlash become invalid . Restart IC agagin will be best method.
            G_SW1SW2 = 0x6A80;
            return;
        }

        // User can debug error cause by judge the two return value.
        if(ret == HEFLASH_CONDITIONERR)
        {
            G_SW1SW2 = 0x6F00;
            return;
        }
        else if(ret == HEFLASH_FAILALREADY)
        {
            // it means HEFLASH_FAIL had already appeared.
            G_SW1SW2 = 0x6F00;
            return;
        }
    }

    //HED_HEFlash_ErasePage have the data compare.The memcmp is option.
    if(memcmp(buf, (uint8_t *)flashAddr, PAGESIZE) != 0)
    {
        //Maybe it means HEFlash become invalid . Restart IC agagin will be the best method.
        G_SW1SW2 = 0x6A80;
        return;
    }

    #ifndef PowerDown
    /* 4. show-how:olny write HEFlash word(0x0007D004) after HED_HEFlash_ErasePage page(0x0007D004) */
    /* Note:if the HEFLASH_TEST_ADDR area be C450,then user could call HED_HEFlash_WriteWord;
    or else, the area be ECC,the user should call HED_HEFlash_Write4Word*/
    flashAddr = HEFLASH_TEST_ADDR + 0x04;
    objValue = 0x12345678;
    checkWord = flashAddr ^ objValue;
    ret = HED_HEFlash_WriteWord(flashAddr, objValue, checkWord);

    if(ret != 0)
    {
        /* !!!NOTE:
           User need to judge ret in detail */
        if(ret == HEFLASH_FAIL)
        {
            //Maybe it means HEFlash become invalid or you don't run HED_HEFlash_ErasePage
        }

        G_SW1SW2 = 0x6A80;
        return;
    }

    //HED_HEFlash_WriteWord have the data compare.The compare is option.
    if(*(uint32_t *)flashAddr != objValue)
    {
        //Maybe it means HEFlash become invalid . Restart IC agagin will be best method.
        G_SW1SW2 = 0x6A80;
        return;
    }

    #endif

    /* 5. show-how:olny erase HEFlash Page(0x0007D000) after HED_HEFlash_WriteNWord page(0x0007D000) */
    memset(buf, 0xFF, PAGESIZE);
    flashAddr = HEFLASH_TEST_ADDR;
    ret = HED_HEFlash_ErasePage(flashAddr, ~flashAddr);

    if(ret != 0)
    {
        /* !!!NOTE:
           User need to judge ret in detail */
        if(ret == HEFLASH_FAIL)
        {
            //Maybe it means HEFlash become invalid . Restart IC agagin will be best method.
            G_SW1SW2 = 0x6A80;
            return;
        }

        // User can debug error cause by judge the two return value.
        if(ret == HEFLASH_CONDITIONERR)
        {
            G_SW1SW2 = 0x6F00;
            return;
        }
        else if(ret == HEFLASH_FAILALREADY)
        {
            // it means HEFLASH_FAIL had already appeared.
            G_SW1SW2 = 0x6F00;
            return;
        }
    }

    //HED_HEFlash_ErasePage have the data compare.The memcmp is option.
    if(memcmp(buf, (uint8_t *)flashAddr, PAGESIZE) != 0)
    {
        //Maybe it means HEFlash become invalid . Restart IC agagin will be the best method.
        G_SW1SW2 = 0x6A80;
        return;
    }

    HED_GetRandom((uint8_t *)rand, 512, &G_paraCheckSum);
    ret = HED_HEFlash_WriteNWord(flashAddr, rand, PAGESIZE >> 2);

    if(ret != 0)
    {
        /* !!!NOTE:
           User need to judge ret in detail */
        if(ret == HEFLASH_FAIL)
        {
            //Maybe it means HEFlash become invalid or you don't run HED_HEFlash_ErasePage
        }

        G_SW1SW2 = 0x6A80;
        return;
    }
    else
    {
        if(memcmp(rand, (uint8_t *)flashAddr, 512))
        {
            G_SW1SW2 = 0x6A80;
            return;
        }
    }

    G_SW1SW2 = 0x9000;
}

