/*******************************************************************************
 * @file    SC16IS7XX.h
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
 * Follow AN10571 - Sleep programming for NXP bridge ICs    Rev.01 (7 Jan    2007)
 *        AN10417 - SC16IS760/762 Fast IrDA mode            Rev.01 (8 June   2006)
 *        AN10386 - Baud rate calculation for Philips UARTs Rev.01 (3 August 2005)
 ******************************************************************************/
/* @page License
 *
 * Copyright (c) 2020-2022 Fabien MAILLY
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
 * 1.0.1    I2C interface rework for I2C DMA use and polling
 * 1.0.0    Release version
 *****************************************************************************/
#ifndef SC16IS7XX_H_INC
#define SC16IS7XX_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------

#if (defined(SC16IS7XX_ONLY_I2C) || !defined(SC16IS7XX_ONLY_SPI)) && !defined(SC16IS7XX_I2C_DEFINED)
#  define SC16IS7XX_I2C_DEFINED
#endif

#if (defined(SC16IS7XX_ONLY_SPI) || !defined(SC16IS7XX_ONLY_I2C)) && !defined(SC16IS7XX_SPI_DEFINED)
#  define SC16IS7XX_SPI_DEFINED
#endif

//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
#ifdef SC16IS7XX_I2C_DEFINED
#  include "I2C_Interface.h"
#endif
#ifdef SC16IS7XX_SPI_DEFINED
#  include "SPI_Interface.h"
#endif
#ifdef USE_GENERICS_DEFINED
#  include "GPIO_Interface.h"
#  include "UART_Interface.h"
#endif
//-----------------------------------------------------------------------------
#ifdef __cplusplus
  extern "C" {
#  define __SC16IS7XX_PACKED__
#  define SC16IS7XX_PACKITEM    __pragma(pack(push, 1))
#  define SC16IS7XX_UNPACKITEM  __pragma(pack(pop))
#else
#  define __SC16IS7XX_PACKED__  __attribute__((packed))
#  define SC16IS7XX_PACKITEM
#  define SC16IS7XX_UNPACKITEM
#endif
//-----------------------------------------------------------------------------

//! This macro is used to check the size of an object. If not, it will raise a "divide by 0" error at compile time
#define SC16IS7XX_CONTROL_ITEM_SIZE(item, size)  enum { item##_size_must_be_##size##_bytes = 1 / (int)(!!(sizeof(item) == size)) }

//-----------------------------------------------------------------------------



// Limits definitions
#define SC16IS7XX_FREQ_MIN        (     1600u ) //! Min Xtal or oscillator frequency
#define SC16IS7XX_XTAL_FREQ_MAX   ( 24000000u ) //! Max Xtal frequency
#define SC16IS7XX_OSC_FREQ_MAX    ( 80000000u ) //! Max oscillator frequency
#define SC16IS7XX_BAUDRATE_MIN    (      100u ) //! Min Baudrate
#define SC16IS7XX_BAUDRATE_MAX    (  5000000u ) //! Max Baudrate
#define SC16IS7XX_IrDA_SPEED_MAX  (   115200u ) //! Max IrDA baudrate for SC16IS740/741/750/752
#define SC16IS76X_IrDA_SPEED_MAX  (  1152000u ) //! Max IrDA baudrate for SC16IS760/762
#define SC16IS7XX_I2C_CLOCK_MAX   (   400000u ) //! Max I2C clock frequency
#define SC16IS7XX_SPI_CLOCK_MAX   (  4000000u ) //! Max SPI clock frequency for SC16IS740/741/750/752
#define SC16IS76X_SPI_CLOCK_MAX   ( 15000000u ) //! Max SPI clock frequency for SC16IS760/762



// Device I2C definitions
#define SC16IS7XX_CHIPADDRESS_BASE  ( 0xE0u ) //!< SC16IS7XX chip base address
#define SC16IS7XX_CHIPADDRESS_MASK  ( 0xF0u ) //!< SC16IS7XX chip base address



//! SC16IS7XX I2C address list ('H' => Vdd ; 'L' => Vss/Gnd ; 'C' => SCL ; 'D' => SDA)
#define SC16IS7XX_ADDRESS_A1H_A0H  ( 0x90 )
#define SC16IS7XX_ADDRESS_A1H_A0L  ( 0x92 )
#define SC16IS7XX_ADDRESS_A1H_A0C  ( 0x94 )
#define SC16IS7XX_ADDRESS_A1H_A0D  ( 0x96 )
#define SC16IS7XX_ADDRESS_A1L_A0H  ( 0x98 )
#define SC16IS7XX_ADDRESS_A1L_A0L  ( 0x9A )
#define SC16IS7XX_ADDRESS_A1L_A0C  ( 0x9C )
#define SC16IS7XX_ADDRESS_A1L_A0D  ( 0x9E )
#define SC16IS7XX_ADDRESS_A1C_A0H  ( 0xA0 )
#define SC16IS7XX_ADDRESS_A1C_A0L  ( 0xA2 )
#define SC16IS7XX_ADDRESS_A1C_A0C  ( 0xA4 )
#define SC16IS7XX_ADDRESS_A1C_A0D  ( 0xA6 )
#define SC16IS7XX_ADDRESS_A1D_A0H  ( 0xA8 )
#define SC16IS7XX_ADDRESS_A1D_A0L  ( 0xAA )
#define SC16IS7XX_ADDRESS_A1D_A0C  ( 0xAC )
#define SC16IS7XX_ADDRESS_A1D_A0D  ( 0xAE )

//-----------------------------------------------------------------------------



//! SC16IS7XX part number enumerator
typedef enum
{
  SC16IS740,  //!< SC16IS740 component
  SC16IS741,  //!< SC16IS741 and SC16IS741A component
  SC16IS750,  //!< SC16IS750 component
  SC16IS752,  //!< SC16IS752 component
  SC16IS760,  //!< SC16IS760 component
  SC16IS762,  //!< SC16IS762 component
  SC16IS7XX_PN_COUNT, // SC16IS7XX device PN count, keep last
} eSC16IS7XX_PN;

//-----------------------------------------------------------------------------



// GPIO pin masks definition
#define SC16IS7XX_GPIO0_MASK  ( 1 << 0 ) //! Mask for GPIO0
#define SC16IS7XX_GPIO1_MASK  ( 1 << 1 ) //! Mask for GPIO1
#define SC16IS7XX_GPIO2_MASK  ( 1 << 2 ) //! Mask for GPIO2
#define SC16IS7XX_GPIO3_MASK  ( 1 << 3 ) //! Mask for GPIO3
#define SC16IS7XX_GPIO4_MASK  ( 1 << 4 ) //! Mask for GPIO4
#define SC16IS7XX_GPIO5_MASK  ( 1 << 5 ) //! Mask for GPIO5
#define SC16IS7XX_GPIO6_MASK  ( 1 << 6 ) //! Mask for GPIO6
#define SC16IS7XX_GPIO7_MASK  ( 1 << 7 ) //! Mask for GPIO7

//-----------------------------------------------------------------------------



//! Limits which defines devices
typedef struct SC16IS7XX_Limits
{
  uint32_t I2C_CLOCK_MAX;
  uint32_t SPI_CLOCK_MAX;
  bool IrDA_1_4_RATIO;
  bool HAVE_GPIO;
  bool HAVE_2_UARTS;
} SC16IS7XX_Limits;

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SC16IS7XX Register list
//********************************************************************************************************************

//! SC16IS7XX registers list
typedef enum
{
  // General register set (Accessible only when LCR[7] = 0)
  RegSC16IS7XX_RHR       = 0x00u, //!< Read  mode: Receive Holding Register
  RegSC16IS7XX_THR       = 0x00u, //!< Write mode: Transmit Holding Register
  RegSC16IS7XX_IER       = 0x01u, //!< R/W   mode: Interrupt Enable Register
  RegSC16IS7XX_IIR       = 0x02u, //!< Read  mode: Interrupt Identification Register
  RegSC16IS7XX_FCR       = 0x02u, //!< Write mode: FIFO Control Register
  RegSC16IS7XX_LCR       = 0x03u, //!< R/W   mode: Line Control Register
  RegSC16IS7XX_MCR       = 0x04u, //!< R/W   mode: Modem Control Register
  RegSC16IS7XX_LSR       = 0x05u, //!< Read  mode: Line Status Register
  RegSC16IS7XX_MSR       = 0x06u, //!< Read  mode: Modem Status Register
  RegSC16IS7XX_SPR       = 0x07u, //!< R/W   mode: Scratchpad Register
  RegSC16IS7XX_TCR       = 0x06u, //!< R/W   mode: Transmission Control Register (Accessible only when MCR[2]=1 and EFR[4]=1)
  RegSC16IS7XX_TLR       = 0x07u, //!< R/W   mode: Trigger Level Register (Accessible only when MCR[2]=1 and EFR[4]=1)
  RegSC16IS7XX_TXLVL     = 0x08u, //!< Read  mode: Transmit FIFO Level Register
  RegSC16IS7XX_RXLVL     = 0x09u, //!< Read  mode: Receive FIFO Level Register
  RegSC16IS7XX_IODir     = 0x0Au, //!< R/W   mode: I/O pin Direction Register (Only available on the SC16IS75X/SC16IS76X)
  RegSC16IS7XX_IOState   = 0x0Bu, //!< R/W   mode: I/O pin States Register (Only available on the SC16IS75X/SC16IS76X)
  RegSC16IS7XX_IOIntEna  = 0x0Cu, //!< R/W   mode: I/O Interrupt Enable Register (Only available on the SC16IS75X/SC16IS76X)
  RegSC16IS7XX_Reserved  = 0x0Du, //!< Reserved Register
  RegSC16IS7XX_IOControl = 0x0Eu, //!< R/W   mode: I/O pins Control Register (Only available on the SC16IS75X/SC16IS76X)
  RegSC16IS7XX_EFCR      = 0x0Fu, //!< R/W   mode: Extra Features Register

  // Special register set (Accessible only when LCR[7]=1 and not 0xBF)
  RegSC16IS7XX_DLL       = 0x00u, //!< R/W   mode: Divisor latch LSB
  RegSC16IS7XX_DLH       = 0x01u, //!< R/W   mode: Divisor latch MSB

  // Enhanced register set (Accessible when LCR = 0xBF)
  RegSC16IS7XX_EFR       = 0x02u, //!< R/W   mode: Enhanced Feature Register
  RegSC16IS7XX_XON1      = 0x04u, //!< R/W   mode: Xon1 word
  RegSC16IS7XX_XON2      = 0x05u, //!< R/W   mode: Xon2 word
  RegSC16IS7XX_XOFF1     = 0x06u, //!< R/W   mode: Xoff1 word
  RegSC16IS7XX_XOFF2     = 0x07u, //!< R/W   mode: Xoff2 word
} eSC16IS7XX_Registers;

//! SC16IS7XX registers list
typedef enum
{
  SC16IS7XX_LCR_VALUE_SET_SPECIAL_REGISTER          = 0x80,         //! Special value of LCR register to access Special Registers (note "The special register set is accessible only when LCR[7] = 1 and not 0xBF" of Table 10 in the datasheet)
  SC16IS7XX_LCR_VALUE_SET_ENHANCED_FEATURE_REGISTER = 0xBF,         //! Special value of LCR register to access Special Registers (note "Enhanced Feature Registers are only accessible when LCR = 0xBF" of Table 10 in the datasheet)
  SC16IS7XX_LCR_VALUE_SET_GENERAL_REGISTER          = ~(0x1u << 7), //!< Special value of LCR[7] register to access General Registers (note "These registers are accessible only when LCR[7] = 0" of Table 10 in the datasheet)
} eSC16IS7XX_AccessTo;





//********************************************************************************************************************
// SC16IS7XX Specific Controller Registers
//********************************************************************************************************************

//! Interrupt Enable Register (Read/Write mode)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_IER_Register
{
    uint8_t IER;
    struct
    {
        uint8_t RxDataInt     : 1; //!< 0 - Receive Holding Register interrupt: '0' => disable the RHR interrupt (normal default condition) ; '1' => enable the RHR interrupt
        uint8_t THRemptyInt   : 1; //!< 1 - Transmit Holding Register interrupt: '0' => disable the THR interrupt (normal default condition) ; '1' => enable the THR interrupt
        uint8_t ReceiveLineInt: 1; //!< 2 - Receive line status interrupt: '0' => disable the receiver line status interrupt (normal default condition) ; '1' => enable the receiver line status interrupt
        uint8_t ModemStatInt  : 1; //!< 3 - Modem status interrupt: '0' => disable the modem status register interrupt (normal default condition) ; '1' => enable the modem status register interrupt
        uint8_t SleepMode     : 1; //!< 4 - Sleep mode (This bit in can only be modified if register bit EFR[4] is enabled): '0' => disable Sleep mode (normal default condition) ; '1' => enable Sleep mode
        uint8_t Xoff          : 1; //!< 5 - Xoff (This bit in can only be modified if register bit EFR[4] is enabled): '0' => disable the Xoff interrupt (normal default condition) ; '1' => enable the Xoff interrupt
        uint8_t RTSIntEnable  : 1; //!< 6 - RTS interrupt enable (This bit in can only be modified if register bit EFR[4] is enabled): '0' => disable the RTS interrupt (normal default condition) ; '1' => enable the RTS interrupt
        uint8_t CTSIntEnable  : 1; //!< 7 - CTS interrupt enable (This bit in can only be modified if register bit EFR[4] is enabled): '0' => disable the CTS interrupt (normal default condition) ; '1' => enable the CTS interrupt
    } Bits;
} SC16IS7XX_IER_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_IER_Register, 1);

