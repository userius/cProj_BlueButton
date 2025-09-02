/****************************************************************************************
 * @file  dom.h
 * @brief Digital Output Module (DOM)
 *
 * @details
 * The Digital Output Module (DOM) controls up to 16 GPIO outputs
 * of the STM32G070 (or compatible MCU) intended for relay driving.
 *
 * ## Features:
 * - Supports up to 16 digital outputs (bitfield in uint16_t).
 * - Activation and deactivation of each channel are controlled by
 *   configurable signal sources:
 *    - None (unused)
 *    - Digital Input Module
 *    - Mixer Module
 *    - Protocol
 * - Configurable input signal type:
 *    - EdgeRise (0 -> 1 transition)
 *    - EdgeFall (1 -> 0 transition)
 *    - EdgeAny  (0 <-> 1 transition)
 *    - State (current logical state)
 * - Activation delay timer (TDA) and Hold timer (THO) per channel.
 * - Configurable timer restart behavior:
 *    - Ignore retrigger
 *    - Restart counting from zero
 * - Special timer values:
 *    - TDA = 0 → output is activated immediately.
 *    - THO = 0 → output remains active until explicit deactivation signal.
 *
 * ## Processing flow:
 * - On activation signal → start TDA timer. When expired, output is set
 *   active and THO timer is started.
 * - On deactivation signal → immediate output deactivation and both timers are reset.
 * - If activation and deactivation are assigned to the same signal type,
 *   the sequence leading to state change is executed.
 ***************************************************************************************/

#include "dig_out.h"
#include "dig_in.h"
#include "dig_mix.h"

/** Static function prototypes ***********************************************/
static void          _dom_all_pins_init( phDOM_t ph );
static void          _dom_all_pins_update( phDOM_t ph );
__STATIC_INLINE void _dom_start_timer( uint16_t *counter, uint16_t value, bool mode );
__STATIC_INLINE bool _dom_timer_expired( uint16_t *counter );
__STATIC_INLINE bool _dom_get_signal( phDOM_t ph, uint8_t Ch, eDOM_InSig_t InSigType );
__STATIC_INLINE bool _dom_process_channel( hDOM_t *ph, uint8_t ChID,     //
                                           bool Activate, bool Deactivate );

/** Variables ***************************************************************/
/** ---------------------------------------------------------------------------
 * @brief   Pin configuration for digital outputs.
 *
 * This array defines the GPIO port and pin number for each digital output
 * channel managed by the Digital Output Module (DOM). The configuration
 * allows up to 16 outputs, but only the first DO_QNTT entries are used.
 *
 * Modify this array to match your specific hardware connections.
 */
static sPin_t asPins[ DO_QNTT ] = {
    { .psPort = GPIOA, .Pin = LL_GPIO_PIN_9 },     //
    { .psPort = GPIOC, .Pin = LL_GPIO_PIN_7 },     //
    { .psPort = GPIOB, .Pin = LL_GPIO_PIN_6 },     //
    { .psPort = GPIOA, .Pin = LL_GPIO_PIN_7 },     //
};

/** ---------------------------------------------------------------------------
 * @brief   Default configuration for the Digital Output Module (DOM).
 * no inversion, each channel activated on rising edge
 * and deactivated on falling edge of corresponding digital input (0..3).
 * TDA=0 (immediate), THO=50 cycles (~500ms if DOM_Update called every 10ms).
 */
