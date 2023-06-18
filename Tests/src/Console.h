/*!*****************************************************************************
 * @file    Console.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.1.0
 * @date    04/06/2023
 * @brief   Some functions for RS-232 console communication
 ******************************************************************************/
/* @page License
 *
 * Copyright (c) 2020-2023 Fabien MAILLY
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

/* Revision history:
 * 1.1.0    Add console Rx basic commands
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef CONSOLE_H_
#define CONSOLE_H_
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "UART_Interface.h"
//-----------------------------------------------------------------------------
#if !defined(__cplusplus)
# include "main.h"
#else
  extern "C" {
  void SetConsoleColor(int text, int fond);
#endif
//-----------------------------------------------------------------------------

#if !defined(__cplusplus)
# define CONSOLE_MEMBER(name)  .name =
# define CONSOLE_WEAK          __attribute__((weak))
# define CONSOLE_EXTERN
# define __FORMATPRINTF12__    __attribute__((__format__(__printf__, 1, 2))) // 1: Format at first argument  ; 2: args at second argument (...)
# define __FORMATPRINTF20__    __attribute__((__format__(__printf__, 2, 0))) // 3: Format at second argument ; 0: for va_list
# define __FORMATPRINTF23__    __attribute__((__format__(__printf__, 2, 3))) // 2: Format at second argument ; 3: args at third argument (...)
# define __FORMATPRINTF30__    __attribute__((__format__(__printf__, 3, 0))) // 3: Format at third argument  ; 0: for va_list
# define __FORMATPRINTF34__    __attribute__((__format__(__printf__, 3, 4))) // 3: Format at third  argument ; 4: args at fourth argument (...)
# define __FORMATPRINTF40__    __attribute__((__format__(__printf__, 4, 0))) // 4: Format at fourth argument ; 0: for va_list
# define vprintf               viprintf
#else
# define CONSOLE_MEMBER(name)
# if defined(_MSC_VER)
#   define CONSOLE_WEAK
#   define CONSOLE_EXTERN      extern
# else
#   define CONSOLE_WEAK        __attribute__((weak))
#   define CONSOLE_EXTERN
# endif
# define __FORMATPRINTF12__
# define __FORMATPRINTF20__
# define __FORMATPRINTF23__
# define __FORMATPRINTF30__
# define __FORMATPRINTF34__
# define __FORMATPRINTF40__
#endif

//-----------------------------------------------------------------------------

//! Macro to get the lower case of a char
#define CONSOLE_LOWERCASE(aChar)  ( (((aChar) >= 'A') && ((aChar) <= 'Z')) ? ((aChar)+32) : (aChar) )

//! Macro to get the upper case of a char
#define CONSOLE_UPPERCASE(aChar)  ( (((aChar) >= 'a') && ((aChar) <= 'z')) ? ((aChar)-32) : (aChar) )

//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//********************************************************************************************************************
// Console Transmit API
//********************************************************************************************************************
#ifdef USE_CONSOLE_TX

//! Circular Buffer for Console transmit structure
typedef struct ConsoleTx ConsoleTx;
struct ConsoleTx
{
  void *UserAPIData;      //!< Optional, can be used to store API data or NULL

  //--- Interface driver call functions ---
#ifdef USE_DYNAMIC_INTERFACE
  UART_Interface* UART;   //!< This is the UART_RxInterface descriptor pointer that will be used to communicate with the device
#else
  UART_Interface  UART;   //!< This is the UART_RxInterface descriptor that will be used to communicate with the device
#endif

  //--- Transmit buffer ---
  volatile size_t InPos;  //!< This is the input position in the buffer (where data will be write before being send to UART)
  volatile size_t OutPos; //!< This is the output position in the buffer (where data will be read and send to UART)
  char *Buffer;           //!< The buffer itself (should be the same size as BufferSize)
  size_t BufferSize;      //!< The buffer size
};

//-----------------------------------------------------------------------------


/*! @brief Initialize the Console transmit
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT InitConsoleTx(ConsoleTx* pApi);

/*! @brief Set char to print buffer
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] aChar Is the char to be sent
 */
void SetCharToConsoleBuffer(ConsoleTx* pApi, const char aChar);

/*! @brief Set char array to print buffer
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *string Pointer to one-line string to be sent (will stop at the first '\0' char)
 */
void SetStrToConsoleBuffer(ConsoleTx* pApi, const char* string);