#define SC16IS7XX_IER_RHR_INTERRUPT_ENABLE            (0x1u << 0) //!< Enable the Receive Holding Register interrupt
#define SC16IS7XX_IER_RHR_INTERRUPT_DISABLE           (0x0u << 0) //!< Disable the Receive Holding Register interrupt
#define SC16IS7XX_IER_THR_INTERRUPT_ENABLE            (0x1u << 1) //!< Enable the Transmit Holding Register interrupt
#define SC16IS7XX_IER_THR_INTERRUPT_DISABLE           (0x0u << 1) //!< Disable the Transmit Holding Register interrupt
#define SC16IS7XX_IER_RLS_INTERRUPT_ENABLE            (0x1u << 2) //!< Enable the Receive Line Status interrupt
#define SC16IS7XX_IER_RLS_INTERRUPT_DISABLE           (0x0u << 2) //!< Disable the Receive Line Status interrupt
#define SC16IS7XX_IER_MODEM_STATUS_INTERRUPT_ENABLE   (0x1u << 3) //!< Enable the Modem Status interrupt
#define SC16IS7XX_IER_MODEM_STATUS_INTERRUPT_DISABLE  (0x0u << 3) //!< Disable the Modem Status interrupt.
#define SC16IS7XX_IER_SLEEP_MODE_ENABLE               (0x1u << 4) //!< Enable the Sleep mode
#define SC16IS7XX_IER_SLEEP_MODE_DISABLE              (0x0u << 4) //!< Disable the Sleep mode
#define SC16IS7XX_IER_XOFF_INTERRUPT_ENABLE           (0x1u << 5) //!< Enable the Xoff interrupt
#define SC16IS7XX_IER_XOFF_INTERRUPT_DISABLE          (0x0u << 5) //!< Disable the Xoff interrupt
#define SC16IS7XX_IER_RTS_INTERRUPT_ENABLE            (0x1u << 6) //!< Enable the RTS interrupt
#define SC16IS7XX_IER_RTS_INTERRUPT_DISABLE           (0x0u << 6) //!< Disable the RTS interrupt
#define SC16IS7XX_IER_CTS_INTERRUPT_ENABLE            (0x1u << 7) //!< Enable the CTS interrupt
#define SC16IS7XX_IER_CTS_INTERRUPT_DISABLE           (0x0u << 7) //!< Disable the CTS interrupt

#define SC16IS7XX_IER_SLEEP_MODE_Mask                 (0x1u << 4) //!< Bitmask for Sleep mode

//! Interrupt Events, can be OR'ed.
typedef enum
{
  SC16IS7XX_NO_INTERRUPT         = 0x00, //!< No interrupt events
  SC16IS7XX_RX_FIFO_INTERRUPT    = 0x01, //!< Receive Holding Register interrupt event. Set by SC16IS7XX_UARTconfig.RxTrigLvl
  SC16IS7XX_TX_FIFO_INTERRUPT    = 0x02, //!< Transmit Holding Register interrupt event. Set by SC16IS7XX_UARTconfig.TxTrigLvl
  SC16IS7XX_RX_LINE_INTERRUPT    = 0x04, //!< Receive Line Status interrupt event. Set when there is an error anywhere in the RX FIFO, and is cleared only when there are no more errors remaining in the FIFO
  SC16IS7XX_MODEM_LINE_INTERRUPT = 0x08, //!< Modem Status interrupt event. Change of state of modem input pins
  SC16IS7XX_XOFF_INTERRUPT       = 0x20, //!< Xoff interrupt event. Set when an Xoff flow character is detected
  SC16IS7XX_RTS_INTERRUPT        = 0x40, //!< RTS interrupt event. RTS pin change state from active (LOW) to inactive (HIGH)
  SC16IS7XX_CTS_INTERRUPT        = 0x80, //!< CTS interrupt event. CTS pin change state from active (LOW) to inactive (HIGH)

  SC16IS7XX_ENABLE_ALL_INTERRUPTS = 0xEF, //!< Enable all interrupts
  SC16IS7XX_INTERRUPTS_FLAGS_MASK = 0xEF, //!< Interrupts flags mask
} eSC16IS7XX_Interrupts;

typedef eSC16IS7XX_Interrupts setSC16IS7XX_Interrupts; //! Set of Interrupt Events (can be OR'ed)

//-----------------------------------------------------------------------------



//! FIFO Control Register (Write mode)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_FCR_Register
{
    uint8_t FCR;
    struct
    {
        uint8_t FIFOenable : 1; //!< 0   - FIFO enable: '0' => disable the transmit and receive FIFO (normal default condition) ; '1' => enable the transmit and receive FIFO
        uint8_t RxFIFOreset: 1; //!< 1   - RX FIFO reset (The user must wait at least 2x Tclk of XTAL1 before reading or writing data to RHR and THR, respectively): '0' => no FIFO receive reset (normal default condition) ; '1' => clears the contents of the receive FIFO and resets the FIFO level logic (the Receive Shift Register is not cleared or altered). This bit will return to a logic 0 after clearing the FIFO
        uint8_t TxFIFOreset: 1; //!< 2   - TX FIFO reset (The user must wait at least 2x Tclk of XTAL1 before reading or writing data to RHR and THR, respectively): '0' => no FIFO transmit reset (normal default condition) ; '1' => clears the contents of the transmit FIFO and resets the FIFO level logic (the Transmit Shift Register is not cleared or altered). This bit will return to a logic 0 after clearing the FIFO
        uint8_t            : 1; //!< 3
        uint8_t TxTrigLevel: 2; //!< 4-5 - TX trigger level. Sets the trigger level for the TX FIFO (This bit in can only be modified if register bit EFR[4] is enabled)
        uint8_t RxTrigLevel: 2; //!< 6-7 - RX trigger level. Sets the trigger level for the RX FIFO
    } Bits;
} SC16IS7XX_FCR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_FCR_Register, 1);

#define SC16IS7XX_FCR_RX_TX_FIFO_ENABLE   (0x1u << 0) //!< Enable the transmit and receive FIFO
#define SC16IS7XX_FCR_RX_TX_FIFO_DISABLE  (0x0u << 0) //!< Disable the transmit and receive FIFO
#define SC16IS7XX_FCR_RESET_RX_FIFO       (0x1u << 1) //!< Clears the contents of the receive FIFO and resets the FIFO level logic (the Receive Shift Register is not cleared or altered). This bit will return to a logic 0 after clearing the FIFO
#define SC16IS7XX_FCR_RESET_TX_FIFO       (0x1u << 2) //!< Clears the contents of the transmit FIFO and resets the FIFO level logic (the Transmit Shift Register is not cleared or altered). This bit will return to a logic 0 after clearing the FIFO

//! Tx interrupt trigger level for the FCR register
typedef enum
{
  SC16IS7XX_TX_TRIGGER_LEVEL_08 = 0b00, //!< Tx interrupt trigger level is  8 characters (default)
  SC16IS7XX_TX_TRIGGER_LEVEL_16 = 0b01, //!< Tx interrupt trigger level is 16 characters
  SC16IS7XX_TX_TRIGGER_LEVEL_32 = 0b10, //!< Tx interrupt trigger level is 32 characters
  SC16IS7XX_TX_TRIGGER_LEVEL_56 = 0b11, //!< Tx interrupt trigger level is 56 characters
} eSC16IS7XX_TxTrigLevel;

#define SC16IS7XX_FCR_TX_TRIGGER_LEVEL_Pos         4
#define SC16IS7XX_FCR_TX_TRIGGER_LEVEL_Mask        (0x3u << SC16IS7XX_FCR_TX_TRIGGER_LEVEL_Pos)
#define SC16IS7XX_FCR_TX_TRIGGER_LEVEL_SET(value)  (((uint8_t)(value) << SC16IS7XX_FCR_TX_TRIGGER_LEVEL_Pos) & SC16IS7XX_FCR_TX_TRIGGER_LEVEL_Mask) //!< Set Tx Interrupt Trigger Level
#define SC16IS7XX_FCR_TX_TRIGGER_LEVEL_GET(value)  (((uint8_t)(value) & SC16IS7XX_FCR_TX_TRIGGER_LEVEL_Mask) >> SC16IS7XX_FCR_TX_TRIGGER_LEVEL_Pos) //!< Get Tx Interrupt Trigger Level

//! Rx interrupt trigger level for the FCR register
typedef enum
{
  SC16IS7XX_RX_TRIGGER_LEVEL_08 = 0b00, //!< Rx interrupt trigger level is  8 characters (default)
  SC16IS7XX_RX_TRIGGER_LEVEL_16 = 0b01, //!< Rx interrupt trigger level is 16 characters
  SC16IS7XX_RX_TRIGGER_LEVEL_56 = 0b10, //!< Rx interrupt trigger level is 56 characters
  SC16IS7XX_RX_TRIGGER_LEVEL_60 = 0b11, //!< Rx interrupt trigger level is 60 characters
} eSC16IS7XX_RxTrigLevel;

#define SC16IS7XX_FCR_RX_TRIGGER_LEVEL_Pos         6
#define SC16IS7XX_FCR_RX_TRIGGER_LEVEL_Mask        (0x3u << SC16IS7XX_FCR_RX_TRIGGER_LEVEL_Pos)
#define SC16IS7XX_FCR_RX_TRIGGER_LEVEL_SET(value)  (((uint8_t)(value) << SC16IS7XX_FCR_RX_TRIGGER_LEVEL_Pos) & SC16IS7XX_FCR_RX_TRIGGER_LEVEL_Mask) //!< Set Rx Interrupt Trigger Level
#define SC16IS7XX_FCR_RX_TRIGGER_LEVEL_GET(value)  (((uint8_t)(value) & SC16IS7XX_FCR_RX_TRIGGER_LEVEL_Mask) >> SC16IS7XX_FCR_RX_TRIGGER_LEVEL_Pos) //!< Get Rx Interrupt Trigger Level

//-----------------------------------------------------------------------------



//! Interrupt Identification Register (Read mode)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_IIR_Register
{
    uint8_t IIR;
    struct
    {
        uint8_t IntStatus  : 1; //!< 0   - Interrupt status: '0' => an interrupt is pending ; '1' => no interrupt is pending
        uint8_t IntIdentify: 5; //!< 1-5 - 5-bit encoded interrupt source
        uint8_t FIFOenable1: 1; //!< 6   - FIFO enable ; mirror the contents of FCR[0]
        uint8_t FIFOenable2: 1; //!< 7   - FIFO enable ; mirror the contents of FCR[0]
    } Bits;
} SC16IS7XX_IIR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_IIR_Register, 1);

#define SC16IS7XX_IIR_NO_INTERRUPT_PENDING    (0x1u << 0) //!< No interrupt is pending
#define SC16IS7XX_IIR_INTERRUPT_PENDING       (0x0u << 0) //!< An interrupt is pending
#define SC16IS7XX_IIR_INTERRUPT_PENDING_Mask  (0x1u << 0) //!< Interrupt is pending mask

//! 5-bit encoded interrupt source for the IIR register
typedef enum
{
  SC16IS7XX_RECEIVER_LINE_STATUS       = 0b00011, //!< Receiver Line Status error
  SC16IS7XX_RECEIVER_TIMEOUT           = 0b00110, //!< Receiver time-out interrupt
  SC16IS7XX_RHR_INTERRUPT              = 0b00010, //!< RHR interrupt
  SC16IS7XX_THR_INTERRUPT              = 0b00001, //!< THR interrupt
  SC16IS7XX_MODEM_INTERRUPT            = 0b00000, //!< Modem interrupt
  SC16IS7XX_INPUT_PIN_CHANGE_STATE     = 0b11000, //!< Input pin change of state
  SC16IS7XX_RECEIVED_XOFF_SIGNAL       = 0b01000, //!< Received Xoff signal/special character
  SC16IS7XX_CTS_RTS_CHANGE_LOW_TO_HIGH = 0b10000, //!< CTS, RTS change of state from active (LOW) to inactive (HIGH)
} eSC16IS7XX_InterruptSource;

#define SC16IS7XX_IIR_INTERRUT_SOURCE_Pos         1
#define SC16IS7XX_IIR_INTERRUT_SOURCE_Mask        (0x1Fu << SC16IS7XX_IIR_INTERRUT_SOURCE_Pos)
#define SC16IS7XX_IIR_INTERRUT_SOURCE_SET(value)  (((uint8_t)(value) << SC16IS7XX_IIR_INTERRUT_SOURCE_Pos) & SC16IS7XX_IIR_INTERRUT_SOURCE_Mask) //!< Set interrupt source
#define SC16IS7XX_IIR_INTERRUT_SOURCE_GET(value)  (((uint8_t)(value) & SC16IS7XX_IIR_INTERRUT_SOURCE_Mask) >> SC16IS7XX_IIR_INTERRUT_SOURCE_Pos) //!< Get interrupt source
#define SC16IS7XX_IIR_FIFOs_ARE_ENABLE            (0x3u << 6) //!< FIFOs (Transmit and receive) are enable ; mirror the contents of FCR[0]
#define SC16IS7XX_IIR_FIFOs_ARE_DISABLE           (0x0u << 6) //!< FIFOs (Transmit and receive) are disable ; mirror the contents of FCR[0]

//-----------------------------------------------------------------------------



