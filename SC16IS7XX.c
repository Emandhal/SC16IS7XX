/*******************************************************************************
 * @file    SC16IS7XX.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.1
 * @date    20/09/2020
 * @brief   SC16IS740, SC16IS741, SC16IS741A, SC16IS750, SC16IS752, SC16IS760,
 *          SC16IS762 driver
 *
 * The SC16IS7XX component is a Single/Double UART with I2C-bus/SPI
 * interface, 64 bytes of transmit and receive FIFOs, IrDA SIR built-in support
 * Follow datasheet SC16IS740_750_760   Rev.7  ( 9 June  2011)
 *                  SC16IS741_1         Rev.01 (29 April 2010)
 *                  SC16IS741A          Rev.1  (18 March 2013)
 *                  SC16IS752_SC16IS762 Rev.9  (22 March 2012)
 * Follow AN10571 - Sleep programming for NXP bridge ICs   Rev.01 (7 Jan  2007)
 *        AN10417 - SC16IS760/762 Fast IrDA mode           Rev.01 (8 June 2006)
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "SC16IS7XX.h"
//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
#  include <cstdint>
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------





//=============================================================================
// Prototypes for private functions
//=============================================================================
// DO NOT USE DIRECTLY, use SC16IS7XX_InitUART() instead! Control Flow needs to be configured with a safe UART configuration to avoid spurious effects, which is done in the SC16IS7XX_InitUART() function
static eERRORRESULT __SC16IS7XX_SetControlFlowConfiguration(SC16IS7XX_UART *pUART, SC16IS7XX_HardControlFlow *pHardFlow, SC16IS7XX_SoftControlFlow *pSoftFlow, bool useAdressChar);
// DO NOT USE DIRECTLY, use SC16IS7XX_InitUART() instead! UART configuration needs to be configured with a safe UART configuration to avoid spurious effects, which is done in the SC16IS7XX_InitUART() function
static eERRORRESULT __SC16IS7XX_SetUARTConfiguration(SC16IS7XX_UART *pUART, const SC16IS7XX_UARTconfig *pUARTConf);
//-----------------------------------------------------------------------------
#define SC16IS7XX_ABSOLUTE(value)  ( (value) < 0.0f ? -(value) : value )
//-----------------------------------------------------------------------------



//=== SC16IS7XX devices limits ================================================
const SC16IS7XX_Limits SC16IS7XX_LIMITS[SC16IS7XX_PN_COUNT] =
{
  { .I2C_CLOCK_MAX = SC16IS7XX_I2C_CLOCK_MAX, .SPI_CLOCK_MAX = SC16IS7XX_SPI_CLOCK_MAX, .IrDA_1_4_RATIO = false, .HAVE_GPIO = false, .HAVE_2_UARTS = false, }, // SC16IS740
  { .I2C_CLOCK_MAX = SC16IS7XX_I2C_CLOCK_MAX, .SPI_CLOCK_MAX = SC16IS7XX_SPI_CLOCK_MAX, .IrDA_1_4_RATIO = false, .HAVE_GPIO = false, .HAVE_2_UARTS = false, }, // SC16IS741
  { .I2C_CLOCK_MAX = SC16IS7XX_I2C_CLOCK_MAX, .SPI_CLOCK_MAX = SC16IS7XX_SPI_CLOCK_MAX, .IrDA_1_4_RATIO = false, .HAVE_GPIO = true , .HAVE_2_UARTS = false, }, // SC16IS750
  { .I2C_CLOCK_MAX = SC16IS7XX_I2C_CLOCK_MAX, .SPI_CLOCK_MAX = SC16IS7XX_SPI_CLOCK_MAX, .IrDA_1_4_RATIO = false, .HAVE_GPIO = true , .HAVE_2_UARTS = true , }, // SC16IS752
  { .I2C_CLOCK_MAX = SC16IS7XX_I2C_CLOCK_MAX, .SPI_CLOCK_MAX = SC16IS76X_SPI_CLOCK_MAX, .IrDA_1_4_RATIO = true , .HAVE_GPIO = true , .HAVE_2_UARTS = false, }, // SC16IS760
  { .I2C_CLOCK_MAX = SC16IS7XX_I2C_CLOCK_MAX, .SPI_CLOCK_MAX = SC16IS76X_SPI_CLOCK_MAX, .IrDA_1_4_RATIO = true , .HAVE_GPIO = true , .HAVE_2_UARTS = true , }, // SC16IS762
};





//**********************************************************************************************************************************************************
//=============================================================================
// SC16IS7XX initialization
//=============================================================================
eERRORRESULT Init_SC16IS7XX(SC16IS7XX *pComp, const SC16IS7XX_Config *pConf)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;

  //--- Check device limits ---------------------------------
  if (pComp->DevicePN >= SC16IS7XX_PN_COUNT) return ERR__UNKNOWN_DEVICE;

  //--- Configure the Interface -----------------------------
#ifdef SC16IS7XX_I2C_DEFINED
  if (pComp->Interface == SC16IS7XX_INTERFACE_I2C)
  {
    if (pComp->fnI2C_Init == NULL) return ERR__PARAMETER_ERROR;
    if (pComp->InterfaceClockSpeed > SC16IS7XX_LIMITS[pComp->DevicePN].I2C_CLOCK_MAX) return ERR__I2C_CONFIG_ERROR;
    Error = pComp->fnI2C_Init(pComp->InterfaceDevice, pComp->InterfaceClockSpeed);                        // Initialize the I2C interface
    if (Error != ERR_OK) return Error;                                                                    // If there is an error while calling fnI2C_Init() then return the error
  }
#endif
#ifdef SC16IS7XX_SPI_DEFINED
  if (pComp->Interface == SC16IS7XX_INTERFACE_SPI)
  {
    if (pComp->fnSPI_Init == NULL) return ERR__PARAMETER_ERROR;
    if (pComp->InterfaceClockSpeed > SC16IS7XX_LIMITS[pComp->DevicePN].SPI_CLOCK_MAX) return ERR__I2C_CONFIG_ERROR;
    Error = pComp->fnSPI_Init(pComp->InterfaceDevice, pComp->SPI_ChipSelect, pComp->InterfaceClockSpeed); // Initialize the SPI interface
    if (Error != ERR_OK) return Error;                                                                    // If there is an error while calling fnSPI_Init() then return the error
  }
#endif

  //--- Reset device ----------------------------------------
  Error = SC16IS7XX_SoftResetDevice(pComp);
  if (Error != ERR_OK) return Error;                                                                      // If there is an error while calling SC16IS7XX_SoftResetDevice() then return the Error

  //--- Test SPI connection ---------------------------------
  Error = SC16IS7XX_HardwareCommTest(pComp);
  if (Error != ERR_OK) return Error;                                                                      // If there is an error while calling SC16IS7XX_HardwareCommTest() then return the Error

  //--- Configure GPIOs -------------------------------------
  if (SC16IS7XX_LIMITS[pComp->DevicePN].HAVE_GPIO && (pConf != NULL))                                     // Only if device PN have GPIOs and a configuration is available
  {
    Error = SC16IS7XX_ConfigureGPIOs(pComp, pConf->StartupPinsDirection, pConf->StartupPinsLevel, pConf->PinsInterruptEnable);
    if (Error != ERR_OK) return Error;                                                                    // If there is an error while calling SC16IS7XX_ConfigureGPIOs() then return the error
  }
  return ERR_OK;
}



//=============================================================================
// Perform a Software Reset the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_SoftResetDevice(SC16IS7XX *pComp)
{
  eERRORRESULT Error;
  Error = SC16IS7XX_WriteRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IOControl, SC16IS7XX_IOCTRL_SOFTWARE_RESET); // Write the IOControl register
  if (Error == ERR__I2C_NACK_DATA) return ERR_OK;                        // Device returns NACK on I2C-bus when set bit "UART software reset" is written
  return Error;
}



//=============================================================================
// Hardware communication tests of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_HardwareCommTest(SC16IS7XX *pComp)
{
  eERRORRESULT Error;
  uint8_t Value;

  //--- Test SPI communication ---
  Error = SC16IS7XX_WriteRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_SPR, 0x55);  // Write 0x55 to Scratchpad Register
  if (Error != ERR_OK) return Error;                                                     // If there is an error while calling SC16IS7XX_WriteRegister() then return the Error
  Error = SC16IS7XX_ReadRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_SPR, &Value); // Read the Scratchpad Register
  if (Error != ERR_OK) return Error;                                                     // If there is an error while calling SC16IS7XX_ReadRegister() then return the Error
  if (Value != 0x55) return ERR__SPI_COMM_ERROR;                                         // If the read back value is not the same as the one writen, return an error

  Error = SC16IS7XX_WriteRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_SPR, 0xAA);  // Write 0xAA to Scratchpad Register
  if (Error != ERR_OK) return Error;                                                     // If there is an error while calling SC16IS7XX_WriteRegister() then return the Error
  Error = SC16IS7XX_ReadRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_SPR, &Value); // Read the Scratchpad Register
  if (Error != ERR_OK) return Error;                                                     // If there is an error while calling SC16IS7XX_ReadRegister() then return the Error
  if (Value != 0xAA) return ERR__SPI_COMM_ERROR;                                         // If the read back value is not the same as the one writen, return an error
  return ERR_OK;
}





//**********************************************************************************************************************************************************
#ifdef SC16IS7XX_I2C_DEFINED
//=============================================================================
// Is the SC16IS7XX device ready
//=============================================================================
bool SC16IS7XX_IsReady(SC16IS7XX *pComp)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return false;
  if (pComp->fnI2C_Transfer == NULL) return false;
#endif
  uint8_t ChipAddrW = pComp->I2Caddress & SC16IS7XX_I2C_WRITE;
  return (pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrW, NULL, 0, true, true) == ERR_OK); // Send only the chip address and get the Ack flag
}
#endif





//=============================================================================
// Read data from the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_ReadData(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, const uint8_t address, uint8_t *data, uint8_t size)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size == 0) return ERR_OK;
  eERRORRESULT Error = ERR_OK;
  uint8_t Address = SC16IS7XX_CHANNEL_SET(channel) | SC16IS7XX_ADDRESS_SET(address);

#ifdef SC16IS7XX_I2C_DEFINED
  if (pComp->Interface == SC16IS7XX_INTERFACE_I2C)
  {
    uint8_t ChipAddrW = ((pComp->I2Caddress & SC16IS7XX_CHIPADDRESS_MASK) & SC16IS7XX_I2C_WRITE);
    uint8_t ChipAddrR = (ChipAddrW | SC16IS7XX_I2C_READ);

    //--- Send the address ---
    Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrW, &Address, sizeof(uint8_t), true, false); // Transfer the address
    if (Error == ERR__I2C_NACK) return ERR__NOT_READY;                                                        // If the device receive a NAK, then the device is not ready
    if (Error != ERR_OK) return Error;                                                                        // If there is an error while calling fnI2C_Transfer() then return the Error
    //--- Get the data ---
    Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrR, data, size, true, true);                 // Restart at first data read transfer, get the data and stop transfer at last byte
  }
#endif
#ifdef SC16IS7XX_SPI_DEFINED
  if (pComp->Interface == SC16IS7XX_INTERFACE_SPI)
  {
    Address |= SC16IS7XX_SPI_READ;

    //--- Send the address ---
    Error = pComp->fnSPI_Transfer(pComp->InterfaceDevice, pComp->SPI_ChipSelect, &Address, NULL, sizeof(uint8_t), false); // Transfer the address
    if (Error != ERR_OK) return Error;                                                                                    // If there is an error while calling fnI2C_Transfer() then return the Error
    //--- Get the data ---
    Error = pComp->fnSPI_Transfer(pComp->InterfaceDevice, pComp->SPI_ChipSelect, data, data, size, true);                 // Get the data and stop transfer at last byte
  }
#endif
  return Error;
}



//=============================================================================
// Write data to the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_WriteData(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, const uint8_t address, uint8_t *data, uint8_t size)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (size == 0) return ERR_OK;
  eERRORRESULT Error = ERR_OK;
  uint8_t Address = SC16IS7XX_CHANNEL_SET(channel) | SC16IS7XX_ADDRESS_SET(address) | SC16IS7XX_SPI_WRITE;

#ifdef SC16IS7XX_I2C_DEFINED
  if (pComp->Interface == SC16IS7XX_INTERFACE_I2C)
  {
    uint8_t ChipAddrW = ((pComp->I2Caddress & SC16IS7XX_CHIPADDRESS_MASK) & SC16IS7XX_I2C_WRITE);

    //--- Send the address ---
    Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrW, &Address, sizeof(uint8_t), true, false); // Transfer the address
    if (Error == ERR__I2C_NACK) return ERR__NOT_READY;                                                        // If the device receive a NAK, then the device is not ready
    if (Error == ERR__I2C_NACK_DATA) return ERR__I2C_INVALID_ADDRESS;                                         // If the device receive a NAK while transferring data, then this is an invalid address
    if (Error != ERR_OK) return Error;                                                                        // If there is an error while calling fnI2C_Transfer() then return the Error
    //--- Send the data ---
    Error = pComp->fnI2C_Transfer(pComp->InterfaceDevice, ChipAddrW, data, size, false, true);                // Continue by transfering the data, and stop transfer at last byte
  }
#endif
#ifdef SC16IS7XX_SPI_DEFINED
  if (pComp->Interface == SC16IS7XX_INTERFACE_SPI)
  {
    //--- Send the address ---
    Error = pComp->fnSPI_Transfer(pComp->InterfaceDevice, pComp->SPI_ChipSelect, &Address, NULL, sizeof(uint8_t), false); // Transfer the address
    if (Error != ERR_OK) return Error;                                                                                    // If there is an error while calling fnI2C_Transfer() then return the Error
    //--- Send the data ---
    Error = pComp->fnSPI_Transfer(pComp->InterfaceDevice, pComp->SPI_ChipSelect, data, NULL, size, true);                 // Send the data and stop transfer at last byte
  }
#endif
  return Error;
}



//=============================================================================
// Modify a register of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_ModifyRegister(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, const uint8_t registerAddr, uint8_t registerValue, uint8_t registerMask)
{
  eERRORRESULT Error;
  uint8_t RegValue;
  Error = SC16IS7XX_ReadRegister(pComp, channel, registerAddr, &RegValue); // Read the register value
  if (Error != ERR_OK) return Error;                                       // If there is an error while calling SC16IS7XX_ReadRegister() then return the error
  RegValue &= ~registerMask;                                               // Clear bits to modify
  RegValue |= (registerValue & registerMask);                              // Set the new value
  return SC16IS7XX_WriteRegister(pComp, channel, registerAddr, RegValue);  // Write the value to register
}



//=============================================================================
// Set register access of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_SetRegisterAccess(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, uint8_t setAccessTo, uint8_t *originalLCRregValue)
{
  eERRORRESULT Error;
  Error = SC16IS7XX_ReadRegister(pComp, channel, RegSC16IS7XX_LCR, originalLCRregValue); // Read the LCR register
  if (Error != ERR_OK) return Error;                                                     // If there is an error while calling SC16IS7XX_ReadRegister() then return the error
  return SC16IS7XX_WriteRegister(pComp, channel, RegSC16IS7XX_LCR, setAccessTo);         // Write the LCR register
}



//=============================================================================
// Return access to general registers of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_ReturnAccessToGeneralRegister(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, uint8_t *originalLCRregValue)
{
  *originalLCRregValue &= SC16IS7XX_LCR_VALUE_SET_GENERAL_REGISTER;                       // Force access to general registers
  return SC16IS7XX_WriteRegister(pComp, channel, RegSC16IS7XX_LCR, *originalLCRregValue); // Write the LCR register
}





//**********************************************************************************************************************************************************
//=============================================================================
// Enable Enhanced Functions of the SC16IS7XX device
//=============================================================================
eERRORRESULT SC16IS7XX_EnableEnhancedFunctions(SC16IS7XX *pComp, eSC16IS7XX_Channel channel)
{
  eERRORRESULT Error;

  //--- Enable access to enhanced registers ---
  SC16IS7XX_LCR_Register OriginalLCR;
  Error = SC16IS7XX_SetRegisterAccess(pComp, channel, SC16IS7XX_LCR_VALUE_SET_ENHANCED_FEATURE_REGISTER, &OriginalLCR.LCR);
  if (Error != ERR_OK) return Error;                                                  // If there is an error while calling SC16IS7XX_SetRegisterAccess() then return the error

  //--- Enable Enhanced Functions ---
  Error = SC16IS7XX_ModifyRegister(pComp, channel, RegSC16IS7XX_EFR, SC16IS7XX_EFR_ENHANCED_FUNCTION_ENABLE, SC16IS7XX_EFR_ENHANCED_FUNCTION_Mask);
  if (Error != ERR_OK) return Error;                                                  // If there is an error while calling SC16IS7XX_ReadRegister() then return the error

  //--- Return access to general registers ---
  Error = SC16IS7XX_ReturnAccessToGeneralRegister(pComp, channel, &OriginalLCR.LCR);  // Return access to general registers
  if (Error != ERR_OK) return Error;                                                  // If there is an error while calling SC16IS7XX_ReturnAccessToGeneralRegister() then return the error

  //--- Enable TCR and TLR ---
  return SC16IS7XX_ModifyRegister(pComp, channel, RegSC16IS7XX_MCR, SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_ENABLE, SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_Mask);
}





//**********************************************************************************************************************************************************
//=============================================================================
// Verify if the SC16IS7XX device has sleep mode enabled
//=============================================================================
eERRORRESULT SC16IS7XX_IsDeviceInSleepMode(SC16IS7XX *pComp, bool* isSleepModeEnable)
{
#ifdef CHECK_NULL_PARAM
  if (isSleepModeEnable == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;
  SC16IS7XX_IER_Register RegIER;
  Error = SC16IS7XX_ReadRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IER, &RegIER.IER); // Read the IER register
  if (Error != ERR_OK) return Error;                                                          // If there is an error while calling SC16IS7XX_ReadRegister() then return the error
  *isSleepModeEnable = ((RegIER.IER & SC16IS7XX_IER_SLEEP_MODE_ENABLE) > 0);
  return ERR_OK;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Configure GPIOs of the SC16IS7XX device
//=============================================================================
eERRORRESULT SC16IS7XX_ConfigureGPIOs(SC16IS7XX *pComp, uint8_t pinsDirection, uint8_t pinsLevel, uint8_t pinsInterruptEnable)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error;
  if (SC16IS7XX_LIMITS[pComp->DevicePN].HAVE_GPIO == false) return ERR__NOT_SUPPORTED;                     // Only if the device have I/O pins

  Error = SC16IS7XX_WriteRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IODir, pinsDirection);         // Write the IODir register
  if (Error != ERR_OK) return Error;                                                                       // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
  Error = SC16IS7XX_WriteRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IOState, pinsLevel);           // Write the IOState register
  if (Error != ERR_OK) return Error;                                                                       // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
  pComp->GPIOsOutState = pinsLevel;                                                                        // Save setted GPIOs state level
  return SC16IS7XX_WriteRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IOIntEna, pinsInterruptEnable); // Write the IOIntEna register
}



//=============================================================================
// Set I/O pins direction of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_SetGPIOPinsDirection(SC16IS7XX *pComp, uint8_t pinsDirection, uint8_t pinsChangeMask)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (SC16IS7XX_LIMITS[pComp->DevicePN].HAVE_GPIO == false) return ERR__NOT_SUPPORTED;      // Only if the device have I/O pins
  return SC16IS7XX_ModifyRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IODir, pinsDirection, pinsChangeMask);
}



//=============================================================================
// Get I/O pins input level of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_GetGPIOPinsInputLevel(SC16IS7XX *pComp, uint8_t *pinsState)
{
#ifdef CHECK_NULL_PARAM
  if ((pComp == NULL) || (pinsState == NULL)) return ERR__PARAMETER_ERROR;
#endif
  if (SC16IS7XX_LIMITS[pComp->DevicePN].HAVE_GPIO == false) return ERR__NOT_SUPPORTED;         // Only if the device have I/O pins

  return SC16IS7XX_ReadRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IOState, pinsState); // Read the IOState register
}



//=============================================================================
// Set I/O pins output level of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_SetGPIOPinsOutputLevel(SC16IS7XX *pComp, uint8_t pinsLevel, uint8_t pinsChangeMask)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (SC16IS7XX_LIMITS[pComp->DevicePN].HAVE_GPIO == false) return ERR__NOT_SUPPORTED;                     // Only if the device have I/O pins

  pComp->GPIOsOutState &= ~pinsChangeMask;                                                                 // Force change bits to 0
  pComp->GPIOsOutState |= (pinsLevel & pinsChangeMask);                                                    // Apply new output level only on changed pins
  return SC16IS7XX_WriteRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IOState, pComp->GPIOsOutState); // Write the IOState register
}



//=============================================================================
// Set I/O pins interrupt enable of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_SetGPIOPinsInterruptEnable(SC16IS7XX *pComp, uint8_t pinsIntEna, uint8_t pinsChangeMask)
{
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (SC16IS7XX_LIMITS[pComp->DevicePN].HAVE_GPIO == false) return ERR__NOT_SUPPORTED;         // Only if the device have I/O pins
  return SC16IS7XX_ModifyRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IOIntEna, pinsIntEna, pinsChangeMask);
}





//**********************************************************************************************************************************************************
//=============================================================================
// SC16IS7XX UART initialization
//=============================================================================
eERRORRESULT SC16IS7XX_InitUART(SC16IS7XX_UART *pUART, const SC16IS7XX_UARTconfig *pUARTConf)
{
#ifdef CHECK_NULL_PARAM
  if ((pUART == NULL) || (pUARTConf == NULL)) return SCR_PARAMETERERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  SC16IS7XX_UARTconfig *pConf = (SC16IS7XX_UARTconfig*) pUARTConf;
  eERRORRESULT Error;

  //--- Check the UART channel ------------------------------
  if (pUART->Channel >= SC16IS7XX_CHANNEL_COUNT) return ERR__UNKNOWN_ELEMENT;
  if ((pUART->Channel == SC16IS7XX_CHANNEL_B) && (SC16IS7XX_LIMITS[pComp->DevicePN].HAVE_2_UARTS == false)) return ERR__UNKNOWN_ELEMENT;

  //--- Enable Enhanced Functions ---------------------------
  Error = SC16IS7XX_EnableEnhancedFunctions(pComp, pUART->Channel);    // Enable the enhanced function of the UART channel
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_EnableEnhancedFunctions() then return the error

  //--- Disable Interrupts, Tx, Rx and clear FIFOs ----------
  SC16IS7XX_IER_Register OriginalIER;
  Error = SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_IER, &OriginalIER.IER); // Read and save configuration of the IER register
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_ReadRegister() then return the error
  Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_IER, 0x00); // Disable all interrupts. And with this, the device sleep will be disabled
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
  Error = SC16IS7XX_TxRxDisable(pUART, true, true);                    // Disable Transmitter and receiver
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_TxRxDisable() then return the error
  Error = SC16IS7XX_ResetFIFO(pUART, true, true);                      // Reset and clear Transmit and receive FIFOs
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_ResetFIFO() then return the error

  //--- Disable control flow --------------------------------
  Error = __SC16IS7XX_SetControlFlowConfiguration(pUART, NULL, NULL, false); // Disable the control flow
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling __SC16IS7XX_SetControlFlowConfiguration() then return the error

  //--- Set UART configuration ------------------------------
  Error = __SC16IS7XX_SetUARTConfiguration(pUART, pUARTConf);          // Configure UART
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_SetUARTConfiguration() then return the error

  //--- Set Baudrate ----------------------------------------
  Error = SC16IS7XX_SetUARTBaudRate(pUART, pUARTConf);                 // Configure UART baudrate
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_SetUARTBaudRate() then return the error

  //--- Set FIFO configuration ------------------------------
  Error = SC16IS7XX_ConfigureFIFOs(pUART, pUARTConf->UseFIFOs, pUARTConf->TxTrigLvl, pUARTConf->RxTrigLvl); // Configure FIFOs
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_ConfigureFIFOs() then return the error

  //--- Return to original interrupt configuration ----------
  Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_IER, OriginalIER.IER); // Re-enable previous interrupts with the sleep state if previously set
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_WriteRegister() then return the error

  //--- Enable Tx and/or Rx configuration -------------------
  Error = SC16IS7XX_TxRxDisable(pUART, pUARTConf->DisableTransmitter, pUARTConf->DisableReceiver); // Disable Transmitter and receiver
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_TxRxDisable() then return the error

  //--- Test UART connection --------------------------------
  Error = SC16IS7XX_UARTCommTest(pUART);                               // Test the UART before Control Flow because the Control Flow may interfere with the test
  if (Error != ERR_OK) return Error;                                   // If there is an error while calling SC16IS7XX_UARTCommTest() then return the error

  //--- Set Control Flow ------------------------------------
  SC16IS7XX_HardControlFlow* pHardFlow = NULL;
  SC16IS7XX_SoftControlFlow* pSoftFlow = NULL;
  bool UseAddressChar = false;
  switch (pUARTConf->UARTtype)
  {
    case SC16IS7XX_UART_RS232:
      switch (pUARTConf->RS232.ControlFlowType)
      {
        case SC16IS7XX_NO_CONTROL_FLOW:
        default: break;
        case SC16IS7XX_HARDWARE_CONTROL_FLOW: pHardFlow = &pConf->RS232.HardFlowControl; break;
        case SC16IS7XX_SOFTWARE_CONTROL_FLOW: pSoftFlow = &pConf->RS232.SoftFlowControl; break;
      }
      break;
    case SC16IS7XX_UART_RS485:
      if (pUARTConf->RS485.UseHardwareControlFlow) pHardFlow = &pConf->RS485.HardFlowControl;
      if (pUARTConf->RS485.AutoRS485mode == SC16IS7XX_AUTO_ADDRESS_DETECT) UseAddressChar = true;
      break;
    case SC16IS7XX_UART_IrDA:
      if (pUARTConf->IrDA.UseSoftwareControlFlow)  pSoftFlow = &pConf->IrDA.SoftFlowControl;
      break;
    case SC16IS7XX_UART_Modem:
      if (pUARTConf->Modem.UseHardwareControlFlow) pHardFlow = &pConf->Modem.HardFlowControl;
      break;
    default: break;
  }
  return __SC16IS7XX_SetControlFlowConfiguration(pUART, pHardFlow, pSoftFlow, UseAddressChar); // Configure the control flow
}



//=============================================================================
// UART communication tests of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_UARTCommTest(SC16IS7XX_UART *pUART)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  eERRORRESULT Error;
  setSC16IS7XX_ReceiveError CharError;
  uint8_t Value;

  //--- Set UART in loopback mode ---
  SC16IS7XX_MCR_Register RegMCR;
  Error = SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_MCR, &RegMCR.MCR); // Read the MCR register
  if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_ReadRegister() then return the error
  RegMCR.MCR |= SC16IS7XX_MCR_LOOPBACK_ENABLE;                                          // Set enable local Loopback mode (internal)
  Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_MCR, RegMCR.MCR); // Write the MCR register
  if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_WriteRegister() then return the error

  //--- Reset UART FIFOs ---
  Error = SC16IS7XX_ResetFIFO(pUART, true, true);                                       // Reset both Transmit and Receive FIFOs
  if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_ResetFIFO() then return the error

  //--- Test UART communication ---
  Error = SC16IS7XX_TransmitChar(pUART, 0x55);                                          // Transmit char 0x55 by UART
  if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_TransmitCharByUART() then return the error
  Error = SC16IS7XX_ReceiveChar(pUART, &Value, &CharError);                             // Receive a char UART by UART
  if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_ReceiveCharByUART() then return the error
  if (CharError != SC16IS7XX_NO_RX_ERROR) return ERR__PERIPHERAL_NOT_VALID;             // If the received data create an error then the peripheral is not valid
  if (Value != 0x55) return ERR__PERIPHERAL_NOT_VALID;                                  // If the read back value is not the same as the one writen, return an error

  Error = SC16IS7XX_TransmitChar(pUART, 0xAA);                                          // Transmit char 0xAA by UART
  if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_TransmitCharByUART() then return the error
  Error = SC16IS7XX_ReceiveChar(pUART, &Value, &CharError);                             // Receive a char UART by UART
  if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_ReceiveCharByUART() then return the error
  if (CharError != SC16IS7XX_NO_RX_ERROR) return ERR__PERIPHERAL_NOT_VALID;             // If the received data create an error then the peripheral is not valid
  if (Value != 0xAA) return ERR__PERIPHERAL_NOT_VALID;                                  // If the read back value is not the same as the one writen, return an error

  RegMCR.MCR &= SC16IS7XX_MCR_NORMAL_OPERATING_MODE;                                    // Set normal operating mode
  return SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_MCR, RegMCR.MCR);  // Write the MCR register
}



//=============================================================================
// Set UART configuration of the SC16IS7XX
//=============================================================================
// DO NOT USE DIRECTLY, use SC16IS7XX_InitUART() instead! UART configuration needs to be configured with a safe UART configuration to avoid spurious effects, which is done in the SC16IS7XX_InitUART() function
eERRORRESULT __SC16IS7XX_SetUARTConfiguration(SC16IS7XX_UART *pUART, const SC16IS7XX_UARTconfig *pUARTConf)
{
#ifdef CHECK_NULL_PARAM
  if ((pUART == NULL) || (pUARTConf == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  eERRORRESULT Error, ErrorReturn = 0;
  SC16IS7XX_MCR_Register RegMCR;
  SC16IS7XX_EFCR_Register RegEFCR;
  SC16IS7XX_IOControl_Register RegIOC;
  SC16IS7XX_EFR_Register RegEFR;
  uint8_t RegIOCmask = 0x00;
  RegEFCR.EFCR = SC16IS7XX_EFCR_IrDA_3_16_PULSE_RATIO | SC16IS7XX_EFCR_9BIT_MODE_DISABLE // Default configuration
               | SC16IS7XX_EFCR_TX_NOT_CONTROL_RTS | SC16IS7XX_EFCR_NORMAL_RTS_PIN;

  //--- Select the IO control mask ---
  RegIOC.IOControl = SC16IS7XX_IOCTRL_GPIO7_4_AS_IO | SC16IS7XX_IOCTRL_GPIO3_0_AS_IO;
  switch (pUART->Channel)
  {
    case SC16IS7XX_CHANNEL_A: RegIOCmask = SC16IS7XX_IOCTRL_UARTA_MODEM_MODE_Mask; break;
    case SC16IS7XX_CHANNEL_B: RegIOCmask = SC16IS7XX_IOCTRL_UARTB_MODEM_MODE_Mask; break;
    default: return ERR__UNKNOWN_ELEMENT;
  }

  //--- Configure the UART type ---
  switch (pUARTConf->UARTtype)
  {
    case SC16IS7XX_UART_RS232:
      RegMCR.MCR = SC16IS7XX_MCR_NORMAL_UART_MODE;
      break;

    case SC16IS7XX_UART_RS485:
      RegMCR.MCR = SC16IS7XX_MCR_NORMAL_UART_MODE;
      switch (pUARTConf->RS485.RTScontrol)
      {
        //--- Configure RTS mode ---
        case SC16IS7XX_RS485_AUTO_RTS:
          if (pUARTConf->RS485.UseHardwareControlFlow) return ERR__CONFIGURATION;          // If auto RTS mode then the hardware control flow shall not be used
          RegEFCR.EFCR |= SC16IS7XX_EFCR_TX_CONTROL_RTS;                                   // Set automatic RTS mode
          break;
        case SC16IS7XX_RS485_HARD_FLOW_CONTROL_RTS:
          if (pUARTConf->RS485.UseHardwareControlFlow == false) return ERR__CONFIGURATION; // If hardware control flow RTS mode then the hardware control flow shall be used
          break;
        case SC16IS7XX_RS485_MANUAL_EXTERNAL_RTS:
          if (pUARTConf->RS485.UseHardwareControlFlow) return ERR__CONFIGURATION;          // If manual/external RTS mode then the hardware control flow shall not be used
          break;
        default: break;
      }
      if (pUARTConf->RS485.RTSoutInversion) RegEFCR.EFCR |= SC16IS7XX_EFCR_INVERT_RTS_PIN; // Invert RTS output if configured
      //--- Configure RS-485 mode ---
      switch (pUARTConf->RS485.AutoRS485mode)
      {
        case SC16IS7XX_NO_AUTO_RS485_MODE: break;
        case SC16IS7XX_MULTIDROP_MODE: RegEFCR.EFCR |= SC16IS7XX_EFCR_9BIT_MODE_ENABLE; break;
        case SC16IS7XX_AUTO_ADDRESS_DETECT:
          RegEFCR.EFCR |= SC16IS7XX_EFCR_9BIT_MODE_ENABLE;
          //--- Enable access to enhanced registers ---
          SC16IS7XX_LCR_Register OriginalLCR;
          Error = SC16IS7XX_SetRegisterAccess(pComp, pUART->Channel, SC16IS7XX_LCR_VALUE_SET_ENHANCED_FEATURE_REGISTER, &OriginalLCR.LCR);
          if (Error != ERR_OK) return Error;                                                                        // If there is an error while calling SC16IS7XX_SetRegisterAccess() then return the error
          //--- Configure the auto address detect ---
          RegEFR.EFR = SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_ENABLE;
          SC16IS7XX_ModifyRegister(pComp, pUART->Channel, RegSC16IS7XX_EFR, RegEFR.EFR, SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_Mask); // Modify the EFR register
          if (Error != ERR_OK) ErrorReturn = Error;                                                                 // If there is an error while calling SC16IS7XX_ModifyRegister() then return the error
          Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_XOFF2, pUARTConf->RS485.AddressChar); // Write the Address char in Xoff2 register
          if (Error != ERR_OK) ErrorReturn = Error;                                                                 // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
          //--- Return access to general registers ---
          Error = SC16IS7XX_ReturnAccessToGeneralRegister(pComp, pUART->Channel, &OriginalLCR.LCR);                 // Return access to general registers
          if (ErrorReturn != ERR_OK) return ErrorReturn;                                                            // If there is an error while Enhanced Features are enabled then return the error
          if (Error != ERR_OK) return Error;                                                                        // If there is an error while calling SC16IS7XX_ReturnAccessToGeneralRegister() then return the error
          break;
        default: return ERR__CONFIGURATION;
      }
      break;

    case SC16IS7XX_UART_IrDA:
      RegMCR.MCR = SC16IS7XX_MCR_IrDA_MODE;                         // Set IrDA mode
      RegEFCR.EFCR = SC16IS7XX_EFCR_IrDA_3_16_PULSE_RATIO;          // By default, set IrDA 3/16 pulse ratio
      if (pUARTConf->IrDA.IrDAmode == SC16IS7XX_IrDA_SIR_1_4_RATIO) // If IrDA mode is 1/4 ratio
      {
        if (SC16IS7XX_LIMITS[pComp->DevicePN].IrDA_1_4_RATIO)       // If 1/4 ratio mode supported
             RegEFCR.EFCR = SC16IS7XX_EFCR_IrDA_1_4_PULSE_RATIO;    // Then set the 1/4 ratio configuration
        else return ERR__NOT_SUPPORTED;                             // Else return an error
      }
      break;

    case SC16IS7XX_UART_Modem:
      RegMCR.MCR = SC16IS7XX_MCR_NORMAL_UART_MODE;
      switch (pUART->Channel)
      {
        case SC16IS7XX_CHANNEL_A: RegIOC.IOControl = SC16IS7XX_IOCTRL_GPIO7_4_AS_MODEM; break;
        case SC16IS7XX_CHANNEL_B: RegIOC.IOControl = SC16IS7XX_IOCTRL_GPIO3_0_AS_MODEM; break;
        default: return ERR__UNKNOWN_ELEMENT;
      }
      break;

    default: return ERR__UNKNOWN_ELEMENT;
  }

  //--- Now apply parameters to registers ---
  SC16IS7XX_ModifyRegister(pComp, pUART->Channel, RegSC16IS7XX_MCR, RegMCR.MCR, SC16IS7XX_MCR_IrDA_MODE_Mask); // Modify the LCR register
  if (Error != ERR_OK) return Error;                                                                           // If there is an error while calling SC16IS7XX_ModifyRegister() then return the error
  SC16IS7XX_ModifyRegister(pComp, pUART->Channel, RegSC16IS7XX_EFCR, RegEFCR.EFCR, SC16IS7XX_EFCR_LINE_CONTROL_MODE_Mask); // Modify the EFCR register
  if (Error != ERR_OK) return Error;                                                                           // If there is an error while calling SC16IS7XX_ModifyRegister() then return the error
  SC16IS7XX_ModifyRegister(pComp, pUART->Channel, RegSC16IS7XX_MCR, RegIOC.IOControl, RegIOCmask);             // Modify the IOControl register
  if (Error != ERR_OK) return Error;                                                                           // If there is an error while calling SC16IS7XX_ModifyRegister() then return the error

  //--- Configuration of the data communication format ---
  SC16IS7XX_LCR_Register RegLCR;
  RegLCR.LCR = SC16IS7XX_LCR_NO_BREAK_CONDITION | SC16IS7XX_LCR_DIVISOR_LATCH_DISABLE
             | SC16IS7XX_LCR_DATA_LENGTH_SET(pUARTConf->UARTwordLen)                                                                 // UART word length
             | (pUARTConf->UARTstopBit != SC16IS7XX_STOP_BIT_1bit ? SC16IS7XX_LCR_EXTENDED_STOP_BIT : SC16IS7XX_LCR_ONLY_1_STOP_BIT) // UART stop bit
             | SC16IS7XX_LCR_PARITY_SET(pUARTConf->UARTparity);                                                                      // UART parity
  return SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_LCR, RegLCR.LCR);                                               // Write the LCR register
}



//=============================================================================
// Set UART baudrate of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_SetUARTBaudRate(SC16IS7XX_UART *pUART, const SC16IS7XX_UARTconfig *pUARTConf)
{
#ifdef CHECK_NULL_PARAM
  if ((pUART == NULL) || (pUARTConf == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  eERRORRESULT Error;

  //--- Check that the device is not in sleep mode ---
  bool DeviceInSleepMode;
  Error = SC16IS7XX_IsDeviceInSleepMode(pComp, &DeviceInSleepMode); // Ask if device is in sleep mode
  if (Error != ERR_OK) return Error;                                // If there is an error while calling SC16IS7XX_IsDeviceInSleepMode() then return the error
  if (DeviceInSleepMode) return ERR__NOT_IN_SLEEP_MODE;             // To perform the set baudrate operation, the device should not be in sleep mode

  //--- Limits tests ---
  if ((pComp->XtalFreq != 0) && (pComp->XtalFreq < SC16IS7XX_FREQ_MIN     )) return ERR__FREQUENCY_ERROR; // The device crystal should not be < 1.6kHz (100 bauds min)
  if ((pComp->XtalFreq != 0) && (pComp->XtalFreq > SC16IS7XX_XTAL_FREQ_MAX)) return ERR__FREQUENCY_ERROR; // The device crystal should not be > 24MHz
  if ((pComp->OscFreq  != 0) && (pComp->OscFreq  < SC16IS7XX_FREQ_MIN     )) return ERR__FREQUENCY_ERROR; // The device oscillator should not be < 1.6kMHz (100 bauds min)
  if ((pComp->OscFreq  != 0) && (pComp->OscFreq  > SC16IS7XX_OSC_FREQ_MAX )) return ERR__FREQUENCY_ERROR; // The device oscillator should not be > 80MHz
  uint32_t CompFreq = 0;
  if (pComp->XtalFreq != 0) CompFreq = pComp->XtalFreq; else CompFreq = pComp->OscFreq;                   // Select the device frequency
  if (CompFreq == 0) return ERR__FREQUENCY_ERROR;                                                         // Both XtalFreq and OscFreq are configured to 0
  if (pUARTConf->UARTbaudrate < SC16IS7XX_BAUDRATE_MIN) return ERR__BAUDRATE_ERROR;                       // The UART baudrate should not be < 100 bauds
  if (pUARTConf->UARTbaudrate > SC16IS7XX_BAUDRATE_MAX) return ERR__BAUDRATE_ERROR;                       // In All modes the baudrate max is 5Mbauds
  if (pUARTConf->UARTtype == SC16IS7XX_UART_IrDA)                                                         // If the device is in IrDA mode
  {
    if (pUARTConf->IrDA.IrDAmode == SC16IS7XX_IrDA_SIR_1_4_RATIO)
    {
      if (SC16IS7XX_LIMITS[pComp->DevicePN].IrDA_1_4_RATIO)
      {
        if (pUARTConf->UARTbaudrate > SC16IS76X_IrDA_SPEED_MAX) return ERR__BAUDRATE_ERROR;  // The UART baudrate should not be > max IrDA baudrate (SC16IS76X only)
      }
      else return ERR__NOT_SUPPORTED;                                                        // IrDA SIR 1/4 ratio not supported on SC16IS7XX whot are not SC16IS76X
    }
    else if (pUARTConf->UARTbaudrate > SC16IS7XX_IrDA_SPEED_MAX) return ERR__BAUDRATE_ERROR; // The UART baudrate should not be > max IrDA baudrate
  }

  //--- Calculate Divisor with Prescaler = 1 and its error ---
  uint32_t DivisorPrescaler1 = (uint32_t)(((float)CompFreq / (float)pUARTConf->UARTbaudrate * 16.0f * 1.0f) + 0.5f);            // (ComponentXtalFreq/1)/(DesiredBaudRate*16)+0.5 for rounding
  if (DivisorPrescaler1 < 0x0001) DivisorPrescaler1 =     1;                                                                    // Cannot be < 1
  if (DivisorPrescaler1 > 0xFFFF) DivisorPrescaler1 = 65535;                                                                    // Cannot be > 65535
  float BaudRatePrescaler1 = (float)CompFreq / ((float)DivisorPrescaler1 * 16.0f * 1.0f);                                       // Get actual baudrate with prescaler set to 1
  float ErrorPrescaler1    = ((BaudRatePrescaler1 - (float)pUARTConf->UARTbaudrate) * 100000) / (float)pUARTConf->UARTbaudrate; // Calculate error with prescaler set to 1
  //--- Calculate Divisor with Prescaler = 4 and its error ---
  uint32_t DivisorPrescaler4 = (uint32_t)(((float)CompFreq / (float)pUARTConf->UARTbaudrate * 16.0f * 4.0f) + 0.5f);            // (ComponentXtalFreq/4)/(DesiredBaudRate*16)+0.5 for rounding
  if (DivisorPrescaler4 < 0x0001) DivisorPrescaler4 =     1;                                                                    // Cannot be < 1
  if (DivisorPrescaler4 > 0xFFFF) DivisorPrescaler4 = 65535;                                                                    // Cannot be > 65535
  float BaudRatePrescaler4 = (float)CompFreq / ((float)DivisorPrescaler4 * 16.0f * 4.0f);                                       // Get actual baudrate with prescaler set to 4
  float ErrorPrescaler4    = ((BaudRatePrescaler4 - (float)pUARTConf->UARTbaudrate) * 100000) / (float)pUARTConf->UARTbaudrate; // Calculate error with prescaler set to 4

  //--- Configure clock divisor ---
  uint32_t DivPresToSet;
  uint8_t RegValue = 0;
  if (SC16IS7XX_ABSOLUTE(ErrorPrescaler1) < SC16IS7XX_ABSOLUTE(ErrorPrescaler4))
  {                                                                                 // Set prescaler to 1 and set divisor for prescaler 1
    *(pUARTConf->UARTbaudrateError) = (int32_t)ErrorPrescaler1;                     // Divide UARTbaudrateError by 1000 to get the real error
    RegValue = SC16IS7XX_MCR_CLOCK_INPUT_DIVIDE_BY_1;                               // Set divide-by-1 clock input
    DivPresToSet = DivisorPrescaler1;
  }
  else                                                                              // Set prescaler to 4 and set divisor for prescaler 4
  {
    *(pUARTConf->UARTbaudrateError) = (int32_t)ErrorPrescaler4;                     // Divide UARTbaudrateError by 1000 to get the real error
    RegValue = SC16IS7XX_MCR_CLOCK_INPUT_DIVIDE_BY_4;                               // Set divide-by-4 clock input
    DivPresToSet = DivisorPrescaler4;
  }
  Error = SC16IS7XX_ModifyRegister(pComp, pUART->Channel, RegSC16IS7XX_MCR, RegValue, SC16IS7XX_MCR_CLOCK_INPUT_DIVIDE_Mask);
  if (Error != ERR_OK) return Error;                                                // If there is an error while calling SC16IS7XX_WriteRegister() then return the error

  //--- Set baudrate divisors ---
  SC16IS7XX_LCR_Register OriginalLCR;
  Error = SC16IS7XX_SetRegisterAccess(pComp, pUART->Channel, SC16IS7XX_LCR_VALUE_SET_SPECIAL_REGISTER, &OriginalLCR.LCR);
  if (Error != ERR_OK) return Error;                                                // If there is an error while calling SC16IS7XX_SetRegisterAccess() then return the error

  Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_DLL, (uint8_t)(DivPresToSet & 0xFF)); // Write the DLL register
  if (Error != ERR_OK) return Error;                                                                        // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
  Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_DLH, (uint8_t)(DivPresToSet >> 8));   // Write the DLH register
  if (Error != ERR_OK) return Error;                                                                        // If there is an error while calling SC16IS7XX_WriteRegister() then return the error

  //--- Return access to general registers ---
  return SC16IS7XX_ReturnAccessToGeneralRegister(pComp, pUART->Channel, &OriginalLCR.LCR); // Return access to general registers
}



//=============================================================================
// Configure an UART Control Flow of the SC16IS7XX
//=============================================================================
// DO NOT USE DIRECTLY, use SC16IS7XX_InitUART() instead! Control Flow needs to be configured with a safe UART configuration to avoid spurious effects, which is done in the SC16IS7XX_InitUART() function
eERRORRESULT __SC16IS7XX_SetControlFlowConfiguration(SC16IS7XX_UART *pUART, SC16IS7XX_HardControlFlow *pHardFlow, SC16IS7XX_SoftControlFlow *pSoftFlow, bool useAdressChar)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  if ((pHardFlow != NULL) && (pSoftFlow != NULL)) return ERR__CONFIGURATION; // The user should not configure Hardware+Software control flow at the same time
  eERRORRESULT Error, ErrorReturn = ERR_OK;
  SC16IS7XX_EFR_Register RegEFR;
  RegEFR.EFR = SC16IS7XX_EFR_SOFT_FLOW_CONTROL_SET(SC16IS7XX_NoTxCtrlFlow_NoRxCtrlFlow) // Disable All Control Flows but keep Enhanced Functions Enabled
             | SC16IS7XX_EFR_ENHANCED_FUNCTION_ENABLE | SC16IS7XX_EFR_RTS_FLOW_CONTROL_DISABLE | SC16IS7XX_EFR_CTS_FLOW_CONTROL_DISABLE
             | (useAdressChar ? SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_ENABLE : SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_DISABLE);

  //--- Set Trigger Control Level ---
  if ((pHardFlow != NULL) || (pSoftFlow != NULL))
  {
    eSC16IS7XX_TriggerCtrlLevel HoldAt, ResumeAt;
    if (pHardFlow != NULL)
         { HoldAt = pHardFlow->HoldAt; ResumeAt = pHardFlow->ResumeAt; }                  // Select the hardware control flow TCL
    else { HoldAt = pSoftFlow->HoldAt; ResumeAt = pSoftFlow->ResumeAt; }                  // Select the software control flow TCL
    if ((uint8_t)HoldAt <= (uint8_t)ResumeAt) return ERR__CONFIGURATION;                  // The TCR such that TCR[3:0] > TCR[7:4]. There is no built-in hardware check to make sure this condition is met
    SC16IS7XX_TCR_Register RegTCR;
    RegTCR.TCR = SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_SET(HoldAt) | SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_SET(ResumeAt);
    Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_TCR, RegTCR.TCR); // Write the TCR register
    if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
  }

  //--- Enable access to enhanced registers ---
  SC16IS7XX_LCR_Register OriginalLCR;
  Error = SC16IS7XX_SetRegisterAccess(pComp, pUART->Channel, SC16IS7XX_LCR_VALUE_SET_ENHANCED_FEATURE_REGISTER, &OriginalLCR.LCR);
  if (Error != ERR_OK) return Error;                                                      // If there is an error while calling SC16IS7XX_SetRegisterAccess() then return the error

  //--- Configure Control Flow ---
  if (ErrorReturn == ERR_OK)
  {
    if ((pHardFlow != NULL) && (pSoftFlow == NULL))                                                            //*** Hardware Control Flow
    {
      RegEFR.EFR = SC16IS7XX_EFR_SOFT_FLOW_CONTROL_SET(SC16IS7XX_NoTxCtrlFlow_NoRxCtrlFlow)                    // Disable Software Control Flow and keep Enhanced Functions Enabled
                  | SC16IS7XX_EFR_ENHANCED_FUNCTION_ENABLE | SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_DISABLE;
      if (pHardFlow->RTSpinControl == SC16IS7XX_AUTOMATIC_PIN_CONTROL) RegEFR.EFR |= SC16IS7XX_EFR_RTS_FLOW_CONTROL_ENABLE;
      if (pHardFlow->CTSpinControl == SC16IS7XX_AUTOMATIC_PIN_CONTROL) RegEFR.EFR |= SC16IS7XX_EFR_CTS_FLOW_CONTROL_ENABLE;
      if (pHardFlow->UseSpecialCharOnXoff2 || useAdressChar)                                                   // Address char use the special character detect
      {
        RegEFR.EFR |= SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_ENABLE;
        if (useAdressChar == false)                                                                            // Set only if it is a special char and not an address char. The address char is set in the function __SC16IS7XX_SetUARTConfiguration()
        {
          Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_XOFF2, pHardFlow->SpecialChar);  // Write the Xoff2 register
          if (Error != ERR_OK) ErrorReturn = Error;                                                            // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
        }
      }
    }

    if ((pHardFlow == NULL) && (pSoftFlow != NULL))                                                            //*** Software Control Flow
    {
      RegEFR.EFR = SC16IS7XX_EFR_SOFT_FLOW_CONTROL_SET(pSoftFlow->Config)                                      // Set Software Control Flow, keep Enhanced Functions Enabled, and disable Hardware Control Flow
                  | SC16IS7XX_EFR_ENHANCED_FUNCTION_ENABLE | SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_DISABLE
                  | SC16IS7XX_EFR_RTS_FLOW_CONTROL_DISABLE | SC16IS7XX_EFR_CTS_FLOW_CONTROL_DISABLE;
      Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_XON1, pSoftFlow->Xon1);              // Write the Xon1 register
      if (Error != ERR_OK) ErrorReturn = Error;                                                                // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
      Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_XON2, pSoftFlow->Xon2);              // Write the Xon2 register
      if (Error != ERR_OK) ErrorReturn = Error;                                                                // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
      Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_XOFF1, pSoftFlow->Xoff1);            // Write the Xoff1 register
      if (Error != ERR_OK) ErrorReturn = Error;                                                                // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
      Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_XOFF2, pSoftFlow->Xoff2SpecialChar); // Write the Xoff2 register
      if (Error != ERR_OK) ErrorReturn = Error;                                                                // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
      if (pSoftFlow->UseSpecialCharOnXoff2) RegEFR.EFR |= SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_ENABLE;
    }
    Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_EFR, RegEFR.EFR);   // Write the EFR register
    if (Error != ERR_OK) ErrorReturn = Error;                                               // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
  }

  //--- Return access to general registers ---
  Error = SC16IS7XX_ReturnAccessToGeneralRegister(pComp, pUART->Channel, &OriginalLCR.LCR); // Return access to general registers
  if (ErrorReturn != ERR_OK) return ErrorReturn;                                            // If there is an error while Enhanced Features are enabled then return the error
  if (Error != ERR_OK) return Error;                                                        // If there is an error while calling SC16IS7XX_ReturnAccessToGeneralRegister() then return the error

  //--- Set Xon Any if set ---
  uint8_t RegValue = 0;
  if ((pSoftFlow != NULL) && pSoftFlow->XonAnyChar) RegValue = SC16IS7XX_MCR_XON_ANY_FUNCTION_ENABLE; // Enable Xon Any function
  return SC16IS7XX_ModifyRegister(pComp, pUART->Channel, RegSC16IS7XX_MCR, RegValue, SC16IS7XX_MCR_XON_ANY_FUNCTION_Mask);
}





//**********************************************************************************************************************************************************
//=============================================================================
// Configure interrupt of the SC16IS7XX device
//=============================================================================
eERRORRESULT SC16IS7XX_ConfigureFIFOs(SC16IS7XX_UART *pUART, bool useFIFOs, eSC16IS7XX_TriggerCtrlLevel txTrigLvl, eSC16IS7XX_TriggerCtrlLevel rxTrigLvl)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  eERRORRESULT Error;

  //--- FIFO enable ---
  SC16IS7XX_FCR_Register RegFCR;
  RegFCR.FCR = (useFIFOs ? SC16IS7XX_FCR_RX_TX_FIFO_ENABLE : SC16IS7XX_FCR_RX_TX_FIFO_DISABLE); // In this driver we use the TLR register to configure the Tx and Rx Trigger Level, so we let the Trigger Level at default value which is 0x00
  Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_FCR, RegFCR.FCR);         // Write the FCR register
  if (Error != ERR_OK) return Error;                                                            // If there is an error while calling SC16IS7XX_WriteRegister() then return the error

  //--- Set Trigger Level ---
  SC16IS7XX_TLR_Register RegTLR;
  RegTLR.TLR = SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_SET(txTrigLvl) | SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_SET(rxTrigLvl);
  return SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_TLR, RegTLR.TLR);                    // Write the TLR register
}



//=============================================================================
// Reset Rx and/or Tx FIFO of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_ResetFIFO(SC16IS7XX_UART *pUART, bool resetTxFIFO, bool resetRxFIFO)
{
#ifdef CHECK_NULL_PARAM
  if ((pUART == NULL) || (pUARTConf == NULL)) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  eERRORRESULT Error;

  //--- Read the IIR register to get the FIFO enable configuration ---
  SC16IS7XX_IIR_Register RegIIR;
  Error = SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_IIR, &RegIIR.IIR); // Read the IIR register
  if (Error != ERR_OK) return Error;                                                    // If there is an error while calling SC16IS7XX_ReadRegister() then return the error

  //--- Set the reset of FIFOs ---
  SC16IS7XX_FCR_Register RegFCR;                                                                             // We need to fill the FCR register with the previous configuration because the FCR is a write only register
  RegFCR.FCR = ((RegIIR.IIR & SC16IS7XX_IIR_FIFOs_ARE_ENABLE) > 0 ? SC16IS7XX_FCR_RX_TX_FIFO_ENABLE : 0x00); // We get the FIFO enable flag in the IIR register because it mirrors the contents of FCR[0]. In this driver we use the TLR register to configure the Tx and Rx Trigger Level, so we let the Trigger Level at default value which is 0x00
  if (resetRxFIFO) RegFCR.FCR |= SC16IS7XX_FCR_RESET_RX_FIFO;                                                // Clears the contents of the receive FIFO and resets the FIFO level logic (the Receive Shift Register is not cleared or altered). This bit will return to a logic 0 after clearing the FIFO
  if (resetTxFIFO) RegFCR.FCR |= SC16IS7XX_FCR_RESET_TX_FIFO;                                                // Clears the contents of the transmit FIFO and resets the FIFO level logic (the Transmit Shift Register is not cleared or altered). This bit will return to a logic 0 after clearing the FIFO
  return SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_FCR, RegFCR.FCR);                       // Write the FCR register
}





//**********************************************************************************************************************************************************
//=============================================================================
// Enable/disable Transmitter and/or receiver of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_TxRxDisable(SC16IS7XX_UART *pUART, bool disableTx, bool disableRx)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif

  SC16IS7XX_EFCR_Register RegEFCR;
  RegEFCR.EFCR = SC16IS7XX_EFCR_TX_ENABLE | SC16IS7XX_EFCR_RX_ENABLE; // Initial value
  if (disableTx) RegEFCR.EFCR |= SC16IS7XX_EFCR_TX_DISABLE;           // Disable Transmitter
  if (disableRx) RegEFCR.EFCR |= SC16IS7XX_EFCR_RX_DISABLE;           // Disable Receiver
  return SC16IS7XX_ModifyRegister(pComp, pUART->Channel, RegSC16IS7XX_EFCR, RegEFCR.EFCR, SC16IS7XX_EFCR_TX_RX_DISABLE_Mask); // Modify the EFCR register
}





//**********************************************************************************************************************************************************
//=============================================================================
// Configure interrupt of the SC16IS7XX device
//=============================================================================
eERRORRESULT SC16IS7XX_ConfigureInterrupt(SC16IS7XX_UART *pUART, setSC16IS7XX_Interrupts interruptsFlags)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  return SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_IER, (uint8_t)interruptsFlags & SC16IS7XX_INTERRUPTS_FLAGS_MASK); // Write the IER register
}



//=============================================================================
// Get interrupt event of the SC16IS7XX device
//=============================================================================
eERRORRESULT SC16IS7XX_GetInterruptEvents(SC16IS7XX_UART *pUART, eSC16IS7XX_InterruptSource *interruptFlag)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  eERRORRESULT Error;
  SC16IS7XX_IIR_Register RegIIR;

  Error = SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_IIR, &RegIIR.IIR);       // Read the IIR register
  if (Error != ERR_OK) return Error;                                                          // If there is an error while calling SC16IS7XX_ReadRegister() then return the error
  *interruptFlag = (eSC16IS7XX_InterruptSource)SC16IS7XX_IIR_INTERRUT_SOURCE_GET(RegIIR.IIR); // Extract interrupt
  return ERR_OK;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Get available space in the transmit FIFO of the SC16IS7XX device
//=============================================================================
eERRORRESULT SC16IS7XX_GetAvailableSpaceTxFIFO(SC16IS7XX_UART *pUART, uint8_t *availableSpace)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  return SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_TXLVL, availableSpace); // Read the TXLVL register
}



//=============================================================================
// Get number of characters stored in receive FIFO of the SC16IS7XX device
//=============================================================================
eERRORRESULT SC16IS7XX_GetDataCountRxFIFO(SC16IS7XX_UART *pUART, uint8_t *dataCount)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  return SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_RXLVL, dataCount); // Read the RXLVL register
}





//**********************************************************************************************************************************************************
//=============================================================================
// Try to transmit data to UART FIFO of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_TryTransmitData(SC16IS7XX_UART *pUART, uint8_t *data, size_t count, size_t *actuallySent)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  eERRORRESULT Error;

  //--- Get free space on Tx FIFO ---
  uint8_t AvailableSpace;
  Error = SC16IS7XX_GetAvailableSpaceTxFIFO(pUART, &AvailableSpace); // Get how many space there is in the transmit FIFO
  if (Error != ERR_OK) return Error;                                 // If there is an error while calling SC16IS7XX_GetAvailableSpaceTxFIFO() then return the error

  //--- Send data if possible ---
  if ((pUART->DriverConfig & SC16IS7XX_DRIVER_SAFE_TX) > 0)          //*** Safe transmit
  {
    *actuallySent = 0;
    size_t CountToSend = (count > (size_t)AvailableSpace ? AvailableSpace : (size_t)count);
    while (CountToSend > 0)
    {
      Error = SC16IS7XX_WriteRegister(pComp, pUART->Channel, RegSC16IS7XX_THR, *data);
      if (Error != ERR_OK) return Error;                             // If there is an error while calling SC16IS7XX_WriteRegister() then return the error
      (*actuallySent)++;
      data++;
      CountToSend--;
    }
  }
  else                                                               //*** Burst transmit
  {
    *actuallySent = (count > AvailableSpace ? AvailableSpace : count);                        // Set how many data will be actually sent
    return SC16IS7XX_WriteData(pComp, pUART->Channel, RegSC16IS7XX_THR, data, *actuallySent); // Send all possible data at once
  }
  return ERR_OK;
}



//=============================================================================
// Transmit data to UART FIFO of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_TransmitChar(SC16IS7XX_UART *pUART, const uint8_t data)
{
  eERRORRESULT Error;
  uint8_t DataToSend = (uint8_t)data;
  size_t ActuallySent = 0;
  do 
  {
    Error = SC16IS7XX_TryTransmitData(pUART, &DataToSend, 1, &ActuallySent);
    if (Error != ERR_OK) return Error; // If there is an error while calling SC16IS7XX_TryTransmitData() then return the error
  } while (ActuallySent == 0);
  return ERR_OK;
}



//=============================================================================
// Receive available data from UART FIFO of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_ReceiveData(SC16IS7XX_UART *pUART, uint8_t *data, size_t *actuallyReceived, setSC16IS7XX_ReceiveError *lastCharError)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  eERRORRESULT Error;
  SC16IS7XX_LSR_Register RegLSR;

  //--- Get available data count in Rx FIFO ---
  uint8_t AvailableData;
  Error = SC16IS7XX_GetDataCountRxFIFO(pUART, &AvailableData);                                     // Get how many characters there is in the receive FIFO
  if (Error != ERR_OK) return Error;                                                               // If there is an error while calling SC16IS7XX_GetDataCountRxFIFO() then return the error

  //--- Send data if possible ---
  if ((pUART->DriverConfig & SC16IS7XX_DRIVER_SAFE_RX) > 0)                                        //*** Safe receive
  {
    *actuallyReceived = 0;
    size_t CountToGet = (size_t)AvailableData;
    while (CountToGet > 0)
    {
      //--- Verify current char ---
      Error = SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_LSR, &RegLSR.LSR);        // Read the LSR register
      if (Error != ERR_OK) return Error;                                                           // If there is an error while calling SC16IS7XX_ReadRegister() then return the error
      *lastCharError = (setSC16IS7XX_ReceiveError)(RegLSR.LSR & (uint8_t)SC16IS7XX_RX_ERROR_Mask); // Get last received char error
      Error = SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_RHR, data);               // Receive the next char in FIFO
      if (Error != ERR_OK) return Error;                                                           // If there is an error while calling SC16IS7XX_ReadRegister() then return the error
      (*actuallyReceived)++;
      if (*lastCharError != SC16IS7XX_NO_RX_ERROR) return ERR__RECEIVE_ERROR;
      data++;
      CountToGet--;
    }
  }
  else                                                                                           //*** Burst receive
  {
    *actuallyReceived = (size_t)AvailableData;                                                   // Set how many data will be actually received
    return SC16IS7XX_ReadData(pComp, pUART->Channel, RegSC16IS7XX_RHR, data, *actuallyReceived); // Receive all possible data at once
  }
  return ERR_OK;
}



//=============================================================================
// Receive data from UART FIFO of the SC16IS7XX
//=============================================================================
eERRORRESULT SC16IS7XX_ReceiveChar(SC16IS7XX_UART *pUART, uint8_t *data, setSC16IS7XX_ReceiveError *charError)
{
  eERRORRESULT Error;
  size_t ActuallyReceived = 0;
  do 
  {
    Error = SC16IS7XX_ReceiveData(pUART, data, &ActuallyReceived, charError);
    if (Error != ERR_OK) return Error; // If there is an error while calling SC16IS7XX_ReceiveData() then return the error
  } while (ActuallyReceived == 0);
  return ERR_OK;
}





//**********************************************************************************************************************************************************
//=============================================================================
// Get control pins (CD, RI, DSR, CTS) status of the SC16IS7XX device
//=============================================================================
eERRORRESULT SC16IS7XX_GetControlPinStatus(SC16IS7XX_UART *pUART, uint8_t *controlPinsStatus)
{
#ifdef CHECK_NULL_PARAM
  if (pUART == NULL) return ERR__PARAMETER_ERROR;
#endif
  SC16IS7XX* pComp = pUART->Device; // Get the SC16IS7XX device of this UART
#ifdef CHECK_NULL_PARAM
  if (pComp == NULL) return ERR__UNKNOWN_DEVICE;
#endif
  return SC16IS7XX_ReadRegister(pComp, pUART->Channel, RegSC16IS7XX_MSR, controlPinsStatus); // Read the MSR register
}



//=============================================================================
// Get control pins (CD, RI, DSR, CTS) status of the SC16IS7XX device
//=============================================================================
bool SC16IS7XX_IsClearToSend(SC16IS7XX_UART *pUART)
{
  eERRORRESULT Error;
  uint8_t Value;

  //--- Get control pins status ---
  Error = SC16IS7XX_GetControlPinStatus(pUART, &Value);
  if (Error != ERR_OK) return false;                    // If there is an error while calling SC16IS7XX_GetControlPinStatus() then return that the communication is not clear to send
  return ((Value & SC16IS7XX_MSR_CTS_PIN_IS_HIGH) > 0); // Return the Clear To Send status
}





//-----------------------------------------------------------------------------
/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
//-----------------------------------------------------------------------------