/*! @brief Try to send next char in the console print buffer
 *
 * This function is for programs that need to send only in a main loop like in a while(true){} in the main
 * @param[in] *pApi Is the Console transmit API to work with
 * @return If the next char is actually sending
 */
bool TrySendingNextCharToConsole(ConsoleTx* pApi);

/*! @brief Indicate if there is a char to send to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @return If there is a char to send to console
 */
inline bool IsCharToSendToConsole(ConsoleTx* pApi)
{
  if (pApi == NULL) return false;
  return (pApi->OutPos != pApi->InPos);
}

/*! @brief Convert a char to a digit value
 *
 * @param[in] aChar The char to convert
 * @return return the digit value
 */
inline uint32_t CharToDigit(const char aChar)
{
  return (uint32_t)(aChar - '0');
}

//-----------------------------------------------------------------------------

//! Log type, sorted by severity.
typedef enum
{
  lsTitle,   //!< Show a title
  lsFatal,   //!< Fatal error! application will abort shortly
  lsError,   //!< Error! the application may work improperly
  lsWarning, //!< Warning! There's something wrong
  lsInfo,    //!< For your information, the application is safe
  lsTrace,   //!< Trace log only
  lsDebug,   //!< For Debugging purpose. Emitted only when DEBUG is defined
  lsSpecial, //!< For Debugging purpose. Emitted only when DEBUG is defined
  lsLast_,   //!< Special value. Do not use and keep this the last value
} eSeverity;

//! Windows console colors
typedef enum
{
  wccBLACK   = 0,
  wccNAVY    = 1,
  wccGREEN   = 2,
  wccTEAL    = 3,
  wccMAROON  = 4,
  wccPURPLE  = 5,
  wccOLIVE   = 6,
  wccSILVER  = 7,
  wccGRAY    = 8,
  wccBLUE    = 9,
  wccLIME    = 10,
  wccAQUA    = 11,
  wccRED     = 12,
  wccFUCHISA = 13,
  wccYELLOW  = 14,
  wccWHITE   = 15,
  wccLast_, // KEEP LAST!
} eWinConsoleColor;

//-----------------------------------------------------------------------------


/*! @brief Send a formated Logs to console
 *
 * @note DO NOT USE DIRECTLY, use LOG*() instead.
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *context Context string (usually, the system name emitting the log).
 * @param[in] whiteText If the text after the time counter change to white
 * @param[in] *format Format string (printf format), followed by arguments.
 * @param[in] args Arguments for format specification
 */
void __LOG(ConsoleTx* pApi, const char* context, bool whiteText, const char* format, va_list args) __FORMATPRINTF40__;

/*! @brief Send a formated Logs to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] severity This is the log severity.
 * @param[in] *format Format string (printf format), followed by arguments.
 * @param[in] ... Arguments for format specification
 */
void LOG(ConsoleTx* pApi, eSeverity severity, const char* format, ...) __FORMATPRINTF34__;

//-----------------------------------------------------------------------------


#ifdef __cplusplus
/*! @brief Set the Windows console color
 *
 * @param[in] text Is the text color of the Windows console
 * @param[in] fond Is the background color of the Windows console
 */
void SetConsoleColor(eWinConsoleColor text, eWinConsoleColor background);

/*! @brief Send a formated Simulation Logs to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *format Format string (printf format), followed by arguments.
 * @param[in] ... Arguments for format specification
 */
void LOGSIM(ConsoleTx* pApi, const char* format, ...);
#endif

//-----------------------------------------------------------------------------

#if (defined(DEBUG) || defined(_DEBUG))
/*! @brief Show the hexadecimal dump of the memory to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *context Is the text to show for the dump
 * @param[in] *src This is the source pointer of the begining of data to dump
 * @param[in] size The size of data to dump.
 */
void __HexDump(ConsoleTx* pApi, const char* context, const void* src, unsigned int size);

/*! @brief Show the binary dump of the memory to console
 *
 * @param[in] *pApi Is the Console transmit API to work with
 * @param[in] *context Is the text to show for the dump
 * @param[in] *src This is the source pointer of the begining of data to dump
 * @param[in] size The size of data to dump
 */
void __BinDump(ConsoleTx* pApi, const char* context, const void* src, unsigned int size);
#endif

//-----------------------------------------------------------------------------

