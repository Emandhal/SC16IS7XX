/*******************************************************************************
    File name:    SPI_V71.h
    Author:       FMA
    Version:      1.0
    Date (d/m/y): 18/04/2021
    Description:  SPI driver for Atmel MCUs
                  This interface implements a synchronous use of the SPI and
                  an asynchronous use of SPI by using a DMA
    History :
*******************************************************************************/
#ifndef SPI_V71_H_INC
#define SPI_V71_H_INC
//=============================================================================

//-----------------------------------------------------------------------------
#include <stdlib.h>
//#include "spi.h"    // For Spi struct of the MCU
#include "sysclk.h" // For system clocks of the MCU
//#include "core_cm7.h
//-----------------------------------------------------------------------------
#include "ErrorsDef.h"
#include "SPI_Interface.h"
#include "XDMAC_V71.h"
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------



// Limits definitions
#define SPI_SPICLOCK_MAN  ( 51000000u ) //!< Max SPI clock frequency (maximum pad speed)


// Definitions
#define SPI_SCBR_MIN    (   1 ) //! Min SCBS value
#define SPI_SCBR_MAX    ( 255 ) //! Max SCBS value
#define SPI_DLYBCS_MIN  (   6 ) //! Min DLYBCS value
#define SPI_DLYBCS_MAX  ( 255 ) //! Max DLYBCS value
#define SPI_DLYBCT_MIN  (   0 ) //! Min DLYBCT value
#define SPI_DLYBCT_MAX  ( 255 ) //! Max DLYBCT value
#define SPI_DLYBS_MIN   (   0 ) //! Min DLYBS value
#define SPI_DLYBS_MAX   ( 255 ) //! Max DLYBS value
#define SPI_BITS_MIN    (   8 ) //! Min data bits value
#define SPI_BITS_MAX    (  16 ) //! Max data bits value

#define SPI_INVALID_PERIPHERAL  ( 0xFFFFFFFF ) //! Invalid peripheral value
#define SPI_ALL_INTERRUPTS      ( 0x0007000F ) //! Select all interrupts
#define SPI_FAULT_STATUS        ( SPI_SR_NACK | SPI_SR_ARBLST | SPI_SR_OVRE | SPI_SR_UNRE ) //! Fault status of the SPI for the interrupt handler

#define XDMAC_SPI_PERID_Base  ( 1 ) //! Base of the SPI HW Interface Number (XDMAC_CC.PERID)

#define SPI_TIMEOUT  ( 30000 ) //! Time-out value (number of attempts)

//-----------------------------------------------------------------------------

#if (SAM4S || SAM3S || SAM3N || SAM3U || SAM4E || SAM4N || SAMG51|| SAMG53|| SAMG54)
#  define SPI_COUNT  1
#else
#  if (SAM3XA || SAM4C || SAM4CP || SAM4CM || SAMV71 || SAMV70 || SAME70 || SAMS70)
#    define SPI_COUNT  2
#  else
#    if (SAMG55)
#      define SPI_COUNT  8
#    else
#      define SPI_COUNT  1
#    endif
#  endif
#endif

//-----------------------------------------------------------------------------

//! Generate Peripheral Chip Select Value from Chip Select ID
#define SPI_PCS_GET(chipselect)  ( (~(1u << (chipselect))) & 0x0F )

//-----------------------------------------------------------------------------

//! SPI XDMAC channels handler structure
typedef struct SPI_XDMAChandles
{
  HandleXDMAC Tx; //!< Tx XDMAC handle
  HandleXDMAC Rx; //!< Rx XDMAC handle
} SPI_XDMAChandles;

//-----------------------------------------------------------------------------

//! SPI transfer status enumerator
typedef enum
{
  SPI_STATUS_UNINITIALIZED, //!< SPI uninitialized
  SPI_STATUS_READY,         //!< SPI transfer is ready
  SPI_STATUS_IN_PROGRESS,   //!< SPI transfer in progress
  SPI_STATUS_COMPLETE,      //!< SPI transfer complete
  SPI_STATUS_FAULT,         //!< SPI transfer in fault
} eSPI_TransferStatus;

