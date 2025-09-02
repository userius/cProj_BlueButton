#ifndef __KPB_OSD_H__
#define __KPB_OSD_H__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus

#include "main.h"

#define KPB_TICK_PERIOD 25U     // 25 mSec

#define KPB_R_PULLUP    223U      // 22 kOhm
#define KPB_R_DOWN      471U      // 470 Ohm
#define KPB_R_RIGHT     682U      // 6.8 kOhm
#define KPB_R_UP        153U      // 15 kOhm
#define KPB_R_LEFT      273U      // 27 kOhm
#define KPB_R_ENTER     453U      // 45 kOhm
#define KPB_R_TOLERANCE 10.0f     // +/- 10%

#define KPB_ADC_RESOLUTION 12U
#define KPB_ADC_FULL       ( ( 1U << KPB_ADC_RESOLUTION ) - 1U )
#define KPB_ADC_1D16       ( KPB_ADC_FULL >> 4 )
#define KPB_ADC_1D32       ( KPB_ADC_FULL >> 5 )

#define KPB_DEBOUNCE_MASK 0x03U

#define KPB_FLAG_TICK      0x01
#define KPB_FLAG_CONV_CPLT 0x02

#define KPB_KEY_FLAG_STATE    0x01
#define KPB_KEY_FLAG_CB_FIRST 0x02

#define KPB_REPEATE_NUM_SLOW  3      //
#define KPB_REPEATE_SKIP_SLOW 16     // 400 ms delay
#define KPB_REPEATE_SKIP_FAST 8      // 200 ms delay

#define KPB_HOLD_TIMEOUT_EVENT_1 3      // seconds
#define KPB_HOLD_TIMEOUT_EVENT_2 6      // seconds
#define KPB_HOLD_TIMEOUT_EVENT_3 10     // seconds

#define KPB_TIME_SLOW ( KPB_TICK_PERIOD * KPB_REPEATE_NUM_SLOW * KPB_REPEATE_SKIP_SLOW )

#define KPB_CALC_RPTCNT_VAL( sec )                                                                 \
  ( ( ( sec ) * 1000 < KPB_TIME_SLOW ) ?                                                           \
        ( ( sec ) * 1000 / ( KPB_TICK_PERIOD * KPB_REPEATE_SKIP_SLOW ) ) :                         \
        ( ( ( sec ) * 1000 - KPB_TIME_SLOW ) / ( KPB_TICK_PERIOD * KPB_REPEATE_SKIP_FAST ) +       \
          KPB_REPEATE_NUM_SLOW ) )

  typedef enum _eKPB_KeyID {
    KPB_KEY_NOISE_DETECTED = -2,     //
    KPB_KEY_NONE           = -1,     //
    KPB_KEY_DOWN_ID        = 0,      //
    KPB_KEY_RIGHT_ID,                //
    KPB_KEY_UP_ID,                   //
    KPB_KEY_LEFT_ID,                 //
    KPB_KEY_ENTER_ID,                //
    KPB_KEYS_NUM                     // number of keys
  } eKPB_Key_t;

  typedef enum _eKPB_Evt {
    KPB_EVT_NONE = 0,
    KPB_EVT_PRESS,
    KPB_EVT_RELEASE,
    KPB_EVT_REPEATE,
    KPB_EVT_HELD_TIME_1,
    KPB_EVT_HELD_TIME_2,
    KPB_EVT_HELD_TIME_3,
  } eKPB_Evt_t;

  typedef struct _sKPB_Limits {
    uint32_t min;
    uint32_t max;
  } sLimits_t, *psLimits_t;

  // typedef struct _sKPB_Data {} sKPB_Data_t, *psKPB_Data_t;

  typedef struct _sKPB_Key {
    sLimits_t sLim;
    uint8_t   Debounce;
    uint8_t   Flags;     // 1- pressed
    uint8_t   SkipCnt;
    uint8_t   RepeateCnt;
  } sKPB_Key, *psKPB_Key;

  typedef struct _hKPB {
    ADC_HandleTypeDef *phADC;
    uint16_t           Flags;
    uint16_t           RawData;
    eKPB_Key_t         KeyRecognized;
    sKPB_Key           asKeys[ KPB_KEYS_NUM ];
    // debounce filter;
  } hKPB_t, *phKPB_t;

  void MX_KPB_Init( void );
  void KPB_Tick( void *ptr );
  void KPB_Serve( phKPB_t ph );
  void KPB_KeyEventCallback( phKPB_t ph, eKPB_Key_t eKey, eKPB_Evt_t eEvt );

  extern phKPB_t phKPB;

#ifdef __cplusplus
}
#endif     // __cplusplus
#endif     // __KPB_OSD_H__