//! Line Control Register (Read/Write mode)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_LCR_Register
{
    uint8_t LCR;
    struct
    {
        uint8_t WordLength : 2; //!< 0-1 - Word length bit. These two bits specify the word length to be transmitted or received
        uint8_t StopBit    : 1; //!< 2   - Number of stop bits. Specifies the number of stop bits: '0' => 1 stop bit ; '1' => if Word Length is 5 bits then it's 1½ stop bit else it's stop bits
        uint8_t Parity     : 3; //!< 3-5 - Parity enable: '0' => no parity (normal default condition) ; '1' => a parity bit is generated during transmission and the receiver checks for received parity
        uint8_t SetBreak   : 1; //!< 6   - Break control bit. When enabled, the break control bit causes a break condition to be transmitted (the TX output is forced to a logic 0 state). This condition exists until disabled by setting LCR[6] to a logic 0. '0' => no TX break condition (normal default condition) ; '1' => forces the transmitter output (TX) to a logic 0 to alert the communication terminal to a line break condition
        uint8_t DivLatchEna: 1; //!< 7   - Divisor Latch Enable: '0' => divisor latch disabled (normal default condition) ; '1' => divisor latch enabled
    } Bits;
} SC16IS7XX_LCR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_LCR_Register, 1);

//! Data length of the UART configuration for the LCR register
typedef enum
{
  SC16IS7XX_DATA_LENGTH_5bits = 0b00, //!< Data length is 5 bits
  SC16IS7XX_DATA_LENGTH_6bits = 0b01, //!< Data length is 6 bits
  SC16IS7XX_DATA_LENGTH_7bits = 0b10, //!< Data length is 7 bits
  SC16IS7XX_DATA_LENGTH_8bits = 0b11, //!< Data length is 8 bits
} eSC16IS7XX_DataLength;

#define SC16IS7XX_LCR_DATA_LENGTH_Pos         0
#define SC16IS7XX_LCR_DATA_LENGTH_Mask        (0x3u << SC16IS7XX_LCR_DATA_LENGTH_Pos)
#define SC16IS7XX_LCR_DATA_LENGTH_SET(value)  (((uint8_t)(value) << SC16IS7XX_LCR_DATA_LENGTH_Pos) & SC16IS7XX_LCR_DATA_LENGTH_Mask) //!< Set data length
#define SC16IS7XX_LCR_DATA_LENGTH_GET(value)  (((uint8_t)(value) & SC16IS7XX_LCR_DATA_LENGTH_Mask) >> SC16IS7XX_LCR_DATA_LENGTH_Pos) //!< Get data length

//! SC16IS740/750/760 UART Stop Bit Length
typedef enum
{
  SC16IS7XX_STOP_BIT_1bit,  //!< UART stop bit length 1 bit (word length = 5, 6, 7, 8)
  SC16IS7XX_STOP_BIT_1bit5, //!< UART stop bit length 1.5 bit (word length = 5)
  SC16IS7XX_STOP_BIT_2bits, //!< UART stop bit length 2 bits (word length = 6, 7, 8)
} eSC16IS7XX_StopBit;

#define SC16IS7XX_LCR_EXTENDED_STOP_BIT   (0x1u << 2) //!< Set stop bit to 1.5 bit or 2 bits
#define SC16IS7XX_LCR_ONLY_1_STOP_BIT     (0x0u << 2) //!< Set stop bit to 1 bit

//! Parity of the UART configuration for the LCR register
typedef enum
{
  SC16IS7XX_NO_PARITY       = 0b000, //!< No parity
  SC16IS7XX_ODD_PARITY      = 0b001, //!< Odd parity
  SC16IS7XX_EVEN_PARITY     = 0b011, //!< Even parity
  SC16IS7XX_FORCED_1_PARITY = 0b101, //!< Forced '1' parity
  SC16IS7XX_FORCED_0_PARITY = 0b111, //!< Forced '0' parity
} eSC16IS7XX_Parity;

#define SC16IS7XX_LCR_PARITY_Pos                     3
#define SC16IS7XX_LCR_PARITY_Mask                    (0x7u << SC16IS7XX_LCR_PARITY_Pos)
#define SC16IS7XX_LCR_PARITY_SET(value)              (((uint8_t)(value) << SC16IS7XX_LCR_PARITY_Pos) & SC16IS7XX_LCR_PARITY_Mask) //!< Set parity
#define SC16IS7XX_LCR_PARITY_GET(value)              (((uint8_t)(value) & SC16IS7XX_LCR_PARITY_Mask) >> SC16IS7XX_LCR_PARITY_Pos) //!< Get parity
#define SC16IS7XX_LCR_FORCE_TRANSMITTER_OUTPUT_TO_0  (0x1u << 6) //!< Forces the transmitter output (TX) to a logic 0 to alert the communication terminal to a line break condition
#define SC16IS7XX_LCR_NO_BREAK_CONDITION             (0x0u << 6) //!< No TX break condition
#define SC16IS7XX_LCR_DIVISOR_LATCH_ENABLE           (0x1u << 7) //!< Enable divisor latch
#define SC16IS7XX_LCR_DIVISOR_LATCH_DISABLE          (0x0u << 7) //!< Disable divisor latch

#define SC16IS7XX_LCR_LINE_CONTROL_Mask  ( SC16IS7XX_LCR_PARITY_Mask | SC16IS7XX_LCR_EXTENDED_STOP_BIT | SC16IS7XX_LCR_DATA_LENGTH_Mask )

//-----------------------------------------------------------------------------



//! Modem Control Register (Read/Write mode)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_MCR_Register
{
    uint8_t MCR;
    struct
    {
        uint8_t DTR           : 1; //!< 0 - DTR/(IO5) (Only available on the SC16IS75X/76X) ; If GPIO5 is selected as DTR modem pin through IOControl register bit 1, the state of DTR pin can be controlled as follow. Writing to IOState bit 5 will not have any effect on this pin ; '0' => force DTR output to inactive (HIGH) ; '1' => force DTR output to active (LOW)
        uint8_t RTS           : 1; //!< 1 - RTS: '0' => force RTS output to inactive (HIGH) ; '1' => force RTS output to active (LOW). In Loopback mode, controls MSR[4]. If Auto RTS is enabled, the RTS output is controlled by hardware flow control
        uint8_t TCR_TLREnable : 1; //!< 2 - TCR and TLR enable (This bit in can only be modified if register bit EFR[4] is enabled): '0' => disable the TCR and TLR register ; '1' => enable the TCR and TLR register
        uint8_t               : 1; //!< 3
        uint8_t EnableLoopback: 1; //!< 4 - Loopback enable: '0' => normal operating mode ; '1' => enable local Loopback mode (internal). In this mode the MCR[1:0] signals are looped back into MSR[4:5] and the TX output is looped back to the RX input internally
        uint8_t XonAny        : 1; //!< 5 - Xon Any (This bit in can only be modified if register bit EFR[4] is enabled): '0' => disable Xon Any function ; '1' => enable Xon Any function
        uint8_t IrDAModeEnable: 1; //!< 6 - IrDA mode enable (This bit in can only be modified if register bit EFR[4] is enabled): '0' => normal UART mode ; '1' => IrDA mode
        uint8_t ClockDivisor  : 1; //!< 7 - Clock divisor (This bit in can only be modified if register bit EFR[4] is enabled): '0' => divide-by-1 clock input ; '1' => divide-by-4 clock input
    } Bits;
} SC16IS7XX_MCR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_MCR_Register, 1);

#define SC16IS7XX_MCR_FORCE_DTR_OUTPUT_ACTIVE       (0x1u << 0) //!< Force DTR output to active (LOW)
#define SC16IS7XX_MCR_FORCE_DTR_OUTPUT_INACTIVE     (0x0u << 0) //!< Force DTR output to inactive (HIGH)
#define SC16IS7XX_MCR_FORCE_RTS_OUTPUT_ACTIVE       (0x1u << 1) //!< Force RTS output to active (LOW). In Loopback mode, controls MSR[4]. If Auto-RTS is enabled, the RTS output is controlled by hardware flow control
#define SC16IS7XX_MCR_FORCE_RTS_OUTPUT_INACTIVE     (0x0u << 1) //!< Force RTS output to inactive (HIGH)
#define SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_ENABLE   (0x1u << 2) //!< Enable the TCR and TLR register
#define SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_DISABLE  (0x0u << 2) //!< Disable the TCR and TLR register
#define SC16IS7XX_MCR_LOOPBACK_ENABLE               (0x1u << 4) //!< Enable local Loopback mode (internal). In this mode the MCR[1:0] signals are looped back into MSR[4:5] and the TX output is looped back to the RX input internally
#define SC16IS7XX_MCR_LOOPBACK_DISABLE              (0x0u << 4) //!< Disable local loopback mode, i.e normal operating mode
#define SC16IS7XX_MCR_XON_ANY_FUNCTION_ENABLE       (0x1u << 5) //!< Enable Xon Any function
#define SC16IS7XX_MCR_XON_ANY_FUNCTION_DISABLE      (0x0u << 5) //!< Disable Xon Any function
#define SC16IS7XX_MCR_IrDA_MODE                     (0x1u << 6) //!< IrDA mode enable
#define SC16IS7XX_MCR_NORMAL_UART_MODE              (0x0u << 6) //!< Normal UART mode
#define SC16IS7XX_MCR_CLOCK_INPUT_DIVIDE_BY_4       (0x1u << 7) //!< Divide-by-4 clock input
#define SC16IS7XX_MCR_CLOCK_INPUT_DIVIDE_BY_1       (0x0u << 7) //!< Divide-by-1 clock input

#define SC16IS7XX_MCR_TCR_AND_TLR_REGISTER_Mask  (0x1u << 2) //!< Bitmask the TCR and TLR register
#define SC16IS7XX_MCR_XON_ANY_FUNCTION_Mask      (0x1u << 5) //!< Bitmask the Xon Any function
#define SC16IS7XX_MCR_CLOCK_INPUT_DIVIDE_Mask    (0x1u << 7) //!< Bitmask the clock input divide
#define SC16IS7XX_MCR_NORMAL_OPERATING_MODE      ( ~(0x1u << 4) ) //!< Bitmask the normal operating mode
#define SC16IS7XX_MCR_IrDA_MODE_Mask             (0x1u << 6) //!< Bitmask the IrDA operating mode

//-----------------------------------------------------------------------------



//! Line Status Register (Read mode only)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_LSR_Register
{
    uint8_t LSR;
    struct
    {
        uint8_t DataIn        : 1; //!< 0 - Data in receiver: '0' => no data in receive FIFO (normal default condition) ; '1' => at least one character in the RX FIFO
        uint8_t OverrunError  : 1; //!< 1 - Overrun error: '0' => no overrun error (normal default condition) ; '1' => overrun error has occurred
        uint8_t ParityError   : 1; //!< 2 - Parity error: '0' => no parity error (normal default condition) ; '1' => parity error in data being read from RX FIFO
        uint8_t FramingError  : 1; //!< 3 - Framing error: '0' => no framing error in data being read from RX FIFO (normal default condition) ; '1' => framing error occurred in data being read from RX FIFO, that is, received data did not have a valid stop bit
        uint8_t BreakInt      : 1; //!< 4 - Break interrupt: '0' => no break condition (normal default condition) ; '1' => a break condition occurred and associated character is 0x00, that is, RX was LOW for one character time frame
        uint8_t THRempty      : 1; //!< 5 - THR empty: '0' => transmit hold register is not empty ; '1' => transmit hold register is empty. The host can now load up to 64 characters of data into the THR if the TX FIFO is enabled
        uint8_t THRandTSRempty: 1; //!< 6 - THR and TSR empty: '0' => transmitter hold and shift registers are not empty ; '1' => transmitter hold and shift registers are empty
        uint8_t FifoDataError : 1; //!< 7 - FIFO data error: '0' => no error (normal default condition) ; '1' => at least one parity error, framing error, or break indication is in the receiver FIFO. This bit is cleared when no more errors are present in the FIFO
    } Bits;
} SC16IS7XX_LSR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_LSR_Register, 1);

#define SC16IS7XX_LSR_DATA_IN_RX_FIFO        (0x1u << 0) //!< At least one character in the RX FIFO
#define SC16IS7XX_LSR_NO_DATA_IN_RX_FIFO     (0x0u << 0) //!< No data in receive FIFO
#define SC16IS7XX_LSR_OVERRUN_ERROR          (0x1u << 1) //!< Overrun error has occurred
#define SC16IS7XX_LSR_NO_OVERRUN_ERROR       (0x0u << 1) //!< No overrun error
#define SC16IS7XX_LSR_PARITY_ERROR           (0x1u << 2) //!< Parity error in data being read from RX FIFO
#define SC16IS7XX_LSR_NO_PARITY_ERROR        (0x0u << 2) //!< No parity error
#define SC16IS7XX_LSR_FRAMING_ERROR          (0x1u << 3) //!< Framing error occurred in data being read from RX FIFO (received data did not have a valid stop bit)
#define SC16IS7XX_LSR_NO_FRAMING_ERROR       (0x0u << 3) //!< No framing error in data being read from RX FIFO
#define SC16IS7XX_LSR_BREAK_CONDITION_OCCUR  (0x1u << 4) //!< A break condition occurred and associated character is 0x00 (RX was LOW for one character time frame)
#define SC16IS7XX_LSR_NO_BREAK_CONDITION     (0x0u << 4) //!< No break condition
#define SC16IS7XX_LSR_THR_EMPTY              (0x1u << 5) //!< Transmit Hold Register is empty. The host can now load up to 64 characters of data into the THR if the TX FIFO is enabled
#define SC16IS7XX_LSR_THR_NOT_EMPTY          (0x0u << 5) //!< Transmit Hold Register is not empty
#define SC16IS7XX_LSR_THR_AND_TSR_EMPTY      (0x1u << 6) //!< Transmitter hold and shift registers are empty
#define SC16IS7XX_LSR_THR_AND_TSR_NOT_EMPTY  (0x0u << 6) //!< Transmitter hold and shift registers are not empty
#define SC16IS7XX_LSR_FIFO_DATA_ERROR        (0x1u << 7) //!< At least one parity error, framing error, or break indication is in the receiver FIFO. This bit is cleared when no more errors are present in the FIFO
#define SC16IS7XX_LSR_NO_ERROR               (0x0u << 7) //!< No FIFO data error