sDOM_Cfg_t sCfg = {
    .OutsMaskXOR = 0x000F,     // clang-format off
    .asChCfg     = {    
        {   // Channel 0
            .uAct   = { .SourceID = DOM_SRC_DI, .GroupID = DOM_SIG_GR_EDGE_RISE, .ChanID = 0 },
            .uDeact = { .SourceID = DOM_SRC_DI, .GroupID = DOM_SIG_GR_EDGE_FALL, .ChanID = 0 },
            .uCfgTDA       = { .Ticks = 1,  .Mode = DOM_TIM_MODE_RESTART },
            .uCfgTHO       = { .Ticks = 50, .Mode = DOM_TIM_MODE_IGNORE  },
        },
        {   // Channel 1
            .uAct   = { .SourceID = DOM_SRC_DI, .GroupID = DOM_SIG_GR_EDGE_RISE, .ChanID = 1 },
            .uDeact = { .SourceID = DOM_SRC_DI, .GroupID = DOM_SIG_GR_EDGE_FALL, .ChanID = 1 },
            .uCfgTDA       = { .Ticks = 1,  .Mode = DOM_TIM_MODE_RESTART },
            .uCfgTHO       = { .Ticks = 50, .Mode = DOM_TIM_MODE_IGNORE  },
        },
        {   // Channel 2
            .uAct   = { .SourceID = DOM_SRC_DI, .GroupID = DOM_SIG_GR_EDGE_RISE, .ChanID = 2 },
            .uDeact = { .SourceID = DOM_SRC_DI, .GroupID = DOM_SIG_GR_EDGE_FALL, .ChanID = 2 },
            .uCfgTDA       = { .Ticks = 1,  .Mode = DOM_TIM_MODE_RESTART },
            .uCfgTHO       = { .Ticks = 50, .Mode = DOM_TIM_MODE_IGNORE  },
        },
        {   // Channel 3
            .uAct   = { .SourceID = DOM_SRC_DI, .GroupID = DOM_SIG_GR_EDGE_RISE, .ChanID = 3 },
            .uDeact = { .SourceID = DOM_SRC_DI, .GroupID = DOM_SIG_GR_EDGE_FALL, .ChanID = 3 },
            .uCfgTDA       = { .Ticks = 1,  .Mode = DOM_TIM_MODE_RESTART },
            .uCfgTHO       = { .Ticks = 50, .Mode = DOM_TIM_MODE_IGNORE  },
        }     
    },
};     // clang-format on

hDOM_t  hDOM;
hDOM_t *phDOM = NULL;

/** Public functions ***********************************************************/
/** ---------------------------------------------------------------------------
 * @brief Initialize the Digital Output Module (DOM).
 *
 * This function initializes all digital output pins defined in the DOM
 * handler structure. It configures each pin as a push-pull output and
 * sets their initial states to low (reset).
 *
 * @note This function should be called once during system initialization
 * before using the DOM functionalities.
 */
void DOM_Init( void ) {
  //
  hDOM.psCfg = &sCfg;
  for ( size_t i = 0; i < DO_QNTT; i++ ) hDOM.aChState[ i ] = ( sDOM_ChSt_t ){ 0, 0 };
  hDOM.asPinDO   = asPins;
  hDOM.psOutsDIM = &phDIM->sOutsDIM;
  hDOM.psOutsMIX = &phMIX->sOutsMIX;
  hDOM.OutStates = 0;
  hDOM.QnttOuts  = DO_QNTT;

  phDOM = &hDOM;
  _dom_all_pins_init( phDOM );
  _dom_all_pins_update( phDOM );

  return;
}

/** ---------------------------------------------------------------------------
 * @brief Periodic update of the Digital Output Module
 *
 * @param[in,out] ph   Pointer to DOM handle (::hDOM_t).
 *
 * @details
 * This function must be called periodically (cyclic task or timer tick).
 * It performs the following operations for each configured output channel:
 *  - Evaluates activation and deactivation signals.
 *  - Starts or processes the Activation Delay Timer (TDA).
 *  - If TDA expires, sets the output active and starts the Hold Timer (THO).
 *  - If THO expires, resets the output (unless THO=0 → latch until deactivation).
 *  - On deactivation signal, immediately resets output and clears timers.
 *
 * The resulting output states are stored in ::hDOM_t.OutStates
 * as a 16-bit bitfield, one bit per output channel.
 *
 * @note
 * Call frequency of this function defines the timer resolution ("ticks").
 */
