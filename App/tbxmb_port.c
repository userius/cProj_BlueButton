/****************************************************************************************
 * \file         demos/modbus/ARM_CORTEXM_ST_Nucleo_F429ZI/tbxmb_port.c
 * \brief        Modbus hardware specific port source file.
 * \details      This MicroTBX-Modbus port for the Nucleo-F429ZI board supports three
 *               serial ports:
 *
 *                 - TBX_MB_UART_PORT1 = USART6
 *                 - TBX_MB_UART_PORT2 = USART3
 *                 - TBX_MB_UART_PORT3 = USART2
 *
 *               On the Nucleo-F429ZI board, USART3 on PD8 and PD9 is connected to the
 *               on-board ST-Link/V2.1 debugger interface, which exposes it as a virtual
 *               COM-port on the PC using the USB-CDC class.
 *
 *               USART6 is configured for PG9 and PG14 with the idea that it's used in
 *               combination with a Waveshare RS485/CAN shield. This Arduino type shield
 *               offers an RS485 transceiver, allowing MicroTBX-Modbus to be tested on
 *               an RS485 network. Make sure to set the UART jumpers of RX2 and TX2 on the
 *               shield.
 *
 *               The RS485 transceiver on this shield has the usual Driver Enable (DE) and
 *               Receiver Enable (RE) inputs. These inputs are connected together and can
 *               be controlled with the single PF13 digital output.
 *
 *               USART2 is configured for PD5 and PD6 with the idea that you can connect
 *               these pins to USART6 PG9 and PG14 for a loopback between USART6 and
 *               USART2:
 *                 - PD5 = USART2_TX <--> USART6_RX = PG9
 *                 - PD6 = USART2_RX <--> USART6_TX = PG14
 *
 *               In such a loopback configuration you could for example have both a client
 *               and a server on one and the same board, for testing and simulation
 *               purposes.
 *
 *               The 20 kHz free running timer counter, needed for exact Modbus-RTU
 *               timings, is realized with the help of TIM7.
 *
 *               Note that the implementation of this MicroTBX-Modbus port assumes that
 *               the following topics are handled by the application, upon initialization:
 *
 *                 - Enabling of the USART2, USART3, USART6 and TIM7 peripheral clocks.
 *                 - Configuration of the USART2, USART3, USART6 and RS485 DE/NRE GPIO
 *                   pins.
 *                 - Enabling the USART2, USART3 and USART6 interrupts in the NVIC.
 *
 *               In the demo programs, this is handled by function BspInit().
 * \internal
 *----------------------------------------------------------------------------------------
 *                          C O P Y R I G H T
 *----------------------------------------------------------------------------------------
 *   Copyright (c) 2024 by Feaser     www.feaser.com     All rights reserved
 *
 *----------------------------------------------------------------------------------------
 *                            L I C E N S E
 *----------------------------------------------------------------------------------------
 *
 * SPDX-License-Identifier: MIT
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * \endinternal
 ****************************************************************************************/

/** Include files. ------------------------------------------------------------------- */

#include "microtbx.h"       /* MicroTBX library                   */
#include "microtbxmodbus.h" /* MicroTBX-Modbus library            */
#include "main.h"           /* STM32 CPU and HAL                  */

/* Select which timer to use for Modbus timing */
#define TBXMB_TIM TIM3
#ifndef TBXMB_TIM
#error "You must define TBXMB_TIM: TIM1, TIM2, TIM3 or TIM4"
#endif

typedef struct _driver_enable_pin {
  GPIO_TypeDef *psPort;
  uint16_t      Pin;
} sDrvEnPin_t, *psDrvEnPin_t;

typedef struct {
  UART_HandleTypeDef *phUart;     /**< USART channel handle.                    */
  USART_TypeDef      *psInstance; /**< USART instance pointer.                  */
  psDrvEnPin_t        psDrvEn;    /**< Driver enable pin configuration.         */
  uint8_t             RxByte;     /**< USART single byte reception buffer.      */
} sTbxMbPort_t, *psTbxMbPort_t;

/** Function prototypes. ------------------------------------------------------------- */
static void TbxMbPortUartDriverEnable( tTbxMbUartPort port, uint8_t value );
static void TbxMb_HAL_UART_MspInit( UART_HandleTypeDef *ph );
static void TbxMb_HAL_UART_MspDeInit( UART_HandleTypeDef *ph );
static void TbxMb_HAL_UART_TxCpltCallback( UART_HandleTypeDef *ph );
static void TbxMb_HAL_UART_RxCpltCallback( UART_HandleTypeDef *ph );
static void TbxMb_HAL_UART_ErrorCallback( UART_HandleTypeDef *ph );

/** Local data declarations. --------------------------------------------------------- */
extern UART_HandleTypeDef huart2;     // USART2 handle.

static sTbxMbPort_t asTbxMbPorts[] = {
    // TBX_MB_UART_PORT1 mapped to USART2.
    { .phUart = &huart2, .psInstance = USART2, .psDrvEn = NULL },
    // { .psInstance = USART1 },     // TBX_MB_UART_PORT2 mapped to USART1.
    // { .instance = USART3 },       // TBX_MB_UART_PORT3 mapped to USART3.
};

/** -------------------------------------------------------------------------------------
 * \brief     Initializes the UART channel.
 * \attention It is assumed that the following items were already handled by the
 *            application, upon initialization:
 *            - Enabling of the UART channel's peripheral clock.
 *            - Configuration of the UART Tx and Rx GPIO pins.
 *            - Enabling the UART interrupts in the NVIC.
 * \param     port The serial port to use. The actual meaning of the serial port is
 *            hardware dependent. It typically maps to the UART peripheral number. E.g.
 *            TBX_MB_UART_PORT1 = USART1 on an STM32, although this is not mandatory.
 * \param     baudrate The desired communication speed.
 * \param     databits Number of databits for a character.
 * \param     stopbits Number of stop bits at the end of a character.
 * \param     parity Parity bit type to use.
 */
void TbxMbPortUartInit( tTbxMbUartPort port, tTbxMbUartBaudrate baudrate,
                        tTbxMbUartDatabits databits, tTbxMbUartStopbits stopbits,
                        tTbxMbUartParity parity ) {
  //
  const uint32_t baudrateLookup[ TBX_MB_UART_NUM_BAUDRATE ] = {
      1200,      // TBX_MB_UART_1200BPS
      2400,      // TBX_MB_UART_2400BPS
      4800,      // TBX_MB_UART_4800BPS
      9600,      // TBX_MB_UART_9600BPS
      19200,     // TBX_MB_UART_19200BPS
      38400,     // TBX_MB_UART_38400BPS
      57600,     // TBX_MB_UART_57600BPS
      115200     // TBX_MB_UART_115200BPS
  };
  const uint32_t stopbitsLookup[ TBX_MB_UART_NUM_STOPBITS ] = {
      UART_STOPBITS_1,     // TBX_MB_UART_1_STOPBITS
      UART_STOPBITS_2      // TBX_MB_UART_2_STOPBITS
  };
  const uint32_t parityLookup[ TBX_MB_UART_NUM_PARITY ] = {
      UART_PARITY_ODD,      // TBX_MB_ODD_PARITY
      UART_PARITY_EVEN,     // TBX_MB_EVEN_PARITY
      UART_PARITY_NONE      // TBX_MB_NO_PARITY
  };

  /* Make sure the requested serial port is actually supported by this module. */
  TBX_ASSERT( port < ( sizeof( asTbxMbPorts ) / sizeof( asTbxMbPorts[ 0 ] ) ) );

  psTbxMbPort_t       psPort = &asTbxMbPorts[ port ];
  UART_HandleTypeDef *phUart = psPort->phUart;

  /* Initialize the channel's handle. */
  do {
    phUart->Instance = psPort->psInstance;

    UART_InitTypeDef *psInit = &phUart->Init;
    psInit->Mode             = UART_MODE_TX_RX;
    psInit->HwFlowCtl        = UART_HWCONTROL_NONE;
    psInit->OverSampling     = UART_OVERSAMPLING_16;
    psInit->BaudRate         = baudrateLookup[ baudrate ];
    psInit->Parity           = parityLookup[ parity ];
    psInit->StopBits         = stopbitsLookup[ stopbits ];

    /* Initialize the number of databits to 8. This covers these configurations:
     * - 7 databits with parity enabled.
     * - 8 databits with parity disabled.    */
    uint32_t wordLength = UART_WORDLENGTH_8B;
    /* The STM32F4's UART peripheral only supports 7 databits if you also have parity
     * enabled. Meaning that this configuration is not possible:
     * - 7 databits with parity disabled.    */
    if ( ( parity == TBX_MB_NO_PARITY ) && ( databits == TBX_MB_UART_7_DATABITS ) ) {
      /* Trigger assertion error due to an invalid configuration. */
      TBX_ASSERT( TBX_FALSE );
    }
    /* One other configuration might still be possible:
     * - 8 databits with parity enabled. */
    else if ( ( databits == TBX_MB_UART_8_DATABITS ) && ( parity != TBX_MB_NO_PARITY ) ) {
      wordLength = UART_WORDLENGTH_9B;
    }
    psInit->WordLength = wordLength;

    HAL_UART_RegisterCallback( phUart, HAL_UART_MSPINIT_CB_ID, TbxMb_HAL_UART_MspInit );
    HAL_UART_RegisterCallback( phUart, HAL_UART_MSPDEINIT_CB_ID, TbxMb_HAL_UART_MspDeInit );
    HAL_UART_RegisterCallback( phUart, HAL_UART_TX_COMPLETE_CB_ID, TbxMb_HAL_UART_TxCpltCallback );
    HAL_UART_RegisterCallback( phUart, HAL_UART_RX_COMPLETE_CB_ID, TbxMb_HAL_UART_RxCpltCallback );
    HAL_UART_RegisterCallback( phUart, HAL_UART_ERROR_CB_ID, TbxMb_HAL_UART_ErrorCallback );
  } while ( 0 );

  /* Initialize the channel. */
  HAL_UART_Init( phUart );
  /* Kick off first byte reception. */
  HAL_UART_Receive_IT( phUart, &psPort->RxByte, 1 );

  return;
}

/** -------------------------------------------------------------------------------------
 * \brief     Starts the transfer of len bytes from the data array on the specified
 *            serial port.
 * \attention This function has mutual exclusive access to the bytes in the data[] array,
 *            until this port module calls TbxMbUartTransmitComplete(). This means that
 *            you do not need to copy the data bytes to a local buffer. This approach
 *            keeps RAM requirements low and benefits the run-time performance. Just make
 *            sure to call TbxMbUartTransmitComplete() once all bytes are transmitted or
 *            an error was detected, to release access to the data[] array.
 * \param     port The serial port to start the data transfer on.
 * \param     data Byte array with data to transmit.
 * \param     len Number of bytes to transmit.
 * \return    TBX_OK if successful, TBX_ERROR otherwise.
 */
uint8_t TbxMbPortUartTransmit( tTbxMbUartPort port, uint8_t const *data, uint16_t len ) {
  //
  uint8_t result = TBX_ERROR;

  /* Make sure the requested serial port is actually supported by this module. */
  TBX_ASSERT( port <= ( sizeof( asTbxMbPorts ) / sizeof( asTbxMbPorts[ 0 ] ) ) );

  /* Switch the hardware from reception to transmission mode. */
  TbxMbPortUartDriverEnable( port, TBX_ON );
  /* Start the interrupt driven transmission of the data bytes. */
  if ( HAL_UART_Transmit_IT( asTbxMbPorts[ port ].phUart, (uint8_t *) data, len ) == HAL_OK ) {
    result = TBX_OK;
  }

  return result;
} /*** end of TbxMbPortUartTransmit ***/

/** -------------------------------------------------------------------------------------
 * \brief     Changes the state of the driver enable pin for the available serial port.
 *            This is only applicable for RS485 networks, which require an RS485
 *            transceiver.
 * \attention It is assumed that the following items were already handled by the
 *            application, upon initialization:
 *            - Configuration of the DE/NRE GPIO pin(s) as a digital output.
 * \details   When communicating via RS485, the transceiver typically has a driver enable
 *            (DE) pin which is active high, and a receiver enable (NRE) pin which is
 *            active low. Both pins must be set high when transmitting data and low when
 *            receiving data. Note that on most PCBs, the DE and NRE pins are connected
 *            together, in which case you only need to change one output pin, as opposed
 *            to two.
 * \param     port The serial port to change the state of the driver enable pin for.
 * \param     value TBX_ON to set the DE/NRE pins to logic high, TBX_OFF for logic low.
 */
static void TbxMbPortUartDriverEnable( tTbxMbUartPort port, uint8_t value ) {
  /**
   *  Determine the number of available ports.
   *  Make sure the requested serial port is actually supported by this module.
   *  If the port number is valid, proceed.
   *    Get pointer to the driver enable pin configuration structure.
   *    If a driver enable pin was configured, set it to the requested value.
   *      Set the DE/NRE pin to the requested value.
   */
  static uint8_t _PortsQntt = ( sizeof( asTbxMbPorts ) / sizeof( asTbxMbPorts[ 0 ] ) );
  TBX_ASSERT( port < _PortsQntt );
  if ( port < _PortsQntt ) {
    psDrvEnPin_t _psDrv = asTbxMbPorts[ port ].psDrvEn;
    if ( _psDrv != NULL )
      HAL_GPIO_WritePin( _psDrv->psPort, _psDrv->Pin,
                         ( value == TBX_ON ) ? GPIO_PIN_SET : GPIO_PIN_RESET );
  }

  return;
}

/** -------------------------------------------------------------------------------------
 * \brief     Obtains the free running counter value of the timer defined by TBXMB_TIM.
 * \attention The application must define the macro TBXMB_TIM to select which timer is
 *            used (TIM1, TIM2, TIM3, or TIM4 on STM32F103RB).
 *
 * \details   The Modbus RTU protocol requires precise character timing:
 *              - T1_5 (1.5 character times) for maximum inter-character delay
 *              - T3_5 (3.5 character times) for minimum inter-frame delay
 *
 *            To support this, a free running counter running at 20 kHz
 *            (50 microseconds per tick) is used as a time reference.
 *
 *            This function performs a one-time initialization of the selected timer:
 *              - Enables the timer’s peripheral clock.
 *              - Configures the timer in up-counting mode with a prescaler so that
 *                the counter ticks at 20 kHz.
 *              - Sets the counter to free-running mode with a 16-bit period (0xFFFF).
 *              - Starts the timer in base mode (no interrupts required).
 *
 *            Note: TIM1 resides on the APB2 bus, while TIM2–TIM4 are on APB1. The
 *            function automatically selects the correct clock source frequency.
 *
 * \return    Current 16-bit free running counter value of TBXMB_TIM.
 */
uint16_t TbxMbPortTimerCount( void ) {
  static uint8_t           IsTimInit = TBX_FALSE;
  static TIM_HandleTypeDef hTim      = { 0 };

  if ( IsTimInit == TBX_FALSE ) {
    IsTimInit = TBX_TRUE;
    /* Enable peripheral clock depending on selected timer */
    switch ( (uint32_t) TBXMB_TIM ) {
      case (uint32_t) TIM1: __HAL_RCC_TIM1_CLK_ENABLE( ); break;
      case (uint32_t) TIM2: __HAL_RCC_TIM2_CLK_ENABLE( ); break;
      case (uint32_t) TIM3: __HAL_RCC_TIM3_CLK_ENABLE( ); break;
      case (uint32_t) TIM4: __HAL_RCC_TIM4_CLK_ENABLE( ); break;
      /* Unsupported timer selected for TBXMB_TIM on STM32F103RB. */
      default: TBX_ASSERT( TBX_FALSE ); break;
    }
    /**
     * Get timer clock frequency. HAL automatically accounts for APB prescaler.
     * TIM1 is on APB2, others on APB1 for STM32F103RB.
     */
    uint32_t timFreq = ( TBXMB_TIM != TIM1 ) ? HAL_RCC_GetPCLK1Freq( ) : HAL_RCC_GetPCLK2Freq( );

    /* Configure timer for 20 kHz free running counter (50 us per tick) */
    hTim.Instance               = TBXMB_TIM;
    hTim.Init.Prescaler         = ( timFreq / 20000U ) - 1U;
    hTim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    hTim.Init.Period            = 0xFFFF;
    hTim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init( &hTim );
    HAL_TIM_Base_Start( &hTim );
  }

  /* Return current counter value */
  return (uint16_t) __HAL_TIM_GET_COUNTER( &hTim );
}

/**                     C A L L B A C K   R O U T I N E S                              */
/** -------------------------------------------------------------------------------------
 * \brief     UART MSP initialization callback.
 */