#define SC16IS7XX_LSR_DATA_RECEIVE_ERROR_Mask  ( SC16IS7XX_LSR_FIFO_DATA_ERROR | SC16IS7XX_LSR_BREAK_CONDITION_OCCUR | SC16IS7XX_LSR_FRAMING_ERROR | SC16IS7XX_LSR_PARITY_ERROR | SC16IS7XX_LSR_OVERRUN_ERROR ) //!< At least one parity error, framing error, overrun, or break indication is in the receiver FIFO

// Data receive error enum
typedef enum
{
  SC16IS7XX_NO_RX_ERROR   = 0x00,                                //!< No error on the last character received
  SC16IS7XX_OVERRUN_ERROR = SC16IS7XX_LSR_OVERRUN_ERROR,         //!< Overrun error has occurred
  SC16IS7XX_PARITY_ERROR  = SC16IS7XX_LSR_PARITY_ERROR,          //!< Parity error in data being read from RX FIFO
  SC16IS7XX_FRAMING_ERROR = SC16IS7XX_LSR_FRAMING_ERROR,         //!< Framing error occurred in data being read from RX FIFO (received data did not have a valid stop bit)
  SC16IS7XX_BREAK_ERROR   = SC16IS7XX_LSR_BREAK_CONDITION_OCCUR, //!< A break condition occurred and associated character is 0x00 (RX was LOW for one character time frame)

  SC16IS7XX_RX_ERROR_Mask = ( SC16IS7XX_LSR_BREAK_CONDITION_OCCUR | SC16IS7XX_LSR_FRAMING_ERROR | SC16IS7XX_LSR_PARITY_ERROR | SC16IS7XX_LSR_OVERRUN_ERROR )
} eSC16IS7XX_ReceiveError;

typedef eSC16IS7XX_ReceiveError setSC16IS7XX_ReceiveError; //! Set of receive errors (can be OR'ed)

//-----------------------------------------------------------------------------



//! Modem Status Register (Read mode only)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_MSR_Register
{
    uint8_t MSR;
    struct
    {
        uint8_t CTSchange    : 1; //!< 0 - CTS change ; Indicates that CTS input has changed state. Cleared on a read
        uint8_t DSR_IO4change: 1; //!< 1 - DSR/(IO4) change (Only available on the SC16IS75X/76X) ; Indicates that DSR input has changed state. Cleared on a read
        uint8_t RI_IO7change : 1; //!< 2 - RI/(IO7) change (Only available on the SC16IS75X/76X) ; Indicates that RI input has changed state from LOW to HIGH. Cleared on a read
        uint8_t CD_IO6change : 1; //!< 3 - CD/(IO6) change (Only available on the SC16IS75X/76X) ; Indicates that CD input has changed state. Cleared on a read
        uint8_t CTS          : 1; //!< 4 - CTS (active HIGH, logical 1). This bit is the complement of the CTS input
        uint8_t DSR_IO4      : 1; //!< 5 - DSR/(IO4) (Only available on the SC16IS75X/76X) ; If GPIO4 is selected as DSR modem pin through IOControl register bit 1, the state of DSR pin can be read from this bit. This bit is the complement of the DSR input. Reading IOState bit 4 does not reflect the true state of DSR pin
        uint8_t RI_IO7       : 1; //!< 6 - RI/(IO7) (Only available on the SC16IS75X/76X) ; If GPIO7 is selected as RI modem pin through IOControl register bit 1, the state of RI pin can be read from this bit. This bit is the complement of the RI input. Reading IOState bit 6 does not reflect the true state of RI pin
        uint8_t CD_IO6       : 1; //!< 7 - CD/(IO6) (Only available on the SC16IS75X/76X) ; If GPIO6 is selected as CD modem pin through IOControl register bit 1, the state of CD pin can be read from this bit. This bit is the complement of the CD input. Reading IOState bit 6 does not reflect the true state of CD pin
    } Bits;
} SC16IS7XX_MSR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_MSR_Register, 1);

#define SC16IS7XX_MSR_CTS_INPUT_CHANGE  (0x1u << 0) //!< dCTS. Indicates that CTS input has changed state. Cleared on a read
#define SC16IS7XX_MSR_DSR_INPUT_CHANGE  (0x1u << 1) //!< dDSR. Indicates that DSR input has changed state. Cleared on a read
#define SC16IS7XX_MSR_RI_INPUT_CHANGE   (0x1u << 2) //!< dRI. Indicates that RI input has changed state from LOW to HIGH. Cleared on a read
#define SC16IS7XX_MSR_CD_INPUT_CHANGE   (0x1u << 3) //!< dCD. Indicates that CD input has changed state. Cleared on a read
#define SC16IS7XX_MSR_CTS_PIN_IS_LOW    (0x1u << 4) //!< CTS (active HIGH, logical 1). This bit is the complement of the CTS input
#define SC16IS7XX_MSR_CTS_PIN_IS_HIGH   (0x0u << 4) //!< CTS (active HIGH, logical 1). This bit is the complement of the CTS input
#define SC16IS7XX_MSR_DSR_PIN_IS_LOW    (0x1u << 5) //!< DSR (active HIGH, logical 1). If GPIO4 or GPIO0 is selected as DSR modem pin through IOControl register bit 1 or bit 2, the state of DSR pin can be read from this bit. This bit is the complement of the DSR input. Reading IOState bit 4 or bit 0 does not reflect the true state of DSR pin
#define SC16IS7XX_MSR_DSR_PIN_IS_HIGH   (0x0u << 5) //!< DSR (active HIGH, logical 1). If GPIO4 or GPIO0 is selected as DSR modem pin through IOControl register bit 1 or bit 2, the state of DSR pin can be read from this bit. This bit is the complement of the DSR input. Reading IOState bit 4 or bit 0 does not reflect the true state of DSR pin
#define SC16IS7XX_MSR_RI_PIN_IS_LOW     (0x1u << 6) //!< RI (active HIGH, logical 1). If GPIO7 or GPIO3 is selected as RI modem pin through IOControl register bit 1 or bit 2, the state of RI pin can be read from this bit. This bit is the complement of the RI input. Reading IOState bit 7 or bit 3 does not reflect the true state of RI pin
#define SC16IS7XX_MSR_RI_PIN_IS_HIGH    (0x0u << 6) //!< RI (active HIGH, logical 1). If GPIO7 or GPIO3 is selected as RI modem pin through IOControl register bit 1 or bit 2, the state of RI pin can be read from this bit. This bit is the complement of the RI input. Reading IOState bit 7 or bit 3 does not reflect the true state of RI pin
#define SC16IS7XX_MSR_CD_PIN_IS_LOW     (0x1u << 7) //!< CD (active HIGH, logical 1). If GPIO6 or GPIO2 is selected as CD modem pin through IOControl register bit 1 or bit 2, the state of CD pin can be read from this bit. This bit is the complement of the CD input. Reading IOState bit 6 or bit 2 does not reflect the true state of CD pin
#define SC16IS7XX_MSR_CD_PIN_IS_HIGH    (0x0u << 7) //!< CD (active HIGH, logical 1). If GPIO6 or GPIO2 is selected as CD modem pin through IOControl register bit 1 or bit 2, the state of CD pin can be read from this bit. This bit is the complement of the CD input. Reading IOState bit 6 or bit 2 does not reflect the true state of CD pin

//-----------------------------------------------------------------------------



/*! Transmission Control Register
 * @warning TCR[3:0] > TCR[7:4]. Also, the TCR must be programmed with this condition before auto RTS or software flow control is enabled to avoid spurious operation of the device.
 */
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_TCR_Register
{
    uint8_t TCR;
    struct
    {
        uint8_t RxTrigHaltTx: 4; //!< 0-3 - RX FIFO trigger level to ask peer to halt transmission. Trigger levels is available from 0 to 60 characters with a granularity of four
        uint8_t RxTrigResume: 4; //!< 4-7 - RX FIFO trigger level to ask peer to resume transmission. Trigger levels is available from 0 to 60 characters with a granularity of four
    } Bits;
} SC16IS7XX_TCR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_TCR_Register, 1);

//! Trigger level to ask peer to resume/hold transmission
typedef enum
{
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_0_CHAR , //!< Ask peer to resume/hold transmission when there is 0 char in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_4_CHAR , //!< Ask peer to resume/hold transmission when there are 4 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_8_CHAR , //!< Ask peer to resume/hold transmission when there are 8 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_12_CHAR, //!< Ask peer to resume/hold transmission when there are 12 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_16_CHAR, //!< Ask peer to resume/hold transmission when there are 16 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_20_CHAR, //!< Ask peer to resume/hold transmission when there are 20 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_24_CHAR, //!< Ask peer to resume/hold transmission when there are 24 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_28_CHAR, //!< Ask peer to resume/hold transmission when there are 28 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_32_CHAR, //!< Ask peer to resume/hold transmission when there are 32 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_36_CHAR, //!< Ask peer to resume/hold transmission when there are 36 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_40_CHAR, //!< Ask peer to resume/hold transmission when there are 40 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_44_CHAR, //!< Ask peer to resume/hold transmission when there are 44 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_48_CHAR, //!< Ask peer to resume/hold transmission when there are 48 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_52_CHAR, //!< Ask peer to resume/hold transmission when there are 52 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_56_CHAR, //!< Ask peer to resume/hold transmission when there are 56 chars in the RX FIFO
  SC16IS7XX_RESUME_WHEN_RX_FIFO_AT_60_CHAR, //!< Ask peer to resume/hold transmission when there are 60 chars in the RX FIFO
} eSC16IS7XX_TriggerCtrlLevel;

#define SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_Pos           0
#define SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_Mask          (0xFu << SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_Pos)
#define SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_SET(value)    (((uint8_t)(value) << SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_Pos) & SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_Mask) //!< Set RX FIFO trigger level to halt transmission
#define SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_GET(value)    (((uint8_t)(value) & SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_Mask) >> SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_Pos) //!< Get RX FIFO trigger level to halt transmission
#define SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_Pos         4
#define SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_Mask        (0xFu << SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_Pos)
#define SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_SET(value)  (((uint8_t)(value) << SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_Pos) & SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_Mask) //!< Set RX FIFO trigger level to resume
#define SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_GET(value)  (((uint8_t)(value) & SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_Mask) >> SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_Pos) //!< Get RX FIFO trigger level to resume

#define SC16IS7XX_TCR_HALT_TRIGGER_LEVEL_CHAR_COUNT_DIV4_SET(value)    ( (uint8_t)(value) >> 2 ) //! Calc RX FIFO trigger level to ask peer to halt transmission. Trigger levels is available from 0 to 60 characters with a granularity of four
#define SC16IS7XX_TCR_RESUME_TRIGGER_LEVEL_CHAR_COUNT_DIV4_SET(value)  ( (uint8_t)(value) >> 2 ) //! Calc RX FIFO trigger level to ask peer to resume transmission. Trigger levels is available from 0 to 60 characters with a granularity of four

//-----------------------------------------------------------------------------



//! Trigger Level Register
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_TLR_Register
{
    uint8_t TLR;
    struct
    {
        uint8_t TxFIFOlevelTrig: 4; //!< 0-3 - TX FIFO trigger levels (4 to 60), number of spaces available
        uint8_t RxFIFOlevelTrig: 4; //!< 4-7 - RX FIFO trigger levels (4 to 60), number of characters available
    } Bits;
} SC16IS7XX_TLR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_TLR_Register, 1);

//! Tx FIFO Trigger level of characters available for interrupt
typedef enum
{
  SC16IS7XX_TX_FIFO_TRIGGER_AT_4_CHAR_SPACE  =  1, //!< TX FIFO trigger at a level of 4 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_8_CHAR_SPACE  =  2, //!< TX FIFO trigger at a level of 8 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_12_CHAR_SPACE =  3, //!< TX FIFO trigger at a level of 12 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_16_CHAR_SPACE =  4, //!< TX FIFO trigger at a level of 16 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_20_CHAR_SPACE =  5, //!< TX FIFO trigger at a level of 20 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_24_CHAR_SPACE =  6, //!< TX FIFO trigger at a level of 24 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_28_CHAR_SPACE =  7, //!< TX FIFO trigger at a level of 28 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_32_CHAR_SPACE =  8, //!< TX FIFO trigger at a level of 32 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_36_CHAR_SPACE =  9, //!< TX FIFO trigger at a level of 36 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_40_CHAR_SPACE = 10, //!< TX FIFO trigger at a level of 40 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_44_CHAR_SPACE = 11, //!< TX FIFO trigger at a level of 44 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_48_CHAR_SPACE = 12, //!< TX FIFO trigger at a level of 48 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_52_CHAR_SPACE = 13, //!< TX FIFO trigger at a level of 52 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_56_CHAR_SPACE = 14, //!< TX FIFO trigger at a level of 56 characters spaces available
  SC16IS7XX_TX_FIFO_TRIGGER_AT_60_CHAR_SPACE = 15, //!< TX FIFO trigger at a level of 60 characters spaces available
} eSC16IS7XX_IntTxTriggerLevel;