//! Log Title, use it instead of LOG!
#define LOGTITLE_(api, format, ...)            LOG(api, lsTitle, format, ##__VA_ARGS__)
//! Log Fatal, use it instead of LOG!
#define LOGFATAL_(api, format, ...)            LOG(api, lsFatal, format, ##__VA_ARGS__)
//! Log Error, use it instead of LOG!
#define LOGERROR_(api, format, ...)            LOG(api, lsError, format, ##__VA_ARGS__)
//! Log Warning, use it instead of LOG!
#define LOGWARN_(api, format, ...)             LOG(api, lsWarning, format, ##__VA_ARGS__)
//! Log Information, use it instead of LOG!
#define LOGINFO_(api, format, ...)             LOG(api, lsInfo, format, ##__VA_ARGS__)
//! Log Trace, use it instead of LOG!
#define LOGTRACE_(api, format, ...)            LOG(api, lsTrace, format, ##__VA_ARGS__)
#if (defined(DEBUG) || defined(_DEBUG))
    //! Log Debug, use it instead of LOG!
#   define LOGDEBUG_(api, format, ...)         LOG(api, lsDebug, format, ##__VA_ARGS__)
    //! Log Special, use it instead of LOG!
#   define LOGSPECIAL_(api, format, ...)       LOG(api, lsSpecial, format, ##__VA_ARGS__)
    //! Hexadecimal dump of memory
#   define HEXDUMP_(api, context, src, size)   __HexDump(api, context, src, size)
    //! Binary dump of memory
#   define BINDUMP_(api, context, src, size)   __BinDump(api, context, src, size)
#else
#   define LOGDEBUG_(api, format, ...)         do{}while(false)
#   define LOGSPECIAL_(api, format, ...)       do{}while(false)
#   define HEXDUMP_(api, context, src, size)   do{}while(false)
#   define BINDUMP_(api, context, src, size)   do{}while(false)
#endif

#else /* USE_CONSOLE_TX */

//! Log Title, use it instead of LOG!
#define LOGTITLE_(api, format, ...)            do{}while(false)
//! Log Fatal, use it instead of LOG!
#define LOGFATAL_(api, format, ...)            do{}while(false)
//! Log Error, use it instead of LOG!
#define LOGERROR_(api, format, ...)            do{}while(false)
//! Log Warning, use it instead of LOG!
#define LOGWARN_(api, format, ...)             do{}while(false)
//! Log Information, use it instead of LOG!
#define LOGINFO_(api, format, ...)             do{}while(false)
//! Log Trace, use it instead of LOG!
#define LOGTRACE_(api, format, ...)            do{}while(false)
//! Log Debug, use it instead of LOG!
#define LOGDEBUG_(api, format, ...)            do{}while(false)
//! Log Special, use it instead of LOG!
#define LOGSPECIAL_(api, format, ...)          do{}while(false)
//! Hexadecimal dump of memory
#define HEXDUMP_(api, context, src, size)      do{}while(false)
//! Binary dump of memory
#define BINDUMP_(api, context, src, size)      do{}while(false)

//-----------------------------------------------------------------------------
#endif /* USE_CONSOLE_TX */





//********************************************************************************************************************
// Console Receive API
//********************************************************************************************************************

/*! @defgroup ConsoleHash Calculus of the Rx console hash for commands
 * @details The hash of commands allows to do compiler time hash generation for all commands supported by a RX console
 */
//! @addtogroup ConsoleHash
//! @{
#define CONSOLE_HASH_INITIAL_VAL              ( 0x00000000u )                                                            //!< Initial value of the ROL5 XOR hash
#define CONSOLE_ROL5_XOR_CHAR(value,newData)  ( newData > 0 ? ((((value) >> 27) | ((value) << 5)) ^ (newData)) : value ) //!< Perform a Rol 5 of hash following a xor of the char of the string