void DOM_Update( hDOM_t *ph ) {
  /**
   *  Loop through all configured output channels
   *    Get input signals for the channel
   *    Process channel logic
   *  Update outputs
   *  Apply to GPIO pins
   */
  uint16_t _NewOuts = 0;

  for ( uint8_t _Ch = 0; _Ch < ph->QnttOuts; _Ch++ ) {
    bool _Act = _dom_get_signal( ph, _Ch, DOM_IN_SIG_ACTIVATION );
    bool _Dea = _dom_get_signal( ph, _Ch, DOM_IN_SIG_DEACTIVATION );
    if ( _dom_process_channel( ph, _Ch, _Act, _Dea ) )     //
      _NewOuts |= ( 1U << _Ch );
  }
  ph->OutStates = _NewOuts;
  _dom_all_pins_update( ph );

  return;
}

/** Static functions *********************************************************/
/** --------------------------------------------------------------------------
 * @brief   Initialize all digital output pins.
 * @param   ph  Pointer to the digital output module handler structure
 *               (of type @ref hDOM_t).
 */
static void _dom_all_pins_init( phDOM_t ph ) {
  /**
   * Enable GPIO port clock.
   * Set initial output state before switching to output mode.
   * Configure as output, push-pull, low speed.
   */
  for ( uint8_t id = 0; id < ph->QnttOuts; id++ ) {
    GPIO_TypeDef *_psPort = ph->asPinDO[ id ].psPort;
    uint32_t      _Pin    = ph->asPinDO[ id ].Pin;
    switch ( (uint32_t) _psPort ) {
      case ( (uint32_t) GPIOA ): LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOA ); break;
      case ( (uint32_t) GPIOB ): LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOB ); break;
      case ( (uint32_t) GPIOC ): LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOC ); break;
      case ( (uint32_t) GPIOD ): LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOD ); break;
      default: continue;
    }
    LL_GPIO_ResetOutputPin( _psPort, _Pin );
    LL_GPIO_SetPinSpeed( _psPort, _Pin, LL_GPIO_SPEED_FREQ_LOW );
    LL_GPIO_SetPinOutputType( _psPort, _Pin, LL_GPIO_OUTPUT_PUSHPULL );
    LL_GPIO_SetPinMode( _psPort, _Pin, LL_GPIO_MODE_OUTPUT );
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   Update all digital output pins to match current states.
 * @param   ph  Pointer to the digital output module handler structure
 *               (of type @ref hDOM_t).
 */