//! SPI & XDMAC transfer structure
typedef struct SPI_TransferStruct
{
  //--- Buffer Data ---
  uint8_t* TxData;            //!< Tx Data buffer
  uint8_t* RxData;            //!< Rx Data buffer
  size_t Size;                //!< Size of the buffer

  //--- Transfer Status ---
  bool IsAsserted;            //!< Is the transfer already asserted
  eSPI_TransferStatus Status; //!< Transfer status
  eERRORRESULT Error;         //!< Error status
  SPI_Conf Config;            //!< Transfer configuration
  uint8_t TransactionCounter; //!< The current transaction counter
} SPI_TransferStruct;

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SPI driver API
//********************************************************************************************************************

//! SPI Chip Select behavior modes while transferring
typedef enum SPI_CSbehavior
{
  SPI_CS_KEEP_LOW    = SPI_CSR_CSAAT,  //!< CS does not rise until a new transfer is requested on different chip select
  SPI_CS_RISE_FORCED = 0,              //!< CS rises if there is no more data to transfer
  SPI_CS_RISE_NO_TX  = SPI_CSR_CSNAAT, //!< CS is de-asserted systematically during a time DLYBCS
} SPI_CSbehavior;

//! SPI Chip Select configuration structure
typedef struct SPI_ChipSelectConfig
{
  uint8_t DLYBCT;     //!< Delay Between Consecutive Transfers
  uint8_t DLYBS;      //!< Delay Before SPCK
  uint8_t BITS;       //!< Bits Per Transfer
  SPI_CSbehavior CSB; //!< SPI Chip Select behavior modes while transferring
} SPI_ChipSelectConfig;

//! SPI configuration structure
typedef struct SPI_Config
{
  bool VariablePS;             //!< Peripheral Select: 'false' = Fixed Peripheral Select ; 'true' = Variable Peripheral Select
  bool CSdecoder;              //!< Chip Select Decode: 'false' = The chip select lines are directly connected to a peripheral device ; 'true' = The four NPCS chip select lines are connected to a 4-bit to 16-bit decoder
  bool ModeFaultDetect;        //!< Mode Fault Detection: 'false' = Mode fault detection disabled ; 'true' = Mode fault detection enabled
  bool WaitRead;               //!< Wait Data Read Before Transfer: 'false' = No Effect. In Master mode, a transfer can be initiated regardless of the SPI_RDR state ; 'true' = In Master mode, a transfer can start only if the SPI_RDR is empty, i.e., does not contain any unread data. This mode prevents overrun error in reception
  uint8_t DLYBCS;              //!< Delay Between Chip Selects
  SPI_ChipSelectConfig CSR[4]; //!< Chip Select configuration
} SPI_Config;

//-----------------------------------------------------------------------------



/*! @brief Atmel SPI peripheral initialization
 *
 * It initialize the SPI peripheral and its timings
 * @param[in] *pSPI Is the TWIHS peripheral to initialize
 * @param[in] chipSelect Is the CS to use for initialization
 * @param[in] mode Is the SPI mode. It contains SPI/SDI/SQI and the 0/1/2/3 mode
 * @param[in] sclFreq Is the desired frequency of the SCL pin of the TWIHS
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SPI_Init(Spi* pSPI, const SPI_Config* pConf);


/*! @brief Atmel SPI master initialization
 *
 * It initialize the desired SPI+CS peripheral mode and clock speed
 * @param[in] *pSPI Is the TWIHS peripheral to initialize
 * @param[in] chipSelect Is the CS to use for initialization
 * @param[in] mode Is the SPI mode. It contains SPI/SDI/SQI and the 0/1/2/3 mode
 * @param[in] sclFreq Is the desired frequency of the SCL pin of the TWIHS
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SPI_MasterInit(Spi* pSPI, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreq);
eERRORRESULT SPI_MasterInit_Gen(SPI_Interface* pIntDev, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreqq);


/*! @brief Get peripheral ID of the Atmel SPI
 *
 * @param[in] *pSPI Is the SPI peripheral to use
 * @return The SPI peripheral ID. Returns SPI_INVALID_PERIPHERAL if not found
 */
uint32_t SPI_GetPeripheralID(Spi* pSPI);


/*! @brief Get peripheral number of the Atmel SPI
 *
 * @param[in] *pSPI Is the SPI peripheral to use
 * @return The SPI peripheral number. Returns TWIHS_INVALID_PERIPHERAL if not found
 */
uint32_t SPI_GetPeripheralNumber(Spi* pSPI);

//-----------------------------------------------------------------------------