//! Rx FIFO Trigger level of characters available for interrupt
typedef enum
{
  SC16IS7XX_RX_FIFO_TRIGGER_AT_4_CHAR_AVAILABLE  =  1, //!< RX FIFO trigger at a level of 4 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_8_CHAR_AVAILABLE  =  2, //!< RX FIFO trigger at a level of 8 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_12_CHAR_AVAILABLE =  3, //!< RX FIFO trigger at a level of 12 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_16_CHAR_AVAILABLE =  4, //!< RX FIFO trigger at a level of 16 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_20_CHAR_AVAILABLE =  5, //!< RX FIFO trigger at a level of 20 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_24_CHAR_AVAILABLE =  6, //!< RX FIFO trigger at a level of 24 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_28_CHAR_AVAILABLE =  7, //!< RX FIFO trigger at a level of 28 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_32_CHAR_AVAILABLE =  8, //!< RX FIFO trigger at a level of 32 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_36_CHAR_AVAILABLE =  9, //!< RX FIFO trigger at a level of 36 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_40_CHAR_AVAILABLE = 10, //!< RX FIFO trigger at a level of 40 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_44_CHAR_AVAILABLE = 11, //!< RX FIFO trigger at a level of 44 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_48_CHAR_AVAILABLE = 12, //!< RX FIFO trigger at a level of 48 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_52_CHAR_AVAILABLE = 13, //!< RX FIFO trigger at a level of 52 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_56_CHAR_AVAILABLE = 14, //!< RX FIFO trigger at a level of 56 characters available
  SC16IS7XX_RX_FIFO_TRIGGER_AT_60_CHAR_AVAILABLE = 15, //!< RX FIFO trigger at a level of 60 characters available
} eSC16IS7XX_IntRxTriggerLevel;

#define SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_Pos         0
#define SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_Mask        (0xFu << SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_Pos)
#define SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_SET(value)  (((uint8_t)(value) << SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_Pos) & SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_Mask) //!< Set TX FIFO trigger levels (4 to 60), number of spaces available
#define SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_GET(value)  (eSC16IS7XX_IntTxTriggerLevel)(((uint8_t)(value) & SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_Mask) >> SC16IS7XX_TLR_TX_FIFO_TRIGGER_LEVEL_Pos) //!< Get TX FIFO trigger levels (4 to 60), number of spaces available
#define SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_Pos         4
#define SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_Mask        (0xFu << SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_Pos)
#define SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_SET(value)  (((uint8_t)(value) << SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_Pos) & SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_Mask) //!< Set RX FIFO trigger levels (4 to 60), number of characters available
#define SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_GET(value)  (eSC16IS7XX_IntRxTriggerLevel)(((uint8_t)(value) & SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_Mask) >> SC16IS7XX_TLR_RX_FIFO_TRIGGER_LEVEL_Pos) //!< Get RX FIFO trigger levels (4 to 60), number of characters available

//-----------------------------------------------------------------------------



//! I/O pins Control Register (Only available on the SC16IS75X/SC16IS76X)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_IOControl_Register
{
    uint8_t IOControl;
    struct
    {
        uint8_t Latch         : 1; //!< 0   - Latch: '0' => input values are not latched ; '1' => input values are latched
        uint8_t IOorModemUARTA: 1; //!< 1   - I/O[7:4] or RI, CD, DTR, DSR of UARTA: '0' => GPIO[7:4] behave as I/O pins ; '1' => GPIO[7:4] behave as RI, CD, DTR, DSR (Only available on the SC16IS75X/76X)
        uint8_t IOorModemUARTB: 1; //!< 2   - I/O[3:0] or RI, CD, DTR, DSR of UARTB: '0' => GPIO[3:0] behave as I/O pins ; '1' => GPIO[3:0] behave as RI, CD, DTR, DSR (Only available on the SC16IS752/762)
        uint8_t UARTSoftReset : 1; //!< 3   - UART software reset (Device returns NACK on I2C-bus when this bit is written) ; A write to bit will reset the device. Once the device is reset this bit is automatically set to ‘0’
        uint8_t               : 4; //!< 4-7
    } Bits;
} SC16IS7XX_IOControl_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_IOControl_Register, 1);

#define SC16IS7XX_IOCTRL_LATCH_INPUT_VALUES_ENABLE   (0x1u << 0) //!< Input values are latched. A change in the input generates an interrupt and the input logic value is loaded in the bit of the corresponding input state register (IOState). A read of the IOState register clears the interrupt. If the input pin goes back to its initial logic state before the interrupt register is read, then the interrupt is not cleared and the corresponding bit of the IOState register keeps the logic value that initiates the interrupt
#define SC16IS7XX_IOCTRL_LATCH_INPUT_VALUES_DISABLE  (0x0u << 0) //!< Input value are not latched. A change in any input generates an interrupt. A read of the input register clears the interrupt. If the input goes back to its initial logic state before the input register is read, then the interrupt is cleared
#define SC16IS7XX_IOCTRL_GPIO7_4_AS_MODEM            (0x1u << 1) //!< GPIO[7:4] behave as RIA, CDA, DTRA, DSRA
#define SC16IS7XX_IOCTRL_GPIO7_4_AS_IO               (0x0u << 1) //!< GPIO[7:4] behave as I/O pins
#define SC16IS7XX_IOCTRL_GPIO3_0_AS_MODEM            (0x1u << 2) //!< GPIO[3:0] behave as RIB, CDB, DTRB, DSRB
#define SC16IS7XX_IOCTRL_GPIO3_0_AS_IO               (0x0u << 2) //!< GPIO[3:0] behave as I/O pins
#define SC16IS7XX_IOCTRL_SOFTWARE_RESET              (0x1u << 3) //!< Software Reset. A write to this bit will reset the device. Once the device is reset this bit is automatically set to logic 0

#define SC16IS7XX_IOCTRL_UARTA_MODEM_MODE_Mask  ( SC16IS7XX_IOCTRL_GPIO7_4_AS_MODEM ) //!< Bitmask for the UARTA modem mode
#define SC16IS7XX_IOCTRL_UARTB_MODEM_MODE_Mask  ( SC16IS7XX_IOCTRL_GPIO3_0_AS_MODEM ) //!< Bitmask for the UARTB modem mode

//-----------------------------------------------------------------------------



//! Extra Features Register (Read/Write mode)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_EFCR_Register
{
    uint8_t EFCR;
    struct
    {
        uint8_t Mode9bitEnable    : 1; //!< 0 - Enable 9-bit or Multidrop mode (RS-485): '0' => normal RS-232 mode ; '1' => enables RS-485 mode
        uint8_t ReceiverDisable   : 1; //!< 1 - Receiver disable: '0' => receiver is enabled ; '1' => receiver is disabled
        uint8_t TransmitterDisable: 1; //!< 2 - Transmitter disable: '0' => transmitter is enabled ; '1' => transmitter is disabled
        uint8_t                   : 1; //!< 3
        uint8_t AutoRS485RTSdir   : 1; //!< 4 - Auto RS-485 RTS direction control: '0' => transmitter does not control RTS pin ; '1' => transmitter controls RTS pin
        uint8_t AutoRS485RTSinv   : 1; //!< 5 - Auto RS-485 RTS output inversion: '0' => RTS = 0 during transmission and RTS = 1 during reception ; '1' => RTS = 1 during transmission and RTS = 0 during reception
        uint8_t                   : 1; //!< 6
        uint8_t IrDAmode          : 1; //!< 7 - IrDA mode (slow/fast) (IrDA mode slow/fast for SC16IS76X, slow only for SC16IS75X): '0' => IrDA SIR, 3/16 pulse ratio, data rate up to 115.2 kbit/s ; '1' => IrDA SIR, 1/4 pulse ratio, data rate up to 1.152 Mbit/s
    } Bits;
} SC16IS7XX_EFCR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_EFCR_Register, 1);

#define SC16IS7XX_EFCR_9BIT_MODE_ENABLE       (0x1u << 0) //!< Enable 9-bit or Multidrop mode (RS-485)
#define SC16IS7XX_EFCR_9BIT_MODE_DISABLE      (0x0u << 0) //!< Disable 9-bit or Multidrop mode (RS-485)
#define SC16IS7XX_EFCR_RX_DISABLE             (0x1u << 1) //!< Receiver is disabled
#define SC16IS7XX_EFCR_RX_ENABLE              (0x0u << 1) //!< Receiver is enabled
#define SC16IS7XX_EFCR_TX_DISABLE             (0x1u << 2) //!< Transmitter is disabled
#define SC16IS7XX_EFCR_TX_ENABLE              (0x0u << 2) //!< Transmitter is enabled
#define SC16IS7XX_EFCR_TX_CONTROL_RTS         (0x1u << 4) //!< Transmitter controls RTS pin
#define SC16IS7XX_EFCR_TX_NOT_CONTROL_RTS     (0x0u << 4) //!< Transmitter does not control RTS pin
#define SC16IS7XX_EFCR_INVERT_RTS_PIN         (0x1u << 5) //!< RTS = 1 during transmission and RTS = 0 during reception
#define SC16IS7XX_EFCR_NORMAL_RTS_PIN         (0x0u << 5) //!< RTS = 0 during transmission and RTS = 1 during reception
#define SC16IS7XX_EFCR_IrDA_1_4_PULSE_RATIO   (0x1u << 7) //!< IrDA SIR, 1⁄4 pulse ratio, data rate up to 1.152 Mbit/s
#define SC16IS7XX_EFCR_IrDA_3_16_PULSE_RATIO  (0x0u << 7) //!< IrDA SIR, 3⁄16 pulse ratio, data rate up to 115.2 kbit/s

#define SC16IS7XX_EFCR_TX_RX_DISABLE_Mask      ( (0x1u << 2) | (0x1u << 1) ) //!< Bitmask for the transmitter/receiver disable
#define SC16IS7XX_EFCR_LINE_CONTROL_MODE_Mask  ( SC16IS7XX_EFCR_9BIT_MODE_ENABLE | SC16IS7XX_EFCR_TX_CONTROL_RTS | SC16IS7XX_EFCR_INVERT_RTS_PIN | SC16IS7XX_EFCR_IrDA_1_4_PULSE_RATIO ) //!< Bitmask for the line control mode

//-----------------------------------------------------------------------------



//! Enhanced Feature Register (Read/Write mode)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_EFR_Register
{
    uint8_t EFR;
    struct
    {
        uint8_t SoftFlowCtrl   : 4; //!< 0-3 - Software flow transmitter and receiver control bits
        uint8_t EnaEnhancedFunc: 1; //!< 4   - Enable enhanced functions: '0' => disables enhanced functions and writing to IER[7:4], FCR[5:4], MCR[7:5] ; '1' => enables the enhanced function IER[7:4], FCR[5:4], and MCR[7:5] so that they can be modified
        uint8_t SpecialCharDet : 1; //!< 5   - Special character detect: '0' => Special character detect disabled (normal default condition) ; '1' => Special character detect enabled. Received data is compared with Xoff2 data. If a match occurs, the received data is transferred to FIFO and IIR[4] is set to a logical 1 to indicate a special character has been detected
        uint8_t AutoRTS        : 1; //!< 6   - Auto RTS: '0' => RTS flow control is disabled (normal default condition) ; '1' => RTS flow control is enabled. The RTS pin goes HIGH when the receiver FIFO halt trigger level TCR[3:0] is reached, and goes LOW when the receiver FIFO resume transmission trigger level TCR[7:4] is reached
        uint8_t AutoCTS        : 1; //!< 7   - Auto CTS: '0' => CTS flow control is disabled (normal default condition) ; '1' => CTS flow control is enabled. Transmission will stop when a HIGH signal is detected on the CTS pin
    } Bits;
} SC16IS7XX_EFR_Register;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_EFR_Register, 1);

//! Software flow transmitter and receiver control configuration for the EFR register
typedef enum
{
  SC16IS7XX_NoTxCtrlFlow_NoRxCtrlFlow               = 0b0000, //!< No transmit flow control, No receive flow control
  SC16IS7XX_NoTxCtrlFlow_RxXon1Xoff1                = 0b0001, //!< No transmit flow control, Receiver compares Xon1 + Xoff1
  SC16IS7XX_NoTxCtrlFlow_RxXon2Xoff2                = 0b0010, //!< No transmit flow control, Receiver compares Xon2 + Xoff2
  SC16IS7XX_NoTxCtrlFlow_RxXon1and2Xoff1and2        = 0b0011, //!< No transmit flow control, Receiver compares Xon1 and Xon2 + Xoff1 and Xoff2
  SC16IS7XX_TxXon1Xoff1_NoRxCtrlFlow                = 0b0100, //!< Transmit Xon1 + Xoff1, No receive flow control
  SC16IS7XX_TxXon1Xoff1_RxXon1Xoff1                 = 0b0101, //!< Transmit Xon1 + Xoff1, Receiver compares Xon1 + Xoff1
  SC16IS7XX_TxXon1Xoff1_RxXon2Xoff2                 = 0b0110, //!< Transmit Xon1 + Xoff1, Receiver compares Xon2 + Xoff2
  SC16IS7XX_TxXon1Xoff1_RxXon1or2Xoff1or2           = 0b0111, //!< Transmit Xon1 + Xoff1, Receiver compares Xon1 or Xon2 + Xoff1 or Xoff2
  SC16IS7XX_TxXon2Xoff2_NoRxCtrlFlow                = 0b1000, //!< Transmit Xon2 + Xoff2, No receive flow control
  SC16IS7XX_TxXon2Xoff2_RxXon1Xoff1                 = 0b1001, //!< Transmit Xon2 + Xoff2, Receiver compares Xon1 + Xoff1
  SC16IS7XX_TxXon2Xoff2_RxXon2Xoff2                 = 0b1010, //!< Transmit Xon2 + Xoff2, Receiver compares Xon2 + Xoff2
  SC16IS7XX_TxXon2Xoff2_RxXon1or2Xoff1or2           = 0b1011, //!< Transmit Xon2 + Xoff2, Receiver compares Xon1 or Xon2 + Xoff1 or Xoff2
  SC16IS7XX_TxXon1and2Xoff1and2_NoRxCtrlFlow        = 0b1100, //!< Transmit Xon1 and Xon2 + Xoff1 and Xoff2, No receive flow control
  SC16IS7XX_TxXon1and2Xoff1and2_RxXon1Xoff1         = 0b1101, //!< Transmit Xon1 and Xon2 + Xoff1 and Xoff2, Receiver compares Xon1 + Xoff1
  SC16IS7XX_TxXon1and2Xoff1and2_RxXon2Xoff2         = 0b1110, //!< Transmit Xon1 and Xon2 + Xoff1 and Xoff2, Receiver compares Xon2 + Xoff2
  SC16IS7XX_TxXon1and2Xoff1and2_RxXon1and2Xoff1and2 = 0b1111, //!< Transmit Xon1 and Xon2 + Xoff1 and Xoff2, Receiver compares Xon1 and Xon2 + Xoff1 and Xoff2
} eSC16IS7XX_SoftFlowCtrl;

