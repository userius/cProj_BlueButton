#ifndef __LED_CTRL_H__
#define __LED_CTRL_H__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus

#include "main.h"
#include <stdbool.h>

/** @defgroup LC_PinInversion_define
 */
#define LC_PIN_ACTIVE_HIGH false
#define LC_PIN_ACTIVE_LOW  true

/** @defgroup LC_Pattern_define
 */
#define LC_PATTERN_OFF   ( (uint32_t) 0x00000000U )
#define LC_PATTERN_ON    ( (uint32_t) 0xFFFFFFFFU )
#define LC_PATTERN_M1000 ( (uint32_t) 0x00001FFEU )
#define LC_PATTERN_M240  ( (uint32_t) 0x0000000EU )
#define LC_PATTERN_M80  ( (uint32_t) 0x00000002U )
#define LC_PATTERN_2S1L  ( (uint32_t) 0x0007E082U )
#define LC_PATTERN_1S1L  ( (uint32_t) 0x00001F82U )
#define LC_PATTERN_1L1S  ( (uint32_t) 0x0000107EU )
#define LC_PATTERN_1L2S  ( (uint32_t) 0x0004107EU )
#define LC_PATTERN_3S    ( (uint32_t) 0x00002082U )
#define LC_PATTERN_2S    ( (uint32_t) 0x00000082U )
#define LC_PATTERN_1S    ( (uint32_t) 0x00000002U )

/** @defgroup LC_Duration_define
 */
#define LC_DURATION_32 ( (uint32_t) 0xFFFFFFFFU )
#define LC_DURATION_31 ( (uint32_t) 0x7FFFFFFFU )
#define LC_DURATION_30 ( (uint32_t) 0x3FFFFFFFU )
#define LC_DURATION_29 ( (uint32_t) 0x1FFFFFFFU )
#define LC_DURATION_28 ( (uint32_t) 0x0FFFFFFFU )
#define LC_DURATION_25 ( (uint32_t) 0x01FFFFFFU )
#define LC_DURATION_24 ( (uint32_t) 0x00FFFFFFU )
#define LC_DURATION_23 ( (uint32_t) 0x007FFFFFU )
#define LC_DURATION_22 ( (uint32_t) 0x003FFFFFU )
#define LC_DURATION_21 ( (uint32_t) 0x001FFFFFU )
#define LC_DURATION_20 ( (uint32_t) 0x000FFFFFU )
#define LC_DURATION_16 ( (uint32_t) 0x0000FFFFU )
#define LC_DURATION_15 ( (uint32_t) 0x00007FFFU )
#define LC_DURATION_14 ( (uint32_t) 0x00003FFFU )
#define LC_DURATION_08 ( (uint32_t) 0x000000FFU )
#define LC_DURATION_06 ( (uint32_t) 0x0000003FU )
#define LC_DURATION_04 ( (uint32_t) 0x0000000FU )
#define LC_DURATION_02 ( (uint32_t) 0x00000003U )

/** @defgroup LC_SPD_define Stucture with pattern and duration
 */
#define LC_SPD_ON                                                                                  \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_ON, .DurationMask = LC_DURATION_32 }
#define LC_SPD_OFF                                                                                 \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_OFF, .DurationMask = LC_DURATION_32 }
#define LC_SPD_M80                                                                                \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_M80, .DurationMask = LC_DURATION_02 }
#define LC_SPD_M240                                                                                \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_M240, .DurationMask = LC_DURATION_06 }
#define LC_SPD_M1000                                                                               \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_M1000, .DurationMask = LC_DURATION_25 }
#define LC_SPD_2S1L                                                                                \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_2S1L, .DurationMask = LC_DURATION_31 }
#define LC_SPD_1S1L                                                                                \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_1S1L, .DurationMask = LC_DURATION_25 }
#define LC_SPD_1L1S                                                                                \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_1L1S, .DurationMask = LC_DURATION_25 }
#define LC_SPD_1L2S                                                                                \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_1L2S, .DurationMask = LC_DURATION_30 }
#define LC_SPD_1S                                                                                  \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_1S, .DurationMask = LC_DURATION_14 }
#define LC_SPD_2S                                                                                  \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_2S, .DurationMask = LC_DURATION_21 }
#define LC_SPD_3S                                                                                  \
  ( sLC_Pat_t ) { .Pattern = LC_PATTERN_3S, .DurationMask = LC_DURATION_25 }

  typedef enum _eLC_Errors {     //
    LC_ERR_NONE = 0,
    LC_ERR_ARGS
  } eLC_Err_t;

  typedef struct _sLC_Pin {          //
    GPIO_TypeDef *psPort;            // GPIOA, GPIOB ...
    uint16_t      Pin;               // @defgroup GPIO_pins_define
    uint16_t      IsInverse : 1;     // @defgroup LC_PinInversion_define
    uint16_t      Unused : 15;       // @defgroup LC_Pattern_define
  } sLC_Pin_t, *psLC_Pin_t;

  typedef struct _sLC_Pattern {     //
    uint32_t Pattern;               // @defgroup LC_Pattern_define
    uint32_t DurationMask;          // @defgroup LC_Duration_define
  } sLC_Pat_t, *psLC_Pat_t;

  typedef struct _hLC {          //
    sLC_Pin_t sPin;              // Port, pin and direction
    sLC_Pat_t sPatBG;            // Background pattern
    sLC_Pat_t sPatEvt;           // Event pattern
    uint8_t   EvtRepeateCnt;     //
    uint8_t   BitCnt;            //
  } hLC_t, *phLC_t;

  void      MX_LC_Init( void );
  eLC_Err_t LC_Init( phLC_t ph, psLC_Pin_t psPin );
  eLC_Err_t LC_Serve( phLC_t ph );
  eLC_Err_t LC_Set( phLC_t ph, uint8_t Repeates, psLC_Pat_t psPat );

  /**
   * @brief   Blinking background pattern
   * @param   psPat Use @defgroup LC_SPD_define
   */
  __STATIC_FORCEINLINE eLC_Err_t LC_Background( phLC_t ph, psLC_Pat_t psPat ) {     //
    return LC_Set( ph, 0, psPat );
  }

  /**
   * @brief   Blinking event pattern
   * @param   psPat Use @defgroup LC_SPD_define
   */
  __STATIC_FORCEINLINE eLC_Err_t LC_Event( phLC_t ph, uint8_t Repeates, psLC_Pat_t psPat ) {     //
    return LC_Set( ph, Repeates, psPat );
  }

  extern phLC_t phLedGreen;

#ifdef __cplusplus
}
#endif     // __cplusplus
#endif     // __LED_CTRL_H__