static void _dom_all_pins_update( phDOM_t ph ) {
  /**
   * Apply XOR mask and set/reset GPIO pins accordingly
   */
  uint16_t _outs = ph->OutStates ^ ph->psCfg->OutsMaskXOR;
  for ( uint8_t id = 0; id < ph->QnttOuts; id++ ) {
    GPIO_TypeDef *_psPort = ph->asPinDO[ id ].psPort;
    uint32_t      _Pin    = ph->asPinDO[ id ].Pin;
    bool          _State  = 0 != ( _outs & ( 1U << id ) );
    _State ? LL_GPIO_SetOutputPin( _psPort, _Pin ) :     //
             LL_GPIO_ResetOutputPin( _psPort, _Pin );
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   Retrieve the current state of a specified input signal.
 * @param   ph         Pointer to the DOM handler structure (hDOM_t).
 * @param   Ch         Channel index (0 to QnttOuts-1).
 * @param   InSigType  Type of input signal (activation or deactivation).
 *
 * @return  true if the specified signal is active, false otherwise.
 */
__STATIC_INLINE bool _dom_get_signal( phDOM_t ph, uint8_t Ch, eDOM_InSig_t InSigType ) {
  //
  puDOM_SigID_t puSig = DOM_IN_SIG_ACTIVATION == InSigType ? &ph->psCfg->asChCfg[ Ch ].uAct :
                                                             &ph->psCfg->asChCfg[ Ch ].uDeact;
  psMOS_t       psOut = DOM_SRC_DI == puSig->SourceID  ? ph->psOutsDIM :
                        DOM_SRC_MIX == puSig->SourceID ? ph->psOutsMIX :
                                                         NULL;
  if ( !psOut ) return false;

  uint16_t _Sigs = DOM_SIG_GR_EDGE_RISE == puSig->GroupID ? psOut->EdgesRise :
                   DOM_SIG_GR_EDGE_FALL == puSig->GroupID ? psOut->EdgesFall :
                   DOM_SIG_GR_EDGE_ANY == puSig->GroupID  ? psOut->EdgesAny :
                                                            psOut->States;

  return 0 != ( _Sigs & ( 1U << puSig->ChanID ) );
}

/** --------------------------------------------------------------------------
 * @brief Start (or optionally restart) a timer according to restart mode.
 * @param counter Pointer to the timer countdown register
 * @param ticks   Duration in ticks (0 => disabled/immediate).
 * @param mode    Timer restart mode (DOM_TIM_MODE_RESTART or DOM_TIM_MODE_IGNORE)
 */
__STATIC_INLINE void _dom_start_timer( uint16_t *counter, uint16_t ticks, bool mode ) {
  if ( ticks == 0 ) {     // timer disabled => no counting
    *counter = 0;         // Immediate action
    return;
  }
  if ( *counter == 0 || mode == DOM_TIM_MODE_RESTART ) { *counter = ticks; }
}

/** \brief Tick a countdown timer and report expiration.
 *  \param[in,out] counter  Pointer to countdown in ticks (saturates at 0).
 *  \return true  if it just reached 0 on this call,
 *          false otherwise.
 */
__STATIC_INLINE bool _dom_timer_expired( uint16_t *counter ) {
  if ( *counter == 0U ) return false;
  ( *counter )--;
  return ( *counter == 0U );
}

/** \brief Process one DOM channel state machine for this tick.
 *  \param[in,out] ph          DOM handle.
 *  \param[in]     ChID        Channel index [0..QnttOuts-1].
 *  \param[in]     Activate    Activation event (EdgeRise/Fall/Any per config).
 *  \param[in]     Deactivate  Deactivation event (EdgeRise/Fall/Any per config).
 *  \return        New output state for this channel (true = active).
 */
__STATIC_INLINE bool _dom_process_channel( hDOM_t *ph, uint8_t ChID, bool Activate,
                                           bool Deactivate ) {
  sDOM_ChSt_t   *psChSt   = &ph->aChState[ ChID ];
  uDOM_TimCfg_t *puCfgTDA = &ph->psCfg->asChCfg[ ChID ].uCfgTDA;
  uDOM_TimCfg_t *puCfgTHO = &ph->psCfg->asChCfg[ ChID ].uCfgTHO;

  bool isActive = ( ph->OutStates & ( 1U << ChID ) ) != 0U;

  /* Immediate deactivation path: cancels both timers and forces output low. */
  if ( Deactivate ) {
    if ( isActive ) { isActive = false; }
    psChSt->tda_counter = 0U;
    psChSt->tho_counter = 0U;
  }

  /* Activation handling. */
  if ( Activate ) {
    if ( !isActive ) {
      /* If no TDA configured => activate immediately and start THO right away. */
      if ( puCfgTDA->Ticks == 0U ) {
        isActive = true;
        _dom_start_timer( &psChSt->tho_counter, puCfgTHO->Ticks, puCfgTHO->Mode );
      }
      else {
        /* TDA is used => (re)start depending on restart mode. */
        _dom_start_timer( &psChSt->tda_counter, puCfgTDA->Ticks, puCfgTDA->Mode );
      }
    }
    else {
      /* Already active: allow THO retrigger per its restart mode (if THO is used). */
      if ( puCfgTHO->Ticks > 0U ) {
        _dom_start_timer( &psChSt->tho_counter, puCfgTHO->Ticks, puCfgTHO->Mode );
      }
    }
  }

  /* TDA countdown & arming THO upon expiry (only when currently inactive). */
  if ( !isActive && ( psChSt->tda_counter > 0U ) ) {
    if ( _dom_timer_expired( &psChSt->tda_counter ) ) {
      isActive = true;
      _dom_start_timer( &psChSt->tho_counter, puCfgTHO->Ticks, puCfgTHO->Mode );
    }
  }

  /* THO countdown & auto-deactivate upon expiry (only when THO is actually used). */
  if ( isActive && ( puCfgTHO->Ticks > 0U ) && ( psChSt->tho_counter > 0U ) ) {
    if ( _dom_timer_expired( &psChSt->tho_counter ) ) { isActive = false; }
  }

  return isActive;
}

// --- end of file ----------------------------------------------------------------