/*! @brief Get the ROL5 XOR hash of a maximum 8-char string
 * @warning Use this macro only on constant value of n to use the compiler simplification, else create a function with the code in details
 * @details This is the equivalent of: @code{ uint32_t Hash = CONSOLE_HASH_INITIAL_VAL; for (size_t idx = 0; idx < 8; ++idx) { uint32_t newData = (idx < (sizeof(str) - 1) ? str[idx] : 0); newData = (((newData >= 'a') && (newData <= 'z')) ? (newData - 32) : newData); Hash = ((Hash >> 27) | (Hash << 5)) ^ newData; } return Hash; }@endcode
 * @note Unfortunately, it is impossible to do a str[idx] in C pre-processing so all the first 8 chars shall be set manually
 * @param[in] char1 Is the first char of the string for which to generate a hash
 * @param[in] char2 Is the second char of the string for which to generate a hash
 * @param[in] char3 Is the third char of the string for which to generate a hash
 * @param[in] char4 Is the fourth char of the string for which to generate a hash
 * @param[in] char5 Is the fifth char of the string for which to generate a hash
 * @param[in] char6 Is the sixth char of the string for which to generate a hash
 * @param[in] char7 Is the seventh char of the string for which to generate a hash
 * @param[in] char8 Is the eighth char of the string for which to generate a hash
 * @return The ROL5 XOR hash of the 8 chars not '\0' set in argument
 */
#define CONSOLE_ROL5XOR_HASH(char1, char2, char3, char4, char5, char6, char7, char8)  ( CONSOLE_ROL5_XOR_CHAR(                                                     \
                                                                                        CONSOLE_ROL5_XOR_CHAR(                                                     \
                                                                                        CONSOLE_ROL5_XOR_CHAR(                                                     \
                                                                                        CONSOLE_ROL5_XOR_CHAR(                                                     \
                                                                                        CONSOLE_ROL5_XOR_CHAR(                                                     \
                                                                                        CONSOLE_ROL5_XOR_CHAR(                                                     \
                                                                                        CONSOLE_ROL5_XOR_CHAR(                                                     \
                                                                                        CONSOLE_ROL5_XOR_CHAR(CONSOLE_HASH_INITIAL_VAL, CONSOLE_UPPERCASE(char1)), \
                                                                                                                                        CONSOLE_UPPERCASE(char2)), \
                                                                                                                                        CONSOLE_UPPERCASE(char3)), \
                                                                                                                                        CONSOLE_UPPERCASE(char4)), \
                                                                                                                                        CONSOLE_UPPERCASE(char5)), \
                                                                                                                                        CONSOLE_UPPERCASE(char6)), \
                                                                                                                                        CONSOLE_UPPERCASE(char7)), \
                                                                                                                                        CONSOLE_UPPERCASE(char8)) )
//! @}

//-----------------------------------------------------------------------------

#ifdef USE_CONSOLE_RX

/*! @defgroup ConsoleReceive Console receive API
 * @details The console receive API helps to perform received commands from a console
 */
//! @addtogroup ConsoleReceive
//! @{
#define CONSOLE_NULL  0x00 //!< Null            '\0'
#define CONSOLE_SOH   0x01 //!< Start Of Heading
#define CONSOLE_STX   0x02 //!< Start of TeXt
#define CONSOLE_ETX   0x03 //!< End of TeXt
#define CONSOLE_EOT   0x04 //!< End Of Transmission
#define CONSOLE_ENQ   0x05 //!< ENQuiry (End of Line)
#define CONSOLE_ACK   0x06 //!< ACKnowledge
#define CONSOLE_BEL   0x07 //!< BELl            '\a'
#define CONSOLE_BS    0x08 //!< BackSpace       '\b'
#define CONSOLE_HT    0x09 //!< Horizontal Tab  '\t'
#define CONSOLE_LF    0x0A //!< Line Feed       '\n'
#define CONSOLE_VT    0x0B //!< Vertical Tab    '\v'
#define CONSOLE_FF    0x0C //!< Form Feed
#define CONSOLE_CR    0x0D //!< Carriage Return '\r'
#define CONSOLE_SO    0x0E //!< Shift Out
#define CONSOLE_SI    0x0F //!< Shift In
#define CONSOLE_DLE   0x10 //!< Data Link Escape
#define CONSOLE_DC1   0x11 //!< Device Control 1
#define CONSOLE_DC2   0x12 //!< Device Control 2
#define CONSOLE_DC3   0x13 //!< Device Control 3
#define CONSOLE_DC4   0x14 //!< Device Control 4
#define CONSOLE_NAK   0x15 //!< Negative AcKnowledge
#define CONSOLE_SYN   0x16 //!< Synchronous Idle
#define CONSOLE_ETB   0x17 //!< End of Transmission Block
#define CONSOLE_CAN   0x18 //!< CANcel
#define CONSOLE_SUB   0x1A //!< SUBstitute
#define CONSOLE_ESC   0x1B //!< ESCape
#define CONSOLE_FS    0x1C //!< File Separator
#define CONSOLE_GS    0x1D //!< Group Separator
#define CONSOLE_RS    0x1E //!< Record Separator
#define CONSOLE_US    0x1F //!< Unit Separator
#define CONSOLE_DEL   0x7F //!< DELete

