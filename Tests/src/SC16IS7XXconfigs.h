/*******************************************************************************
  File name:    SC16IS7XXconfigs.h
  Author:       FMA
  Version:      1.0
  Date (d/m/y): 20/09/2020
  Description:  SC16IS7XX driver and controllers configurations for the DEMO

  History :
*******************************************************************************/
#ifndef SC16IS7XXCONFIGS_H_
#define SC16IS7XXCONFIGS_H_
//=============================================================================

//-----------------------------------------------------------------------------
#include "stdio.h"
#include <stdarg.h>
#include "SPI_V71.h"
#include "TWIHS_V71.h"
#include "SC16IS7XX.h"
#include "ErrorsDef.h"
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------

//! Chip select (nCSx)
#define SPI_CS_EXT1       ( 1 )    // SPI_nCS1 for the MCP2517FD Click on EXT1
//! Chip select (nCSx)
#define SPI_CS_EXT2       ( 3 )    // SPI_nCS3 for the MCP2517FD Click on EXT2

//-----------------------------------------------------------------------------





//********************************************************************************************************************

//! Delay before SPCK
# define SPI_DLYBS         ( 0x01 ) // Tspick/2 needed
//! Delay between consecutive transfers
# define SPI_DLYBCT        ( 0x01 ) // To conform last SCK rise to nCS rise time (1 Tspick)
//! Delay Between Chip Selects
# define SPI_DLYBCS        ( 0x01 ) // To conform 1 Tspick needed

extern SPI_Interface SPI0_Interface; //! SPI0 interface container structure on the V71
extern SPI_Config SPI0_Config;       //! Configuration of the SPI0 on the V71
//-----------------------------------------------------------------------------

extern I2C_Interface I2C0_Interface; //! I2C0 interface container structure on the V71
//-----------------------------------------------------------------------------





//********************************************************************************************************************
// Structure of the SC16IS740 on EXT1 with hard SPI0 used in the demo
extern struct SC16IS7XX SC16IS7XX_EXT1;
#define SC16IS740_EXT1  &SC16IS7XX_EXT1

extern SC16IS7XX_Config SC16IS7XX_EXT1_Config;
//-----------------------------------------------------------------------------

// Channel 0 UART Configuration structure of SC16IS740_EXT1 (SC16IS7XX)
extern struct SC16IS7XX_UART UART_Chan0_EXT1;
//extern struct UART_Interface UART_Int0_EXT1;
#define UART0_EXT1  &UART_Chan0_EXT1

extern int32_t Baudrate_UART0_Ext1;
extern SC16IS7XX_UARTconfig UART0_EXT1_RS232config;
//-----------------------------------------------------------------------------





//********************************************************************************************************************
// Structure of the SC16IS750 with hard I2C used in the demo
extern struct SC16IS7XX SC16IS7XX_I2C;
#define SC16IS750_I2C  &SC16IS7XX_I2C

extern SC16IS7XX_Config SC16IS7XX_I2C_Config;
//-----------------------------------------------------------------------------

// Channel 0 UART Configuration structure of SC16IS750_I2C (SC16IS7XX)
extern struct SC16IS7XX_UART UART_Chan0_I2C;
//extern struct UART_Interface UART_Int0_I2C;
#define UART0_I2C  &UART_Chan0_I2C

extern int32_t Baudrate_UART0_I2C;
extern SC16IS7XX_UARTconfig UART0_I2C_RS232config;
//-----------------------------------------------------------------------------





//********************************************************************************************************************
// Structure of the SC16IS752 on EXT2 with hard SPI0 used in the demo
extern struct SC16IS7XX SC16IS7XX_EXT2;
#define SC16IS752_EXT2  &SC16IS7XX_EXT2

extern SC16IS7XX_Config SC16IS7XX_EXT2_Config;
//-----------------------------------------------------------------------------
// Channel 0 UART Configuration structure of SC16IS752_EXT2 (SC16IS7XX)
extern struct SC16IS7XX_UART UART_Chan0_EXT2;
//extern struct UART_Interface UART_Int0_EXT2;
#define UART0_EXT2  &UART_Chan0_EXT2
//-----------------------------------------------------------------------------
// Channel 1 UART Configuration structure of SC16IS752_EXT2 (SC16IS7XX)
extern struct SC16IS7XX_UART UART_Chan1_EXT2;
//extern struct UART_Interface UART_Int1_EXT2;
#define UART1_EXT2  &UART_Chan1_EXT2
//-----------------------------------------------------------------------------
extern int32_t Baudrate_UART_Ext2;
extern SC16IS7XX_UARTconfig UART_EXT2_RS232config;
//-----------------------------------------------------------------------------





//********************************************************************************************************************
/*! @brief Get millisecond
 *
 * This function will be called when the driver need to get current millisecond
 */
uint32_t GetCurrentms_V71(void);

//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* SC16IS7XXCONFIGS_H_ */