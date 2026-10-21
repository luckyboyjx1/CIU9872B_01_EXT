#include "Io_I2C.h"
#include "Io_SPI.h"
#include "Io_Swp.h"
#include "chipHead.h"

#if (HAL__IO__SPI||HAL__IO__I2C||HAL__IO__SWP)

//void IOInterface_init(void)
//{
//	switch(sysConfig_GetPROTOCLTYPE())
//	{
//		case 1:
//			Spi_Init( );
//			break;
//		case 2:
//			I2C_Init( );
//			break;
//		case 3:
//			Swp_Init( );
//			break;
//	}
//}

//void IOInterface_SendStatus()
//{
//	switch(sysConfig_GetPROTOCLTYPE())
//	{
//		case 1:
//			Spi_SendStatus();
//			break;
//		case 2:
//			I2C_SendStatus();
//			break;
//		case 3:
//			Swp_SendStatus();
//			break;
//	}
//}

//u8 IOInterface_HandleCommand()
//{
//	boolean	isReturn;

//	isReturn	= false;
//	
//	
//	switch(sysConfig_GetPROTOCLTYPE())
//	{
//		case 1:
//			isReturn	= Spi_HandleCommand();
//			break;
//		case 2:
//			isReturn	= I2C_HandleCommand();
//			break;
//		case 3:
//			isReturn	= Swp_HandleCommand();
//			break;
//		
//		default:
//			return false;
//	}
//	return isReturn;

//}



#endif



