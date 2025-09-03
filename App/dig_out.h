#ifndef __DIG_OUT_H__
#define __DIG_OUT_H__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus

#include "main.h"
#include <stdbool.h>
#include "dig_com.h"

#define DO_QNTT ( 4U )     // Number of digital outputs, max 16

/**
 * @defgroup Timer restart behavior
 */
#define DOM_TIM_MODE_IGNORE  false     // Ignore new trigger if timer is running
#define DOM_TIM_MODE_RESTART true      // Restart timer on new trigger

/**
 * @defgroup Signals Source ID
 */
#define DOM_SRC_PROT ( 0U )     // Protocol only
#define DOM_SRC_DI   ( 1U )     // Digital input moudle | Protocol
#define DOM_SRC_MIX  ( 2U )     // Digital mixer moudle | protocol
#define DOM_SRC_NONE ( 3U )     // Source disabled

/**
 * @defgroup Signals Group ID in Signals Source
 */
#define DOM_SIG_GR_STATE     ( 0U )     //
#define DOM_SIG_GR_EDGE_RISE ( 1U )     //
#define DOM_SIG_GR_EDGE_FALL ( 2U )     //
#define DOM_SIG_GR_EDGE_ANY  ( 3U )     //

  typedef enum _dom_channel_input_signal_name {
    DOM_IN_SIG_DEACTIVATION = 0,
    DOM_IN_SIG_ACTIVATION
  } eDOM_InSig_t;

  /**
   * @brief Signal source for Digital Output Module
   */
  typedef union _dom_signal_source_id {
    uint8_t RegSrcID;
    struct {
      uint8_t ChanID : 4;       // bit (channel) index in signals group
      uint8_t GroupID : 2;      // @defgroup Signals Group ID
      uint8_t SourceID : 2;     // @defgroup Signals Source ID
    };
  } uDOM_SigID_t, *puDOM_SigID_t;

  /**
   * @brief Timer config for Digital Output Module
   */
  typedef union _dom_tim_cgf {
    uint16_t RegCgf;
    struct {
      uint16_t Ticks : 15;     // Time in ticks
      uint16_t Mode : 1;       // see @defgroup Timer restart behavior
    };
  } uDOM_TimCfg_t, *puDOM_TimCfg_t;

  /**
   * @brief Channel configuration for Digital Output Module
   */
  typedef struct _dom_chanel_cfg {
    uDOM_SigID_t  uAct;               // Activation signal source ID
    uDOM_SigID_t  uDeact;             // Deactivation signal source ID
    uDOM_TimCfg_t uCfgTDA;            // Time Delay Activation (TDA)
    uDOM_TimCfg_t uCfgTHO;            // Time Hold Output (THO)
  } sDOM_ChCfg_t, *psDOM_ChCfg_t;     // 6 bytes

  /**
   * @brief Digital Output Module configuration structure
   */
  typedef struct _dom_configuration {
    sDOM_ChCfg_t asChCfg[ DO_QNTT ];     // 96 bytes
    uint16_t     OutsMaskXOR;            //
  } sDOM_Cfg_t, *psDOM_Cfg_t;            // 98 bytes

  typedef struct _dom_timer_state {     //
    uint16_t Counter;                   // countdown in ticks (saturates at 0)
  } sDOM_TimSt_t, *psDOM_TimSt_t;

  /**
   * @brief Runtime state for a single channel
   */
  typedef struct _channel_state {
    sDOM_TimSt_t sTDA;     ///< TDA countdown
    sDOM_TimSt_t sTHO;     ///< THO countdown
  } sDOM_ChSt_t, *psDOM_ChSt_t;

  /**
   * @brief Digital Output Module runtime handle
   *
   * @details
   * Stores runtime state and references for the entire DOM instance.
   * This handle is allocated once, initialized with configuration,
   * and updated cyclically by ::DOM_Update().
   *
   * Members:
   * - `psCfg`     → pointer to static configuration (::sDOM_Cfg_t).
   * - `aChState[]`→ per-channel runtime state (::sDOM_ChSt_t).
   * - `psOutsDIM` → pointer to Digital Input Module signals.
   * - `psOutsMIX` → pointer to Mixer Module signals.
   * - `OutStates` → current output states (bitfield, one bit per channel).
   * - `QnttOuts`  → total number of configured outputs (max 16).
   */
  typedef struct {
    psDOM_Cfg_t psCfg;                   ///< Pointer to configuration structure
    sDOM_ChSt_t aChState[ DO_QNTT ];     ///< Array of per-channel state
    psPin_t     asPinDO;                 ///< Array of output pin configurations
    psMOS_t     psOutsDIM;               ///< Digital Input Module outputs
    psMOS_t     psOutsMIX;               ///< Mixer Module outputs
    uint16_t    OutStates;               ///< Current output states (bitfield)
    uint8_t     QnttOuts;                ///< Total number of digital outputs (max 16)
  } hDOM_t, *phDOM_t;

  void DOM_Init( void );
  void DOM_Update( hDOM_t *ph );

  extern hDOM_t *phDOM;

#ifdef __cplusplus
}
#endif     // __cplusplus
#endif     // __DIG_OUT_H__