/*! @brief Enable interrupts of the Atmel SPI
 *
 * @param[in] *pSPI Is the SPI peripheral to use
 * @param[in] sourcesInterrupts Source interrupts to enable (can be OR'ed)
 * @param[in] enableNVIC Set to 'true' to enable the peripheral in the NVIC. Setting 'false' does not disable the peripheral in the NVIC
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SPI_InterruptEnable(Spi* pSPI, uint32_t sourcesInterrupts, bool enableNVIC);


/*! @brief Disable interrupts of the Atmel SPI
 *
 * @param[in] *pSPI Is the SPI peripheral to use
 * @param[in] sourcesInterrupts Source interrupts to disable (can be OR'ed)
 * @param[in] disableNVIC Set to 'true' to disable the peripheral in the NVIC. Setting 'false' does not enable the peripheral in the NVIC
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SPI_InterruptDisable(Spi* pSPI, uint32_t sourcesInterrupts, bool disableNVIC);


/*! @brief Get interrupt status of the Atmel SPI
 *
 * @param[in] *pSPI Is the SPI peripheral to use
 * @return Returns the actual interrupt status of the peripheral
 */
inline uint32_t SPI_GetInterruptStatus(Spi* pSPI)
{
  return pSPI->SPI_SR;
}

//-----------------------------------------------------------------------------



/*! @brief Reset the Atmel SPI
 * @param[in] *pSPI Is the SPI peripheral to reset
 */
void SPI_Reset(Spi* pSPI);

//-----------------------------------------------------------------------------



/*! @brief Set the SPI SCK clock in Hertz of the Atmel SPI
 *
 * Calculate the timings parameter and set it to the SPI+CS peripheral
 * @param[in] *pSPI Is the TWIHS peripheral to use
 * @param[in] chipSelect Is the SPI device to configure
 * @param[in] desiredClockHz Is the desired SCK clock speed
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SPI_SetSPIclockHz(Spi* pSPI, uint8_t chipSelect, uint32_t desiredClockHz);

//-----------------------------------------------------------------------------



/*! @brief Hardware SPI data transfer communication for the Atmel SPI
 *
 * This function takes care of the transfer of the specified packet following the SPI_Interface specification
 * @param[in] *pSPI Is the SPI peripheral to use
 * @param[in] *pPacketDesc(deviceAddress,chipSelect,*txData,*rxData,size) Is the packet configuration to transfer
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SPI_Transfer(Spi* pSPI, SPIInterface_Packet* const pPacketDesc);
eERRORRESULT SPI_Transfer_Gen(SPI_Interface *pIntDev, uint8_t chipSelect, uint8_t *txData, uint8_t *rxData, size_t size);

//-----------------------------------------------------------------------------





//********************************************************************************************************************
// SPI with DMA driver API
//********************************************************************************************************************

/*! @brief Atmel SPI master with DMA initialization
 *
 * It initialize the desired SPI+CS peripheral mode and clock speed. It will prepare both DMA channel for Tx and Rx
 * @param[in] *pIntDev Is the TWIHS peripheral to initialize
 * @param[in] chipSelect Is the CS to use for initialization
 * @param[in] mode Is the SPI mode. It contains SPI/SDI/SQI and the 0/1/2/3 mode
 * @param[in] sclFreq Is the desired frequency of the SCL pin of the TWIHS
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SPI_DMA_MasterInit(Spi *pIntDev, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreq);
eERRORRESULT SPI_DMA_MasterInit_Gen(SPI_Interface *pIntDev, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreq);


/*! @brief Hardware SPI data transfer with DMA communication for the Atmel SPI
 *
 * This function takes care of the transfer of the specified packet following the SPI_Interface specification
 * It configures the DMA channels for the transfer. If no DMA transfer is specified, the function will redirect to the SPI_Transfer() function
 * @param[in] *pIntDev Is the SPI peripheral to use
 * @param[in] *pPacketDesc Is the packet configuration to transfer
 * @return Returns an #eERRORRESULT value enum
 */
eERRORRESULT SPI_PacketTransfer(Spi *pIntDev, SPIInterface_Packet* const pPacketDesc);
eERRORRESULT SPI_PacketTransfer_Gen(SPI_Interface *pIntDev, SPIInterface_Packet* const pPacketDesc);

//********************************************************************************************************************





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* SPI_V71_H_INC */