//-----------------------------------------------------------------------------

#if !defined(CONSOLE_RX_CURRENT_BUFFER_SIZE)
#  define CONSOLE_RX_CURRENT_BUFFER_SIZE  (size_t)50 //!< Console Rx command acquisition default buffer size (for CurrentBuff)
#endif

//! Circular Buffer for Console receive structure
typedef struct ConsoleRx ConsoleRx;
struct ConsoleRx
{
  void *UserAPIData;          //!< Optional, can be used to store API data or NULL

  //--- Interface driver call functions ---
#ifdef USE_DYNAMIC_INTERFACE
  UART_Interface* UART;       //!< This is the UART_RxInterface descriptor pointer that will be used to communicate with the device
#else
  UART_Interface  UART;       //!< This is the UART_RxInterface descriptor that will be used to communicate with the device
#endif

  //--- Receive buffer ---
  volatile size_t RxIdx;      //!< This is the receive input index in the buffer (where data will be write after receive from UART)
  volatile size_t ProcessIdx; //!< This is the processing index in the buffer (where data will be read from CurrentBuff when processed)
  size_t CursorIdx;           //!< This is the cursor index in the buffer (where data will be inserted into CurrentBuff when processed)
  uint8_t CurrentBuff[CONSOLE_RX_CURRENT_BUFFER_SIZE]; // Current receive buffer (working buffer)

#ifdef CONSOLE_RX_USE_COMMAND_RECALL
  //--- Command buffer ---
  size_t StartIdx;            //!< This is the start of command line index in the buffer
  size_t CurrentIdx;          //!< This is the current index of the command line in the buffer
  char *CommandBuffer;        //!< The buffer itself (should be the same size as BufferSize)
  size_t BufferSize;          //!< The buffer size
#endif
};

//-----------------------------------------------------------------------------



/*! @brief Initialize the Console receive
 *
 * @param[in] *pApi Is the Console receive API to work with
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT InitConsoleRx(ConsoleRx* pApi);

/*! @brief Receive char from console
 * @note This can be used in an interrupt
 *
 * @param[in] *pApi Is the Console receive API to work with
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT ConsoleRx_ReceiveChar(ConsoleRx* pApi);

/*! @brief Process received char from console
 *
 * @param[in] *pApi Is the Console receive API to work with
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT ConsoleRx_ProcessReceivedChars(ConsoleRx* pApi);

/*! @brief Process received command Callback
 * @note This function is weak. Thus the used shall implement is own function
 *
 * @param[in] *pCmd Is the command string first char (NULL terminated string)
 * @param[in] size Is the char count of the command string pCmd
 * @return Returns an #eERRORRESULT value enum
 */
CONSOLE_EXTERN eERRORRESULT ConsoleRx_ProcessReceivedCommandCallBack(const uint8_t* pCmd, size_t size) CONSOLE_WEAK;

//-----------------------------------------------------------------------------


/*! @brief Specific command function
 * @param[in] *pCmd Is the command string first char (NULL terminated string)
 * @param[in] size Is the char count of the command string pCmd
 * @return Returns an #eERRORRESULT value enum
 */
typedef eERRORRESULT (*RxCommand_Func)(const uint8_t* pCmd, size_t size);

/*! Console command hash + function tuple
 * @details The first member of each supported commands is a hash of a specific string.
 * This string is the first parameter of the string (ie. from index 0 to the first space or null character
 * The second is the function that will be called if the hash match
 */
typedef struct ConsoleCommand
{
  uint32_t Hash;                   //!< Hash of the first parameter of the command
  size_t Length;                   //!< Length of the string in parameter
  RxCommand_Func fnCommandProcess; //!< This function will be called when the hash will match
} ConsoleCommand;

//-----------------------------------------------------------------------------

//! Console interfaces actions enumerator
typedef enum
{
  Action_None,   //!< No console action
  Action_Read,   //!< Console action read
  Action_Write,  //!< Console action write
  Action_Set,    //!< Console action set
  Action_Clear,  //!< Console action clear
  Action_Toggle, //!< Console action toggle
  Action_Dir,    //!< Console action direction
} eConsoleActions;

//-----------------------------------------------------------------------------

