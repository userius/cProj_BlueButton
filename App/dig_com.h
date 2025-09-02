#ifndef __DIG_COM_H__
#define __DIG_COM_H__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus

#include "main.h"

  typedef struct _pinout {
    GPIO_TypeDef *psPort;     // pointer to GPIO registers
    uint32_t      Pin;        // @defgroup GPIO_LL_EC_PIN PIN
  } sPin_t, *psPin_t;

  /**
   * @brief   Module Output Signals structure
   */
  typedef struct _module_out_sigs {
    uint16_t States;        // Result of updating all signals of module
    uint16_t EdgesRise;     // Bit transition from 0 to 1 in StableStates
    uint16_t EdgesFall;     // Bit transition from 1 to 0 in StableStates
    uint16_t EdgesAny;      // Bit transition in StableStates
  } sMOS_t, *psMOS_t;

#ifdef __cplusplus
}
#endif     // __cplusplus
#endif     // __DIG_COM_H__
