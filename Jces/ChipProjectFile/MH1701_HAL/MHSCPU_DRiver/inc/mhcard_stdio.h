#ifndef _MHCARD_STDIO_H_
#define _MHCARD_STDIO_H_

    #ifdef __cplusplus
    extern "C"
    {
    #endif
    /* add your code hear */
    #include "stdint.h"
    #define STD_IO_PORT_NULL        (0)
    #define STD_IO_PORT_SCI         (1)
    #define STD_IO_PORT_SPI         (2)
    #define STD_IO_PORT_SCI_T0      (3)
    #define STD_IO_PORT_SCI_UART    (4)  
    #define STD_IO_PORT_GPIO        (5)
    #define STD_IO_PORT_SCI_MSI     (6)//Master-Slave-Inversion
    /*1
        select the io port sci or spi
    */
//    #define STD_IO_PORT         (STD_IO_PORT_NULL)
    
    void mhcard_StdIOInit(uint32_t u32BaudRate, uint32_t u32Parity);
    
    #ifdef __cplusplus
    }
    #endif

#endif
