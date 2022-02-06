/*
 * Hardware setup:
 *
 * 1) Plug the mikroBus Xplained Pro adapter board into connector EXT1 of the SAM V71 Xplained Ultra evaluation kit.
 * 2) Plug a UART I2C/SPI click into adapter board (configured in SPI).
 * 3) Plug the mikroBus Xplained Pro adapter board into connector EXT2 of the SAM V71 Xplained Ultra evaluation kit.
 * 4) Plug a  click into adapter board.
 * 6) Power SAM V71 Xplained by connecting a USB cable to the DEBUG connector and plugging it into your PC.
 */
//=============================================================================

//-----------------------------------------------------------------------------
#include <asf.h> // Use Atmel Software Framework (ASF)
#include <stdint.h>
#include "string.h"
#include "Main.h"
#include "SC16IS7XXconfigs.h"
#include "SC16IS7XX.h"
//-----------------------------------------------------------------------------

#define DEVICE_COUNT  3

// Variables of program
volatile uint32_t msCount;                //! Milli-seconds count from start of the system
static bool DevicesPresent[DEVICE_COUNT]; //! Indicate which device is present
int_fast8_t DeviceSelected = -1;          //! Indicate which device is selected by the command "*device" (-1 => no device selected)

static bool UARTsPresent[DEVICE_COUNT+1];         //! Indicate which UART is present
int_fast8_t UARTSelected = -1;                    //! Indicate which UART is selected by the command "*uart" (-1 => no UART selected)
static SC16IS7XX_UART* NewUARTs[DEVICE_COUNT+1] = //! Pointers to UARTs
{
  UART0_EXT1,
  UART0_I2C,
  UART0_EXT2,
  UART1_EXT2,
};

//-----------------------------------------------------------------------------

const char UARTsStringsNames[DEVICE_COUNT+1][10+1/* \0 */] =
{
  "UART0_EXT1",
  "UART0_I2C",
  "UART0_EXT2",
  "UART1_EXT2",
};

#define UARTx  UARTsStringsNames[UARTSelected]

//-----------------------------------------------------------------------------

// Tests data
static size_t CurrentCharToSend = 0;
static size_t CurrentCharReceived = 0;
#define TEST_RECEIVE_BUFFER_LENGTH  500
static char RxBufferTests[TEST_RECEIVE_BUFFER_LENGTH];

// Test strings
#define RS232_TEST_LENGTH  64
const char RS232_TEST[RS232_TEST_LENGTH+1/* \0 */] = "0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz";

//-----------------------------------------------------------------------------





//=============================================================================
// Show the current error
//=============================================================================
void ShowError(eERRORRESULT error)
{
  char* pStr = NULL;
/*  switch (error)
  {
#   define X(a, b, c) case a: pStr = (char*)c; break;
    ERRORS_TABLE
#   undef X
  }*/
  pStr = (char*)ERR_ErrorStrings[error];

  if (pStr != NULL)
       LOGERROR("Device error: %s", pStr);
  else LOGERROR("Device error: Unknown error (%u)", (unsigned int)error);
}



//=============================================================================
// Access test of the first 10K bytes of external SDRAM
//=============================================================================
static eERRORRESULT SDRAM_AccessTest(void)
{
#define SDRAMC_TEST_LENGTH    (10 * 1024)
#define SDRAMC_TEST_ODD_TAG   0xAA5555AAu
#define SDRAMC_TEST_EVEN_TAG  0x55AAAA55u
	uint32_t *pSDRAMarray = (uint32_t *)BOARD_SDRAM_ADDR;
  size_t z;

  //--- Write data ---
	for (z = 0; z < SDRAMC_TEST_LENGTH; ++z)
  {
		if (z & 1)
			   pSDRAMarray[z] = SDRAMC_TEST_ODD_TAG | (1 << z);
		else pSDRAMarray[z] = SDRAMC_TEST_EVEN_TAG | (1 << z);
	}
  //--- Read and check data ---
	for (z = 0; z < SDRAMC_TEST_LENGTH; ++z)
  {
		if (z & 1)
    {
			if (pSDRAMarray[z] != (SDRAMC_TEST_ODD_TAG | (1 << z))) return ERR__BAD_DATA;
		} else {
			if (pSDRAMarray[z] != (SDRAMC_TEST_EVEN_TAG | (1 << z))) return ERR__BAD_DATA;
		}
	}
	return ERR_OK;
#undef SDRAMC_TEST_LENGTH
#undef SDRAMC_TEST_ODD_TAG
#undef SDRAMC_TEST_EVEN_TAG
}