#define SC16IS7XX_EFR_SOFT_FLOW_CONTROL_Pos         0
#define SC16IS7XX_EFR_SOFT_FLOW_CONTROL_Mask        (0xFu << SC16IS7XX_EFR_SOFT_FLOW_CONTROL_Pos)
#define SC16IS7XX_EFR_SOFT_FLOW_CONTROL_SET(value)  (((uint8_t)(value) << SC16IS7XX_EFR_SOFT_FLOW_CONTROL_Pos) & SC16IS7XX_EFR_SOFT_FLOW_CONTROL_Mask) //!< Set Software flow transmitter and receiver control configuration
#define SC16IS7XX_EFR_SOFT_FLOW_CONTROL_GET(value)  (((uint8_t)(value) & SC16IS7XX_EFR_SOFT_FLOW_CONTROL_Mask) >> SC16IS7XX_EFR_SOFT_FLOW_CONTROL_Pos) //!< Get Software flow transmitter and receiver control configuration
#define SC16IS7XX_EFR_ENHANCED_FUNCTION_ENABLE      (0x1u << 4) //!< Enables the enhanced function IER[7:4], FCR[5:4], and MCR[7:5] so that they can be modified
#define SC16IS7XX_EFR_ENHANCED_FUNCTION_DISABLE     (0x0u << 4) //!< Disables enhanced functions and writing to IER[7:4], FCR[5:4], MCR[7:5]
#define SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_ENABLE    (0x1u << 5) //!< Special character detect enabled. Received data is compared with Xoff2 data. If a match occurs, the received data is transferred to FIFO and IIR[4] is set to a logical 1 to indicate a special character has been detected
#define SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_DISABLE   (0x0u << 5) //!< Special character detect disabled
#define SC16IS7XX_EFR_RTS_FLOW_CONTROL_ENABLE       (0x1u << 6) //!< RTS flow control is enabled. The RTS pin goes HIGH when the receiver FIFO halt trigger level TCR[3:0] is reached, and goes LOW when the receiver FIFO resume transmission trigger level TCR[7:4] is reached
#define SC16IS7XX_EFR_RTS_FLOW_CONTROL_DISABLE      (0x0u << 6) //!< RTS flow control is disabled
#define SC16IS7XX_EFR_CTS_FLOW_CONTROL_ENABLE       (0x1u << 7) //!< CTS flow control is enabled. Transmission will stop when a HIGH signal is detected on the CTS pin
#define SC16IS7XX_EFR_CTS_FLOW_CONTROL_DISABLE      (0x0u << 7) //!< CTS flow control is disabled

#define SC16IS7XX_EFR_ENHANCED_FUNCTION_Mask        (0x1u << 4) //!< Bitmask for the enhanced function
#define SC16IS7XX_EFR_SPECIAL_CHAR_DETECT_Mask      (0x1u << 5) //!< Bitmask for the special character detect

//********************************************************************************************************************



//! SPI byte command (Read/Write mode)
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_SPIcommand
{
    uint8_t SPIdata;
    struct
    {
        uint8_t          : 1; //!< 0
        uint8_t CH1CH0   : 2; //!< 1-2 - Channel select: '00' => channel A ; '01' => channel B ; '1x' => reserved
        uint8_t Address  : 4; //!< 3-6 - UART’s internal register select
        uint8_t ReadWrite: 1; //!< 7   - Read/Write: '0' => write to Device ; '1' => read from Device
    } Bits;
} SC16IS7XX_SPIcommand;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_SPIcommand, 1);

#define SC16IS7XX_SPI_READ   (0x1u << 7) //!< Set the SPI read
#define SC16IS7XX_SPI_WRITE  (0x0u << 7) //!< Set the SPI write

//! I2C byte command
SC16IS7XX_PACKITEM
typedef union __SC16IS7XX_PACKED__ SC16IS7XX_I2Ccommand
{
    uint8_t I2Cdata;
    struct
    {
        uint8_t        : 1; //!< 0
        uint8_t CH1CH0 : 2; //!< 1-2 - Channel select: '00' => channel A ; '01' => channel B ; '1x' => reserved
        uint8_t Address: 4; //!< 3-6 - UART’s internal register select
        uint8_t        : 1; //!< 7
    } Bits;
} SC16IS7XX_I2Ccommand;
SC16IS7XX_UNPACKITEM
SC16IS7XX_CONTROL_ITEM_SIZE(SC16IS7XX_I2Ccommand, 1);

#define SC16IS7XX_CHANNEL_Pos          1
#define SC16IS7XX_CHANNEL_Mask         (0x3u << SC16IS7XX_CHANNEL_Pos)
#define SC16IS7XX_CHANNEL_SET(value)   (((uint8_t)(value) << SC16IS7XX_CHANNEL_Pos) & SC16IS7XX_CHANNEL_Mask)               //!< Set Channels bits
#define SC16IS7XX_CHANNEL_GET(value)   ((eSC16IS7XX_Channels)(((value) & SC16IS7XX_CHANNEL_Mask) >> SC16IS7XX_CHANNEL_Pos)) //!< Get Channels bits
#define SC16IS7XX_ADDRESS_Pos          3
#define SC16IS7XX_ADDRESS_Mask         (0xFu << SC16IS7XX_ADDRESS_Pos)
#define SC16IS7XX_ADDRESS_SET(value)   (((uint8_t)(value) << SC16IS7XX_ADDRESS_Pos) & SC16IS7XX_ADDRESS_Mask) //!< Set Address bits
#define SC16IS7XX_ADDRESS_GET(value)   (((value) & SC16IS7XX_ADDRESS_Mask) >> SC16IS7XX_ADDRESS_Pos)          //!< Get Address bits

//-----------------------------------------------------------------------------



//! Interface select
typedef enum
{
  SC16IS7XX_INTERFACE_SPI, //!< Select Interface SPI
  SC16IS7XX_INTERFACE_I2C, //!< Select Interface I2C
} eSC16IS7XX_Interface;

//! Channel select
typedef enum
{
  SC16IS7XX_NO_CHANNEL = 0, //!< Select no specific channel
  SC16IS7XX_CHANNEL_A  = 0, //!< Select channel A
  SC16IS7XX_CHANNEL_B  = 1, //!< Select channel B
  SC16IS7XX_CHANNEL_COUNT , // Keep last
} eSC16IS7XX_Channel;

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SC16IS7XX Driver API
//********************************************************************************************************************
typedef struct SC16IS7XX SC16IS7XX; //! SC16IS7XX component object structure



//! SC16IS7XX device object structure
struct SC16IS7XX
{
  //--- Device configuration ---
  void *UserDriverData;           //!< Optional, can be used to store driver data or NULL
  uint32_t XtalFreq;              //!< Component Xtal frequency (maximum 80MHz). Set it to 0 if oscillator is used
  uint32_t OscFreq;               //!< Component oscillator frequency (maximum 24MHz). Set it to 0 if crystal is used
  eSC16IS7XX_PN DevicePN;         //!< Part number of the device

  //--- Interface driver call functions ---
  eSC16IS7XX_Interface Interface; //!< Interface to use with this device
#ifdef SC16IS7XX_I2C_DEFINED
  uint8_t I2Caddress;             //!< Address I2C of the device (0x90 to 0xAE). Use defines SC16IS7XX_ADDRESS_A1x_A0x
#  ifdef USE_DYNAMIC_INTERFACE
  I2C_Interface* I2C;             //!< This is the I2C_Interface descriptor pointer that will be used to communicate with the device
#  else
  I2C_Interface I2C;              //!< This is the I2C_Interface descriptor that will be used to communicate with the device
#  endif
#endif
#ifdef SC16IS7XX_SPI_DEFINED
  uint8_t SPIchipSelect;          //!< This is the Chip Select index that will be set at the call of a transfer
#  ifdef USE_DYNAMIC_INTERFACE
  SPI_Interface* SPI;             //!< This is the SPI_Interface descriptor pointer that will be used to communicate with the device
#  else
  SPI_Interface SPI;              //!< This is the SPI_Interface descriptor that will be used to communicate with the device
#  endif
#endif
  uint32_t InterfaceClockSpeed;   //!< SPI/I2C clock speed in Hertz

  //--- GPIO configuration ---
  uint8_t GPIOsOutState;          //!< GPIOs pins output state (0 = set to '0' ; 1 = set to '1'). Used to speed up output change
};

//-----------------------------------------------------------------------------



//! SC16IS7XX device configuration structure
typedef struct SC16IS7XX_Config
{
  //--- GPIOs configuration ---
  uint8_t StartupPinsDirection; //!< Startup GPIOs direction (0 = set to '0' ; 1 = set to '1')
  uint8_t StartupPinsLevel;     //!< Startup GPIOs output level (0 = output ; 1 = input)
  uint8_t PinsInterruptEnable;  //!< GPIOs individual Interrupt (0 = disable ; 1 = enable)
} SC16IS7XX_Config;

//********************************************************************************************************************





/*! @brief SC16IS7XX initialization
 *
 * This function initializes the SC16IS7XX driver, call the initialization of the interface driver, and soft reset the device. It also checks the hardware communication with the device
 * @param[in] *pComp Is the pointed structure of the device to be initialized
 * @param[in] *pConf Is the pointed structure of the device configuration. This is mainly the GPIOs startup configuration. This parameter can be NULL if the configuration of GPIO is not helpful
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT Init_SC16IS7XX(SC16IS7XX *pComp, const SC16IS7XX_Config *pConf);


/*! @brief Perform a Software Reset the SC16IS7XX
 *
 * @param[in] *pComp Is the pointed structure of the device to reset
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_SoftResetDevice(SC16IS7XX *pComp);


/*! @brief Hardware communication tests of the SC16IS7XX
 *
 * @param[in] *pComp Is the pointed structure of the device to test
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_HardwareCommTest(SC16IS7XX *pComp);

//-----------------------------------------------------------------------------



#ifdef SC16IS7XX_I2C_DEFINED
/*! @brief Is the SC16IS7XX device ready
 *
 * Poll the acknowledge from the SC16IS7XX
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns 'true' if ready else 'false'
 */
bool SC16IS7XX_IsReady(SC16IS7XX *pComp);
#endif

//-----------------------------------------------------------------------------



