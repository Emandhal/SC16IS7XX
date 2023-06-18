/*!*****************************************************************************
 * @file    Console_V71InterfaceSync.h
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.1.0
 * @date    04/06/2023
 * @brief   Console interface for the Console Transmit and Receive
 *          This unit interface the Console API with the current hardware
 *          This interface implements the synchronous use of the API on a SAMV71
 *          and is also specific with the SAMV71 Xplained Ultra board
*******************************************************************************/

//-----------------------------------------------------------------------------
#include "Console_V71Interface.h"
//-----------------------------------------------------------------------------
#if !defined(__cplusplus)
#  include <asf.h>
#else
#  include <cstdint>
extern "C" {
#endif
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//********************************************************************************************************************
// UART of V71
//********************************************************************************************************************
//! UART console interface definition for dynamic interfaces
#ifdef USE_DYNAMIC_INTERFACE
static UART_Interface Console_UART =
{
  UART_MEMBER(InterfaceDevice) CONSOLE_UART,
  UART_MEMBER(fnUART_Transmit) UARTtransmit_V71,
  UART_MEMBER(fnUART_Receive)  UARTreceive_V71,
  UART_MEMBER(Channel)         0,
};
#endif

//-----------------------------------------------------------------------------


//=============================================================================
// Console UART Tx initialization for the ATSAMV71
//=============================================================================
void ConsoleUART_TxInit_V71(void)
{
  usart_serial_options_t uart_serial_options = {};
  uart_serial_options.baudrate   = CONF_CONSOLE_BAUDRATE;
  uart_serial_options.charlength = CONF_CONSOLE_CHAR_LENGTH;
  uart_serial_options.paritytype = CONF_CONSOLE_PARITY;
  uart_serial_options.stopbits   = CONF_CONSOLE_STOP_BITS;

  //--- Enable the peripheral clock in the PMC ---
  sysclk_enable_peripheral_clock(CONSOLE_UART_ID);

  //--- Configure console UART ---
  //  usart_serial_init(CONSOLE_UART, &uart_serial_options); // For Console API
  stdio_serial_init(CONSOLE_UART, &uart_serial_options); // For printf

  //--- Enable Tx function ---
  usart_enable_tx(CONSOLE_UART);

  //--- Configure and enable interrupt of USART ---
  usart_disable_interrupt(CONSOLE_UART, US_IER_TXRDY | US_IER_TXEMPTY);
  NVIC_EnableIRQ(USART1_IRQn);                                          // *** USE WITH INTERRUPT CHAR SEND. IN SEND WHILE IDLE (while(true) in the main()) COMMENT THIS LINE
}


//=============================================================================
// UART transmit char function interface of the ATSAMV71
//=============================================================================
eERRORRESULT UARTtransmit_V71(UART_Interface *pIntDev, uint8_t *data, size_t size, size_t *actuallySent)
{
#ifdef CHECK_NULL_PARAM
  if (pIntDev == NULL) return ERR__PARAMETER_ERROR;
#endif
  Usart* pUART = (Usart*)(pIntDev->InterfaceDevice); // Get the V71 USART device of this UART port
  *actuallySent = 0;
  if (size <= 0) return ERR_NONE;
  if ((pUART->US_CSR & US_CSR_TXRDY  ) == 0) return ERR__NOT_READY; // Character is in the US_THR
  if ((pUART->US_CSR & US_CSR_TXEMPTY) == 0) return ERR__NOT_READY;
//  if ((pUART->US_IMR & US_IMR_TXRDY) >  0) return ERR__NOT_READY; // TX Ready interrupt is set  // *** USE WITH INTERRUPT CHAR SEND. IN SEND WHILE IDLE (while(true) in the main()) COMMENT THIS LINE

  pUART->US_THR = US_THR_TXCHR(*data);             // Send the char
  pUART->US_IER = (US_IER_TXRDY | US_IER_TXEMPTY); // Enable interrupts
  *actuallySent = 1;                               // Always 1 by 1 with this USART
  return ERR_NONE;
}

//-----------------------------------------------------------------------------


//=============================================================================
// Console UART Rx initialization for the ATSAMV71
//=============================================================================
void ConsoleUART_RxInit_V71(void)
{
  usart_serial_options_t uart_serial_options = {};
  uart_serial_options.baudrate   = CONF_CONSOLE_BAUDRATE;
  uart_serial_options.charlength = CONF_CONSOLE_CHAR_LENGTH;
  uart_serial_options.paritytype = CONF_CONSOLE_PARITY;
  uart_serial_options.stopbits   = CONF_CONSOLE_STOP_BITS;

  //--- Enable the peripheral clock in the PMC ---
  sysclk_enable_peripheral_clock(CONSOLE_UART_ID);

  //--- Configure console UART ---
  //  usart_serial_init(CONSOLE_UART, &uart_serial_options); // For Console API
  stdio_serial_init(CONSOLE_UART, &uart_serial_options); // For printf

  //--- Enable Rx function ---
  usart_enable_rx(CONSOLE_UART);

  //--- Configure and enable interrupt of USART ---
  usart_enable_interrupt(CONSOLE_UART, US_IER_RXRDY);
  NVIC_EnableIRQ(USART1_IRQn);
}


//=============================================================================
// UART receive char function interface of the ATSAMV71
//=============================================================================
eERRORRESULT UARTreceive_V71(UART_Interface *pIntDev, uint8_t *data, size_t size, size_t *actuallyReceived, uint8_t *lastCharError)
{
#ifdef CHECK_NULL_PARAM
  if (pIntDev == NULL) return ERR__PARAMETER_ERROR;
#endif
  Usart* pUART = (Usart*)(pIntDev->InterfaceDevice); // Get the V71 USART device of this UART port
  *actuallyReceived = 0;
  if (size <= 0) return ERR_NONE;
  if ((pUART->US_CSR & US_CSR_RXRDY) == 0) return ERR__NO_DATA_AVAILABLE;

  *data = (char)(pUART->US_RHR & US_RHR_RXCHR_Msk); // Get the char
  *actuallyReceived = 1;                            // Always 1 by 1 with this USART
  *lastCharError = pUART->US_CSR & 0xE4;            // Get only Rx errors
  return ERR_NONE;
}

//-----------------------------------------------------------------------------


//! The current Command Input buffer
CommandInputBuf CommandInput;

//=============================================================================
// Handler for Console USART interrupt.
//=============================================================================
void USART1_Handler(void)
{
  //--- Transmission interrupts ---
#ifdef USE_CONSOLE_TX
  if ((CONSOLE_UART->US_CSR & (US_CSR_TXRDY | US_CSR_TXEMPTY)) > 0) // Transmit interrupt rises
  {
    CONSOLE_UART->US_IDR = (US_IDR_TXRDY | US_IDR_TXEMPTY);         // Disable interrupts
    TrySendingNextCharToConsole(CONSOLE_TX);
  }
#endif

  //--- Reception interrupts ---
#ifdef USE_CONSOLE_RX
  if ((CONSOLE_UART->US_CSR & US_CSR_RXRDY) > 0)                    // Receive interrupt rises
  {
    ConsoleRx_ReceiveChar(CONSOLE_RX);
  }
#endif
}

//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//********************************************************************************************************************
// Console Transmit API
//********************************************************************************************************************
#ifdef USE_CONSOLE_TX

//! Console Tx configuration
ConsoleTx Console_TxConf =
{
  CONSOLE_MEMBER(UserAPIData) NULL,
  //--- Interface driver call functions ---
#ifdef USE_DYNAMIC_INTERFACE
  CONSOLE_MEMBER(UART) &Console_UART,
#else
  CONSOLE_MEMBER(UART)
  {
    UART_MEMBER(InterfaceDevice) CONSOLE_UART,
    UART_MEMBER(fnUART_Transmit) UARTtransmit_V71,
    UART_MEMBER(fnUART_Receive)  NULL,             // Not used for Tx
    UART_MEMBER(Channel)         0,
  },
#endif
  //--- Transmit buffer ---
  CONSOLE_MEMBER(Buffer    ) &ConsoleTxBuffer[0],
  CONSOLE_MEMBER(BufferSize) CONSOLE_TX_BUFFER_SIZE,
};

#endif // USE_CONSOLE_TX
//-----------------------------------------------------------------------------





//**********************************************************************************************************************************************************
//********************************************************************************************************************
// Console Receive API
//********************************************************************************************************************
#ifdef USE_CONSOLE_RX

//! Console Rx configuration
ConsoleRx Console_RxConf =
{
  CONSOLE_MEMBER(UserAPIData) NULL,
  //--- Interface driver call functions ---
#ifdef USE_DYNAMIC_INTERFACE
  CONSOLE_MEMBER(UART) &Console_UART,
#else
  CONSOLE_MEMBER(UART)
  {
    UART_MEMBER(InterfaceDevice) CONSOLE_UART,
    UART_MEMBER(fnUART_Transmit) NULL,            // Not used for Rx
    UART_MEMBER(fnUART_Receive)  UARTreceive_V71,
    UART_MEMBER(Channel)         0,
  },
#endif

#ifdef CONSOLE_RX_USE_COMMAND_RECALL
  //--- Command buffer ---
  CONSOLE_MEMBER(CommandBuffer) &ConsoleRxCommandBuffer[0],
  CONSOLE_MEMBER(BufferSize   ) CONSOLE_RX_COMMAND_BUFFER_SIZE,
#endif
};

//-----------------------------------------------------------------------------


#ifdef USE_CONSOLE_GPIO_COMMANDS
//==============================================================================
// Process GPIO command Callback
//==============================================================================
void ConsoleRx_GPIOcommandCallBack(eConsoleActions action, eGPIO_PortPin portPin, uint8_t pinNum, uint32_t value, uint32_t mask)
{
  //--- Set GPIO command ---
  if (portPin == No_PORTpin) return;
  uint32_t Result = 0;
  switch (action)
  {
    default:
    case Action_None:
      break;

    case Action_Read:
      if ((portPin >= PORTA) && (portPin < PORTa_Max)) Result = ioport_get_port_level(((uint32_t)portPin - (uint32_t)PORTA), mask);
      else Result = ioport_get_pin_level(((((uint32_t)portPin - (uint32_t)PA) * 32) + pinNum));
      LOGINFO("GPIO Direction: 0x%x", (unsigned int)Result);
      break;

    case Action_Write:
      if ((portPin >= PORTA) && (portPin < PORTa_Max)) ioport_set_port_level(((uint32_t)portPin - (uint32_t)PORTA), mask, value);
      else ioport_set_pin_level(((((uint32_t)portPin - (uint32_t)PA) * 32) + pinNum), value);
      break;

    case Action_Set:
      if ((portPin >= PORTA) && (portPin < PORTa_Max)) ioport_set_port_level(((uint32_t)portPin - (uint32_t)PORTA), mask, IOPORT_PIN_LEVEL_HIGH);
      else ioport_set_pin_level(((((uint32_t)portPin - (uint32_t)PA) * 32) + pinNum), IOPORT_PIN_LEVEL_HIGH);
      break;

    case Action_Clear:
      if ((portPin >= PORTA) && (portPin < PORTa_Max)) ioport_set_port_level(((uint32_t)portPin - (uint32_t)PORTA), mask, IOPORT_PIN_LEVEL_LOW);
      else ioport_set_pin_level(((((uint32_t)portPin - (uint32_t)PA) * 32) + pinNum), IOPORT_PIN_LEVEL_LOW);
      break;

    case Action_Toggle:
      if ((portPin >= PORTA) && (portPin < PORTa_Max)) ioport_toggle_port_level(((uint32_t)portPin - (uint32_t)PORTA), mask);
      else ioport_toggle_pin_level(((((uint32_t)portPin - (uint32_t)PA) * 32) + pinNum));
      break;

    case Action_Dir:
      if ((portPin >= PORTA) && (portPin < PORTa_Max)) ioport_set_port_dir(((uint32_t)portPin - (uint32_t)PORTA), mask, value);
      else ioport_set_pin_dir(((((uint32_t)portPin - (uint32_t)PA) * 32) + pinNum), value);
      break;
  }
}
#endif

#endif // USE_CONSOLE_RX
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------