/*!*****************************************************************************
 * @file    SPI_V71.c
 * @author  Fabien 'Emandhal' MAILLY
 * @version 1.0.0
 * @date    18/04/2021
 * @brief   SPI driver for Atmel MCUs
 * @details This interface implements a synchronous use of the SPI and
 *          an asynchronous use of SPI by using a DMA
 ******************************************************************************/

//-----------------------------------------------------------------------------
#include "SPI_V71.h"
//-----------------------------------------------------------------------------
#ifndef __cplusplus
#  include <asf.h>
#else
#  include <cstdint>
extern "C" {
#endif
//-----------------------------------------------------------------------------

static SPI_XDMAChandles __HasReservedDMAchannel[SPI_COUNT] =
{
  { .Tx = XDMAC_INVALID_HANDLE, .Rx = XDMAC_INVALID_HANDLE, },
#if (SPI_COUNT > 1)
  { .Tx = XDMAC_INVALID_HANDLE, .Rx = XDMAC_INVALID_HANDLE, },
#endif
#if (SPI_COUNT > 2)
  { .Tx = XDMAC_INVALID_HANDLE, .Rx = XDMAC_INVALID_HANDLE, },
  { .Tx = XDMAC_INVALID_HANDLE, .Rx = XDMAC_INVALID_HANDLE, },
  { .Tx = XDMAC_INVALID_HANDLE, .Rx = XDMAC_INVALID_HANDLE, },
  { .Tx = XDMAC_INVALID_HANDLE, .Rx = XDMAC_INVALID_HANDLE, },
  { .Tx = XDMAC_INVALID_HANDLE, .Rx = XDMAC_INVALID_HANDLE, },
  { .Tx = XDMAC_INVALID_HANDLE, .Rx = XDMAC_INVALID_HANDLE, },
  #endif
};

static volatile SPI_TransferStruct __SPItransferList[SPI_COUNT] =
{
  { .TxData = NULL, .RxData = NULL, .IsAsserted = false, .Status = SPI_STATUS_UNINITIALIZED, .Error = ERR_OK, .TransactionCounter = 0, }, // SPI/SPI0/FLEXCOM0
#if (SPI_COUNT > 1)
  { .TxData = NULL, .RxData = NULL, .IsAsserted = false, .Status = SPI_STATUS_UNINITIALIZED, .Error = ERR_OK, .TransactionCounter = 0, }, // SPI1/FLEXCOM1
#endif
#if (SPI_COUNT > 2)
  { .TxData = NULL, .RxData = NULL, .IsAsserted = false, .Status = SPI_STATUS_UNINITIALIZED, .Error = ERR_OK, .TransactionCounter = 0, }, // FLEXCOM2
  { .TxData = NULL, .RxData = NULL, .IsAsserted = false, .Status = SPI_STATUS_UNINITIALIZED, .Error = ERR_OK, .TransactionCounter = 0, }, // FLEXCOM3
  { .TxData = NULL, .RxData = NULL, .IsAsserted = false, .Status = SPI_STATUS_UNINITIALIZED, .Error = ERR_OK, .TransactionCounter = 0, }, // FLEXCOM4
  { .TxData = NULL, .RxData = NULL, .IsAsserted = false, .Status = SPI_STATUS_UNINITIALIZED, .Error = ERR_OK, .TransactionCounter = 0, }, // FLEXCOM5
  { .TxData = NULL, .RxData = NULL, .IsAsserted = false, .Status = SPI_STATUS_UNINITIALIZED, .Error = ERR_OK, .TransactionCounter = 0, }, // FLEXCOM6
  { .TxData = NULL, .RxData = NULL, .IsAsserted = false, .Status = SPI_STATUS_UNINITIALIZED, .Error = ERR_OK, .TransactionCounter = 0, }, // FLEXCOM7
#endif
};

COMPILER_WORD_ALIGNED static XDMAC_ChannelConfig __XDMAC_SPIconfig; // XDMAC SPI channel configuration

//-----------------------------------------------------------------------------

#define ns_TO_DLYBCS(clk, ns)  ( (((ns)     ) * ((clk) / 1000000)) / 1000 ) //!< Convert ns to SPI DLYBCS
#define ns_TO_DLYBS(clk, ns)   ( (((ns)     ) * ((clk) / 1000000)) / 1000 ) //!< Convert ns to SPI DLYBS
#define ns_TO_DLYBCT(clk, ns)  ( (((ns) / 32) * ((clk) / 1000000)) / 1000 ) //!< Convert ns to SPI DLYBCT

//-----------------------------------------------------------------------------


#define SPI_IRQ_level 0


//=============================================================================
// Atmel SPI peripheral initialization
//=============================================================================
eERRORRESULT SPI_Init(Spi* pSPI, const SPI_Config* pConf)
{
#ifdef CHECK_NULL_PARAM
  if ((pSPI == NULL) || (pConf == NULL)) return ERR__PARAMETER_ERROR;
#endif
  uint32_t ConvVal, Value = 0;

  //--- Enable peripheral clock ---
  uint32_t PeriphID = SPI_GetPeripheralID(pSPI);
  if (PeriphID == SPI_INVALID_PERIPHERAL) return ERR__PERIPHERAL_NOT_VALID;
  sysclk_enable_peripheral_clock(PeriphID);

  //--- Reset SPI peripheral ---
  pSPI->SPI_CR = SPI_CR_SPIDIS | SPI_CR_LASTXFER; // Disable SPI
  SPI_Reset(pSPI);                                // Reset the SPI peripheral

  //--- Disable SPI interrupts ---
  SPI_InterruptDisable(pSPI, ~0ul, false);

  //--- Configure SPI ---
  Value |= (pConf->VariablePS      ? SPI_MR_PS      : 0);
  Value |= (pConf->CSdecoder       ? SPI_MR_PCSDEC  : 0);
  Value |= (pConf->ModeFaultDetect ? 0 : SPI_MR_MODFDIS);
  Value |= (pConf->WaitRead        ? SPI_MR_WDRBT   : 0);
  //--- Set Delay Between Chip Selects --
  ConvVal = ns_TO_DLYBCS(sysclk_get_peripheral_hz(), pConf->DLYBCS_ns);
  if (ConvVal < SPI_DLYBCS_MIN) ConvVal = SPI_DLYBCS_MIN;
  if (ConvVal > SPI_DLYBCS_MAX) ConvVal = SPI_DLYBCS_MAX;
  Value |= SPI_MR_DLYBCS(ConvVal);
  //--- Set value to register ---
  pSPI->SPI_MR = Value; //** Set MR register value

  //--- Configure Chip Select ---
  for (size_t z = 0; z < 4; ++z)
  {
    Value = (uint32_t)pConf->CSR[z].CSbehavior;
    if ((pConf->CSR[z].BitsPerTransfer < SPI_BITS_MIN) || (pConf->CSR[z].BitsPerTransfer > SPI_BITS_MAX)) return ERR__SPI_CONFIG_ERROR;
    Value |= SPI_CSR_BITS(pConf->CSR[z].BitsPerTransfer - 8);
    //--- Set Delay Before SPCK ---
    ConvVal = ns_TO_DLYBS(sysclk_get_peripheral_hz(), pConf->CSR[z].DLYBS_ns);
    if (ConvVal < SPI_DLYBS_MIN) ConvVal = SPI_DLYBS_MIN;
    if (ConvVal > SPI_DLYBS_MAX) ConvVal = SPI_DLYBS_MAX;
    Value |= SPI_CSR_DLYBS(ConvVal);
    //--- Set Delay Between Consecutive Transfers ---
    ConvVal = ns_TO_DLYBCT(sysclk_get_peripheral_hz(), pConf->CSR[z].DLYBCT_ns);
    if (ConvVal < SPI_DLYBCT_MIN) ConvVal = SPI_DLYBCT_MIN;
    if (ConvVal > SPI_DLYBCT_MAX) ConvVal = SPI_DLYBCT_MAX;
    Value |= SPI_CSR_DLYBCT(ConvVal);
    //--- Set value to register ---
    pSPI->SPI_CSR[z] = Value; //** Set CRSx register value
  }
  return ERR_OK;
}



//=============================================================================
// Atmel SPI master initialization
//=============================================================================
eERRORRESULT SPI_MasterInit(Spi* pSPI, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreq)
{
#ifdef CHECK_NULL_PARAM
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
#endif
  if (SPI_PIN_COUNT_GET(mode) > 1) return ERR__NOT_SUPPORTED;
  if (chipSelect > 4) return ERR__PARAMETER_ERROR;
  eERRORRESULT Error;

  //--- Configure SPI mode ---
  if (SPI_CPOL_GET(mode) > 0)
       pSPI->SPI_CSR[chipSelect] |= SPI_CSR_CPOL;
  else pSPI->SPI_CSR[chipSelect] &= (~SPI_CSR_CPOL);
  if (SPI_CPHA_GET(mode) > 0)
       pSPI->SPI_CSR[chipSelect] &= (~SPI_CSR_NCPHA);
  else pSPI->SPI_CSR[chipSelect] |= SPI_CSR_NCPHA;

  //--- Set SPI SCK clock frequency ---
  Error = SPI_SetSPIclockHz(pSPI, chipSelect, sckFreq); // Set the SCK frequency
  
  //--- Enable SPI peripheral ---
  pSPI->SPI_MR |= SPI_MR_MSTR; // Set master mode
  if ((pSPI->SPI_MR & SPI_MR_PS) > 0) pSPI->SPI_CR = SPI_CR_SPIEN; // Enable SPI only on variable PS
  return Error;
}

eERRORRESULT SPI_MasterInit_Gen(SPI_Interface* pIntDev, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreq)
{
#ifdef CHECK_NULL_PARAM
  if (pIntDev == NULL) return ERR__PARAMETER_ERROR;
#endif
  Spi* pSPI = (Spi*)pIntDev->InterfaceDevice;
  return SPI_MasterInit(pSPI, chipSelect, mode, sckFreq);
}



//=============================================================================
// Get peripheral ID of the Atmel SPI
//=============================================================================
uint32_t SPI_GetPeripheralID(Spi* pSPI)
{
#if (SAM4S || SAM3S || SAM3N || SAM3U || SAM4E || SAM4N || SAMG51|| SAMG53|| SAMG54)
  if (pSPI == SPI) {
    return ID_SPI;
  }  
#elif (SAM3XA || SAM4C || SAM4CP || SAM4CM || SAMV71 || SAMV70 || SAME70 || SAMS70)
  if (pSPI == SPI0) {
    return ID_SPI0;
  }
#  ifdef SPI1
  else if (pSPI == SPI1) {
    return ID_SPI1;
  }
#  endif
#elif (SAMG55)
  if (pSPI == SPI0) {
    return ID_FLEXCOM0;
  }
#  ifdef SPI1
  else if (pSPI == SPI1) {
    return ID_FLEXCOM1;
  }
#  endif
#  ifdef SPI2
  else if (pSPI == SPI2) {
    return ID_FLEXCOM2;
  }
#  endif
#  ifdef SPI3
  else if (pSPI == SPI3) {
    return ID_FLEXCOM3;
  }
#  endif
#  ifdef SPI4
  else if (pSPI == SPI4) {
    return ID_FLEXCOM4;
  }
#  endif
#  ifdef SPI5
  else if (pSPI == SPI5) {
    return ID_FLEXCOM5;
  }
#  endif
#  ifdef SPI6
  else if (pSPI == SPI6) {
    return ID_FLEXCOM6;
  }
#  endif
#  ifdef SPI7
  else if (pSPI == SPI7) {
    return ID_FLEXCOM7;
  }
#  endif
#elif SAM4L
  if (pSPI == SPI) {
    return ID_SPI;
  }
#endif
  return SPI_INVALID_PERIPHERAL;
}



//=============================================================================
// Get peripheral number of the Atmel SPI
//=============================================================================
uint32_t SPI_GetPeripheralNumber(Spi* pSPI)
{
#if (SAM4S || SAM3S || SAM3N || SAM3U || SAM4E || SAM4N || SAMG51|| SAMG53|| SAMG54)
  if (pSPI == SPI) {
    return 0;
  }
#elif (SAM3XA || SAM4C || SAM4CP || SAM4CM || SAMV71 || SAMV70 || SAME70 || SAMS70)
  if (pSPI == SPI0) {
    return 0;
  } else if (pSPI == SPI1) {
    return 1;
  }
#elif (SAMG55)
  if (pSPI == SPI0) {
    return 0;
  } else if (pSPI == SPI1) {
    return 1;
  } else if (pSPI == SPI2) {
    return 2;
  } else if (pSPI == SPI3) {
    return 3;
  } else if (pSPI == SPI4) {
    return 4;
  } else if (pSPI == SPI5) {
    return 5;
  } else if (pSPI == SPI6) {
    return 6;
  } else if (pSPI == SPI7) {
    return 7;
  }
#elif SAM4L
  if (pSPI == SPI) {
    return 0;
  }
#endif
  return SPI_INVALID_PERIPHERAL;
}





//********************************************************************************************************************
//=============================================================================
// Enable interrupts of the Atmel SPI
//=============================================================================
eERRORRESULT SPI_InterruptEnable(Spi* pSPI, uint32_t sourcesInterrupts, bool enableNVIC)
{
#ifdef CHECK_NULL_PARAM
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
#endif
  //--- Enable NVIC IRQs ? ---
  if (enableNVIC)
  {
    uint32_t PeriphID = SPI_GetPeripheralID(pSPI);
    if (PeriphID == SPI_INVALID_PERIPHERAL) return ERR__PERIPHERAL_NOT_VALID;
    NVIC_EnableIRQ(PeriphID);
  }
  //--- Enable the specified interrupts ---
  pSPI->SPI_IER = sourcesInterrupts;
  return ERR_OK;
}


//=============================================================================
// Disable interrupts of the Atmel SPI
//=============================================================================
eERRORRESULT SPI_InterruptDisable(Spi* pSPI, uint32_t sourcesInterrupts, bool disableNVIC)
{
#ifdef CHECK_NULL_PARAM
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
#endif
  //--- Disable also NVIC IRQs ? ---
  if (disableNVIC)
  {
    uint32_t PeriphID = SPI_GetPeripheralID(pSPI);
    if (PeriphID == SPI_INVALID_PERIPHERAL) return ERR__PERIPHERAL_NOT_VALID;
    NVIC_DisableIRQ(PeriphID);
  }
  //--- Disable the specified interrupts ---
  pSPI->SPI_IDR = sourcesInterrupts;
  (void)SPI_GetInterruptStatus(pSPI); // Clear Interrupts status
  return ERR_OK;
}





//********************************************************************************************************************
//=============================================================================
// Reset the Atmel SPI
//=============================================================================
void SPI_Reset(Spi* pSPI)
{
#ifdef CHECK_NULL_PARAM
  if (pSPI == NULL) return;
#endif
  pSPI->SPI_CR = SPI_CR_SWRST; // Set SWRST bit to reset SPI peripheral
  (void)(pSPI->SPI_SR);        // Clear status register
}





//********************************************************************************************************************
//=============================================================================
// Set the SPI SCL clock in Hertz of the Atmel SPI
//=============================================================================
eERRORRESULT SPI_SetSPIclockHz(Spi* pSPI, uint8_t chipSelect, uint32_t desiredClockHz)
{
  //--- Check parameters ---
  if (desiredClockHz > SPI_SPICLOCK_MAX) return ERR__SPI_FREQUENCY_ERROR; // SPI SCL frequency in master mode is 400kHz
  uint32_t PeripheralClock = sysclk_get_peripheral_hz();

  //--- Calculate Clock divider ---
  uint32_t Div = div_ceil(PeripheralClock, desiredClockHz);
  if (Div < SPI_SCBR_MIN) return ERR__SPI_CONFIG_ERROR; // The value of Div is from 1 to 255 in the SCBR field
  if (Div > SPI_SCBR_MAX) Div = SPI_SCBR_MAX;
  pSPI->SPI_CSR[chipSelect] &= (~SPI_CSR_SCBR_Msk);
  pSPI->SPI_CSR[chipSelect] |= SPI_CSR_SCBR(Div);

  return ERR_OK;
}





//********************************************************************************************************************
//=============================================================================
// Hardware SPI data transfer communication for the Atmel SPI
//=============================================================================
eERRORRESULT SPI_Transfer(Spi* pSPI, SPIInterface_Packet* const pPacketDesc)
{
#ifdef CHECK_NULL_PARAM
  if (pSPI == NULL) return ERR__PARAMETER_ERROR;
#endif
  const uint_fast8_t UseDummyByte = ((pPacketDesc->Config.Value & SPI_USE_DUMMYBYTE_FOR_RECEIVE) == SPI_USE_DUMMYBYTE_FOR_RECEIVE);
  uint32_t PeriphNumber = SPI_GetPeripheralNumber(pSPI);
  if (PeriphNumber == SPI_INVALID_PERIPHERAL) return ERR__PERIPHERAL_NOT_VALID;
  bool ForceTerminate = false;
  uint32_t DataToSend, DataRead;
  uint32_t Timeout = SPI_TIMEOUT;

  //--- Check the state of the transfer ---
  if (PeriphNumber == SPI_INVALID_PERIPHERAL) return ERR__PERIPHERAL_NOT_VALID;
  switch (__SPItransferList[PeriphNumber].Status)
  {
    case SPI_STATUS_COMPLETE:      //** SPI transfer complete
    case SPI_STATUS_FAULT:         //** SPI transfer in fault
    case SPI_STATUS_IN_PROGRESS:   //** SPI transfer in progress
      return ERR__SPI_OTHER_BUSY;  // These status are related to another transfer

    case SPI_STATUS_UNINITIALIZED: //** SPI uninitialized
    case SPI_STATUS_READY:         //** SPI transfer is ready
    default:                       // Configure the transfer with the following lines
      break;
  }

  //--- Endianness configuration for data striding ---
  const eSPI_EndianTransform EndianTransform = SPI_ENDIAN_TRANSFORM_GET(pPacketDesc->Config.Value); // Only the endianness configuration is important for this driver
  const size_t BlockSize = (EndianTransform == SPI_NO_ENDIAN_CHANGE ? 1 : (size_t)EndianTransform); // Get block size. No endian change = 8-bits data
  if ((pPacketDesc->DataSize % BlockSize) > 0) return ERR__DATA_MODULO;                             // Data block size shall be a multiple of data size
  size_t CurrentBlockPos = BlockSize;

  //--- Prepare data ---
  uint8_t* pTxData = (pPacketDesc->TxData != NULL ? &pPacketDesc->TxData[BlockSize - 1] : NULL);    // Adjust the start of data for endianness of TxData
  uint8_t* pRxData = (pPacketDesc->RxData != NULL ? &pPacketDesc->RxData[BlockSize - 1] : NULL);    // Adjust the start of data for endianness of RxData
  if (__SPItransferList[PeriphNumber].IsAsserted == false)    // Start a transfer if not already done
  {
    if ((pSPI->SPI_MR & SPI_MR_PS) == 0)                      // Fixed chip select?
    {
      pSPI->SPI_MR &= (~SPI_MR_PCS_Msk);
      if ((pSPI->SPI_MR & SPI_MR_PCSDEC) == 0)                // Do not use Chip Select Decode?
           pSPI->SPI_MR |= SPI_MR_PCS(SPI_PCS_GET(pPacketDesc->ChipSelect));
      else pSPI->SPI_MR |= SPI_MR_PCS(pPacketDesc->ChipSelect);
      pSPI->SPI_CR = SPI_CR_SPIEN;                            // Enable SPI here, on fixed PS
    }
    __SPItransferList[PeriphNumber].IsAsserted = true;
  }
  //--- Transfer data ---
  size_t RemainingBytes = pPacketDesc->DataSize;
  while (RemainingBytes > 0)
  {
    //--- Prepare transmit data ---
    if ((pTxData != NULL) && (UseDummyByte == false))
    {
      DataToSend = *pTxData;
      //--- Adjust Tx data address with data striding ---
      --CurrentBlockPos;
      if (CurrentBlockPos == 0)
      {
        pTxData += (2 * BlockSize) - 1;
        CurrentBlockPos = BlockSize;
      }
      else --pTxData;
    }
    else DataToSend = pPacketDesc->DummyByte;
    if ((pSPI->SPI_MR & SPI_MR_PS) > 0)                       // Variable chip select?
    {
      DataToSend |= SPI_TDR_PCS(SPI_PCS_GET(pPacketDesc->ChipSelect));
      if ((RemainingBytes == 1) && (pPacketDesc->Terminate)) DataToSend |= SPI_TDR_LASTXFER;
    }

    //--- Transmit data ---
    Timeout = SPI_TIMEOUT;
    while ((pSPI->SPI_SR & SPI_SR_TDRE) == 0)                 // SPI Tx ready?
      if ((Timeout--) == 0) { ForceTerminate = true; break; } // Timeout ? return an error
    pSPI->SPI_TDR = DataToSend;                               // Send the data

    //--- Receive data ---
    Timeout = SPI_TIMEOUT;
    while ((pSPI->SPI_SR & SPI_SR_RDRF) == 0)                 // SPI Rx ready?
      if ((Timeout--) == 0) { ForceTerminate = true; break; } // Timeout ? return an error
    DataRead = (uint8_t)(pSPI->SPI_RDR & 0xFF);               // Read the receive register
    if (pRxData != NULL)
    {
      *pRxData = DataRead;
      //--- Adjust Rx data address with data striding ---
      --CurrentBlockPos;
      if (CurrentBlockPos == 0)
      {
        pRxData += (2 * BlockSize) - 1;
        CurrentBlockPos = BlockSize;
      }
      else --pRxData;
    }
    --RemainingBytes;
  }
  if (pPacketDesc->Terminate || ForceTerminate)
  {
    //--- Stop transfer ---
    pSPI->SPI_CR |= SPI_CR_LASTXFER;
    __SPItransferList[PeriphNumber].IsAsserted = false;
    if ((pSPI->SPI_MR & SPI_MR_PS) == 0) pSPI->SPI_CR = SPI_CR_SPIDIS; // Disable SPI only on fixed PS
  }

  //--- Endianness result ---
  pPacketDesc->Config.Value &= ~SPI_ENDIAN_RESULT_Mask;
  pPacketDesc->Config.Value |= SPI_ENDIAN_RESULT_SET(EndianTransform); // Indicate that the endian transform have been processed
  return (ForceTerminate ? ERR__SPI_TIMEOUT : ERR_OK);
}

eERRORRESULT SPI_Transfer_Gen(SPI_Interface *pIntDev, uint8_t chipSelect, uint8_t *txData, uint8_t *rxData, size_t size)
{
  if (pIntDev == NULL) return ERR__SPI_PARAMETER_ERROR;
  Spi* pSPI = (Spi*)(pIntDev->InterfaceDevice);
  SPIInterface_Packet PacketDesc =
  {
    SPI_MEMBER(Config.Value) SPI_NO_POLLING | SPI_ENDIAN_TRANSFORM_SET(SPI_NO_ENDIAN_CHANGE),
    SPI_MEMBER(ChipSelect  ) chipSelect,
    SPI_MEMBER(DummyByte   ) 0x00,
    SPI_MEMBER(TxData      ) txData,
    SPI_MEMBER(RxData      ) rxData,
    SPI_MEMBER(DataSize    ) size,
    SPI_MEMBER(Terminate   ) true,
  };
  return SPI_Transfer(pSPI, &PacketDesc);
}





//********************************************************************************************************************
// SPI with DMA driver API
//********************************************************************************************************************
//=============================================================================
// SPI DMA interrupt handler
//=============================================================================
static void SPI_DMA_Handler(HandleXDMAC dmaChannel, uintptr_t context, setXDMACchan_InterruptEvents interrupts)
{
  Spi* pSPI = (Spi*)context;
  uint32_t PeriphNumber = SPI_GetPeripheralNumber(pSPI);

  //--- Get DMA Status ---
  if ((interrupts & XDMAC_CIS_BIS) > 0) // End of Block Interrupt Status Bit
  {
    //--- Disable DMA ---
    (void)XDMAC_InterruptDisable(dmaChannel, XDMAC_CIE_BIE, true); // Disable general DMA channel interrupt and more specifically the End of Block Interrupt
    (void)XDMAC_ChannelDisable(dmaChannel);
    return;
  }

  //--- Generate fault status ---
  __SPItransferList[PeriphNumber].Status = SPI_STATUS_FAULT;
  __SPItransferList[PeriphNumber].Error  = ERR__DMA_ERROR;     // Generic DMA error
  if ((interrupts & XDMAC_CIE_ROIE) > 0) __SPItransferList[PeriphNumber].Error = ERR__DMA_OVERFLOW_ERROR;  // Request Overflow Error Interrupt Enable Bit
  if ((interrupts & XDMAC_CIE_WBIE) > 0) __SPItransferList[PeriphNumber].Error = ERR__DMA_WRITE_BUS_ERROR; // Write Bus Error Interrupt Enable Bit
  if ((interrupts & XDMAC_CIE_RBIE) > 0) __SPItransferList[PeriphNumber].Error = ERR__DMA_READ_BUS_ERROR;  // Read Bus Error Interrupt Enable Bit
}





//=============================================================================
// Hardware SPI data transfer with DMA communication for the Atmel SPI
//=============================================================================
static eERRORRESULT __SPI_DMA_Transfer(Spi *pSPI, SPIInterface_Packet* const pPacketDesc)
{
  eERRORRESULT Error;
  uint32_t PeriphNumber = SPI_GetPeripheralNumber(pSPI);
  if (PeriphNumber == SPI_INVALID_PERIPHERAL) return ERR__PERIPHERAL_NOT_VALID;
  uint32_t PeriphID = SPI_GetPeripheralID(pSPI);
  if (PeriphID == SPI_INVALID_PERIPHERAL) return ERR__PERIPHERAL_NOT_VALID;

  //--- Check the state of the transfer ---
  switch (__SPItransferList[PeriphNumber].Status)
  {
    case SPI_STATUS_UNINITIALIZED: return ERR__SPI_CONFIG_ERROR; //** SPI uninitialized
    case SPI_STATUS_IN_PROGRESS:   return ERR__SPI_BUSY;         //** SPI transfer in progress

    case SPI_STATUS_COMPLETE:                                    //** SPI transfer complete
#ifdef CONF_BOARD_ENABLE_CACHE
      SCB_InvalidateDCache_by_Addr((uint32_t*)__SPItransferList[PeriphNumber].Data, __SPItransferList[PeriphNumber].Size);
#endif
      pPacketDesc->Config.Value = __SPItransferList[PeriphNumber].Config.Value;
//      pPacketDesc->Config.Value |= SPI_ENDIAN_RESULT_SET(SPI_ENDIAN_TRANSFORM_GET(pPacketDesc->Config.Value)); // Indicate that the endian transform have been processed
      __SPItransferList[PeriphNumber].Status = SPI_STATUS_READY; // Set status as ready for the next transfer
      __SPItransferList[PeriphNumber].Config.Value = 0;          // Clear current configuration
      __SPItransferList[PeriphNumber].IsAsserted = false;        // Transfer is deasserted
      return ERR_OK;

    case SPI_STATUS_FAULT:                                       //** SPI transfer in fault
      __SPItransferList[PeriphNumber].Status = SPI_STATUS_READY; // Set ready for a new transfer
      __SPItransferList[PeriphNumber].Config.Value = 0;          // Clear current configuration
      return __SPItransferList[PeriphNumber].Error;              // Return the last transfer error

    case SPI_STATUS_READY:                                       //** SPI transfer is ready
    default:                                                     // Configure the transfer with the following lines
      break;
  }
  if (pPacketDesc->DataSize == 0) return ERR_OK;                 // Nothing to send, return

  //--- Configure the transfer ---
  if ((pSPI->SPI_MR & SPI_MR_PS) == 0)                           // Fixed chip select?
  {
    pSPI->SPI_MR &= (~SPI_MR_PCS_Msk);
    if ((pSPI->SPI_MR & SPI_MR_PCSDEC) == 0)                     // Do not use Chip Select Decode?
    pSPI->SPI_MR |= SPI_MR_PCS(SPI_PCS_GET(pPacketDesc->ChipSelect));
    else pSPI->SPI_MR |= SPI_MR_PCS(pPacketDesc->ChipSelect);
  }
  Error = SPI_InterruptDisable(pSPI, SPI_ALL_INTERRUPTS, false);
  if (Error != ERR_OK) return Error;

  //--- Configure status ---
  __SPItransferList[PeriphNumber].TxData         = pPacketDesc->TxData;
  __SPItransferList[PeriphNumber].RxData         = pPacketDesc->RxData;
  __SPItransferList[PeriphNumber].Size           = pPacketDesc->DataSize;
  __SPItransferList[PeriphNumber].IsAsserted     = false;
  __SPItransferList[PeriphNumber].Status         = SPI_STATUS_IN_PROGRESS;
  __SPItransferList[PeriphNumber].Error          = ERR_OK;
  __SPItransferList[PeriphNumber].TransactionCounter++;
  if (__SPItransferList[PeriphNumber].TransactionCounter > SPI_TRANSACTION_NUMBER_Mask) __SPItransferList[PeriphNumber].TransactionCounter = 1; // Value cannot be 0
  __SPItransferList[PeriphNumber].Config.Value   = (pPacketDesc->Config.Value & ~SPI_ENDIAN_RESULT_Mask)
                                                   | SPI_TRANSACTION_NUMBER_SET(__SPItransferList[PeriphNumber].TransactionCounter);
  pPacketDesc->Config.Value |= SPI_TRANSACTION_NUMBER_SET(__SPItransferList[PeriphNumber].TransactionCounter); // Set the transaction number for the driver

  //--- Configure the XDMAC ---
  if (pPacketDesc->TxData != NULL)
  {
    //--- Configure the DMA for a write ---
    __XDMAC_SPIconfig.MBR_SA  = (uint32_t)pPacketDesc->TxData; // Source Address Member
    __XDMAC_SPIconfig.MBR_DA  = (uint32_t)pSPI->SPI_TDR;       // Destination Address Member
    __XDMAC_SPIconfig.MBR_BC  = 0;                             // Block Control Member. Microblock count is 1 per blocks
    __XDMAC_SPIconfig.MBR_UBC = pPacketDesc->DataSize;         // Microblock Control Member (size)
    __XDMAC_SPIconfig.MBR_CFG = XDMAC_CC_TYPE_PER_TRAN         // Synchronized mode (Peripheral to Memory or Memory to Peripheral Transfer)
                              | XDMAC_CC_MBSIZE_SINGLE         // The memory burst size is set to one
                              | XDMAC_CC_DSYNC_MEM2PER         // Memory transfer to Peripheral
//                              | XDMAC_CC_SWREQ_HWR_CONNECTED   // Hardware request line is connected to the peripheral request line
                              | XDMAC_CC_MEMSET_NORMAL_MODE    // Memset is not activated
                              | XDMAC_CC_CSIZE_CHK_1           // Chunk Size 1 data transferred
                              | XDMAC_CC_DWIDTH_BYTE           // The data size is set to 8 bits
                              | XDMAC_CC_SIF_AHB_IF0           // The data is read through the system bus interface 0
                              | XDMAC_CC_DIF_AHB_IF1           // The data is written through the system bus interface 1
                              | XDMAC_CC_SAM_INCREMENTED_AM    // The Source Addressing mode is incremented (the increment size is set to the data size)
                              | XDMAC_CC_DAM_FIXED_AM          // The Destination Address remains unchanged
                              | XDMAC_CC_PERID(XDMAC_SPI_PERID_Base + 0 + (PeriphNumber * 2)); // Channel x Peripheral Identifier
    __XDMAC_SPIconfig.MBR_DS  = 0;                             // Data Stride Member
    __XDMAC_SPIconfig.MBR_SUS = 0;                             // Source Microblock Stride Member.
    __XDMAC_SPIconfig.MBR_DUS = 0;                             // Destination Microblock Stride Member
    __XDMAC_SPIconfig.MBR_NDA = 0;                             // Next Descriptor Address
    __XDMAC_SPIconfig.MBR_NDC = 0;                             // Next Descriptor Control
    __XDMAC_SPIconfig.NDAIF   = 0;                             // Next Descriptor Interface
    //--- Set XDMA interrupts ---
    __XDMAC_SPIconfig.Interrupts = XDMAC_CIE_BIE               // End of Block Interrupt Enable Bit
                                 | XDMAC_CIE_DIE               // End of Disable Interrupt Enable Bit
                                 | XDMAC_CIE_FIE               // End of Flush Interrupt Enable Bit
                                 | XDMAC_CIE_RBIE              // Read Bus Error Interrupt Enable Bit
                                 | XDMAC_CIE_WBIE              // Write Bus Error Interrupt Enable Bit
                                 | XDMAC_CIE_ROIE;             // Request Overflow Error Interrupt Enable Bit
    //--- Configure and enable DMA channel ---
    Error = XDMAC_ConfigureTransfer(__HasReservedDMAchannel[PeriphNumber].Tx, &__XDMAC_SPIconfig);
    if (Error != ERR_OK) return Error;
#ifdef CONF_BOARD_ENABLE_CACHE
    SCB_CleanDCache_by_Addr((uint32_t*)__SPItransferList[PeriphNumber].TxData, __SPItransferList[PeriphNumber].Size);
#endif
    Error = XDMAC_ChannelEnable(__HasReservedDMAchannel[PeriphNumber].Tx);
    if (Error != ERR_OK) return Error;
  }
  if (pPacketDesc->RxData != NULL)
  {
    //--- Configure the DMA for a read ---
    __XDMAC_SPIconfig.MBR_SA  = (uint32_t)pSPI->SPI_RDR;       // Source Address Member
    __XDMAC_SPIconfig.MBR_DA  = (uint32_t)pPacketDesc->RxData; // Destination Address Member
    __XDMAC_SPIconfig.MBR_BC  = 0;                             // Block Control Member. Microblock count is 1 per blocks
    __XDMAC_SPIconfig.MBR_UBC = pPacketDesc->DataSize;         // Microblock Control Member (size)
    __XDMAC_SPIconfig.MBR_CFG = XDMAC_CC_TYPE_PER_TRAN         // Synchronized mode (Peripheral to Memory or Memory to Peripheral Transfer)
                              | XDMAC_CC_MBSIZE_SINGLE         // The memory burst size is set to one
                              | XDMAC_CC_DSYNC_PER2MEM         // Peripheral to Memory transfer
//                                | XDMAC_CC_SWREQ_HWR_CONNECTED   // Hardware request line is connected to the peripheral request line
                              | XDMAC_CC_MEMSET_NORMAL_MODE    // Memset is not activated
                              | XDMAC_CC_CSIZE_CHK_1           // Chunk Size 1 data transferred
                              | XDMAC_CC_DWIDTH_BYTE           // The data size is set to 8 bits
                              | XDMAC_CC_SIF_AHB_IF1           // The data is read through the system bus interface 1
                              | XDMAC_CC_DIF_AHB_IF0           // The data is written through the system bus interface 0
                              | XDMAC_CC_SAM_FIXED_AM          // The Source Address remains unchanged
                              | XDMAC_CC_DAM_INCREMENTED_AM    // The Destination Addressing mode is incremented (the increment size is set to the data size)
                              | XDMAC_CC_PERID(XDMAC_SPI_PERID_Base + 1 + (PeriphNumber * 2)); // Channel x Peripheral Identifier
    __XDMAC_SPIconfig.MBR_DS  = 0;                             // Data Stride Member
    __XDMAC_SPIconfig.MBR_SUS = 0;                             // Source Microblock Stride Member.
    __XDMAC_SPIconfig.MBR_DUS = 0;                             // Destination Microblock Stride Member
    __XDMAC_SPIconfig.MBR_NDA = 0;                             // Next Descriptor Address
    __XDMAC_SPIconfig.MBR_NDC = 0;                             // Next Descriptor Control
    __XDMAC_SPIconfig.NDAIF   = 0;                             // Next Descriptor Interface
    //--- Set XDMA interrupts ---
    __XDMAC_SPIconfig.Interrupts = XDMAC_CIE_BIE               // End of Block Interrupt Enable Bit
                                 | XDMAC_CIE_DIE               // End of Disable Interrupt Enable Bit
                                 | XDMAC_CIE_FIE               // End of Flush Interrupt Enable Bit
                                 | XDMAC_CIE_RBIE              // Read Bus Error Interrupt Enable Bit
                                 | XDMAC_CIE_WBIE              // Write Bus Error Interrupt Enable Bit
                                 | XDMAC_CIE_ROIE;             // Request Overflow Error Interrupt Enable Bit
    //--- Configure and enable DMA channel ---
    Error = XDMAC_ConfigureTransfer(__HasReservedDMAchannel[PeriphNumber].Rx, &__XDMAC_SPIconfig);
    if (Error != ERR_OK) return Error;
#ifdef CONF_BOARD_ENABLE_CACHE
    SCB_CleanDCache_by_Addr((uint32_t*)__SPItransferList[PeriphNumber].RxData, __SPItransferList[PeriphNumber].Size);
#endif
    Error = XDMAC_ChannelEnable(__HasReservedDMAchannel[PeriphNumber].Rx);
    if (Error != ERR_OK) return Error;
  }
  return ERR__SPI_BUSY;
}





//********************************************************************************************************************
//=============================================================================
// Atmel SPI master with DMA initialization
//=============================================================================
eERRORRESULT SPI_DMA_MasterInit(Spi *pSPI, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreq)
{
  if (pSPI == NULL) return ERR__SPI_PARAMETER_ERROR;

  //--- Configure DMA ---
  uint32_t PeriphNumber = SPI_GetPeripheralNumber(pSPI);
  if (PeriphNumber == SPI_INVALID_PERIPHERAL) return ERR__PERIPHERAL_NOT_VALID;
  if (__HasReservedDMAchannel[PeriphNumber].Tx == XDMAC_INVALID_HANDLE) // Tx DMA
  {
    __SPItransferList[PeriphNumber].Status = SPI_STATUS_READY;
    __HasReservedDMAchannel[PeriphNumber].Tx = XDMAC_OpenChannel(XDMAC, SPI_DMA_Handler, (uintptr_t)pSPI); // Open a DMA channel for this SPI peripheral for Tx
  }
  if (__HasReservedDMAchannel[PeriphNumber].Rx == XDMAC_INVALID_HANDLE) // Rx DMA
  {
    __SPItransferList[PeriphNumber].Status = SPI_STATUS_READY;
    __HasReservedDMAchannel[PeriphNumber].Rx = XDMAC_OpenChannel(XDMAC, SPI_DMA_Handler, (uintptr_t)pSPI); // Open a DMA channel for this SPI peripheral for Rx
  }
  if ((__SPItransferList[PeriphNumber].Status != SPI_STATUS_UNINITIALIZED)
   && (__SPItransferList[PeriphNumber].Status != SPI_STATUS_READY)) return ERR__SPI_OTHER_BUSY;

  //--- Configuration of the TWI interface ---
  return SPI_MasterInit(pSPI, chipSelect, mode, sckFreq);
}

eERRORRESULT SPI_DMA_MasterInit_Gen(SPI_Interface *pIntDev, uint8_t chipSelect, eSPIInterface_Mode mode, const uint32_t sckFreq)
{
  if (pIntDev == NULL) return ERR__SPI_PARAMETER_ERROR;
  Spi* pSPI = (Spi*)(pIntDev->InterfaceDevice);
  return SPI_DMA_MasterInit(pSPI, chipSelect, mode, sckFreq);
}



//=============================================================================
// Hardware SPI data packet transfer communication for the Atmel SPI
//=============================================================================
eERRORRESULT SPI_PacketTransfer(Spi *pSPI, SPIInterface_Packet* const pPacketDesc)
{
  if (pSPI == NULL) return ERR__SPI_PARAMETER_ERROR;
  uint32_t PeriphNumber = SPI_GetPeripheralNumber(pSPI);

  //--- Use polling? ---
  if ((pPacketDesc->Config.Value & SPI_USE_POLLING) == SPI_USE_POLLING)
  {
    if (__HasReservedDMAchannel[PeriphNumber].Tx == XDMAC_INVALID_HANDLE) return ERR__DMA_NOT_CONFIGURED;
    if (__HasReservedDMAchannel[PeriphNumber].Rx == XDMAC_INVALID_HANDLE) return ERR__DMA_NOT_CONFIGURED;
    const uint8_t CurrentTransaction = SPI_TRANSACTION_NUMBER_GET(__SPItransferList[PeriphNumber].Config.Value);
    const uint8_t NewTransaction     = SPI_TRANSACTION_NUMBER_GET(pPacketDesc->Config.Value);
    if (NewTransaction != CurrentTransaction) return ERR__SPI_OTHER_BUSY; // It is a transfer from an another driver
    return __SPI_DMA_Transfer(pSPI, pPacketDesc);
  }
  
  //--- Do a regular transfer ---
  return SPI_Transfer(pSPI, pPacketDesc);
}

eERRORRESULT SPI_PacketTransfer_Gen(SPI_Interface *pIntDev, SPIInterface_Packet* const pPacketDesc)
{
  Spi* pSPI = (Spi*)(pIntDev->InterfaceDevice);
  return SPI_PacketTransfer(pSPI, pPacketDesc);
}





//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------