//-----------------------------------------------------------------------------





//=============================================================================
// Check if a device is selected and show an error if not
//=============================================================================
static bool CheckDeviceSelected(void)
{
  if (DeviceSelected == -1)
  {
    LOGERROR("No device selected, select a device with *device command");
    return false;
  }
  if (DevicesPresent[DeviceSelected] == false)
  {
    LOGERROR("Device %d not present", (int)(DeviceSelected + 1));
    return false;
  }
  return true;
}



//=============================================================================
// Check if a UART is selected and show an error if not
//=============================================================================
static bool CheckUARTselected(void)
{
  if (UARTSelected == -1)
  {
    LOGERROR("No UART selected, select a device with *uart command");
    return false;
  }
  if (UARTsPresent[UARTSelected] == false)
  {
    LOGERROR("%s not present", UARTsStringsNames[UARTSelected]);
    return false;
  }
  return true;
}



//=============================================================================
// Show registers of the device selected
//=============================================================================
const char SC16IS7XX_RegGenNames[16][9+1/* \0 */] =
{
  "         ",
  "IER      ",
  "IIR      ",
  "LCR      ",
  "MCR      ",
  "LSR      ",
  "MSR      ",
  "SPR      ",
  "TXLVL    ",
  "RXLVL    ",
  "IODir    ",
  "IOState  ",
  "IOIntEna ",
  "Reserved ",
  "IOControl",
  "EFCR     ",
};
const char SC16IS7XX_RegSpeNames[2][9+1/* \0 */] =
{
  "DLL      ",
  "DLH      ",
};
const char SC16IS7XX_RegEnhNames[8][9+1/* \0 */] =
{
  "         ",
  "         ",
  "EFR      ",
  "         ",
  "XON1     ",
  "XON2     ",
  "XOFF1    ",
  "XOFF2    ",
};
const char SC16IS7XX_RegTxRNames[2][9+1/* \0 */] =
{
  "TCR      ",
  "TLR      ",
};

static void ShowRegistersUARTSelected(void)
{
  if (UARTSelected < 0) return;
  SC16IS7XX* pComp = (SC16IS7XX*)NewUARTs[UARTSelected]->Device;
  const uint8_t Channel = NewUARTs[UARTSelected]->Channel;
  eERRORRESULT Error;
  uint8_t RegValue;

  //--- Read General register set ---
  LOGINFO("%s: General register set:", UARTx);
  for (uint8_t zReg = 0x1; zReg <= 0xF; ++zReg)
  {
    Error = SC16IS7XX_ReadRegister(pComp, Channel, zReg, &RegValue);
    if (Error == ERR_OK)
         LOGINFO("%s:   %s = 0x%02X", UARTx, SC16IS7XX_RegGenNames[zReg], RegValue);
    else ShowError(Error);
  }

  //--- Show TCR and TLR ---
  Error = SC16IS7XX_ModifyRegister(pComp, Channel, RegSC16IS7XX_MCR, SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_ENABLE, SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_Mask); // Enable access
  if (Error == ERR_OK)
  {
    Error = SC16IS7XX_ReadRegister(pComp, Channel, RegSC16IS7XX_TCR, &RegValue);
    if (Error == ERR_OK)
         LOGINFO("%s:   %s = 0x%02X", UARTx, SC16IS7XX_RegTxRNames[0], RegValue);
    else ShowError(Error);
    Error = SC16IS7XX_ReadRegister(pComp, Channel, RegSC16IS7XX_TLR, &RegValue);
    if (Error == ERR_OK)
         LOGINFO("%s:   %s = 0x%02X", UARTx, SC16IS7XX_RegTxRNames[1], RegValue);
    else ShowError(Error);
    Error = SC16IS7XX_ModifyRegister(pComp, Channel, RegSC16IS7XX_MCR, SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_DISABLE, SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_Mask); // Disable access
    if (Error != ERR_OK) ShowError(Error);
  }
  else ShowError(Error);

  //--- Read Special register set ---
  SC16IS7XX_LCR_Register OriginalLCR;
  Error = SC16IS7XX_SetRegisterAccess(pComp, Channel, SC16IS7XX_LCR_VALUE_SET_SPECIAL_REGISTER, &OriginalLCR.LCR);
  if (Error == ERR_OK)
  {
    LOGINFO("%s: Special register set:", UARTx);
    for (uint8_t zReg = 0x0; zReg <= 0x1; ++zReg)
    {
      Error = SC16IS7XX_ReadRegister(pComp, Channel, zReg, &RegValue);
      if (Error == ERR_OK)
      {
        LOGINFO("%s:   %s = 0x%02X", UARTx, SC16IS7XX_RegSpeNames[zReg], RegValue);
      }
      else ShowError(Error);
    }
  }
  else ShowError(Error);

  //--- Read Enhanced register set ---
  Error = SC16IS7XX_SetRegisterAccess(pComp, Channel, SC16IS7XX_LCR_VALUE_SET_ENHANCED_FEATURE_REGISTER, &RegValue);
  if (Error == ERR_OK)
  {
    LOGINFO("%s: Enhanced register set:", UARTx);
    for (uint8_t zReg = 0x2; zReg <= 0x7; ++zReg)
    {
      if (zReg == 3) continue;
      Error = SC16IS7XX_ReadRegister(pComp, Channel, zReg, &RegValue);
      if (Error == ERR_OK)
      {
        LOGINFO("%s:   %s = 0x%02X", UARTx, SC16IS7XX_RegEnhNames[zReg], RegValue);
      }
      else ShowError(Error);
    }
  }
  else ShowError(Error);
  
  //--- Return to good configuration ---
  Error = SC16IS7XX_ReturnAccessToGeneralRegister(pComp, Channel, OriginalLCR.LCR);
}