/*! @brief Read a register of the SC16IS7XX
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] channel Is the UART channel where to read data
 * @param[in] registerAddr Is the register address to be read
 * @param[out] *registerValue Is where the data will be stored
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_ReadRegister(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, const uint8_t registerAddr, uint8_t *registerValue);

//-----------------------------------------------------------------------------



/*! @brief Write a register of the SC16IS7XX
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] channel Is the UART channel where to write data
 * @param[in] registerAddr Is the register address where data will be written
 * @param[in] registerValue Is the data to write
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_WriteRegister(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, const uint8_t registerAddr, uint8_t registerValue);


/*! @brief Modify a register of the SC16IS7XX
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] channel Is the UART channel where to modify data
 * @param[in] registerAddr Is the register address where data will be written
 * @param[in] registerValue Is the data to write
 * @param[in] registerMask If the bit is set to '1', then the corresponding register's bit have to be modified
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_ModifyRegister(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, const uint8_t registerAddr, uint8_t registerValue, uint8_t registerMask);


/*! @brief Set register access of the SC16IS7XX
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] channel Is the UART channel to use
 * @param[in] setAccessTo Is the register set to give access to
 * @param[out] *originalLCRregValue Is the data of the LCR register before setting access
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_SetRegisterAccess(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, eSC16IS7XX_AccessTo setAccessTo, uint8_t *originalLCRregValue);


/*! @brief Return access to general registers of the SC16IS7XX
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] channel Is the UART channel to use
 * @param[in] originalLCRregValue Is the data of the LCR register before setting access
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_ReturnAccessToGeneralRegister(SC16IS7XX *pComp, const eSC16IS7XX_Channel channel, uint8_t originalLCRregValue);

//-----------------------------------------------------------------------------



/*! @brief Enable Enhanced Functions of the SC16IS7XX
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] channel Is the channel on which the enhanced functions will be enabled
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_EnableEnhancedFunctions(SC16IS7XX *pComp, eSC16IS7XX_Channel channel);

//-----------------------------------------------------------------------------



/*! @brief Activate the sleep mode of the SC16IS7XX
 *
 * This function activate the sleep mode on the device
 * @warning Before using this function, the device must have Enhanced Functions enabled by calling SC16IS7XX_EnableEnhancedFunctions()
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT SC16IS7XX_ActivateSleepMode(SC16IS7XX *pComp)
{
  return SC16IS7XX_ModifyRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IER, SC16IS7XX_IER_SLEEP_MODE_ENABLE, SC16IS7XX_IER_SLEEP_MODE_Mask);
}


/*! @brief Verify if the SC16IS7XX device has sleep mode enabled
 *
 * This function verifies if the device is has sleep mode enable
 * @warning Before using this function, the device must have Enhanced Functions enabled by calling SC16IS7XX_EnableEnhancedFunctions()
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[out] *isSleepModeEnable Indicate if the sleep mode is enabled
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_IsDeviceInSleepMode(SC16IS7XX *pComp, bool* isSleepModeEnable);


/*! @brief Manually wake up the SC16IS7XX
 *
 * @warning Before using this function, the device must have Enhanced Functions enabled by calling SC16IS7XX_EnableEnhancedFunctions()
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT SC16IS7XX_WakeUp(SC16IS7XX *pComp)
{
  return SC16IS7XX_ModifyRegister(pComp, SC16IS7XX_NO_CHANNEL, RegSC16IS7XX_IER, SC16IS7XX_IER_SLEEP_MODE_DISABLE, SC16IS7XX_IER_SLEEP_MODE_Mask);
}

//-----------------------------------------------------------------------------



/*! @brief Configure GPIOs of the SC16IS75X/76X
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] pinsDirection Set the IO pins direction, if bit is '1' then the corresponding GPIO is input else it's output
 * @param[in] pinsLevel Set the IO pins output level, if bit is '1' then the corresponding GPIO is level high else it's level low
 * @param[in] pinsInterruptEnable Set the IO pins interrupt enable, if bit is '1' then a change in the corresponding GPIO input will generate an interrupt else a change in the GPIO input pin will not generate an interrupt
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_ConfigureGPIOs(SC16IS7XX *pComp, uint8_t pinsDirection, uint8_t pinsLevel, uint8_t pinsInterruptEnable);


/*! @brief Set I/O pins direction of the SC16IS75X/76X
 *
 * @param[in] *pComp/pIntDev Is the pointed structure of the device to be used
 * @param[in] pinsDirection Set the IO pins direction, if bit is '1' then the corresponding GPIO is input else it's output
 * @param[in] pinsChangeMask If the bit is set to '1', then the corresponding GPIO have to be modified
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_SetGPIOPinsDirection(SC16IS7XX *pComp, const uint8_t pinsDirection, const uint8_t pinsChangeMask);
#ifdef USE_GENERICS_DEFINED
eERRORRESULT SC16IS7XX_SetGPIOPinsDirection_Gen(GPIO_Interface *pIntDev, const uint32_t pinsDirection, const uint32_t pinsChangeMask);
#endif


/*! @brief Get I/O pins input level of the SC16IS75X/76X
 *
 * @param[in] *pComp/pIntDev Is the pointed structure of the device to be used
 * @param[out] *pinsState Return the actual level of all I/O pins. If bit is '1' then the corresponding GPIO is level high else it's level low
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_GetGPIOPinsInputLevel(SC16IS7XX *pComp, uint8_t *pinsState);
#ifdef USE_GENERICS_DEFINED
eERRORRESULT SC16IS7XX_GetGPIOPinsInputLevel_Gen(GPIO_Interface *pIntDev, uint32_t *pinsState);
#endif


/*! @brief Set I/O pins output level of the SC16IS75X/76X
 *
 * @param[in] *pComp/pIntDev Is the pointed structure of the device to be used
 * @param[in] pinsLevel Set the IO pins output level, if bit is '1' then the corresponding GPIO is level high else it's level low
 * @param[in] pinsChangeMask If the bit is set to '1', then the corresponding GPIO have to be modified
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_SetGPIOPinsOutputLevel(SC16IS7XX *pComp, const uint8_t pinsLevel, const uint8_t pinsChangeMask);
#ifdef USE_GENERICS_DEFINED
eERRORRESULT SC16IS7XX_SetGPIOPinsOutputLevel_Gen(GPIO_Interface *pIntDev, const uint32_t pinsLevel, const uint32_t pinsChangeMask);
#endif


/*! @brief Set I/O pins interrupt enable of the SC16IS75X/76X
 *
 * @param[in] *pComp Is the pointed structure of the device to be used
 * @param[in] pinsIntEna Set the IO pins interrupt enable, if bit is '1' then a change in the corresponding GPIO input will generate an interrupt else a change in the GPIO input pin will not generate an interrupt
 * @param[in] pinsChangeMask If the bit is set to '1', then the corresponding GPIO have to be modified
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_SetGPIOPinsInterruptEnable(SC16IS7XX *pComp, uint8_t pinsIntEna, uint8_t pinsChangeMask);

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SC16IS7XX UART Driver API
//********************************************************************************************************************

// Driver configuration enum
typedef enum
{
  SC16IS7XX_DRIVER_BURST_TX       = 0x00, //!< The UART driver will send data to FIFO at once (data sent depend on available space in FIFO)
  SC16IS7XX_DRIVER_SAFE_TX        = 0x01, //!< The UART driver will send data to FIFO one data at a time and will check if everything went well (slower)
  SC16IS7XX_DRIVER_BURST_RX       = 0x00, //!< The UART driver will receive data from FIFO at once, without checking if each char was received correctly
  SC16IS7XX_DRIVER_SAFE_RX        = 0x02, //!< The UART driver will receive data from FIFO one data at a time and will check if everything went well (slower)
  SC16IS7XX_TEST_LOOPBACK_AT_INIT = 0x80, //!< Test the UART loopback at startup. It sends 2 chars by loopback UART (just internally) to test the UART configuration (slow at initialization, especially on slow UARTs)
} eSC16IS7XX_DriverConfig;

typedef eSC16IS7XX_DriverConfig setSC16IS7XX_DriverConfig; //! Set of Driver configuration (can be OR'ed)

//-----------------------------------------------------------------------------

//! SC16IS7XX UART buffer structure
typedef struct SC16IS7XX_Buffer
{
  uint8_t* pData;    //!< Pointer to a buffer (Tx or Rx). This buffer will be a ring buffer
  size_t BufferSize; //!< Buffer size in bytes
  size_t PosIn;      //!< Input position in the buffer. Will increment on each byte added to the buffer
  size_t PosOut;     //!< Output position in the buffer. Will increment on each byte sent to the UART FIFO (Tx) or received by application (Rx)
  bool IsFull;       //!< Is the buffer full?
} SC16IS7XX_Buffer;

//-----------------------------------------------------------------------------



typedef struct SC16IS7XX_UART SC16IS7XX_UART; //! SC16IS7XX UART object structure


/*! @brief SC16IS7XX UART object structure
 * @warning Each Channel and Device tuple should be unique. Only 1 possible tuple on SC16IS7X0 and 2 possible tuple on SC16IS7X2 devices
 */
struct SC16IS7XX_UART
{
  //--- UART configuration ---
  eSC16IS7XX_Channel Channel;             //!< UART channel of the SC16IS7XX
  setSC16IS7XX_DriverConfig DriverConfig; //!< UART driver configuration. Configuration can be OR'ed

  //--- Device configuration ---
  void *UserDriverData;                   //!< Optional, can be used to store driver data or NULL
  SC16IS7XX *Device;                      //!< SC16IS7XX device where this UART comes from

#ifdef SC16IS7XX_USE_BUFFERS
  //--- Tx/Rx buffers ---
  SC16IS7XX_Buffer TxBuffer;              //!< Tx ring buffer. Only used with SC16IS7XX_DRIVER_BURST_TX
  SC16IS7XX_Buffer RxBuffer;              //!< Rx ring buffer. Only used with SC16IS7XX_DRIVER_BURST_RX
#endif
};

//-----------------------------------------------------------------------------





//! SC16IS7XX UART Type enumerator
typedef enum
{
  SC16IS7XX_UART_RS232, //!< UART type RS-232, RS-422
  SC16IS7XX_UART_RS485, //!< UART type RS-485
  SC16IS7XX_UART_IrDA , //!< UART type IrDA
  SC16IS7XX_UART_Modem, //!< UART type Modem
} eSC16IS7XX_UARTtype;


//! SC16IS7XX RS-485 RTS configuration enumerator
typedef enum
{
  SC16IS7XX_RS485_AUTO_RTS,              //!< The transmitter will control the state of the RTS pin
  SC16IS7XX_RS485_HARD_FLOW_CONTROL_RTS, //!< The logic state of the RTS pin is controlled by the hardware flow control circuitry
  SC16IS7XX_RS485_MANUAL_EXTERNAL_RTS,   //!< The control of the RTS pin is manually or in external way
} eSC16IS7XX_RS485RTSconfig;


//! SC16IS7XX Auto RS-485 mode enumerator
typedef enum
{
  SC16IS7XX_NO_AUTO_RS485_MODE , //!< No automatic RS-485 mode
  SC16IS7XX_MULTIDROP_MODE     , //!< Normal multidrop mode (without automatic address detect)
  SC16IS7XX_AUTO_ADDRESS_DETECT, //!< Auto address mode. The address of this RS-485 is stored in Xoff2
} eSC16IS7XX_AutoRS485;


//! SC16IS7XX IrDA configuration enumerator
typedef enum
{
  SC16IS7XX_IrDA_SIR_3_16_RATIO, //!< IrDA SIR (Standard InfraRed), 3/16 pulse ratio, data rate up to 115.2 kbit/s (default)
  SC16IS7XX_IrDA_SIR_1_4_RATIO,  //!< IrDA SIR (Standard InfraRed), 1/4 pulse ratio, data rate up to 1.152 Mbit/s (SC16IS76X only). This is not an IrDA MIR (Medium-speed InfraRed)
} eSC16IS7XX_IrDAconfig;

//-----------------------------------------------------------------------------



//! SC16IS7XX FIFO configuration enumerator
typedef enum
{
  SC16IS7XX_NO_CONTROL_FLOW      , //!< No control flow (no CTS+RTS, no Xon+Xoff)
  SC16IS7XX_HARDWARE_CONTROL_FLOW, //!< Hardware control flow (use CTS, RTS)
  SC16IS7XX_SOFTWARE_CONTROL_FLOW, //!< Software control flow (use Xon, Xoff, special char)
} eSC16IS7XX_ControlFlowType;


//! SC16IS7XX Pin control mode enumerator
typedef enum
{
  SC16IS7XX_AUTOMATIC_PIN_CONTROL, //!< Automatic pin control
  SC16IS7XX_MANUAL_PIN_CONTROL,    //!< Manual pin control
} eSC16IS7XX_PinControlType;



//! SC16IS7XX UART hardware configuration structure
typedef struct SC16IS7XX_HardControlFlow
{
  eSC16IS7XX_TriggerCtrlLevel HoldAt;      //!< Trigger level to ask peer to hold transmission. Trigger levels is available from 0 to 60 characters with a granularity of four
  eSC16IS7XX_TriggerCtrlLevel ResumeAt;    //!< Trigger level to ask peer to resume transmission. Trigger levels is available from 0 to 60 characters with a granularity of four
  bool UseSpecialCharOnXoff2;              //!< Use special char on Xoff2
  eSC16IS7XX_PinControlType CTSpinControl; //!< The CTS pin controls the transmitter. If 'SC16IS7XX_MANUAL_PIN_CONTROL', the user needs to read the pin manually and stop the transmission. If 'SC16IS7XX_AUTOMATIC_PIN_CONTROL', the transmitter is controlled by the state of the CTS
  eSC16IS7XX_PinControlType RTSpinControl; //!< The RTS pin is controlled by the receiver. If 'SC16IS7XX_MANUAL_PIN_CONTROL', the user needs to set the pin manually to stop the peer transmission. If 'SC16IS7XX_AUTOMATIC_PIN_CONTROL', the receiver controls the state of RTS according to HoldAt and ResumeAt value
  uint8_t SpecialChar;                     //!< Special character to use (Xoff2). If not used by Conf, leave not configured
} SC16IS7XX_HardControlFlow;


//! SC16IS7XX UART software configuration structure
typedef struct SC16IS7XX_SoftControlFlow
{
  eSC16IS7XX_TriggerCtrlLevel HoldAt;   //!< Trigger level to ask peer to hold transmission. Trigger levels is available from 0 to 60 characters with a granularity of four
  eSC16IS7XX_TriggerCtrlLevel ResumeAt; //!< Trigger level to ask peer to resume transmission. Trigger levels is available from 0 to 60 characters with a granularity of four
  bool UseSpecialCharOnXoff2;           //!< Use special char on Xoff2
  eSC16IS7XX_SoftFlowCtrl Config;       //!< This is the configuration of the Software flow control
  bool XonAnyChar;                      //!< Xon on any character
  uint8_t Xon1;                         //!< Xon1 character to use. If not used by 'Config' and XonAnyChar, leave not configured
  uint8_t Xon2;                         //!< Xon2 character to use. If not used by 'Config' and XonAnyChar, leave not configured
  uint8_t Xoff1;                        //!< Xoff1 character to use. If not used by 'Config', leave not configured
  uint8_t Xoff2SpecialChar;             //!< Xoff2/Special character to use. If not used by 'Config', leave not configured
} SC16IS7XX_SoftControlFlow;

