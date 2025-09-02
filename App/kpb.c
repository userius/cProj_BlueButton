/***************************************************************************
 * @file  kpb.c
 * @note  OSD Key-Press Board, such as RunCam or Foxeer KEY23.
 * ************************************************************************* */

#include "kpb.h"
#include <math.h>       // for round
#include <string.h>     //
// #include "EventRecorder.h"

hKPB_t  hKPB;
phKPB_t phKPB = NULL;

extern ADC_HandleTypeDef hadc1;

static void   _ADC_Init( phKPB_t phkpb );
static void   _ADC_MspInit( ADC_HandleTypeDef *ph );
static void   _ADC_ConvCpltCallback( ADC_HandleTypeDef *ph );
static float  _DecodeResistorCode( uint16_t code );
static int8_t _CalcDividerLimits( psLimits_t psResult,                    //
                                  uint16_t R1_code, uint16_t R2_code,     //
                                  float Tolerance, uint8_t Resolution );
static void   _KeyRecognition( phKPB_t ph );
static void   _KeyProcess( phKPB_t ph );

/** --------------------------------------------------------------------------
 * @brief   Initializing resources for the key-press board
 */
void MX_KPB_Init( void ) {
  //
  uint16_t aResCodes[] = { KPB_R_DOWN, KPB_R_RIGHT, KPB_R_UP,     //
                           KPB_R_LEFT, KPB_R_ENTER };

  _ADC_Init( &hKPB );

  hKPB.Flags         = 0;
  hKPB.RawData       = KPB_ADC_FULL;
  hKPB.KeyRecognized = KPB_KEY_NONE;

  for ( size_t i = 0; i < KPB_KEYS_NUM; i++ ) {
    //
    psKPB_Key psKey = &hKPB.asKeys[ i ];
    memset( psKey, 0, sizeof( sKPB_Key ) );
    if ( -1 == _CalcDividerLimits( &psKey->sLim, KPB_R_PULLUP, aResCodes[ i ],     //
                                   KPB_R_TOLERANCE, KPB_ADC_RESOLUTION ) )
      Error_Handler( );
  }

  phKPB = &hKPB;

  return;
}

/** --------------------------------------------------------------------------
 * @brief   >|<
 */