//=============================================================================
// Process command in buffer
//=============================================================================
static void ProcessCommand(void)
{
  if (CommandInput.ToProcess == false) return;
  CommandInput.ToProcess = false;
  if (CommandInput.Buffer[0] != '*') return;       // First char should be a '*'
  CommandInput.Buffer[0] = '\0';                   // Break the current command in the buffer

  for (size_t z = 1; z < 7; z++) // Put string in lower case
  CommandInput.Buffer[z] = LowerCase(CommandInput.Buffer[z]);

  char* pBuf = &CommandInput.Buffer[1];

  //--- Check the command ---
  eConsoleCommand ConsoleCmd = NO_COMMAND;
  if (strncmp(pBuf, "device ", 7) == 0) ConsoleCmd = DEVICE;       // "*Device" command
  if (strncmp(pBuf, "uart "  , 5) == 0) ConsoleCmd = UART;         // "*UART" command
  if (strncmp(pBuf, "showreg", 7) == 0) ConsoleCmd = SHOWREG;      // "*ShowReg" command
  if (strncmp(pBuf, "writes ", 7) == 0) ConsoleCmd = WRITE_STRING; // "*WriteS" command
  if (strncmp(pBuf, "write " , 6) == 0) ConsoleCmd = WRITE_HEX;    // "*Write" command
  if (strncmp(pBuf, "clear"  , 5) == 0) ConsoleCmd = CLEAR;        // "*Clear" command

  if (ConsoleCmd == NO_COMMAND) return;
  SetStrToConsoleBuffer(CONSOLE_TX, "\r\n");

  //--- Do stuff ---
  switch (ConsoleCmd)
  {
    case DEVICE:
      if (CommandInput.BufPos < 9)
      {
        LOGERROR("Command invalid, need a device number");
        return;
      }
      pBuf += 7;
      int32_t DeviceValue = String_ToInt32(pBuf);
      if ((DeviceValue < 1) || (DeviceValue > DEVICE_COUNT))
      {
        LOGERROR("Unknown device");
        return;
      }
      DeviceSelected = (int_fast8_t)DeviceValue - 1;
      if (CheckDeviceSelected())
      {
        LOGINFO("Device %d selected", (int)(DeviceSelected + 1));
      }
      break;
      
    case UART:
      if (CommandInput.BufPos < 7)
      {
        LOGERROR("Command invalid, need a device number");
        return;
      }
      pBuf += 5;
      int32_t UARTValue = String_ToInt32(pBuf);
      if ((UARTValue < 0) || (UARTValue > DEVICE_COUNT))
      {
        LOGERROR("Unknown UART");
        return;
      }
      UARTSelected = (int_fast8_t)UARTValue;
      if (CheckUARTselected())
      {
        LOGINFO("%s selected", UARTsStringsNames[UARTSelected]);
      }
      break;
      
    case SHOWREG:
      /*if (CheckUARTselected())*/ ShowRegistersUARTSelected();
      break;
    case DUMP:
      if (CheckDeviceSelected()) ;
      break;
    case WRITE_STRING:
      if (CheckDeviceSelected()) ;
      break;
    case WRITE_HEX:
      if (CheckDeviceSelected()) ;
      break;
    case CLEAR:
      if (CheckDeviceSelected()) ;
      break;

    case NO_COMMAND:
    default: return;
  }
}





//=============================================================================
// SysTick Handler
//=============================================================================
void SysTick_Handler(void)
{
  msCount++;
}





//=============================================================================
// Main
//=============================================================================
int main (void)
{
  eERRORRESULT Error;
  wdt_disable(WDT);

  //--- Configure system clock --------------------------
  sysclk_init();
  SystemCoreClock = sysclk_get_cpu_hz();

  //--- Initialize board --------------------------------
  board_init();
  ioport_set_pin_mode(SPI0_NPCS1_GPIO, SPI0_NPCS1_FLAGS);
  ioport_disable_pin(SPI0_NPCS1_GPIO);
  ioport_set_pin_mode(SPI0_NPCS2_GPIO, SPI0_NPCS2_FLAGS);
  ioport_disable_pin(SPI0_NPCS2_GPIO);
  ioport_set_pin_mode(SPI0_NPCS3_GPIO, SPI0_NPCS3_FLAGS);
  ioport_disable_pin(SPI0_NPCS3_GPIO);
  ioport_set_pin_mode(SPI0_SPCK_GPIO, SPI0_SPCK_FLAGS);
  ioport_disable_pin(SPI0_SPCK_GPIO);
  ioport_set_pin_mode(SPI0_MOSI_GPIO, SPI0_MOSI_FLAGS);
  ioport_disable_pin(SPI0_MOSI_GPIO);
  ioport_set_pin_mode(SPI0_MISO_GPIO, SPI0_MISO_FLAGS);
  ioport_disable_pin(SPI0_MISO_GPIO);
  
  ioport_set_pin_dir(EXT1_PIN_IRQ, IOPORT_DIR_INPUT);
  ioport_set_pin_mode(EXT1_PIN_IRQ, IOPORT_MODE_PULLUP);
  ioport_set_pin_sense_mode(EXT1_PIN_IRQ, IOPORT_SENSE_FALLING);
  ioport_set_pin_dir(EXT2_PIN_IRQ, IOPORT_DIR_INPUT);
  ioport_set_pin_mode(EXT2_PIN_IRQ, IOPORT_MODE_PULLUP);
  ioport_set_pin_sense_mode(EXT2_PIN_IRQ, IOPORT_SENSE_FALLING);

  //--- Initialize the console UART ---------------------
  InitConsoleTx(CONSOLE_TX);
  InitConsoleRx(CONSOLE_RX);

  //--- Demo start --------------------------------------
  printf("\r\n\r\n");
  LOGTITLE("SC16IS7XX Demo start...");

  //--- Configure SysTick base timer --------------------
  SysTick_Config(SystemCoreClock * SYSTEM_TICK_MS / 1000); // (Fmck(Hz)*1/1000)=1ms

  //--- Configure SPI0 ----------------------------------
  Error = SPI_Init(SPI0, &SPI0_Config);
  if (Error != ERR_OK)
  {
    ioport_set_pin_level(LED0_GPIO, LED0_ACTIVE_LEVEL);
    ioport_set_pin_level(LED1_GPIO, LED1_ACTIVE_LEVEL);
    LOGFATAL("Unable to configure SPI0 (error code: %u), END OF DEMO", (unsigned int)Error);
    while (true) TrySendingNextCharToConsole(CONSOLE_TX); // Stay stuck here
  }

  //--- Configure SC16IS740 on EXT1 --------------------------------
  Error = Init_SC16IS7XX(SC16IS740_EXT1, &SC16IS7XX_EXT1_Config);
  if (Error == ERR_OK)
  {
    LOGTRACE("Device SC16IS740 detected, 1 new UART channel available");
    DevicesPresent[0] = true;
    ioport_set_pin_level(LED0_GPIO, LED0_INACTIVE_LEVEL);

    Error = SC16IS7XX_InitUART((SC16IS7XX_UART*)UART0_EXT1, &UART0_EXT1_RS232config);
    if (Error != ERR_OK)
    {
      UARTsPresent[0] = false;
      ShowError(Error);
    }
    else UARTsPresent[0] = true;
  }
  else
  {
    ioport_set_pin_level(LED0_GPIO, LED0_ACTIVE_LEVEL);
    DevicesPresent[0] = false;
    UARTsPresent[0]   = false;
    ShowError(Error);
  }

  //--- Configure SC16IS750 on I2C --------------------------------
  Error = Init_SC16IS7XX(SC16IS750_I2C, &SC16IS7XX_I2C_Config);
  if (Error == ERR_OK)
  {
    LOGTRACE("Device SC16IS750 detected, 1 new UART channel available");
    DevicesPresent[1] = true;
    ioport_set_pin_level(LED0_GPIO, LED0_INACTIVE_LEVEL);

    Error = SC16IS7XX_InitUART(UART0_I2C, &UART0_I2C_RS232config);
    if (Error != ERR_OK)
    {
      UARTsPresent[1] = false;
      ShowError(Error);
    }
    else UARTsPresent[1] = true;
  }
  else
  {
    if (DevicesPresent[0] == false) ioport_set_pin_level(LED0_GPIO, LED0_ACTIVE_LEVEL);
    DevicesPresent[1] = false;
    UARTsPresent[1]   = false;
    ShowError(Error);
  }

  //--- Configure SC16IS752 on EXT2 --------------------------------
  Error = Init_SC16IS7XX(SC16IS752_EXT2, &SC16IS7XX_EXT2_Config);
  if (Error == ERR_OK)
  {
    LOGTRACE("Device SC16IS752 detected, 2 new UART channel available");
    DevicesPresent[2] = true;
    ioport_set_pin_level(LED1_GPIO, LED1_INACTIVE_LEVEL);

    Error = SC16IS7XX_InitUART(UART0_EXT2, &UART_EXT2_RS232config);
    if (Error != ERR_OK)
    {
      UARTsPresent[2] = false;
      ShowError(Error);
    }
    else UARTsPresent[2] = true;
    Error = SC16IS7XX_InitUART(UART1_EXT2, &UART_EXT2_RS232config);
    if (Error != ERR_OK)
    {
      UARTsPresent[3] = false;
      ShowError(Error);
    }
    else UARTsPresent[3] = true;
  }
  else
  {
    ioport_set_pin_level(LED1_GPIO, LED1_ACTIVE_LEVEL);
    DevicesPresent[2] = false;
    UARTsPresent[2]   = false;
    UARTsPresent[3]   = false;
    ShowError(Error);
  }

  //--- Configure SDMAC ---------------------------------
  pmc_enable_periph_clk(ID_SDRAMC); // Enable SDRAMC peripheral clock
  sdramc_init((sdramc_memory_dev_t *)&SDRAM_ISSI_IS42S16100E, sysclk_get_cpu_hz()); // Complete SDRAM configuration
  sdram_enable_unaligned_support(); // uint8_t *pSDRAM = (uint8_t *)BOARD_SDRAM_ADDR; (BOARD_SDRAM_SIZE is 2MB)
#ifdef CONF_BOARD_ENABLE_CACHE_AT_INIT
  SCB_CleanInvalidateDCache();
#endif
  Error = SDRAM_AccessTest();
  if (Error != ERR_OK)
  {
    ioport_set_pin_level(LED0_GPIO, LED0_ACTIVE_LEVEL);
    ioport_set_pin_level(LED1_GPIO, LED1_ACTIVE_LEVEL);
    LOGFATAL("SDRAM fail (error code: %u), END OF DEMO", (unsigned int)Error);
    while (true) TrySendingNextCharToConsole(CONSOLE_TX); // Stay stuck here
  }

  //--- Reset watchdog ----------------------------------
  wdt_restart(WDT);

  //--- Log ---------------------------------------------
  LOGTRACE("Initialization complete");

  //--- Display menu ------------------------------------
  LOGINFO("Available commands:");
  LOGINFO("  *Device X  : Select the device X to work with");
  LOGINFO("  *UART X    : Select the UART X to work with");
  LOGINFO("  *ShowReg   : Show all registers states");
  LOGINFO("  *Dump A X  : Dump memory at hex address A of X bytes");
  LOGINFO("  *WriteS A S: Write S string (%u char max) at hex address A", (unsigned int)(COMMAND_BUFFER_SIZE - 16));
  LOGINFO("  *Write A H : Write H hex bytes (%u char max) at hex address A", (unsigned int)(COMMAND_BUFFER_SIZE - 16));
  LOGINFO("  *Clear     : Clear the entire device memory by writing 0xFF on all bytes");




  size_t CharSentCount, RemainingCharCount = 0, ReceivedCharCount = 0;
  eSC16IS7XX_InterruptSource LastInterruptFlag;
  uint8_t LastCharError;
  bool IsIntPending;

  //=== Test RS-232 (no interrupts) ===
  CurrentCharToSend = 0;
  CurrentCharReceived = 0;
  while ((CurrentCharToSend < RS232_TEST_LENGTH) || (CurrentCharReceived < RS232_TEST_LENGTH))
  {
    //--- Send data ---
    RemainingCharCount = RS232_TEST_LENGTH - CurrentCharToSend;
    if (RemainingCharCount > 0)
    {
      Error = SC16IS7XX_TransmitData(UART0_EXT2, (uint8_t*)&RS232_TEST[CurrentCharToSend], RemainingCharCount, &CharSentCount);
      if (Error != ERR_OK) { ShowError(Error); break; }
      CurrentCharToSend += CharSentCount;
    }

    //--- Receive data ---
    Error = SC16IS7XX_ReceiveData(UART1_EXT2, (uint8_t*)&RxBufferTests[CurrentCharReceived], TEST_RECEIVE_BUFFER_LENGTH - CurrentCharReceived, &ReceivedCharCount, &LastCharError);
    if (Error != ERR_OK)
    {
      ShowError(Error);
      if (Error == ERR__RECEIVE_ERROR) LOGERROR("  Last char error: %u", (unsigned int)LastCharError);
      break;
    }
    CurrentCharReceived += ReceivedCharCount;
  }
  if (strncmp(&RS232_TEST[0], &RxBufferTests[0], RS232_TEST_LENGTH) != 0)
       LOGERROR("RS-232 basic test (no interrupts) FAILED!");
  else LOGSPECIAL("RS-232 basic test (no interrupts) success");


  //=== Test RS-232 (with interrupts) ===
  CurrentCharToSend = 0;
  CurrentCharReceived = 0;
  while ((CurrentCharToSend < RS232_TEST_LENGTH) || (CurrentCharReceived < RS232_TEST_LENGTH))
  {
    if (ioport_get_pin_level(EXT2_PIN_IRQ) == 0)                                           // Check INT pin status of the SC16IS752
    {
      //--- Send data ---
      Error = SC16IS7XX_GetInterruptEvents(UART0_EXT2, &IsIntPending, &LastInterruptFlag); // Get interrupts UART0
      if (Error != ERR_OK) { ShowError(Error); break; }
      if (IsIntPending && ((LastInterruptFlag & SC16IS7XX_THR_INTERRUPT) > 0))             // Check THR
      {
        RemainingCharCount = RS232_TEST_LENGTH - CurrentCharToSend;
        if (RemainingCharCount > 0)
        {
          Error = SC16IS7XX_TransmitData(UART0_EXT2, (uint8_t*)&RS232_TEST[CurrentCharToSend], RemainingCharCount, &CharSentCount);
          if (Error != ERR_OK) { ShowError(Error); break; }
          CurrentCharToSend += CharSentCount;
        }
      }
    }

    if (ioport_get_pin_level(EXT2_PIN_IRQ) == 0)                                           // Check INT pin status of the SC16IS752
    {
      //--- Receive data ---
      Error = SC16IS7XX_GetInterruptEvents(UART1_EXT2, &IsIntPending, &LastInterruptFlag); // Get interrupts UART1
      if (Error != ERR_OK) { ShowError(Error); break; }
      if (IsIntPending && ((LastInterruptFlag & SC16IS7XX_RHR_INTERRUPT) > 0))             // Check RHR
      {
        Error = SC16IS7XX_ReceiveData(UART1_EXT2, (uint8_t*)&RxBufferTests[CurrentCharReceived], TEST_RECEIVE_BUFFER_LENGTH - CurrentCharReceived, &ReceivedCharCount, &LastCharError);
        if (Error != ERR_OK)
        {
          ShowError(Error);
          if (Error == ERR__RECEIVE_ERROR) LOGERROR("  Last char error: %u", (unsigned int)LastCharError);
          break;
        }
        CurrentCharReceived += ReceivedCharCount;
      }
    }
  }
  if (strncmp(&RS232_TEST[0], &RxBufferTests[0], RS232_TEST_LENGTH) != 0)
       LOGERROR("RS-232 basic test (with interrupts) FAILED!");
  else LOGSPECIAL("RS-232 basic test (with interrupts) success");




  //=== The main loop ===================================
  while(1)
  {
    //--- Flush char by char console buffer ---
    TrySendingNextCharToConsole(CONSOLE_TX);

    //--- Process command if any available ---
    ProcessCommand();

    //=== Tests ===


    nop();
  }
}