static void TbxMb_HAL_UART_MspInit( UART_HandleTypeDef *ph ) {
  //
  if ( ph->Instance == USART2 ) {
    __HAL_RCC_USART2_CLK_ENABLE( );     // USART2 clock enable
    __HAL_RCC_GPIOA_CLK_ENABLE( );      // GPIOA clock enable
    // PA2 -> USART2_TX, PA3 -> USART2_RX
    HAL_GPIO_Init( GPIOA, &( GPIO_InitTypeDef ){
                              .Pin   = USART_TX_Pin | USART_RX_Pin,
                              .Mode  = GPIO_MODE_AF_PP,
                              .Speed = GPIO_SPEED_FREQ_LOW,
                          } );
    HAL_NVIC_SetPriority( USART2_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( USART2_IRQn );     // USART2 interrupt Init
  }
  return;
}

/** -------------------------------------------------------------------------------------
 * \brief     UART de-initialization callback.
 */
static void TbxMb_HAL_UART_MspDeInit( UART_HandleTypeDef *ph ) {
  //
  if ( ph->Instance == USART2 ) {
    __HAL_RCC_USART2_CLK_DISABLE( );     // Peripheral clock disable
    HAL_GPIO_DeInit( GPIOA, USART_TX_Pin | USART_RX_Pin );
    HAL_NVIC_DisableIRQ( USART2_IRQn );     // USART2 interrupt Deinit
  }
}

/** -------------------------------------------------------------------------------------
 * \brief     UART transmit complete callback.
 * \param     ph  Pointer to the channel's handle.
 */
static void TbxMb_HAL_UART_TxCpltCallback( UART_HandleTypeDef *ph ) {
  /**
   *  Loop over all available ports to find the right one.
   *    Get pointer to this port structure.
   *    Check if this is the port we are looking for.
   *      Switch the hardware from transmission to reception mode.
   *      Inform the Modbus UART module about the transmission completed event.
   *      Stop the loop, now that the port was located.
   */
  static uint8_t _PortsQntt = ( sizeof( asTbxMbPorts ) / sizeof( asTbxMbPorts[ 0 ] ) );
  for ( uint8_t _PortId = 0; _PortId < _PortsQntt; _PortId++ ) {
    psTbxMbPort_t _psPort = &asTbxMbPorts[ _PortId ];
    if ( asTbxMbPorts[ _PortId ].phUart == ph ) {
      TbxMbPortUartDriverEnable( _PortId, TBX_OFF );
      TbxMbUartTransmitComplete( _PortId );
      break;
    }
  }
  return;
}

/** -------------------------------------------------------------------------------------
 * \brief     UART reception complete callback.
 * \param     ph Pointer to the channel's handle.
 */
static void TbxMb_HAL_UART_RxCpltCallback( UART_HandleTypeDef *ph ) {
  /** 
   *  Loop over all available ports to find the right one.
   *    Get pointer to this port structure.
   *    Check if this is the port we are looking for.
   *      Obtain the error code to check if any error occurred during reception.
   *      Only process the byte if no noise, framing or parity error was detected.
   *        Inform the Modbus UART module about the newly received data byte.
   *      Restart reception for the next byte.
   *      Stop the loop, now that the port was located.
   */
  static uint8_t _PortsQntt = ( sizeof( asTbxMbPorts ) / sizeof( asTbxMbPorts[ 0 ] ) );
  for ( uint8_t _PortId = 0; _PortId < _PortsQntt; _PortId++ ) {
    psTbxMbPort_t _psPort = &asTbxMbPorts[ _PortId ];
    if ( _psPort->phUart == ph ) {
      uint32_t errorCode = HAL_UART_GetError( ph );
      if ( ( errorCode & ( HAL_UART_ERROR_NE | HAL_UART_ERROR_PE | HAL_UART_ERROR_FE ) ) == 0 ) {
        TbxMbUartDataReceived( _PortId, &_psPort->RxByte, 1U );
      }
      HAL_UART_Receive_IT( ph, &_psPort->RxByte, 1 );
      break;
    }
  }
  return;
}

/** -------------------------------------------------------------------------------------
 * \brief     UART error callback.
 * \param     ph Pointer to the channel's handle.
*/
static void TbxMb_HAL_UART_ErrorCallback( UART_HandleTypeDef *ph ) {
  /**
   *  Loop over all available ports to find the right one.
   *    Get pointer to this port structure.
   *    Check if this is the port we are looking for.
   *      Clear the overrun error flag if it was set.
   *      Restart reception for the next byte.
   *      Stop the loop, now that the port was located.
   */
  static uint8_t _PortsQntt = ( sizeof( asTbxMbPorts ) / sizeof( asTbxMbPorts[ 0 ] ) );
  for ( uint8_t _PortId = 0; _PortId < _PortsQntt; _PortId++ ) {
    psTbxMbPort_t _psPort = &asTbxMbPorts[ _PortId ];
    if ( _psPort->phUart == ph ) {
      uint32_t _ErrCode = HAL_UART_GetError( ph );
      if ( _ErrCode & HAL_UART_ERROR_ORE ) __HAL_UART_CLEAR_OREFLAG( ph );
      HAL_UART_Receive_IT( ph, &_psPort->RxByte, 1 );
      break;
    }
  }
  return;
}

/**             I N T E R R U P T   S E R V I C E   R O U T I N E S                    */
/** -------------------------------------------------------------------------------------
 * \brief     USARTx interrupt service routine.
 */
__weak void USART2_IRQHandler( void ) {
  /* Pass event on to the HAL driver for further handling. */
  HAL_UART_IRQHandler( &huart2 );
  return;
}

/*********************************** end of tbxmb_port.c *******************************/
