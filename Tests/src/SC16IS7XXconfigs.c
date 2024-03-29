/*******************************************************************************
  File name:    SC16IS7XXconfigs.c
  Author:       FMA
  Version:      1.0
  Date (d/m/y): 20/09/2020
  Description:  SC16IS7XX driver and controllers configurations for the DEMO

  History :
*******************************************************************************/

//-----------------------------------------------------------------------------
#include "Main.h"
#include "SC16IS7XXconfigs.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
#  include "stdafx.h"
extern "C" {
#endif
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// SPI0 interface container structure on the V71
//=============================================================================
SPI_Interface SPI0_Interface =
{
  .InterfaceDevice = SPI0,
  .fnSPI_Init      = SPI_MasterInit_Gen,
  .fnSPI_Transfer  = SPI_PacketTransfer_Gen,
  .Channel         = 0,
};



//=============================================================================
// Configuration of the SPI0 on the V71
//=============================================================================
SPI_Config SPI0_Config =
{
  .VariablePS      = true,
  .CSdecoder       = false,
  .ModeFaultDetect = false,
  .WaitRead        = true,
  .DLYBCS_ns       = SPI_DLYBCS,
  .CSR             =
  {
    { .DLYBCT_ns = SPI_DLYBCT, .DLYBS_ns = SPI_DLYBS, .BitsPerTransfer = 8, .CSbehavior = SPI_CS_KEEP_LOW, },
    { .DLYBCT_ns = SPI_DLYBCT, .DLYBS_ns = SPI_DLYBS, .BitsPerTransfer = 8, .CSbehavior = SPI_CS_KEEP_LOW, },
    { .DLYBCT_ns = SPI_DLYBCT, .DLYBS_ns = SPI_DLYBS, .BitsPerTransfer = 8, .CSbehavior = SPI_CS_KEEP_LOW, },
    { .DLYBCT_ns = SPI_DLYBCT, .DLYBS_ns = SPI_DLYBS, .BitsPerTransfer = 8, .CSbehavior = SPI_CS_KEEP_LOW, },
  },
};
//-----------------------------------------------------------------------------



//=============================================================================
// I2C0 interface container structure on the V71
//=============================================================================
I2C_Interface I2C0_Interface =
{
  .InterfaceDevice = TWIHS0,
  .fnI2C_Init      = TWIHS_MasterInit_Gen,
  .fnI2C_Transfer  = TWIHS_PacketTransfer_Gen,
  .Channel         = 0,
};
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// Device configuration structure of the SC16IS740 on EXT1 with hard SPI0 on the V71
//=============================================================================
struct SC16IS7XX SC16IS7XX_EXT1 =
{
  //--- Device configuration ---
  .UserDriverData      = NULL,
  .XtalFreq            = 0,
  .OscFreq             = 1843200, // 1.8432MHz Oscillator
  .DevicePN            = SC16IS740,

  //--- Interface driver call functions ---
  .Interface           = SC16IS7XX_INTERFACE_SPI,
  .SPIchipSelect       = SPI_CS_EXT1,
  .SPI                 = &SPI0_Interface,
  .InterfaceClockSpeed = 4000000, // SPI speed at 4MHz

  //--- GPIO configuration ---
  .GPIOsOutLevel       = 0, // No GPIO on this device
};


SC16IS7XX_Config SC16IS7XX_EXT1_Config =
{
  //--- GPIOs configuration ---
  .StartupPinsDirection = 0, // No GPIO on this device
  .StartupPinsLevel     = 0, // No GPIO on this device
  .PinsInterruptEnable  = 0, // No GPIO on this device
};

//-----------------------------------------------------------------------------



// @warning Each Channel and Device tuple should be unique. Only 1 possible tuple on SC16IS740 devices
//=============================================================================
// Channel 0 UART Configuration structure of SC16IS740_EXT1 (SC16IS7XX)
//=============================================================================
struct SC16IS7XX_UART UART_Chan0_EXT1 = // SC16IS7XX UART
{
  //--- UART configuration ---
  .Channel        = SC16IS7XX_CHANNEL_A,
  .DriverConfig   = SC16IS7XX_DRIVER_SAFE_TX | SC16IS7XX_DRIVER_SAFE_RX,
  //--- Device configuration ---
  .UserDriverData = NULL,
  .Device         = SC16IS740_EXT1,

#ifdef SC16IS7XX_USE_BUFFERS
  .TxBuffer       =
  {
    .pData        = NULL,  // No Tx buffer
    .BufferSize   = 0,
    .PosIn        = 0,     // No need to fill (set a default value)
    .PosOut       = 0,     // No need to fill (set a default value)
    .IsFull       = false, // No need to fill (set a default value)
  },
  .RxBuffer       =
  {
    .pData        = NULL,  // No Rx buffer
    .BufferSize   = 0,
    .PosIn        = 0,     // No need to fill (set a default value)
    .PosOut       = 0,     // No need to fill (set a default value)
    .IsFull       = false, // No need to fill (set a default value)
  },
#endif
};

/*struct UART_Interface UART_Int0_EXT1 = // UART Interface generic
{
  //--- Device configuration ---
  .InterfaceDevice = UART0_EXT1, // Shall be a SC16IS7XX_UART device
  .fnUART_Transmit = SC16IS7XX_TransmitData_Gen,
  .fnUART_Receive  = SC16IS7XX_ReceiveData_Gen,
  //--- I2C configuration ---
  .Channel         = 0,
};*/

int32_t Baudrate_UART0_EXT1;


// UART0_EXT1 configured into RS-232 (115200-8-N-1), no control flow
SC16IS7XX_UARTconfig UART0_EXT1_RS232config =
{
  //--- UART configuration ---
  .UARTtype           = SC16IS7XX_UART_RS232,        // Type of the UART: RS232
  .UARTwordLen        = SC16IS7XX_DATA_LENGTH_8bits, // UART data length 8 bits
  .UARTparity         = SC16IS7XX_NO_PARITY,         // No UART parity
  .UARTstopBit        = SC16IS7XX_STOP_BIT_1bit,     // UART 1-bit stop length
  .UARTbaudrate       = 115200,                      // UART desired baudrate 115200 bauds
  .UARTbaudrateError  = &Baudrate_UART0_EXT1,
  .RS232 =
  {
    .ControlFlowType  = SC16IS7XX_NO_CONTROL_FLOW,   // No control flow
  },
  .UseSpecialChar     = false,                       // No special char on Xoff2
  .DisableTransmitter = false,                       // Enable transmitter
  .DisableReceiver    = false,                       // Enable receiver

  //--- FIFO configuration ---
  .UseFIFOs           = true,                        // Enable FIFO at startup
  .TxTrigLvl          = SC16IS7XX_TX_FIFO_TRIGGER_AT_16_CHAR_SPACE,    // FIFO Tx trigger level used for interrupt generation
  .RxTrigLvl          = SC16IS7XX_RX_FIFO_TRIGGER_AT_4_CHAR_AVAILABLE, // FIFO Rx trigger level used for interrupt generation

  //--- Interrupt configuration ---
  .Interrupts         = SC16IS7XX_RX_FIFO_INTERRUPT | SC16IS7XX_TX_FIFO_INTERRUPT, // Interrupt configuration of the UART
};

//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// Device configuration structure of the SC16IS750 with hard I2C on the V71
//=============================================================================
struct SC16IS7XX SC16IS7XX_I2C =
{
  //--- Device configuration ---
  .UserDriverData      = NULL,
  .XtalFreq            = 14745600, // 14.7456MHz crystal
  .OscFreq             = 0,
  .DevicePN            = SC16IS750,

  //--- Interface driver call functions ---
  .Interface           = SC16IS7XX_INTERFACE_I2C,
  .I2Caddress          = SC16IS7XX_ADDRESS_A1L_A0L,
  .I2C                 = &I2C0_Interface,
  .InterfaceClockSpeed = 400000, // I2C speed at 400kHz

  //--- GPIO configuration ---
  .GPIOsOutLevel       = 0, // Set all GPIO to 0
};


SC16IS7XX_Config SC16IS7XX_I2C_Config =
{
  //--- GPIOs configuration ---
  .StartupPinsDirection = 0, // Startup GPIOs direction (0 = output ; 1 = input)
  .StartupPinsLevel     = 0, // Startup GPIOs output level (0 = set to '0' ; 1 = set to '1')
  .PinsInterruptEnable  = 0, // GPIOs individual Interrupt (0 = disable ; 1 = enable)
};

//-----------------------------------------------------------------------------



// @warning Each Channel and Device tuple should be unique. Only 1 possible tuple on SC16IS750 devices
//=============================================================================
// Channel 0 UART Configuration structure of SC16IS750_I2C (SC16IS7XX)
//=============================================================================
struct SC16IS7XX_UART UART_Chan0_I2C = // SC16IS7XX UART
{
  //--- UART configuration ---
  .Channel        = SC16IS7XX_CHANNEL_A,
  .DriverConfig   = SC16IS7XX_DRIVER_BURST_TX | SC16IS7XX_DRIVER_BURST_RX/* | SC16IS7XX_TEST_LOOPBACK_AT_INIT*/,
//  .DriverConfig   = SC16IS7XX_DRIVER_SAFE_TX | SC16IS7XX_DRIVER_SAFE_RX,
  //--- Device configuration ---
  .UserDriverData = NULL,
  .Device         = SC16IS750_I2C,

#ifdef SC16IS7XX_USE_BUFFERS
  .TxBuffer       =
  {
    .pData        = NULL,  // No Tx buffer
    .BufferSize   = 0,
    .PosIn        = 0,     // No need to fill (set a default value)
    .PosOut       = 0,     // No need to fill (set a default value)
    .IsFull       = false, // No need to fill (set a default value)
  },
  .RxBuffer       =
  {
    .pData        = NULL,  // No Rx buffer
    .BufferSize   = 0,
    .PosIn        = 0,     // No need to fill (set a default value)
    .PosOut       = 0,     // No need to fill (set a default value)
    .IsFull       = false, // No need to fill (set a default value)
  },
#endif
};

/*struct UART_Interface UART_Int0_I2C = // UART Interface generic
{
  //--- Device configuration ---
  .InterfaceDevice = UART0_I2C, // Shall be a SC16IS7XX_UART device
  .fnUART_Transmit = SC16IS7XX_TransmitData_Gen,
  .fnUART_Receive  = SC16IS7XX_ReceiveData_Gen,
  //--- I2C configuration ---
  .Channel         = 0,
};*/

int32_t Baudrate_UART0_I2C;


// UART0_I2C configured into RS-232 (115200-8-N-1), no control flow
SC16IS7XX_UARTconfig UART0_I2C_RS232config =
{
  //--- UART configuration ---
  .UARTtype           = SC16IS7XX_UART_RS232,        // Type of the UART: RS232
  .UARTwordLen        = SC16IS7XX_DATA_LENGTH_8bits, // UART data length 8 bits
  .UARTparity         = SC16IS7XX_NO_PARITY,         // No UART parity
  .UARTstopBit        = SC16IS7XX_STOP_BIT_1bit,     // UART 1-bit stop length
  .UARTbaudrate       = 115200,                      // UART desired baudrate 115200 bauds
  .UARTbaudrateError  = &Baudrate_UART0_I2C,
  .RS232 =
  {
    .ControlFlowType  = SC16IS7XX_NO_CONTROL_FLOW,   // No control flow
  },
  .UseSpecialChar     = false,                       // No special char on Xoff2
  .DisableTransmitter = false,                       // Enable transmitter
  .DisableReceiver    = false,                       // Enable receiver

  //--- FIFO configuration ---
  .UseFIFOs           = true,                        // Enable FIFO at startup
  .TxTrigLvl          = SC16IS7XX_TX_FIFO_TRIGGER_AT_32_CHAR_SPACE,    // FIFO Tx trigger level used for interrupt generation
  .RxTrigLvl          = SC16IS7XX_RX_FIFO_TRIGGER_AT_4_CHAR_AVAILABLE, // FIFO Rx trigger level used for interrupt generation

  //--- Interrupt configuration ---
  .Interrupts         = /*SC16IS7XX_RX_FIFO_INTERRUPT | */SC16IS7XX_TX_FIFO_INTERRUPT, // Interrupt configuration of the UART
};

//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//=============================================================================
// Device configuration structure of the SC16IS752 on EXT2 with hard SPI0 on the V71
//=============================================================================
struct SC16IS7XX SC16IS7XX_EXT2 =
{
  //--- Device configuration ---
  .UserDriverData      = NULL,
  .XtalFreq            = 1843200, // 1.8432MHz Crystal
  .OscFreq             = 0,
  .DevicePN            = SC16IS752,

  //--- Interface driver call functions ---
  .Interface           = SC16IS7XX_INTERFACE_SPI,
  .SPIchipSelect       = SPI_CS_EXT2,
  .SPI                 = &SPI0_Interface,
  .InterfaceClockSpeed = 4000000, // SPI speed at 4MHz

  //--- GPIO configuration ---
  .GPIOsOutLevel       = 0, // Set all GPIO to 0
};


SC16IS7XX_Config SC16IS7XX_EXT2_Config =
{
  //--- GPIOs configuration ---
  .StartupPinsDirection = 0x0, // Startup GPIOs direction (0 = output ; 1 = input)
  .StartupPinsLevel     = 0x0, // Startup GPIOs output level (0 = set to '0' ; 1 = set to '1')
  .PinsInterruptEnable  = 0x0, // GPIOs individual Interrupt (0 = disable ; 1 = enable)
};

//-----------------------------------------------------------------------------



//--- UART0_EXT2 buffers configuration ---
#define UART0_EXT2_TXBUFFER_SIZE  200
uint8_t UART0_EXT2_TxBuffer[UART0_EXT2_TXBUFFER_SIZE];
#define UART0_EXT2_RXBUFFER_SIZE  200
uint8_t UART0_EXT2_RxBuffer[UART0_EXT2_RXBUFFER_SIZE];

// @warning Each Channel and Device tuple should be unique. Only 2 possible tuple on SC16IS752 devices
//=============================================================================
// Channel 0 UART Configuration structure of SC16IS752_EXT2 (SC16IS7XX)
//=============================================================================
struct SC16IS7XX_UART UART_Chan0_EXT2 = // SC16IS7XX UART
{
  //--- UART configuration ---
  .Channel        = SC16IS7XX_CHANNEL_A,
  .DriverConfig   = SC16IS7XX_DRIVER_BURST_TX | SC16IS7XX_DRIVER_BURST_RX,
//  .DriverConfig   = SC16IS7XX_DRIVER_SAFE_TX | SC16IS7XX_DRIVER_SAFE_RX,
  //--- Device configuration ---
  .UserDriverData = NULL,
  .Device         = SC16IS752_EXT2,

#ifdef SC16IS7XX_USE_BUFFERS
  .TxBuffer       =
  {
    .pData        = &UART0_EXT2_TxBuffer[0], // Set a Tx buffer
    .BufferSize   = UART0_EXT2_TXBUFFER_SIZE,
    .PosIn        = 0,     // No need to fill (will be changed at UART initialization)
    .PosOut       = 0,     // No need to fill (will be changed at UART initialization)
    .IsFull       = false, // No need to fill (will be changed at UART initialization)
  },
  .RxBuffer       =
  {
    .pData        = &UART0_EXT2_RxBuffer[0], // Set a Rx buffer
    .BufferSize   = UART0_EXT2_RXBUFFER_SIZE,
    .PosIn        = 0,     // No need to fill (will be changed at UART initialization)
    .PosOut       = 0,     // No need to fill (will be changed at UART initialization)
    .IsFull       = false, // No need to fill (will be changed at UART initialization)
  },
#endif
};

/*struct UART_Interface UART_Int0_EXT2 = // UART Interface generic
{
  //--- Device configuration ---
  .InterfaceDevice = UART0_EXT2, // Shall be a SC16IS7XX_UART device
  .fnUART_Transmit = SC16IS7XX_TransmitData_Gen,
  .fnUART_Receive  = SC16IS7XX_ReceiveData_Gen,
  //--- I2C configuration ---
  .Channel         = 0,
};*/


//--- UART1_EXT2 buffers configuration ---
#define UART1_EXT2_TXBUFFER_SIZE  200
uint8_t UART1_EXT2_TxBuffer[UART1_EXT2_TXBUFFER_SIZE];
#define UART1_EXT2_RXBUFFER_SIZE  200
uint8_t UART1_EXT2_RxBuffer[UART1_EXT2_RXBUFFER_SIZE];

//=============================================================================
// Channel 1 UART Configuration structure of SC16IS752_EXT2 (SC16IS7XX)
//=============================================================================
struct SC16IS7XX_UART UART_Chan1_EXT2 = // SC16IS7XX UART
{
  //--- UART configuration ---
  .Channel        = SC16IS7XX_CHANNEL_B,
  .DriverConfig   = SC16IS7XX_DRIVER_BURST_TX | SC16IS7XX_DRIVER_BURST_RX,
//  .DriverConfig   = SC16IS7XX_DRIVER_SAFE_TX | SC16IS7XX_DRIVER_SAFE_RX,
  //--- Device configuration ---
  .UserDriverData = NULL,
  .Device         = SC16IS752_EXT2,

#ifdef SC16IS7XX_USE_BUFFERS
  .TxBuffer       =
  {
    .pData        = &UART1_EXT2_TxBuffer[0], // Set a Tx buffer
    .BufferSize   = UART1_EXT2_TXBUFFER_SIZE,
    .PosIn        = 0,     // No need to fill (will be changed at UART initialization)
    .PosOut       = 0,     // No need to fill (will be changed at UART initialization)
    .IsFull       = false, // No need to fill (will be changed at UART initialization)
  },
  .RxBuffer       =
  {
    .pData        = &UART1_EXT2_RxBuffer[0], // Set a Rx buffer
    .BufferSize   = UART1_EXT2_RXBUFFER_SIZE,
    .PosIn        = 0,     // No need to fill (will be changed at UART initialization)
    .PosOut       = 0,     // No need to fill (will be changed at UART initialization)
    .IsFull       = false, // No need to fill (will be changed at UART initialization)
  },
#endif
};

/*struct UART_Interface UART_Int1_EXT2 = // UART Interface generic
{
  //--- Device configuration ---
  .InterfaceDevice = UART1_EXT2, // Shall be a SC16IS7XX_UART device
  .fnUART_Transmit = SC16IS7XX_TransmitData_Gen,
  .fnUART_Receive  = SC16IS7XX_ReceiveData_Gen,
  //--- I2C configuration ---
  .Channel         = 1,
};*/

int32_t Baudrate_UART_EXT2;


// UART_EXT2 configured into RS-232 (115200-8-N-1), no control flow
SC16IS7XX_UARTconfig UART_EXT2_RS232config =
{
  //--- UART configuration ---
  .UARTtype           = SC16IS7XX_UART_RS232,        // Type of the UART: RS232
  .UARTwordLen        = SC16IS7XX_DATA_LENGTH_8bits, // UART data length 8 bits
  .UARTparity         = SC16IS7XX_NO_PARITY,         // No UART parity
  .UARTstopBit        = SC16IS7XX_STOP_BIT_1bit,     // UART 1-bit stop length
  .UARTbaudrate       = 115200,                      // UART desired baudrate 115200 bauds
  .UARTbaudrateError  = &Baudrate_UART_EXT2,
  .RS232 =
  {
    .ControlFlowType  = SC16IS7XX_NO_CONTROL_FLOW,   // No control flow
  },
  .UseSpecialChar     = false,                       // No special char on Xoff2
  .DisableTransmitter = false,                       // Enable transmitter
  .DisableReceiver    = false,                       // Enable receiver

  //--- FIFO configuration ---
  .UseFIFOs           = true,                        // Enable FIFO at startup
  .TxTrigLvl          = SC16IS7XX_TX_FIFO_TRIGGER_AT_32_CHAR_SPACE,    // FIFO Tx trigger level used for interrupt generation
  .RxTrigLvl          = SC16IS7XX_RX_FIFO_TRIGGER_AT_16_CHAR_AVAILABLE, // FIFO Rx trigger level used for interrupt generation

  //--- Interrupt configuration ---
  .Interrupts         = SC16IS7XX_RX_FIFO_INTERRUPT/* | SC16IS7XX_TX_FIFO_INTERRUPT*/, // Interrupt configuration of the UART
};

//-----------------------------------------------------------------------------





//********************************************************************************************************************
//=============================================================================
// Get millisecond
//=============================================================================
uint32_t GetCurrentms_V71(void)
{
  return msCount;
}

//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif