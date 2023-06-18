/*!*****************************************************************************
 * @file    Console.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.1.0
 * @date    04/06/2023
 * @brief   Some functions for RS-232 console communication
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "Console.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
#  include <stdint.h>
   extern "C" {
#endif
//-----------------------------------------------------------------------------

#ifdef USE_DYNAMIC_INTERFACE
#  define GET_UART_INTERFACE  pApi->UART
#else
#  define GET_UART_INTERFACE  &pApi->UART
#endif

//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
#ifdef USE_CONSOLE_TX
//********************************************************************************************************************
// Console Transmit API
//********************************************************************************************************************

//! Severity line color
#ifdef __cplusplus
void SetConsoleColor(int text, int fond)
{ // 0: noir          8: gris
  // 1: bleu fonce    9: bleu
  // 2: vert         10: vert fluo
  // 3: bleu-gris    11: turquoise
  // 4: marron       12: rouge
  // 5: pourpre      13: rose fluo
  // 6: kaki         14: jaune fluo
  // 7: gris clair   15: blanc
  HANDLE H = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(H, (fond << 4) + text);
}

const int SeverityColors[(size_t)lsLast_] =
{
  lsTitle   = wccLIME  , // lsTitle   -> Color: Text=green  ; Background=black
  lsFatal   = wccRED   , // lsFatal   -> Color: Text=red    ; Background=black
  lsError   = wccRED   , // lsError   -> Color: Text=red    ; Background=black
  lsWarning = wccYELLOW, // lsWarning -> Color: Text=yellow ; Background=black
  lsInfo    = wccAQUA  , // lsInfo    -> Color: Text=blue   ; Background=black
  lsTrace   = wccWHITE , // lsTrace   -> Color: Text=white  ; Background=black
  lsDebug   = wccGRAY  , // lsDebug   -> Color: Text=grey   ; Background=black
  lsSpecial = wccOLIVE , // lsSpecial -> Color: Text=kaki   ; Background=black
};

#else

const char* SeverityColors[(size_t)lsLast_] =
{
  "\x001B[1;32m", // lsTitle   -> Color: Text=green         ; Background=black ; Bold
  "\x001B[1;91m", // lsFatal   -> Color: Text=red bright    ; Background=black ; Bold
  "\x001B[0;91m", // lsError   -> Color: Text=red bright    ; Background=black
  "\x001B[0;93m", // lsWarning -> Color: Text=yellow bright ; Background=black
  "\x001B[0;36m", // lsInfo    -> Color: Text=cyan          ; Background=black
  "\x001B[0;97m", // lsTrace   -> Color: Text=white         ; Background=black
  "\x001B[0;37m", // lsDebug   -> Color: Text=grey "white"  ; Background=black
  "\x001B[0;33m", // lsSpecial -> Color: Text=yellow        ; Background=black
};

#endif

//-----------------------------------------------------------------------------


//=============================================================================
// Initialize the Console Transmit
//=============================================================================
eERRORRESULT InitConsoleTx(ConsoleTx* pApi)
{
#ifdef CHECK_NULL_PARAM
  if (pApi == NULL) return ERR__PARAMETER_ERROR;
#endif

  // Initialize the print buffer
  if (pApi->Buffer == NULL) return ERR__NULL_BUFFER;
  if (pApi->BufferSize == 0) return ERR__BAD_DATA_SIZE;
  pApi->InPos  = 0;
  pApi->OutPos = 0;
  memset(pApi->Buffer, 0, (sizeof(pApi->Buffer[0]) * pApi->BufferSize));
  return ERR_NONE;
}

//-----------------------------------------------------------------------------


//=============================================================================
// Set char to print buffer
//=============================================================================
void SetCharToConsoleBuffer(ConsoleTx* pApi, const char aChar)
{
#ifdef CHECK_NULL_PARAM
  if (pApi == NULL) return;
  if (pApi->Buffer == NULL) return;
  if (aChar == '\0') return;
#endif

  bool BufferFull = false;
  // Check buffer full
  if (pApi->OutPos == 0) BufferFull = (pApi->InPos == (pApi->BufferSize - 1));
  else BufferFull = (pApi->InPos == (pApi->OutPos - 1));
  if (BufferFull)
    while (!TrySendingNextCharToConsole(pApi)) // If buffer full force sending next char
      if (pApi->InPos == pApi->OutPos) break;  // But both InPos and OutPos should not be at the same position
  // Store the new char
  pApi->Buffer[pApi->InPos++] = aChar;
  if (pApi->InPos >= pApi->BufferSize) pApi->InPos = 0;
}


//=============================================================================
// Set char array to print buffer
//=============================================================================
void SetStrToConsoleBuffer(ConsoleTx* pApi, const char* string)
{
  while (*string != '\0')
  {
    SetCharToConsoleBuffer(pApi, *string);
    string++;
  }
}


//=============================================================================
// Try to send next char in the console print buffer
//=============================================================================
bool TrySendingNextCharToConsole(ConsoleTx* pApi)
{
#ifdef CHECK_NULL_PARAM
  if (pApi == NULL) return false;
  if (pApi->Buffer == NULL) return false;
#endif
  UART_Interface* pUART = GET_UART_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
  if (pUART == NULL) return false;
# endif
  if (pUART->fnUART_Transmit == NULL) return false;
#endif
  bool Result = false;
  size_t DataSizeToSend;

  if (pApi->OutPos != pApi->InPos)
  {
    size_t ActuallySent = 0;
    if (pApi->OutPos >= pApi->InPos)
         DataSizeToSend = pApi->BufferSize - pApi->InPos; // Calculate space available to the end of buffer
    else DataSizeToSend = pApi->InPos - pApi->OutPos;     // Calculate space available to Out position
    if (pUART->fnUART_Transmit(pUART, (uint8_t*)&pApi->Buffer[pApi->OutPos], DataSizeToSend, &ActuallySent) == ERR_NONE)
    {
      pApi->Buffer[pApi->OutPos] = 0;
      pApi->OutPos += ActuallySent;
      if (pApi->OutPos >= pApi->BufferSize) pApi->OutPos -= pApi->BufferSize;
      Result = true;
    }
  }
  return Result;
}

//-----------------------------------------------------------------------------



//=============================================================================
// Send a formated Logs to console (DO NOT USE DIRECTLY, use LOG*() instead)
//=============================================================================
void __LOG(ConsoleTx* pApi, const char* context, bool whiteText, const char* format, va_list args)
{
  const char* const FormatLine   = "%s [%u:%02u:%02u:%02u] ";
  const char* const WhiteTextStr = "\x001B[0m";
  const char* const NewLine      = "\r\n";

  // Fast div 1000 (+/- one unit error is not critical for logging purpose)
  uint64_t Val = msCount;
  uint32_t Time = (uint32_t)((Val * 0x00418937) >> 32); // Magic number : Here 0x418937 is 0xFFFFFFFF / 1000d. This is the overflow limit of an uint32
  uint32_t NewTime;
  // Extract fields
  NewTime = (Time / 60); uint32_t Sec = Time - (NewTime * 60); Time = NewTime;
  NewTime = (Time / 60); uint32_t Min = Time - (NewTime * 60); Time = NewTime;
  NewTime = (Time / 24); uint32_t Hor = Time - (NewTime * 24); Time = NewTime;
  uint32_t d = Time;


#ifndef __cplusplus
# define LOG_BUFFER_SIZE  200
  char TmpBuff[LOG_BUFFER_SIZE];
  siprintf(TmpBuff, FormatLine, context, (unsigned int)d, (unsigned int)Hor, (unsigned int)Min, (unsigned int)Sec);
  SetStrToConsoleBuffer(pApi, TmpBuff);
  if (whiteText) SetStrToConsoleBuffer(pApi, WhiteTextStr);
  vsiprintf(TmpBuff, format, args);
  SetStrToConsoleBuffer(pApi, TmpBuff);
  SetStrToConsoleBuffer(pApi, NewLine);
  TrySendingNextCharToConsole(pApi);
#else
  printf(FormatLine, context, (unsigned int)d, (unsigned int)Hor, (unsigned int)Min, (unsigned int)Sec);
  vprintf(format, args);
  printf(NewLine);
#endif
}

//-----------------------------------------------------------------------------


//=============================================================================
// Send a formated Logs to console
//=============================================================================
void LOG(ConsoleTx* pApi, eSeverity severity, const char* format, ...)
{
  va_list args;
  va_start(args, format);

#ifdef __cplusplus
  SetConsoleColor(SeverityColors[(size_t)severity], wccBLACK);
#else
  SetStrToConsoleBuffer(pApi, SeverityColors[(size_t)severity]);
#endif

  bool KeepColorFor = (severity == lsFatal) || (severity == lsDebug);
  __LOG(pApi, "DEMO", !KeepColorFor, format, args);

  va_end(args);
}

//-----------------------------------------------------------------------------


#ifdef __cplusplus
//=============================================================================
// Set the Windows console color
//=============================================================================
void SetConsoleColor(eWinConsoleColor text, eWinConsoleColor background)
{
  HANDLE H = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(H, ((int)background << 4) + (int)text);
}


//=============================================================================
// Send a formated Simulation Logs to console
//=============================================================================
void LOGSIM(ConsoleTx* pApi, const char* format, ...)
{
  va_list args;
  va_start(args, format);

#ifdef __cplusplus
  SetConsoleColor(wccTEAL, wccBLACK);         // Color: Text=blue-grey ; Background=black
#else
  SetStrToConsoleBuffer(pApi,"\x001B[0;96m"); // Color: Text=cyan bright ; Background=black
#endif

  __LOG(pApi, "SIMU", false, format, args);

  va_end(args);
}
#endif

//-----------------------------------------------------------------------------




//**********************************************************************************************************************************************************
#if (defined(DEBUG) || defined(_DEBUG))
//=============================================================================
// Show the hexadecimal dump of the memory to console
//=============================================================================
void __HexDump(ConsoleTx* pApi, const char* context, const void* src, unsigned int size)
{
# define ROW_LENGTH           16 // 16 bytes per row
# define HEXDUMP_BUFFER_SIZE  10 + 3 + (ROW_LENGTH * 3) + 2 + (ROW_LENGTH + 1) + 4
  static const char* Hexa = "0123456789ABCDEF";

  char HexaDump[ROW_LENGTH * 3]; // [2 digit hexa + space] - 1 space + 1 zero terminal
  char HexaChar[ROW_LENGTH + 1]; // [1 char] + 1 zero terminal

  LOGDEBUG_(pApi, "Dump %d bytes at 0x%08X - %s", size, (unsigned int)src, context);

  unsigned char* pSrc = (unsigned char*)src;
  HexaChar[ROW_LENGTH] = 0;
  for (int32_t i = ((size + ROW_LENGTH - 1) / ROW_LENGTH); --i >= 0; pSrc += ROW_LENGTH, size -= ROW_LENGTH)
  {
    memset(HexaDump, ' ', sizeof(HexaDump));
    memset(HexaChar, '.', ROW_LENGTH);
    for (int j = (size >= ROW_LENGTH ? ROW_LENGTH : size); --j >= 0;)
    {
      HexaDump[j * 3 + 0] = Hexa[(pSrc[j] >> 4) & 0xF];
      HexaDump[j * 3 + 1] = Hexa[(pSrc[j] >> 0) & 0xF];
      //HexaDump[j * 3 + 2] = ' ';
      HexaChar[j] = (pSrc[j] < 0x20) ? '.' : pSrc[j];
    }
    HexaDump[ROW_LENGTH * 3 - 1] = 0;

    char TmpBuff[10 + 3 + (ROW_LENGTH * 3) + 2 + (ROW_LENGTH + 1) + 4];
    siprintf(TmpBuff, "  %08X : %s \"%s\"\r\n", (unsigned int)pSrc, HexaDump, HexaChar);
    SetStrToConsoleBuffer(pApi, TmpBuff);
  }
# undef ROW_LENGTH
}


//=============================================================================
// Show the binary dump of the memory to console
//=============================================================================
void __BinDump(ConsoleTx* pApi, const char* context, const void* src, unsigned int size)
{
  static const char* Bin  = "01";
  static const char* Hexa = "0123456789ABCDEF";

# define ROW_LENGTH  4          // 4 bytes per row
  char BinDump[ROW_LENGTH * 9]; // [8 digit bin  + space] - 1 space + 1 zero terminal
  char BinHexa[ROW_LENGTH * 3]; // [2 digit hexa + space] - 1 space + 1 zero terminal

  LOGDEBUG_(pApi, "Dump %d bytes at 0x%08X - %s", size, (unsigned int)src, context);

  unsigned char* pSrc = (unsigned char*)src;
  BinHexa[ROW_LENGTH] = 0;
  for (int32_t i = ((size+ROW_LENGTH-1) / ROW_LENGTH); --i >= 0; pSrc += ROW_LENGTH, size -= ROW_LENGTH)
  {
    memset(BinDump, ' ', sizeof(BinDump));
    memset(BinHexa, ' ', sizeof(BinHexa));
    for (int j = (size >= ROW_LENGTH ? ROW_LENGTH : size); --j >= 0;)
    {
      BinDump[j * 9 + 0] = Bin[(pSrc[j] >> 7) & 0x1];
      BinDump[j * 9 + 1] = Bin[(pSrc[j] >> 6) & 0x1];
      BinDump[j * 9 + 2] = Bin[(pSrc[j] >> 5) & 0x1];
      BinDump[j * 9 + 3] = Bin[(pSrc[j] >> 4) & 0x1];
      BinDump[j * 9 + 4] = Bin[(pSrc[j] >> 3) & 0x1];
      BinDump[j * 9 + 5] = Bin[(pSrc[j] >> 2) & 0x1];
      BinDump[j * 9 + 6] = Bin[(pSrc[j] >> 1) & 0x1];
      BinDump[j * 9 + 7] = Bin[(pSrc[j] >> 0) & 0x1];
      //BinDump[j * 3 + 8] = ' ';
      BinHexa[j * 3 + 0] = Hexa[(pSrc[j] >> 4) & 0xF];
      BinHexa[j * 3 + 1] = Hexa[(pSrc[j] >> 0) & 0xF];
      //BinHexa[j * 3 + 2] = ' ';

    }
    BinDump[ROW_LENGTH * 9 - 1] = 0;
    BinHexa[ROW_LENGTH * 3 - 1] = 0;

    char TmpBuff[10 + 3 + (ROW_LENGTH * 9) + 2 + (ROW_LENGTH * 3) + 4];
    siprintf(TmpBuff, "  %08X : %s - %s\r\n", (unsigned int)pSrc, BinDump, BinHexa);
    SetStrToConsoleBuffer(pApi, TmpBuff);
  }
# undef ROW_LENGTH
}
#endif

//-----------------------------------------------------------------------------
#endif /* USE_CONSOLE_TX */
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
#ifdef USE_CONSOLE_RX
//********************************************************************************************************************
// Console Receive API
//********************************************************************************************************************

/*! @brief Internal process received command Callback
 * @param[in] *pCmd Is the command string first char (NULL terminated string)
 * @param[in] size Is the char count of the command string pCmd
 * @return Returns an #eERRORRESULT value enum
 */
static eERRORRESULT __InternalProcessReceivedCommand(const uint8_t* pCmd, size_t size);

#ifdef USE_CONSOLE_GPIO_COMMANDS
/*! @brief Compare 2 ANSI strings by only the size of the str2
 * @param[in] *pStr1 Is the string to compare
 * @param[in] *pStr2 Is the string to compare. It is also the max size to compare
 * @return Returns the value extracted from string
 */
static int32_t __strscmp(const uint8_t* str1, const char* str2);

/*! @brief Compare 2 ANSI strings by only the size of the str2 and update Str1 pointer only if strings are identical
 * @param[in] *pStr1 Is the string to compare (the original pointer will be advanced) and returns the new position in the string, or the end of the string
 * @param[in] *pStr2 Is the string to compare. It is also the max size to compare
 * @return Returns the value extracted from string
 */
static int32_t __strtcmp(const uint8_t** str1, const char* str2);
/*! @brief Convert a string to int float
 * This function will stop parsing at first char:
 *  - That is not in '0'..'9' in case of decimal string
 *  - That is not in '0'..'9', 'a'..'f', 'A'..'F' in case of hexadecimal string (starting with "0x" or "0X")
 *  - That is not in '0'..'1' in case of binary string (starting with "0b" or "0B")
 * @param[in/out] **pStr Is the string to parse (the original pointer will be advanced) and returns the new position in the string, or the end of the string
 * @return Returns the value extracted from string
 */
static uint32_t __StringToUint(const uint8_t** pStr);
#endif // USE_CONSOLE_GPIO_COMMANDS
//-----------------------------------------------------------------------------
#ifdef USE_CONSOLE_GPIO_COMMANDS
/*! @brief Process GPIO command
 * @details Commands that can be parsed are the following:
 * GPIO <action> <PORT/Pin>[ <value>][ <mask>]
 * Where:
 *  - <action> can be:
 *    - RD, READ: Read <PORT/Pin>
 *    - WR, WRITE: Write <value> to <PORT/Pin>
 *    - SET: Set bitset of <value> to <PORT/Pin>
 *    - CLR, CLEAR: Clear bitset of <value> to <PORT/Pin>
 *    - TG, TOGGLE: Toggle the value of <PORT/Pin>
 *  - <PORT/Pin> can be:
 *    - PORTx: where 'x' can be any of the ports name of the MCU
 *    - Rxy: where 'x' can be any of the ports name and 'y' the number of the pin of the MCU (ex: RA0)
 *  - <value> is the value to apply in case of Write, Set, or Clear. The value can be binary (0b prefix), decimal, hexadecimal (0x prefix). The default value will be 0
 *  - <mask> is the mask value to apply with the 'value' in case of PORT use. The mask can be binary (0b prefix), decimal, hexadecimal (0x prefix). The default value will be 0xFFFFFFFF
 *
 * @param[in] *pCmd Is the command string first char (NULL terminated string)
 * @param[in] size Is the char count of the command string pCmd
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT __ProcessGPIOcommand(const uint8_t* pCmd, size_t size);
#endif // USE_CONSOLE_GPIO_COMMANDS

#ifdef USE_CONSOLE_EEPROM_COMMANDS
/*! @brief Process EEPROM command
 * @details Commands that can be parsed are the following:
 * EEPROM<x> <action>[ <value>][ <mask>]
 * Where:
 *  - <x> is the eeprom index to use
 *  - <action> can be:
 *    - RD, READ: Read <PORT/Pin>
 *    - WR, WRITE: Write <value> to <PORT/Pin>
 *    - SET: Set bitset of <value> to <PORT/Pin>
 *    - CLR, CLEAR: Clear bitset of <value> to <PORT/Pin>
 *    - TG, TOGGLE: Toggle the value of <PORT/Pin>
 *  - <mask> is the mask value to apply with the 'value' in case of PORT use. The mask can be binary (0b prefix), decimal, hexadecimal (0x prefix). The default value will be 0xFFFFFFFF
 *
 * @param[in] *pCmd Is the command string first char (NULL terminated string)
 * @param[in] size Is the char count of the command string pCmd
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT __ProcessEEPROMcommand(const uint8_t* pCmd, size_t size)
#endif // USE_CONSOLE_EEPROM_COMMANDS
//-----------------------------------------------------------------------------





//=============================================================================
// Initialize the Console Reception
//=============================================================================
eERRORRESULT InitConsoleRx(ConsoleRx* pApi)
{
#ifdef CHECK_NULL_PARAM
  if (pApi == NULL) return ERR__PARAMETER_ERROR;
# ifdef CONSOLE_RX_USE_COMMAND_RECALL
  if (pApi->CommandBuffer == NULL) return ERR__NULL_BUFFER;
  if (pApi->BufferSize == 0) return ERR__BAD_DATA_SIZE;
# endif
#endif

  //--- Initialize the print buffer ---
  memset(pApi->CurrentBuff, 0, sizeof(pApi->CurrentBuff));
  pApi->RxIdx      = 0;
  pApi->ProcessIdx = 0;
  pApi->CursorIdx  = 0;
#ifdef CONSOLE_RX_USE_COMMAND_RECALL
  pApi->StartIdx   = 0;
  pApi->CurrentIdx = 1;
  memset(pApi->CommandBuffer, 0, (sizeof(pApi->CommandBuffer[0]) * pApi->BufferSize));
  pApi->CommandBuffer[0] = CONSOLE_STX; // Set the start of command char (Start of TeXt)
#endif
  return ERR_NONE;
}

//-----------------------------------------------------------------------------


//=============================================================================
// [INTERRUPT] Receive char from console
//=============================================================================
eERRORRESULT ConsoleRx_ReceiveChar(ConsoleRx* pApi)
{
#ifdef CHECK_NULL_PARAM
  if (pApi == NULL) return ERR__PARAMETER_ERROR;
#endif
  UART_Interface* pUART = GET_UART_INTERFACE;
#if defined(CHECK_NULL_PARAM)
# if defined(USE_DYNAMIC_INTERFACE)
    if (pUART == NULL) return ERR__PARAMETER_ERROR;
# endif
  if (pUART->fnUART_Receive == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (pApi->RxIdx >= CONSOLE_RX_CURRENT_BUFFER_SIZE) return ERR__BUFFER_FULL;
  eERRORRESULT Error;

  //--- Receive char from UART ---
  const size_t DataSizeToGet = CONSOLE_RX_CURRENT_BUFFER_SIZE - pApi->RxIdx; // Calculate data available to the end of buffer
  size_t ActuallyReceived; uint8_t LastCharError;
  Error = pUART->fnUART_Receive(pUART, &pApi->CurrentBuff[pApi->RxIdx], DataSizeToGet, &ActuallyReceived, &LastCharError);
  if (Error != ERR_NONE) return Error;                                       // If there is an error while receiving data, return the error
  if (ActuallyReceived == 0) return ERR__NO_DATA_AVAILABLE;                  // No data received? Then no data available
  pApi->RxIdx += ActuallyReceived;                                           // Set new RxIdx
  if (LastCharError > 0) return ERR__RECEIVE_ERROR;
  return ERR_NONE;
}


//=============================================================================
// Process received char from console
//=============================================================================
eERRORRESULT ConsoleRx_ProcessReceivedChars(ConsoleRx* pApi)
{
#ifdef CHECK_NULL_PARAM
  if (pApi == NULL) return ERR__PARAMETER_ERROR;
#endif
  eERRORRESULT Error = ERR_NONE;
  size_t RxIdx      = pApi->RxIdx;
  size_t ProcessIdx = pApi->ProcessIdx;

  //--- Process command inputs ---
  while ((ProcessIdx < RxIdx) && (Error == ERR_NONE))
  {
    const uint8_t CurrentData = pApi->CurrentBuff[ProcessIdx];
    switch (CurrentData)
    {
      case CONSOLE_BS: // BackSpace '\b'
        memcpy(&pApi->CurrentBuff[ProcessIdx], &pApi->CurrentBuff[ProcessIdx + 1], (RxIdx - ProcessIdx - 1));              // Remove current backspace by shifting string by 1 char to the left
        --RxIdx;
        if (pApi->CursorIdx > 0)                                                                                           // Handle cursor index not in end of current command and only if cursor index is not at index 0
        {
          memcpy(&pApi->CurrentBuff[pApi->CursorIdx - 1], &pApi->CurrentBuff[pApi->CursorIdx], (RxIdx - pApi->CursorIdx)); // Shift string by 1 char to the left at cursor
          --pApi->CursorIdx;
          --ProcessIdx;
          --RxIdx;
        }
        break;

      case CONSOLE_DEL: // Delete
        memcpy(&pApi->CurrentBuff[ProcessIdx], &pApi->CurrentBuff[ProcessIdx + 1], (RxIdx - ProcessIdx - 1));                  // Remove current delete by shifting string by 1 char to the left
        --RxIdx;
        if (pApi->CursorIdx < ProcessIdx)                                                                                      // Handle cursor index not in end of current command
        {
          memcpy(&pApi->CurrentBuff[pApi->CursorIdx], &pApi->CurrentBuff[pApi->CursorIdx + 1], (RxIdx - pApi->CursorIdx - 1)); // Suppress char at cursor index
          --ProcessIdx;
          --RxIdx;
        }
        break;

      case CONSOLE_CR: // #13 '\r'
      case CONSOLE_LF: // #10 '\n'
        {
          //--- Process command ---
          if (ProcessIdx > 0)
          {
            pApi->CurrentBuff[ProcessIdx] = CONSOLE_NULL;                                                                       // Force a NULL terminal string
            Error = __InternalProcessReceivedCommand(&pApi->CurrentBuff[0], ProcessIdx);                                        // Call to a weak function: Process received command Callback
          }
          //--- Prepare next command ---
          const size_t SizeToMove = (RxIdx - ProcessIdx - 1);
          memcpy(&pApi->CurrentBuff[0], &pApi->CurrentBuff[ProcessIdx + 1], SizeToMove);                                        // Suppress CR/LF at process index and move the rest of non processed command to index 0
          pApi->CursorIdx = 0;
          ProcessIdx = 0;
          RxIdx = SizeToMove;                                                                                                   // Set new Rx index
        }
        break;

      default:
        if (pApi->CursorIdx < ProcessIdx)                                                                                       // Handle cursor index not in end of current command
        {
          memcpy(&pApi->CurrentBuff[pApi->CursorIdx + 1], &pApi->CurrentBuff[pApi->CursorIdx], (ProcessIdx - pApi->CursorIdx)); // Shift string by 1 char to the right
        }
        pApi->CurrentBuff[pApi->CursorIdx] = CurrentData;                                                                       // Cursor index takes the processed data
        ++pApi->CursorIdx;
        ++ProcessIdx;
#ifdef USE_CONSOLE_TX
//        if ((CurrentData != '\r') && (CurrentData != '\n')) SetCharToConsoleBuffer(CONSOLE_TX, CurrentData);                    // Display received char <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif
        break;
    }
  }
  pApi->RxIdx      = RxIdx;
  pApi->ProcessIdx = ProcessIdx;
  return Error;
}

//-----------------------------------------------------------------------------


//! @brief List of supported console receive commands
const ConsoleCommand ConsoleCommandsList[] =
{
#ifdef USE_CONSOLE_GPIO_COMMANDS
  { CONSOLE_ROL5XOR_HASH('G','P','I','O','\0','\0','\0','\0'), 4, __ProcessGPIOcommand },
#endif
#ifdef USE_CONSOLE_EEPROM_COMMANDS
  { CONSOLE_ROL5XOR_HASH('E','E','P','R','O','M','\0','\0'), 6, __ProcessEEPROMcommand },
#endif
};

#define CONSOLE_COMMANDS_COUNT  ( sizeof(ConsoleCommandsList) / sizeof(ConsoleCommandsList[0]) ) //!< Command list count of #ConsoleCommandsList[] list

//=============================================================================
// [STATIC] Internal process received command
//=============================================================================
eERRORRESULT __InternalProcessReceivedCommand(const uint8_t* pCmd, size_t size)
{ // It's a weak function, the user need to create the same function in his project and implement things, thus this function will be discarded
  eERRORRESULT Error = ERR__NOT_SUPPORTED;

  //--- Generate hash of first parameter of the string ---
  uint32_t Hash = CONSOLE_HASH_INITIAL_VAL;
  uint32_t HashTable[8] = { CONSOLE_HASH_INITIAL_VAL, 0, 0, 0, 0, 0, 0, 0 };
  for (size_t idx = 0; idx < 8; ++idx)
  {
    if (idx >= size) break;                                             // Check index is out of string
    if ((pCmd[idx] == ' ') || (pCmd[idx] == CONSOLE_NULL)) break;       // Check char is space or null char
    Hash = ((Hash >> 27) | (Hash << 5)) ^ CONSOLE_UPPERCASE(pCmd[idx]); // Do Rol 5 xor of uppercase data
    HashTable[idx] = Hash;                                              // Fill hash table
  }

  //--- Search into command list matching command and execute ---
  for (size_t zIdx = 0; zIdx < CONSOLE_COMMANDS_COUNT; ++zIdx)
  {
    if (ConsoleCommandsList[zIdx].Length > 0)
      if (HashTable[ConsoleCommandsList[zIdx].Length - 1] == ConsoleCommandsList[zIdx].Hash)
      {
        Error = ConsoleCommandsList[zIdx].fnCommandProcess(pCmd, size);   // Process command in associated function
        if (Error == ERR_NONE) break;                                     // Exit if command have been successfully processed, else give if to another hash
      }
  }
  if (Error == ERR__NOT_SUPPORTED)
  {
    return ConsoleRx_ProcessReceivedCommandCallBack(pCmd, size);        // Call to a weak function: If unknown, call the user implementation
  }
  return Error;
}


#if !defined(_MSC_VER)
//=============================================================================
// [WEAK] Process received command Callback
//=============================================================================
eERRORRESULT ConsoleRx_ProcessReceivedCommandCallBack(const uint8_t* pCmd, size_t size)
{
  (void)pCmd;
  (void)size;
  // It's a weak function, the user need to create the same function in his project and implement things, thus this function will be discarded
  return ERR_NONE;
}
#endif // _MSC_VER

//-----------------------------------------------------------------------------



#if defined(USE_CONSOLE_GPIO_COMMANDS) || defined(USE_CONSOLE_EEPROM_COMMANDS)
//=============================================================================
// [STATIC] Compare 2 ANSI strings by only the size of the str2
//=============================================================================
int32_t __strscmp(const uint8_t* pStr1, const char* pStr2)
{
  int32_t Result = 0;

  if (!pStr1 || !pStr2) return INT32_MIN; // Check NULL strings
  while (*pStr1 || *pStr2)
  {
    Result = CONSOLE_LOWERCASE((int32_t)(*pStr1)) - CONSOLE_LOWERCASE((int32_t)(*pStr2));
    if (Result != 0) break;
    ++pStr1;
    ++pStr2;
  }
  if (*pStr2 == 0) return 0;
  return Result;
}


//=============================================================================
// [STATIC] Compare 2 ANSI strings by only the size of the Str2 and update Str1 pointer only if strings are identical
//=============================================================================
int32_t __strtcmp(const uint8_t** pStr1, const char* pStr2)
{
  const uint8_t* pStr = *pStr1;
  int32_t Result = 0;
  size_t CharsCompared = 0;

  if (!pStr || !pStr1 || !pStr2) return INT32_MAX; // Check NULL strings
  while (*pStr || *pStr2)
  {
    Result = CONSOLE_LOWERCASE((int32_t)(*pStr)) - CONSOLE_LOWERCASE((int32_t)(*pStr2));
    if (Result != 0) break;
    ++CharsCompared;
    ++pStr;
    ++pStr2;
  }
  if (*pStr2 == 0)
  {
    (*pStr1) += CharsCompared;
    return 0;
  }
  return Result;
}


//=============================================================================
// [STATIC] Convert a string to int
//=============================================================================
uint32_t __StringToUint(const uint8_t** pStr)
{
  uint32_t Result = 0;
  if (((*pStr)[0] >= '0') && ((*pStr)[0] <= '9')) // Is a digit?
  {
    switch ((*pStr)[1])
    {
      case 'x':
      case 'X':
          *pStr += 2;                             // Start with "0x"? Pass these chars
          //--- Extract uint32 from hex string ---
          while (**pStr != 0)
          {
            uint32_t CurChar = (uint32_t)(**pStr);
            if ((CurChar - 0x30) <= 9u)           // Char in '0'..'9'?
            {
              Result <<= 4;
              Result += (CurChar - 0x30);         // 0x30 for '0'
            }
            else
            {
              CurChar &= 0xDF;                    // Transform 'a'..'f' into 'A'..'F'
              if ((CurChar - 0x41) <= 5u)         // Char in 'A'..'F'?
              {
                Result <<= 4;
                Result += (CurChar - 0x41) + 10u; // 0x41 for 'A' and add 10 to the value
              }
              else break;
            }
            ++(*pStr);                            // Next char
          }
          break;

      case 'b':
      case 'B':
          //--- Extract uint32 from bin string ---
          *pStr += 2;                             // Start with "0b"? Pass these chars
          while (**pStr != 0)
          {
            uint32_t CurChar = (uint32_t)(**pStr);
            if ((CurChar - 0x30) <= 1u)           // Char in '0'..'1'?
            {
              Result <<= 1;
              Result += (CurChar - 0x30);         // 0x30 for '0'
            }
            else break;
            ++(*pStr);                            // Next char
          }
          break;

      default:
          //--- Extract uint32 from string ---
          while (**pStr != 0)
          {
            uint32_t CurChar = (uint32_t)(**pStr);
            if ((CurChar - 0x30) <= 9u)           // Char in '0'..'9'?
            {
              Result *= 10;                       // Multiply the uint part by 10
              Result += (CurChar - 0x30);         // 0x30 for '0'
            }
            else break;
            ++(*pStr);                            // Next char
          }
          break;
    }
  }
  return Result;
}
#endif // USE_CONSOLE_GPIO_COMMANDS || USE_CONSOLE_EEPROM_COMMANDS

//-----------------------------------------------------------------------------

//! Console string + action tuple
typedef struct ConsoleAction
{
  const char* const Str;  //!< Console interface action string
  eConsoleActions Action; //!< Console interface action
} ConsoleAction;

//! GPIO action list array. Order of parse is important
const ConsoleAction GPIOactionsList[] =
{
  { "READ"  , Action_Read  , },
  { "RD"    , Action_Read  , },
  { "WRITE" , Action_Write , },
  { "WR"    , Action_Write , },
  { "SET"   , Action_Set   , },
  { "CLEAR" , Action_Clear , },
  { "CLR"   , Action_Clear , },
  { "TOGGLE", Action_Toggle, },
  { "TG"    , Action_Toggle, },
  { "DIR"   , Action_Dir   , },
};

#define GPIO_ACTION_LIST_COUNT  ( sizeof(GPIOactionsList) / sizeof(GPIOactionsList[0]) )

#ifdef USE_CONSOLE_GPIO_COMMANDS
//=============================================================================
// [STATIC] Process GPIO command
//=============================================================================
eERRORRESULT __ProcessGPIOcommand(const uint8_t* pCmd, size_t size)
{
  if (size < 9) return ERR__PARSE_ERROR;
  uint32_t Value = 0, Mask = 0xFFFFFFFF;
  eConsoleActions Action = Action_None;
  eGPIO_PortPin PORTpin  = No_PORTpin;
  uint8_t PinNum = 0xFF;
  pCmd += 5;                                                             // Go after "GPIO "

  //--- Parse <action> string ---
  for (size_t zIdx = 0; zIdx < GPIO_ACTION_LIST_COUNT; ++zIdx)
  {
    if (__strscmp(pCmd, GPIOactionsList[zIdx].Str) == 0)                 // Compare 2 ANSI strings by only the size of the str2
    {
      Action = GPIOactionsList[zIdx].Action;
      pCmd += strlen(GPIOactionsList[zIdx].Str) + 1;
      break;
    }
  }
  if (Action == Action_None) return ERR__PARSE_ERROR;                    // An <action> shall be found at this point

  //--- Parse <PORT/Pin> string ---
  if ((__strtcmp(&pCmd, "PORT") == 0) || (__strtcmp(&pCmd, "PIO") == 0)) // Is a port
  {
    char PortName = CONSOLE_UPPERCASE(*pCmd);
    if ((PortName >= 'A') && (PortName <= 'Z')) PORTpin = (eGPIO_PortPin)(PortName - 'A' + (char)PORTA);
    if ((PortName >= '0') && (PortName <= '9')) PORTpin = (eGPIO_PortPin)(PortName - '0' + (char)PORT0);
    ++pCmd;
  }
  if ((PORTpin == No_PORTpin) && (CONSOLE_LOWERCASE(pCmd[0]) == 'p'))    // Not a port? Check pin
  {
    char PortName = CONSOLE_UPPERCASE(pCmd[1]);
    if ((PortName >= 'A') && (PortName <= 'Z')) PORTpin = (eGPIO_PortPin)(PortName - 'A' + (char)PA);
//    if ((PortName >= '0') && (PortName <= '9')) PORTpin = (eGPIO_PortPin)(PortName - '0' + (char)P0);
    pCmd += 2;
    PinNum = __StringToUint(&pCmd);                                      // Extract pin number
    if ((*pCmd != ' ') && (*pCmd != 0)) return ERR__PARSE_ERROR;         // Check the good position of the parse
    if (*pCmd != 0) ++pCmd;
  }
  if (PORTpin == No_PORTpin) return ERR__PARSE_ERROR;                    // A <PORT/Pin> shall be found at this point

  //--- Parse <value> string ---
  if (*pCmd != 0)
  {
    Value = __StringToUint(&pCmd);                                       // Extract <value>
    if ((*pCmd != ' ') && (*pCmd != 0)) return ERR__PARSE_ERROR;         // Check the good position of the parse
    if (*pCmd != 0) ++pCmd;
  }

  //--- Parse <mask> string ---
  if (*pCmd != 0)
  {
    Mask = __StringToUint(&pCmd);                                        // Extract <mask>
    if (*pCmd != 0) return ERR__PARSE_ERROR;                             // Check the good position of the parse
  }

  //--- Set GPIO command ---
  ConsoleRx_GPIOcommandCallBack(Action, PORTpin, PinNum, Value, Mask);
  return ERR_NONE;
}

#if !defined(_MSC_VER)
//==============================================================================
// Process GPIO command Callback
//==============================================================================
void ConsoleRx_GPIOcommandCallBack(eConsoleActions action, eGPIO_PortPin portPin, uint8_t pinNum, uint32_t value, uint32_t mask)
{
  (void)action;
  (void)portPin;
  (void)pinNum;
  (void)value;
  (void)mask;
  // It's a weak function, the user need to create the same function in his project and implement things, thus this function will be discarded
}
#endif // !_MSC_VER
#endif // USE_CONSOLE_GPIO_COMMANDS

//-----------------------------------------------------------------------------

#ifdef USE_CONSOLE_EEPROM_COMMANDS

//=============================================================================
// [STATIC] Process EEPROM command
//=============================================================================
eERRORRESULT __ProcessEEPROMcommand(const uint8_t* pCmd, size_t size)
{
  if (size < 8) return ERR__PARSE_ERROR;
  uint32_t Value = 0;
  pCmd += 7;                                                             // Go after "EEPROM "

  //--- Parse <action> string ---
  for (size_t zIdx = 0; zIdx < EEPROM_ACTION_LIST_COUNT; ++zIdx)
  {
    if (__strscmp(pCmd, EEPROMactionsList[zIdx].Str) == 0)               // Compare 2 ANSI strings by only the size of the str2
    {
      Action = EEPROMactionsList[zIdx].Action;
      pCmd += strlen(EEPROMactionsList[zIdx].Str) + 1;
      break;
    }
  }
  if (Action == Action_None) return ERR__PARSE_ERROR;                    // An <action> shall be found at this point


  //--- Set EEPROM command ---
  ConsoleRx_EEPROMcommandCallBack();
  return ERR_NONE;
}

#if !defined(_MSC_VER)
//==============================================================================
// Process EEPROM command Callback
//==============================================================================
void ConsoleRx_EEPROMcommandCallBack(void)
{
  // It's a weak function, the user need to create the same function in his project and implement things, thus this function will be discarded
}
#endif // !_MSC_VER
#endif

//-----------------------------------------------------------------------------
#endif /* USE_CONSOLE_RX */
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif