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
static void          _dom_set_pins_cfg( void );
static void          _dom_set_cfg( void );
__STATIC_INLINE bool _dom_tim_expired( psDOM_TimSt_t ps );
__STATIC_INLINE void _dom_tim_start( psDOM_TimSt_t ps, puDOM_TimCfg_t puCfg );
__STATIC_INLINE void _dom_tim_reset( psDOM_TimSt_t ps );
__STATIC_INLINE bool _dom_tim_is_counting( psDOM_TimSt_t ps );
__STATIC_INLINE bool _dom_tim_is_configured( puDOM_TimCfg_t pu );
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
 * Fill it in _dom_set_pins_cfg() function.
 */
static sPin_t asPins[ DO_QNTT ];

/** ---------------------------------------------------------------------------
 * @brief   Default configuration for the Digital Output Module (DOM).
 * no inversion, each channel activated on rising edge
 * and deactivated on falling edge of corresponding digital input (0..3).
 * TDA=0 (immediate), THO=50 cycles (~500ms if DOM_Update called every 10ms).
 * Fill it in _dom_set_cfg() function.
 */
static sDOM_Cfg_t sCfg;

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
  _dom_set_cfg( );                           // set default configuration in sCfg
  hDOM.psCfg = &sCfg;                        // link configuration structure
  for ( size_t i = 0; i < DO_QNTT; i++ )     // clear channel states
    hDOM.aChState[ i ] = ( sDOM_ChSt_t ){ 0, 0 };
  _dom_set_pins_cfg( );                  // configure pin array asPins[]
  hDOM.asPinDO   = asPins;               // link pin array
  hDOM.psOutsDIM = &phDIM->sOutsDIM;     // link to DIM outputs
  hDOM.psOutsMIX = &phMIX->sOutsMIX;     // link to MIX outputs
  hDOM.OutStates = 0;                    // all outputs off
  hDOM.QnttOuts  = DO_QNTT;              // total number of outputs

  phDOM = &hDOM;                     // make global pointer
  _dom_all_pins_init( phDOM );       // init all pins as outputs, low state
  _dom_all_pins_update( phDOM );     // apply initial states to pins

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
   *  Clear protocol control signals after processing: Activate, Deactivate
   *  Apply protocol control signals: KeepActive, KeepInactive
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
  ph->sProtCtrl.Activate = ph->sProtCtrl.Deactivate = 0;
  SET_BIT( _NewOuts, ph->sProtCtrl.KeepActive );
  CLEAR_BIT( _NewOuts, ph->sProtCtrl.KeepInactive );
  ph->OutStates = _NewOuts;

  _dom_all_pins_update( ph );

  return;
}

/** Static functions *********************************************************/

/** ---------------------------------------------------------------------------
 * @brief  Set default DOM configuration in sCfg structure.
 */
static void _dom_set_cfg( void ) {
  //
  sCfg.OutsMaskXOR = 0x000F;     // Invert first 4 outputs
  for ( size_t i = 0; i < DO_QNTT; i++ ) {
    puDOM_SigID_t  _puA   = &sCfg.asChCfg[ i ].uAct;
    puDOM_SigID_t  _puD   = &sCfg.asChCfg[ i ].uDeact;
    puDOM_TimCfg_t _puTDA = &sCfg.asChCfg[ i ].uCfgTDA;
    puDOM_TimCfg_t _puTHO = &sCfg.asChCfg[ i ].uCfgTHO;
    switch ( i ) {
      case 0:
      case 1:
      case 2:
      case 3: {
        _puA->SourceID = DOM_SRC_DI;
        _puA->GroupID  = DOM_SIG_GR_EDGE_RISE;
        _puA->ChanID   = i;
        _puD->SourceID = DOM_SRC_DI;
        _puD->GroupID  = DOM_SIG_GR_EDGE_FALL;
        _puD->ChanID   = i;
        _puTDA->Ticks  = 1;     // 1 tick ~ 10ms if called @100Hz
        _puTDA->Mode   = DOM_TIM_MODE_RESTART;
        _puTHO->Ticks  = 150;     // 150 ticks ~ 1.5s if called @100Hz
        _puTHO->Mode   = DOM_TIM_MODE_RESTART;
        break;
      }
      default: {
        _puA->SourceID = DOM_SRC_NONE;
        _puA->GroupID  = DOM_SIG_GR_EDGE_ANY;
        _puA->ChanID   = i;
        _puD->SourceID = DOM_SRC_NONE;
        _puD->GroupID  = DOM_SIG_GR_EDGE_ANY;
        _puD->ChanID   = i;
        _puTDA->Ticks  = 10;     // 10 tick ~ 100ms if called @100Hz
        _puTDA->Mode   = DOM_TIM_MODE_RESTART;
        _puTHO->Ticks  = 100;     // 100 ticks ~ 1.0s if called @100Hz
        _puTHO->Mode   = DOM_TIM_MODE_RESTART;
        break;
      }
    }
  }
  return;
}

/** --------------------------------------------------------------------------
 * @brief   Set pin configurations in the pin array.
 */
static void _dom_set_pins_cfg( void ) {
  //
  for ( size_t i = 0; i < DI_QNTT; i++ ) {
    psPin_t _ps = &asPins[ i ];
    switch ( i ) {     // clang-format off
      case 0:  _ps->psPort = GPIOA; _ps->Pin = LL_GPIO_PIN_9;   break;     // PA9
      case 1:  _ps->psPort = GPIOC; _ps->Pin = LL_GPIO_PIN_7;   break;     // PC7
      case 2:  _ps->psPort = GPIOB; _ps->Pin = LL_GPIO_PIN_6;   break;     // PB6
      case 3:  _ps->psPort = GPIOA; _ps->Pin = LL_GPIO_PIN_7;   break;     // PA7
      default: _ps->psPort = NULL;  _ps->Pin = LL_GPIO_PIN_ALL; break;
    }     // clang-format on
  }
  return;
}

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
    if ( _psPort ) {
      uint32_t _Pin = ph->asPinDO[ id ].Pin;
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
    if ( _psPort ) {
      uint32_t _Pin   = ph->asPinDO[ id ].Pin;
      bool     _State = 0 != ( _outs & ( 1U << id ) );
      _State ? LL_GPIO_SetOutputPin( _psPort, _Pin ) :     //
               LL_GPIO_ResetOutputPin( _psPort, _Pin );
    }
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
__STATIC_INLINE bool _dom_get_signal( phDOM_t ph, uint8_t Ch, eDOM_InSig_t eInSigType ) {
  //
  puDOM_SigID_t _puSig = ( DOM_IN_SIG_ACTIVATION == eInSigType ) ?     //
                             &ph->psCfg->asChCfg[ Ch ].uAct :
                             &ph->psCfg->asChCfg[ Ch ].uDeact;
  psMOS_t       _psOut = ( DOM_SRC_DI == _puSig->SourceID )  ? ph->psOutsDIM :
                         ( DOM_SRC_MIX == _puSig->SourceID ) ? ph->psOutsMIX :
                                                               NULL;
  uint16_t      _Sigs  = 0;

  if ( _psOut )
    _Sigs = ( DOM_SIG_GR_EDGE_RISE == _puSig->GroupID ) ? _psOut->EdgesRise :
            ( DOM_SIG_GR_EDGE_FALL == _puSig->GroupID ) ? _psOut->EdgesFall :
            ( DOM_SIG_GR_EDGE_ANY == _puSig->GroupID )  ? _psOut->EdgesAny :
                                                          _psOut->States;
  //
  bool _Res = 0 != ( _Sigs & ( 1U << _puSig->ChanID ) );     //

  // Add protocol control signals if assigned to this channel.
  if ( DOM_SRC_NONE != _puSig->SourceID )                             //
    _Res |= 0U != READ_BIT( DOM_IN_SIG_ACTIVATION == eInSigType ?     //
                                ph->sProtCtrl.Activate :
                                ph->sProtCtrl.Deactivate,
                            1U << Ch );

  return _Res;
}

/** --------------------------------------------------------------------------
 * @brief Start (or optionally restart) a timer according to restart mode.
 * @param ps      Pointer to the timer state structure.
 * @param puCfg   Pointer to the timer configuration structure.
 */
__STATIC_INLINE void _dom_tim_start( psDOM_TimSt_t ps, puDOM_TimCfg_t puCfg ) {
  //
  if ( !_dom_tim_is_configured( puCfg ) ) {     // timer disabled => no counting
    _dom_tim_reset( ps );                       // Immediate action
    return;
  }
  if ( !_dom_tim_is_counting( ps ) ||            // timer is not running or
       puCfg->Mode == DOM_TIM_MODE_RESTART )     // restart mode
    ps->Counter = puCfg->Ticks;                  // set timer to configured value
  return;
}

/** --------------------------------------------------------------------------
 * @brief   Tick a countdown timer and report expiration.
 * @param   ps  Pointer to timer state structure.
 * @return  true if it just reached 0 on this call, false otherwise.
 */
__STATIC_INLINE bool _dom_tim_expired( psDOM_TimSt_t ps ) {
  //
  if ( ps->Counter == 0U ) return false;
  ps->Counter--;
  return ( ps->Counter == 0U );
}

/** --------------------------------------------------------------------------
 * @brief   Check if a timer is counting (Counter > 0).
 * @param   ps  Pointer to timer state structure.
 * @return  true if the timer is counting, false if it is stopped (Counter==0).
 */
__STATIC_INLINE bool _dom_tim_is_counting( psDOM_TimSt_t ps ) {
  //
  return ( ps->Counter != 0U );
}

/** --------------------------------------------------------------------------
 * @brief   Check if a timer is configured (Ticks > 0).
 * @param   pu  Pointer to timer configuration structure.
 */
__STATIC_INLINE bool _dom_tim_is_configured( puDOM_TimCfg_t pu ) {
  //
  return ( pu->Ticks != 0U );
}

/** --------------------------------------------------------------------------
 * @brief   Reset a countdown timer register.
 * @param   ps  Pointer to timer state structure.
 */
__STATIC_INLINE void _dom_tim_reset( psDOM_TimSt_t ps ) {
  //
  ps->Counter = 0U;
  return;
}

/** --------------------------------------------------------------------------
 *  @brief Process one DOM channel state machine for this tick.
 *  @param  ph          DOM handle.
 *  @param  ChID        Channel index [0..QnttOuts-1].
 *  @param  Activate    Activation event (EdgeRise/Fall/Any per config).
 *  @param  Deactivate  Deactivation event (EdgeRise/Fall/Any per config).
 *  @return New output state for this channel (true = active).
 */
__STATIC_INLINE bool _dom_process_channel( hDOM_t *ph, uint8_t ChID,     //
                                           bool Activate, bool Deactivate ) {
  // Local pointers to channel state and configuration for convenience.
  psDOM_TimSt_t  _psTDA    = &ph->aChState[ ChID ].sTDA;
  psDOM_TimSt_t  _psTHO    = &ph->aChState[ ChID ].sTHO;
  puDOM_TimCfg_t _puCfgTDA = &ph->psCfg->asChCfg[ ChID ].uCfgTDA;
  puDOM_TimCfg_t _puCfgTHO = &ph->psCfg->asChCfg[ ChID ].uCfgTHO;

  bool _IsActive = ( ph->OutStates & ( 1U << ChID ) ) != 0U;

  /* Immediate deactivation path: cancels both timers and forces output low. */
  if ( Deactivate ) {
    _IsActive = false;
    _dom_tim_reset( _psTDA );
    _dom_tim_reset( _psTHO );
  }

  /* Activation handling. */
  if ( Activate ) {
    if ( !_IsActive ) {
      /* If no TDA configured => activate immediately and start THO right away. */
      if ( !_dom_tim_is_configured( _puCfgTDA ) ) {
        _IsActive = true;
        _dom_tim_start( _psTHO, _puCfgTHO );
      }
      else {
        /* TDA is used => (re)start depending on restart mode. */
        _dom_tim_start( _psTDA, _puCfgTDA );
      }
    }
    else {
      /* Already active: allow THO retrigger per its restart mode (if THO is used). */
      if ( _dom_tim_is_configured( _puCfgTHO ) )     //
        _dom_tim_start( _psTHO, _puCfgTHO );
    }
  }

  /* TDA countdown & arming THO upon expiry (only when currently inactive). */
  if ( !_IsActive && _dom_tim_is_counting( _psTDA ) ) {
    if ( _dom_tim_expired( _psTDA ) ) {
      _IsActive = true;
      _dom_tim_start( _psTHO, _puCfgTHO );
    }
  }

  /* THO countdown & auto-deactivate upon expiry (only when THO is actually used). */
  if ( _IsActive &&                               //
       _dom_tim_is_configured( _puCfgTHO ) &&     //
       _dom_tim_is_counting( _psTHO ) )           //
    if ( _dom_tim_expired( _psTHO ) )             //
      _IsActive = false;

  return _IsActive;
}

// --- end of file ----------------------------------------------------------------