void KPB_Tick( void *ptr ) {
  //
  if ( ptr ) {
    phKPB_t phkpb = (phKPB_t) ptr;
    SET_BIT( phkpb->Flags, KPB_FLAG_TICK );
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   >|<
 */
void KPB_Serve( phKPB_t phKPB ) {
  //
  if ( phKPB ) {
    if ( READ_BIT( phKPB->Flags, KPB_FLAG_TICK ) ) {        //
      if ( HAL_OK == HAL_ADC_Start_IT( phKPB->phADC ) )     //
        CLEAR_BIT( phKPB->Flags, KPB_FLAG_TICK );           //
    }
    if ( READ_BIT( phKPB->Flags, KPB_FLAG_CONV_CPLT ) ) {     //
      HAL_ADC_Stop_IT( phKPB->phADC );                        //
      CLEAR_BIT( phKPB->Flags, KPB_FLAG_CONV_CPLT );          //
                                                              // EventStartA( 0 );
      _KeyProcess( phKPB );                                   // Tavg = 17.4 us
                                                              // EventStopA( 0 );
    }
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   >|<
 */
__WEAK void KPB_KeyEventCallback( phKPB_t ph, eKPB_Key_t eKey, eKPB_Evt_t eEvt ) {
  //
  return;
}

/** --------------------------------------------------------------------------
 * @brief   >|<
 */
static void _KeyProcess( phKPB_t ph ) {
  //
  _KeyRecognition( phKPB );

  for ( size_t id = 0; id < KPB_KEYS_NUM; id++ ) {
    //
    psKPB_Key psKey = &ph->asKeys[ id ];
    psKey->Debounce <<= 1;
    SET_BIT( psKey->Debounce, ( id == ph->KeyRecognized ) ? 1 : 0 );
    uint32_t maskedval = psKey->Debounce & KPB_DEBOUNCE_MASK;
    if ( maskedval == KPB_DEBOUNCE_MASK ) {                      // The key is pressed.
      if ( !READ_BIT( psKey->Flags, KPB_KEY_FLAG_STATE ) ) {     // The key was pushed.
        SET_BIT( psKey->Flags, KPB_KEY_FLAG_STATE );
        psKey->SkipCnt    = 0;
        psKey->RepeateCnt = 0;
        KPB_KeyEventCallback( ph, id, KPB_EVT_PRESS );
      }
      else {                                                      // The key is held down.
        if ( ++psKey->SkipCnt >                                   //
             ( ( psKey->RepeateCnt < KPB_REPEATE_NUM_SLOW ) ?     //
                   KPB_REPEATE_SKIP_SLOW :
                   KPB_REPEATE_SKIP_FAST ) ) {
          //
          KPB_KeyEventCallback( ph, id, KPB_EVT_REPEATE );
          if ( KPB_HOLD_TIMEOUT_EVENT_1 ) {
            if ( psKey->RepeateCnt == KPB_CALC_RPTCNT_VAL( KPB_HOLD_TIMEOUT_EVENT_1 ) )
              KPB_KeyEventCallback( ph, id, KPB_EVT_HELD_TIME_1 );
          }
          if ( KPB_HOLD_TIMEOUT_EVENT_2 ) {
            if ( psKey->RepeateCnt == KPB_CALC_RPTCNT_VAL( KPB_HOLD_TIMEOUT_EVENT_2 ) )
              KPB_KeyEventCallback( ph, id, KPB_EVT_HELD_TIME_2 );
          }
          if ( KPB_HOLD_TIMEOUT_EVENT_3 ) {
            if ( psKey->RepeateCnt == KPB_CALC_RPTCNT_VAL( KPB_HOLD_TIMEOUT_EVENT_3 ) )
              KPB_KeyEventCallback( ph, id, KPB_EVT_HELD_TIME_3 );
          }
          psKey->SkipCnt = 0;
          if ( psKey->RepeateCnt < 255 ) psKey->RepeateCnt++;
        }
      }
    }
    else if ( maskedval == 0 ) {                                // The key is not pressed.
      if ( READ_BIT( psKey->Flags, KPB_KEY_FLAG_STATE ) ) {     // The key was released.
        CLEAR_BIT( psKey->Flags, KPB_KEY_FLAG_STATE );          //
        KPB_KeyEventCallback( ph, id, KPB_EVT_RELEASE );
      }
    }
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   >|<
 */
static void _KeyRecognition( phKPB_t ph ) {
  //
  uint16_t val = ph->RawData;

  ph->KeyRecognized = KPB_KEY_NOISE_DETECTED;
  if ( val > ( KPB_ADC_FULL - KPB_ADC_1D32 ) ) { ph->KeyRecognized = KPB_KEY_NONE; }
  else {
    for ( size_t id = 0; id < KPB_KEYS_NUM; id++ ) {
      psLimits_t psLim = &ph->asKeys[ id ].sLim;
      if ( val >= psLim->min && val < psLim->max )     //
        ph->KeyRecognized = (eKPB_Key_t) id;
    }
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   >|<
 static void _ADC_Start( ADC_HandleTypeDef *ph ) {
  //
  HAL_ADC_Start( ph );                                      // Tavg = 28.0us
  HAL_ADC_PollForConversion( ph, 3 );                       // Tavg = 23.6us
  if ( phKPB ) phKPB->RawData = HAL_ADC_GetValue( ph );     // Tavg = 10.2us
  HAL_ADC_Stop( ph );                                       // Tavg = 12.7us

  return;
}
*/

/** --------------------------------------------------------------------------
 * @brief   >|<
 */
static void _ADC_Init( phKPB_t phkpb ) {
  //
  ADC_HandleTypeDef *phadc = &hadc1;
  HAL_ADC_RegisterCallback( phadc, HAL_ADC_MSPINIT_CB_ID, _ADC_MspInit );

  do {     // Common config
    phadc->Instance                   = ADC1;
    phadc->Init.ScanConvMode          = ADC_SCAN_DISABLE;
    phadc->Init.ContinuousConvMode    = DISABLE;
    phadc->Init.DiscontinuousConvMode = DISABLE;
    phadc->Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    phadc->Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    phadc->Init.NbrOfConversion       = 1;
  } while ( 0 );
  if ( HAL_OK != HAL_ADC_Init( phadc ) )     //
    Error_Handler( );

  // Configure Regular Channel
  ADC_ChannelConfTypeDef sCfg = {
      .Channel      = ADC_CHANNEL_4,
      .Rank         = ADC_REGULAR_RANK_1,
      .SamplingTime = ADC_SAMPLETIME_239CYCLES_5,
  };
  if ( HAL_OK != HAL_ADC_ConfigChannel( phadc, &sCfg ) )     //
    Error_Handler( );

  HAL_ADC_RegisterCallback( phadc, HAL_ADC_CONVERSION_COMPLETE_CB_ID, _ADC_ConvCpltCallback );

  HAL_ADCEx_Calibration_Start( phadc );

  phkpb->phADC = phadc;

  return;
}

/** --------------------------------------------------------------------------
 * @brief   >|<
 */
static void _ADC_MspInit( ADC_HandleTypeDef *ph ) {
  //
  if ( ph->Instance == ADC1 ) {
    // ADC1 GPIO Configuration: PA4 --> ADC1_IN4
    __HAL_RCC_GPIOA_CLK_ENABLE( );
    HAL_GPIO_Init( GPIOA, &( GPIO_InitTypeDef ){
                              .Pin  = GPIO_PIN_4,
                              .Mode = GPIO_MODE_ANALOG,
                          } );

    __HAL_RCC_ADC1_CLK_ENABLE( );     // ADC1 clock enable
    // ADC1 interrupt Init
    HAL_NVIC_SetPriority( ADC1_2_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( ADC1_2_IRQn );
  }
  return;
}

/** --------------------------------------------------------------------------
 * @brief   >|<
 */
static void _ADC_ConvCpltCallback( ADC_HandleTypeDef *ph ) {
  //
  if ( ph->Instance == phKPB->phADC->Instance ) {
    phKPB->RawData = HAL_ADC_GetValue( ph );     // Tavg = 10.2us
    SET_BIT( phKPB->Flags, KPB_FLAG_CONV_CPLT );
  }

  return;
}

/** -------------------------------------------------------------------------
 * @brief Decode 3- or 4-digit resistor code to resistance in Ohms.
 * @param code  Resistor code as uint16_t (e.g., 103, 472, 1002, 4703).
 * @return      Resistance in Ohms as float. Returns -1.0f on error.
 */
static float _DecodeResistorCode( uint16_t code ) {
  //
  if ( ( code < 100 && code != 0 ) || code > 9999 ) return -1.0f;

  uint16_t base       = code / 10;
  uint8_t  multiplier = code % 10;

  return (float) base * powf( 10.0f, multiplier );
}

/** -------------------------------------------------------------------------
 * @brief   Calculate the output voltage divider limits for the ADC.
 * @param   psResult     Pointer to result structure.
 * @param   R1_code      Top resistor value (3-digit resistor code).
 * @param   R2_code      Bottom resistor value (3-digit resistor code).
 * @param   Tolerance    Resistor tolerance in percent (e.g., 5.0 for ±5%).
 * @param   Resolution   ADC resolution in bits.
 * @return  0 on success, -1 on error.
 */
static int8_t _CalcDividerLimits( psLimits_t psResult, uint16_t R1_code, uint16_t R2_code,     //
                                  float Tolerance, uint8_t Resolution ) {
  //
  if ( psResult == NULL || Resolution < 6 || Resolution > 16 ||     //
       Tolerance < 0.0f || Tolerance >= 100.0f )                    // Invalid input
    return -1;

  float                                                //
      R1_nom = _DecodeResistorCode( R1_code ),         //
      R2_nom = _DecodeResistorCode( R2_code );         //
  if ( R1_nom < 0.0f || R2_nom < 0.0f ) return -1;     //

  float                                                        //
      TolFactor = Tolerance / 100.0f,                          //
      R1_min    = R1_nom * ( 1.0f - TolFactor ),               //
      R1_max    = R1_nom * ( 1.0f + TolFactor ),               //
      R2_min    = R2_nom * ( 1.0f - TolFactor ),               //
      R2_max    = R2_nom * ( 1.0f + TolFactor ),               //
      denom_max = R2_max + R1_min,                             //
      denom_min = R2_min + R1_max;                             //
  if ( denom_max == 0.0f || denom_min == 0.0f ) return -1;     //

  uint32_t                                                               //
      ADC_full = ( 1U << Resolution ) - 1,                               //
      ADC_1d16 = ADC_full >> 4,                                          //
      ADC_1d32 = ADC_full >> 5,                                          //
      Vmax     = (uint32_t) roundf( ADC_full * R2_max / denom_max ),     //
      Vmin     = (uint32_t) roundf( ADC_full * R2_min / denom_min );

  if ( Vmax < ( ADC_1d16 ) ) Vmax = ADC_1d16;
  if ( Vmin < ( ADC_1d16 ) ) Vmin = 0;
  if ( Vmax > ( ADC_full - ADC_1d16 ) ) Vmax = ADC_full - ADC_1d32;
  if ( Vmin > ( ADC_full - ADC_1d16 ) ) Vmin = ADC_full - ADC_1d16;

  psResult->max = Vmax;
  psResult->min = Vmin;

  return 0;
}
