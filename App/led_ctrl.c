#include "led_ctrl.h"
#include <string.h>

#define is_mask( n )   ( ( n ) != 0 && ( ( ( n ) + 1 ) & ( n ) ) == 0 )
#define is_onebit( n ) ( ( n ) != 0 && ( ( n ) & ( ( n ) - 1 ) ) == 0 )

static void LC_WritePin( psLC_Pin_t psPin, bool PinState );

hLC_t  hLedGreen;
phLC_t phLedGreen = NULL;     // == &hLedGreen after LC_Init(...)

/** -------------------------------------------------------------------------
 * @brief   Custom implementation to initialize all used LEDs and structures
 */
void MX_LC_Init( void ) {
  //
  if ( LC_ERR_NONE ==     //
       LC_Init( &hLedGreen, &( sLC_Pin_t ){
                                .psPort    = LD2_GPIO_Port,         //
                                .Pin       = LD2_Pin,               //
                                .IsInverse = LC_PIN_ACTIVE_HIGH     //
                            } ) ) {
    phLedGreen = &hLedGreen;
  }
  return;
}

/** -------------------------------------------------------------------------
 * @brief   Init LED controller structure and init GPIO pin.
 */
eLC_Err_t LC_Init( phLC_t ph, psLC_Pin_t psPin ) {
  //
  if ( !ph || !psPin ||               //
       !is_onebit( psPin->Pin ) )     //
    return LC_ERR_ARGS;

  memset( ph, 0, sizeof( hLC_t ) );
  memcpy( &ph->sPin, psPin, sizeof( sLC_Pin_t ) );
  ph->sPatBG.DurationMask  = LC_DURATION_32;
  ph->sPatEvt.DurationMask = LC_DURATION_32;

  switch ( (uint32_t) psPin->psPort ) {     // clang-format off
    case (uint32_t) GPIOA: { __HAL_RCC_GPIOA_CLK_ENABLE( ); break; }
    case (uint32_t) GPIOB: { __HAL_RCC_GPIOB_CLK_ENABLE( ); break; }
    case (uint32_t) GPIOC: { __HAL_RCC_GPIOC_CLK_ENABLE( ); break; }
    case (uint32_t) GPIOD: { __HAL_RCC_GPIOD_CLK_ENABLE( ); break; }
    case (uint32_t) GPIOE: { __HAL_RCC_GPIOE_CLK_ENABLE( ); break; }
    default: return LC_ERR_ARGS;
  }     // clang-format on

  LC_WritePin( psPin, false );     // set the pin to inactive state (LED off)

  HAL_GPIO_Init( psPin->psPort,
                 &( GPIO_InitTypeDef ){
                     .Pin   = psPin->Pin,                                 //
                     .Mode  = psPin->IsInverse == LC_PIN_ACTIVE_LOW ?     //
                                 GPIO_MODE_OUTPUT_OD :                   //
                                 GPIO_MODE_OUTPUT_PP,                    //
                     .Pull  = GPIO_NOPULL,                                //
                     .Speed = GPIO_SPEED_FREQ_LOW                         //
                 } );
  return LC_ERR_NONE;
}

/** -------------------------------------------------------------------------
 * @brief   Serve LED controller structure and write GPIO pin.
 * @note    Call this function periodically (every 40 ms).
 */
eLC_Err_t LC_Serve( phLC_t ph ) {
  //
  if ( !ph ) return LC_ERR_ARGS;

  psLC_Pat_t psPat = ph->EvtRepeateCnt ? &ph->sPatEvt : &ph->sPatBG;
  if ( ph->BitCnt >= 32 ||     //
       0 == ( ( 1U << ph->BitCnt ) & psPat->DurationMask ) ) {
    ph->BitCnt = 0;
    if ( ph->EvtRepeateCnt ) --ph->EvtRepeateCnt;
    psPat = ph->EvtRepeateCnt ? &ph->sPatEvt : &ph->sPatBG;
  }
  LC_WritePin( &ph->sPin,     //
               ( psPat->Pattern & ( 1U << ph->BitCnt ) ) ? true : false );
  ++ph->BitCnt;

  return LC_ERR_NONE;
}

/** -------------------------------------------------------------------------
 * @brief   Set pattern for LED.
 * @param   Repeates  0 will set a pattern for the background behavior,
 *                    otherwise - for event.
 * @param   psPat     Use @defgroup LC_SPD_define
 */
eLC_Err_t LC_Set( phLC_t ph, uint8_t Repeates, psLC_Pat_t psPat ) {
  //
  if ( !ph || !psPat ||                    //
       !is_mask( psPat->DurationMask )     // || ( psPat->Pattern > psPat->DurationMask ) //
       )                                   //
    return LC_ERR_ARGS;

  psLC_Pat_t psPattern = Repeates ? &ph->sPatEvt : &ph->sPatBG;
  memcpy( psPattern, psPat, sizeof( sLC_Pat_t ) );
  if ( Repeates ) {
    ph->EvtRepeateCnt = Repeates;
    ph->BitCnt        = 0;
  }
  if ( !ph->EvtRepeateCnt ) ph->BitCnt = 0;

  return LC_ERR_NONE;
}

/** -------------------------------------------------------------------------
 * @brief   Turning the LED on/off.
 * @param   PinState: 0 will turn the LED off,
 *                    1 will turn the LED on.
 */
static void LC_WritePin( psLC_Pin_t psPin, bool PinState ) {
  //
  HAL_GPIO_WritePin( psPin->psPort, psPin->Pin,                             //
                     psPin->IsInverse ?                                     //
                         ( PinState ? GPIO_PIN_RESET : GPIO_PIN_SET ) :     //
                         ( PinState ? GPIO_PIN_SET : GPIO_PIN_RESET ) );
  return;
}