//-----------------------------------------------------------------------------



//! SC16IS7XX RS-232/RS-422 specific configuration structure (Fill this struct if UARTtype == SC16IS7XX_UART_RS232)
typedef struct SC16IS7XX_RS232config
{
  eSC16IS7XX_ControlFlowType ControlFlowType;  //!< Type of control flow (Hardware or Software)
  union
  {
    SC16IS7XX_HardControlFlow HardFlowControl; //!< Hardware control flow configuration (Fill if ControlFlowType == SC16IS7XX_HARDWARE_CONTROL_FLOW)
    SC16IS7XX_SoftControlFlow SoftFlowControl; //!< Software control flow configuration (Fill if ControlFlowType == SC16IS7XX_SOFTWARE_CONTROL_FLOW)
  };
} SC16IS7XX_RS232config;


//! SC16IS7XX RS-485 specific configuration structure (Fill this struct if UARTtype == SC16IS7XX_UART_RS485)
typedef struct SC16IS7XX_RS485config
{
  eSC16IS7XX_RS485RTSconfig RTScontrol;      //!< Determine the RTS control
  bool RTSoutInversion;                      //!< Reverses the polarity of the RTS pin if the UART is in auto RS-485 RTS mode
  eSC16IS7XX_AutoRS485 AutoRS485mode;        //!< Auto RS-485 mode
  uint8_t AddressChar;                       //!< This is the address char. If not used (i.e AutoRS485mode != SC16IS7XX_AUTO_ADDRESS_DETECT), leave not configured. If used (i.e AutoRS485mode == SC16IS7XX_AUTO_ADDRESS_DETECT), set HardFlowControl.UseSpecialCharOnXoff2 to 'false'

  bool UseHardwareControlFlow;               //!< Use hardware control flow? Will not be used if RTScontrol == SC16IS7XX_RS485_HARD_FLOW_CONTROL_RTS
  SC16IS7XX_HardControlFlow HardFlowControl; //!< Hardware control flow configuration
} SC16IS7XX_RS485config;


//! SC16IS7XX RS-485 specific configuration structure (Fill this struct if UARTtype == SC16IS7XX_UART_IrDA)
typedef struct SC16IS7XX_IrDAconfig
{
  eSC16IS7XX_IrDAconfig IrDAmode;            //!< This is the IrDA mode (Slow/fast)

  bool UseSoftwareControlFlow;               //!< Use software control flow?
  SC16IS7XX_SoftControlFlow SoftFlowControl; //!< Software control flow configuration
} SC16IS7XX_IrDAconfig;


//! SC16IS7XX Modem specific configuration structure (Fill this struct if UARTtype == SC16IS7XX_UART_Modem)
typedef struct SC16IS7XX_ModemConfig
{
  bool UseHardwareControlFlow;               //!< Use hardware control flow?
  SC16IS7XX_HardControlFlow HardFlowControl; //!< Hardware control flow configuration
} SC16IS7XX_ModemConfig;



//! SC16IS7XX UART configuration structure
typedef struct SC16IS7XX_UARTconfig
{
  //--- UART configuration ---
  eSC16IS7XX_UARTtype UARTtype;      //!< Type of the UART (ie. RS232, RS485, IrDA, Full Modem, ...)
  eSC16IS7XX_DataLength UARTwordLen; //!< UART data length
  eSC16IS7XX_Parity UARTparity;      //!< UART parity
  eSC16IS7XX_StopBit UARTstopBit;    //!< UART stop bit length
  uint32_t UARTbaudrate;             //!< UART desired baudrate
  int32_t *UARTbaudrateError;        //!< Point to an int32_t variable where the UART baudrate error will be stored (divide by 1000 to get the percentage). Set to NULL if no baudrate error is necessary
  union
  {
    SC16IS7XX_RS232config RS232;     //!< Fill this struct if UARTtype == SC16IS7XX_UART_RS232
    SC16IS7XX_RS485config RS485;     //!< Fill this struct if UARTtype == SC16IS7XX_UART_RS485
    SC16IS7XX_IrDAconfig  IrDA;      //!< Fill this struct if UARTtype == SC16IS7XX_UART_IrDA
    SC16IS7XX_ModemConfig Modem;     //!< Fill this struct if UARTtype == SC16IS7XX_UART_Modem
  };
  bool DisableTransmitter;           //!< Disable transmitter. UART does not send serial data out on the transmit pin, but the transmit FIFO will continue to receive data from host until full
  bool DisableReceiver;              //!< Disable receiver. UART will stop receiving data immediately once this is set to 'true', and any data in the TSR will be sent to the receive FIFO

  //--- FIFO configuration ---
  bool UseFIFOs;                          //!< If use FIFO, then FIFO is enabled at startup
  eSC16IS7XX_IntTxTriggerLevel TxTrigLvl; //!< FIFO Tx trigger level used for interrupt generation, number of spaces available. Levels is available from 4 to 60 characters with a granularity of four
  eSC16IS7XX_IntRxTriggerLevel RxTrigLvl; //!< FIFO Rx trigger level used for interrupt generation, number of characters available. Levels is available from 4 to 60 characters with a granularity of four

  //--- Interrupt configuration ---
  setSC16IS7XX_Interrupts Interrupts;     //!< Interrupt configuration of the UART (can be OR'ed)
} SC16IS7XX_UARTconfig;

//********************************************************************************************************************





/*! @brief SC16IS7XX UART initialization
 *
 * This function initializes the SC16IS7XX UART driver.
 * It sets the baudrate and configure the specified UART according to the UARTConf set in parameter.
 * @warning The pUART->Device should be initialized by using Init_SC16IS7XX() before using this function
 * @param[in] *pUART Is the pointed structure of the UART to be initialized
 * @param[in] *pUARTConf Is the pointed structure of the UART configuration
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_InitUART(SC16IS7XX_UART *pUART, const SC16IS7XX_UARTconfig *pUARTConf);


/*! @brief UART communication tests of the SC16IS7XX UART
 *
 * This function sets the loopback mode and test the UART, then puts the UART in normal operating mode
 * This function fully depends on the UART speed
 * @param[in] *pUART Is the pointed structure of the UART to test
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_UARTCommTest(SC16IS7XX_UART *pUART);


/*! @brief Set UART baudrate of the SC16IS7XX UART
 *
 * Calculate the optimal configuration to get the smallest error and configure the device
 * The error is saved in pUARTxConf->UartBaudrateError, it needs to be divided by 1000 to get the percentage
 * @param[in] *pUART Is the pointed structure of the device to be used
 * @param[in] *pUARTConf Is the pointed structure of the UART configuration
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_SetUARTBaudRate(SC16IS7XX_UART *pUART, const SC16IS7XX_UARTconfig *pUARTConf);

//-----------------------------------------------------------------------------


/*! @brief Reset Rx and/or Tx FIFO of the SC16IS7XX UART
 *
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[in] resetTxFIFO Set to 'true' to reset the transmit FIFO
 * @param[in] resetRxFIFO Set to 'true' to reset the receive FIFO
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_ResetFIFO(SC16IS7XX_UART *pUART, bool resetTxFIFO, bool resetRxFIFO);


/*! @brief Reset Tx FIFO of the SC16IS7XX UART
 *
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT SC16IS7XX_ResetTxFIFO(SC16IS7XX_UART *pUART)
{
  return SC16IS7XX_ResetFIFO(pUART, true, false);
}


/*! @brief Reset Rx FIFO of the SC16IS7XX UART
 *
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @return Returns an #eERRORRESULT value enum
 */
inline eERRORRESULT SC16IS7XX_ResetRxFIFO(SC16IS7XX_UART *pUART)
{
  return SC16IS7XX_ResetFIFO(pUART, false, true);
}

//-----------------------------------------------------------------------------



/*! @brief Enable/disable Transmitter and/or receiver of the SC16IS7XX UART
 *
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[in] disableTx Set to 'true' to disable the transmitter, set to 'false' to enable the transmitter
 * @param[in] disableRx Set to 'true' to disable the receiver, set to 'false' to enable the receiver
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_TxRxDisable(SC16IS7XX_UART *pUART, bool disableTx, bool disableRx);

//-----------------------------------------------------------------------------



/*! @brief Configure interrupt of the SC16IS7XX UART
 *
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[in] interruptsFlags Is the set of flags where interrupts will be enabled. Flags can be OR'ed
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_ConfigureInterrupt(SC16IS7XX_UART *pUART, setSC16IS7XX_Interrupts interruptsFlags);


/*! @brief Get interrupt event of the SC16IS7XX UART
 *
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[out] *intPending Indicate if an interrupt is pending. If 'false' do not take care of *interruptFlag
 * @param[out] *interruptFlag Is the return value of the interrupt event
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_GetInterruptEvents(SC16IS7XX_UART *pUART, bool *intPending, eSC16IS7XX_InterruptSource *interruptFlag);

//-----------------------------------------------------------------------------



/*! @brief Get available space in the transmit FIFO of the SC16IS7XX UART
 *
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[out] *availableSpace Is the count of the available space in the transmit FIFO
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_GetAvailableSpaceTxFIFO(SC16IS7XX_UART *pUART, uint8_t *availableSpace);


/*! @brief Get number of characters stored in receive FIFO of the SC16IS7XX UART
 *
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[out] *dataCount Is the number of characters stored in receive FIFO
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_GetDataCountRxFIFO(SC16IS7XX_UART *pUART, uint8_t *dataCount);

//-----------------------------------------------------------------------------



/*! @brief Try to transmit data to UART FIFO of the SC16IS7XX UART
 *
 * If SC16IS7XX_USE_BUFFERS defined and SC16IS7XX_DRIVER_SAFE_TX set to DriverConfig and TxBuffer ≠ NULL the data will be sent using the TxBuffer
 * @param[in] *pUART/pIntDev Is the pointed structure of the UART to be used
 * @param[in] *data Is the data array to send to the UART transmitter through the transmit FIFO
 * @param[in] size Is the count of data to send to the UART transmitter through the transmit FIFO
 * @param[out] *actuallySent Is the count of data actually sent to the transmit FIFO (0 to 64 chars)
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_TransmitData(SC16IS7XX_UART *pUART, uint8_t *data, size_t size, size_t *actuallySent);
#ifdef USE_GENERICS_DEFINED
eERRORRESULT SC16IS7XX_TransmitData_Gen(UART_Interface *pIntDev, uint8_t *data, size_t size, size_t *actuallySent);
#endif


/*! @brief Transmit data to UART FIFO of the SC16IS7XX UART
 *
 * If SC16IS7XX_USE_BUFFERS defined and SC16IS7XX_DRIVER_SAFE_TX set to DriverConfig and TxBuffer ≠ NULL the data will be sent using the TxBuffer
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[in] data Is the char to send to the UART transmitter through the transmit FIFO
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_TransmitChar(SC16IS7XX_UART *pUART, const char data);

//-----------------------------------------------------------------------------


/*! @brief Receive available data from UART FIFO of the SC16IS7XX UART
 *
 * This function will stop receiving data from FIFO at first char error if the DriverConfig is SC16IS7XX_DRIVER_SAFE_RX
 * If SC16IS7XX_USE_BUFFERS defined and SC16IS7XX_DRIVER_BURST_RX set to DriverConfig and RxBuffer ≠ NULL the data will be received using the RxBuffer
 * @param[in] *pUART/pIntDev Is the pointed structure of the UART to be used
 * @param[out] *data Is where the data will be stored
 * @param[in] size Is the count of data that the data buffer can hold
 * @param[out] *actuallyReceived Is the count of data actually received from the received FIFO (0 to 64 chars)
 * @param[out] *lastDataError Is the last char received error. Set to 0 if no errors
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_ReceiveData(SC16IS7XX_UART *pUART, uint8_t *data, size_t size, size_t *actuallyReceived, setSC16IS7XX_ReceiveError *lastDataError);
#ifdef USE_GENERICS_DEFINED
eERRORRESULT SC16IS7XX_ReceiveData_Gen(UART_Interface *pIntDev, uint8_t *data, size_t size, size_t *actuallyReceived, uint8_t *lastDataError);
#endif


/*! @brief Receive data from UART FIFO of the SC16IS7XX UART
 *
 * If SC16IS7XX_USE_BUFFERS defined and SC16IS7XX_DRIVER_BURST_RX set to DriverConfig and RxBuffer ≠ NULL the data will be received using the RxBuffer
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[out] *data Is where the char will be stored
 * @param[out] *charError Is the char received error. Set to 0 if no errors
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_ReceiveChar(SC16IS7XX_UART *pUART, char *data, setSC16IS7XX_ReceiveError *charError);

//-----------------------------------------------------------------------------



/*! @brief Get control pins (CD, RI, DSR, CTS) status of the SC16IS7XX UART
 *
 * @warning A call to this function clears CD, RI, DSR, CTS change status
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @param[out] *controlPinsStatus Is the actual control pins status
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SC16IS7XX_GetControlPinStatus(SC16IS7XX_UART *pUART, uint8_t *controlPinsStatus);


/*! @brief Is Clear To Send (CTS) of the SC16IS7XX UART
 *
 * @warning A call to this function clears CD, RI, DSR, CTS change status
 * @param[in] *pUART Is the pointed structure of the UART to be used
 * @return Returns 'true' if clear to send data (CTS pin at '0'), and 'false' if not clear to send data (CTS pin at '1')
 */
bool SC16IS7XX_IsClearToSend(SC16IS7XX_UART *pUART);

//********************************************************************************************************************





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* SC16IS7XX_H_INC */