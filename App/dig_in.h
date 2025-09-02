#ifndef __DIG_IN_H__
#define __DIG_IN_H__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus

#include "main.h"
#include <stdbool.h>
#include "dig_com.h"

#define DI_QNTT            4       // Number of digital inputs, max 16
#define DI_THRESHOLD_TRUE  160     // 160 ~ 2/3 of 256
#define DI_THRESHOLD_FALSE 96      // 96 ~ 1/3 of 256

  typedef struct _dig_input {              // Digital input structure
    union {                                //
      uint8_t Reg8;                        // All flags as a byte
      struct {                             //
        uint8_t RawState : 1;              // Last raw digital input state
        uint8_t StableState : 1;           // Stable state after debounce & hysteresis
        uint8_t IsDebounceOngoing : 1;     // Indicates an active transition process
        uint8_t Unused : 5;                // Reserved bits
      };
    } uFlags;
    uint8_t FilterOut;              // Debouncing filter output value
    uint8_t RawStateChangesCnt;     // Raw input changes during debounce
    uint8_t DebounceDuration;       // Duration of the ongoing debounce in update cycles
  } sDI_Sig_t, *psDI_Sig_t;

  typedef struct _dig_in_module_config {     // Configuration structure for DIM
    uint8_t  aTau[ DI_QNTT ];
    uint16_t MaskForLED;
  } sDIM_Cfg_t, *psDIM_Cfg_t;

  /**
   * @brief   Digital Input Module Handler
   * This structure stores the configuration, runtime state, and
   * processed results for a digital input module.
   * It is used by functions such as ::DIM_Serve().
   */
  typedef struct _dig_in_module_handler {
    psDIM_Cfg_t psCfg;         // Pointer to configuration structure
    psDI_Sig_t  asSig;         // array[QnttDIs]
    psPin_t     asPin;         // array[QnttDIs]
    uint16_t    RawStates;     // Read the pins and update sDI_Sig_t after
    sMOS_t      sOutsDIM;      // Module Output Signals structure
    uint8_t     QnttDIs;       // Total number of digital inputs, max 16
  } hDIM_t, *phDIM_t;

  extern phDIM_t phDIM;

  void DIM_Init( void );
  void DIM_Update( void *pArgs );

#ifdef __cplusplus
}
#endif     // __cplusplus
#endif     // __DIG_IN_H__