#ifdef USE_CONSOLE_GPIO_COMMANDS
//! GPIO PORT/pin enumerator
typedef enum
{
  No_PORTpin,                                      //!< No PORT/pin selected
  PORTA, PORTB, PORTC, PORTD, PORTE, PORTF, PORTG,
  PORTH, PORTI, PORTJ, PORTK, PORTL, PORTM, PORTN,
  PORTO, PORTP, PORTQ, PORTR, PORTS, PORTT, PORTU,
  PORTV, PORTW, PORTX, PORTY, PORTZ,
  PORTa_Max,                                       //!< Maximum alphabetical PORT Names
  PORT0, PORT1, PORT2, PORT3, PORT4,
  PORT5, PORT6, PORT7, PORT8, PORT9,
  PORTx_Max,                                       //!< Maximum numerical PORT Names
  PA, PB, PC, PD, PE, PF, PG, PH, PI, PJ, PK, PL, PM,
  PN, PO, PP, PQ, PR, PS, PT, PU, PV, PW, PX, PY, PZ,
  Pa_Max,                                          //!< Maximum alphabetical pin on port names
//  P0, P1, P2, P3, P4, P5, P6, P7, P8, P9,
//  Px_Max,                                          //!< Maximum numerical pin on port names
} eGPIO_PortPin;

/*! @brief Process GPIO command Callback
 * @note This function is weak. Thus the used shall implement is own function
 * @details GPIO Commands parsed are the following:
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
 *    - Pxy: where 'x' can be any of the ports name and 'y' the number of the pin of the MCU (ex: PA0)
 *  - <value> is the value to apply in case of Write, Set, or Clear. The value can be binary (0b prefix), decimal, hexadecimal (0x prefix). The default value will be 0
 *  - <mask> is the mask value to apply with the 'value' in case of PORT use. The mask can be binary (0b prefix), decimal, hexadecimal (0x prefix). The default value will be 0xFFFFFFFF
 *
 * @param[in] action Is the GPIO command action decoded
 * @param[in] portPin Is the GPIO command PORT/Pin decoded
 * @param[in] pinNum Is the pin number when portPin is a pin on port value. Is 0xFF when no pin specified
 * @param[in] value Is the GPIO command value decoded
 * @param[in] mask Is the GPIO command value mask decoded
 */
CONSOLE_EXTERN void ConsoleRx_GPIOcommandCallBack(eConsoleActions action, eGPIO_PortPin portPin, uint8_t pinNum, uint32_t value, uint32_t mask) CONSOLE_WEAK;
#endif // USE_CONSOLE_GPIO_COMMANDS

#ifdef USE_CONSOLE_EEPROM_COMMANDS

/*! @brief Process EEPROM command Callback
 * @note This function is weak. Thus the used shall implement is own function
 * @details EEPROM Commands parsed are the following:
 * EEPROM<x> <action>[ <value>][ <mask>]
 * Where:
 *  - <x> is the eeprom index to use
 *  - <action> can be:
 *    - RD, READ: Read EEPROMx at <Address>
 *    - WR, WRITE: Write <value> to  EEPROMx at <Address>
 *    - SET: Set bitset of <value> to  EEPROMx at <Address>
 *    - CLR, CLEAR: Clear bitset of <value> to  EEPROMx at <Address>
 *    - TG, TOGGLE: Toggle the value of  EEPROMx at <Address>
 *  - <value> is the value to apply in case of Write, Set, or Clear. The value can be binary (0b prefix), decimal, hexadecimal (0x prefix). The default value will be 0
 *  - <mask> is the mask value to apply with the 'value' in case of PORT use. The mask can be binary (0b prefix), decimal, hexadecimal (0x prefix). The default value will be 0xFFFFFFFF
 *
 * @param[in] action Is the GPIO command action decoded
 * @param[in] portPin Is the GPIO command PORT/Pin decoded
 * @param[in] pinNum Is the pin number when portPin is a pin on port value. Is 0xFF when no pin specified
 * @param[in] value Is the GPIO command value decoded
 * @param[in] mask Is the GPIO command value mask decoded
 */
CONSOLE_EXTERN void ConsoleRx_EEPROMcommandCallBack(void) CONSOLE_WEAK;
#endif // USE_CONSOLE_EEPROM_COMMANDS

//-----------------------------------------------------------------------------
//! @}
#endif /* USE_CONSOLE_RX */
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* CONSOLE_H_ */