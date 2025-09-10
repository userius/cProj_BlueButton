/******************************************************************************
 * @brief   Digital input module source file.
 * @file    dig_in.c
 * @author  userius@gmail.com
 * @date    2025-08-15
 * @version 1.0.0
 *****************************************************************************/

#include "dig_in.h"

__STATIC_FORCEINLINE uint8_t     //
            _debounce_via_filter( bool Raw, uint8_t Prev, uint8_t Tau );
static bool _signal_update( psDI_Sig_t ps, bool RawNew, uint8_t Tau );
static void _init_all_di_pins( phDIM_t ph );
static void _set_pin_to_input( psPin_t ps );
static void _set_pin_to_output( psPin_t ps, bool PinState );
static void _set_pins_cfg( void );
static void _set_cfg( void );

static sDIM_Cfg_t sCfg;
static sDI_Sig_t  asSigs[ DI_QNTT ];
static sPin_t     asPins[ DI_QNTT ];

hDIM_t hDIM = {
    .psCfg     = &sCfg,
    .asSig     = asSigs,
    .asPin     = asPins,
    .RawStates = 0,
    .sOutsDIM  = { .States = 0, .EdgesRise = 0, .EdgesFall = 0, .EdgesAny = 0 },
    .QnttDIs   = DI_QNTT,
};
phDIM_t phDIM = &hDIM;

/** --------------------------------------------------------------------------
 * @brief   Init handle structure and input pins.
 */
void DIM_Init( void ) {
  //
  _set_cfg( );
  _set_pins_cfg( );
  _init_all_di_pins( phDIM );

  return;
}

/** --------------------------------------------------------------------------
 * @brief   Perform a full update cycle for the digital input module.
 *
 * This function executes the complete service routine for digital inputs:
 *   1. Configures all associated pins as inputs before reading.
 *   2. Reads the current raw logic levels from all input pins into @ref hDIM_t::RawStates.
 *   3. Applies debounce filtering to produce stable states, stored in @ref hDIM_t::StableStates.
 *   4. Detects signal transitions and updates:
 *        - @ref hDIM_t::EdgeRise : bits with a 0 → 1 transition
 *        - @ref hDIM_t::EdgeFall : bits with a 1 → 0 transition
 *        - @ref hDIM_t::EdgeAny  : bits that changed in any direction
 *   5. Updates output pins (e.g., LEDs) according to the stable states and
 *      the mask @ref sDIM_Params_t::MaskForLED.
 *
 * @param   pArgs   Pointer to the digital input module handler structure
 *                  (of type @ref hDIM_t).
 *
 * @note
 * - Debounce timing for each input is taken from @ref sDIM_Params_t::aTau.
 * - The number of inputs processed is defined by @ref hDIM_t::QnttDIs.
 */
void DIM_Update( void *pArgs ) {
  //
  phDIM_t ph = (phDIM_t) pArgs;

  if ( !ph ) return;

  // --- Step 1: Configure all pins as inputs before reading ---
  for ( uint8_t id = 0; id < ph->QnttDIs; id++ )     //
    _set_pin_to_input( &ph->asPin[ id ] );

  // --- Step 2: Read raw digital input states ---
  uint16_t _NewRaw = 0;
  for ( uint8_t id = 0; id < ph->QnttDIs; id++ ) {
    psPin_t psPin = &ph->asPin[ id ];
    if ( psPin->psPort ) {
      if ( LL_GPIO_IsInputPinSet( psPin->psPort, psPin->Pin ) )     //
        SET_BIT( _NewRaw, 1U << id );
    }
  }

  // --- Step 3: Apply debounce and calculate new stable states ---
  uint16_t _NewStable = 0;
  for ( uint8_t id = 0; id < ph->QnttDIs; id++ ) {
    uint16_t _Mask = 1U << id;
    if ( _signal_update( &ph->asSig[ id ], READ_BIT( _NewRaw, _Mask ) != 0,     //
                         ph->psCfg->aTau[ id ] ) )                              //
      SET_BIT( _NewStable, _Mask );
  }

  // Step 4: Detect edges and save new states
  ph->sOutsDIM.EdgesRise = ( ~ph->sOutsDIM.States ) & _NewStable;     // 0 -> 1
  ph->sOutsDIM.EdgesFall = ph->sOutsDIM.States & ( ~_NewStable );     // 1 -> 0
  ph->sOutsDIM.EdgesAny  = _NewStable ^ ph->sOutsDIM.States;          // 0 -> 1 or 1 -> 0
  ph->sOutsDIM.States    = _NewStable;
  ph->RawStates          = _NewRaw;

  // --- Step 5: Update outputs (LEDs or other indicators) ---
  // Output = StableStates XOR MaskForLED
  for ( uint8_t id = 0; id < ph->QnttDIs; id++ ) {
    bool _OutState = ( ( _NewStable ^ ph->psCfg->MaskForLED ) & ( 1U << id ) ) != 0;
    _set_pin_to_output( &ph->asPin[ id ], _OutState );
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   Set configuration parameters in the config structure.
 */
static void _set_cfg( void ) {
  //
  for ( size_t i = 0; i < DI_QNTT; i++ ) {
    sCfg.aTau[ i ] = 50;     // 50 update cycles ~ 500 ms @ 100 Hz
  }
  sCfg.MaskForLED = 0x000F;     // Invert first 4 inputs on output
  return;
}

/** --------------------------------------------------------------------------
 * @brief   Set pin configurations in the pin array.
 */
static void _set_pins_cfg( void ) {
  //
  for ( size_t i = 0; i < DI_QNTT; i++ ) {
    psPin_t _ps = &asPins[ i ];
    switch ( i ) {     // clang-format off
      case 0:  _ps->psPort = GPIOB; _ps->Pin = LL_GPIO_PIN_5;   break;     // PB5
      case 1:  _ps->psPort = GPIOB; _ps->Pin = LL_GPIO_PIN_4;   break;     // PB4
      case 2:  _ps->psPort = GPIOB; _ps->Pin = LL_GPIO_PIN_10;  break;     // PB10
      case 3:  _ps->psPort = GPIOA; _ps->Pin = LL_GPIO_PIN_8;   break;     // PA8
      default: _ps->psPort = NULL;  _ps->Pin = LL_GPIO_PIN_ALL; break;
    }     // clang-format on
  }
  return;
}

/** --------------------------------------------------------------------------
 * @brief   Initialize all digital input pins as inputs with pulldown.
 * @param   ph  Pointer to the digital input module handler structure
 *               (of type @ref hDIM_t).
 */
static void _init_all_di_pins( phDIM_t ph ) {
  //
  for ( uint8_t id = 0; id < ph->QnttDIs; id++ ) {
    psPin_t psPin = &ph->asPin[ id ];
    switch ( (uint32_t) psPin->psPort ) {
      case ( (uint32_t) GPIOA ): LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOA ); break;
      case ( (uint32_t) GPIOB ): LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOB ); break;
      case ( (uint32_t) GPIOC ): LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOC ); break;
      case ( (uint32_t) GPIOD ): LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOD ); break;
      default: break;
    }
    _set_pin_to_input( psPin );
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   Change pin mode to input with pulldown.
 * @param   ps  Pointer to the pin structure (of type @ref sPin_t).
 */
static void _set_pin_to_input( psPin_t ps ) {
  //
  if ( ps->psPort ) {
    LL_GPIO_SetPinPull( ps->psPort, ps->Pin, LL_GPIO_PULL_DOWN );
    LL_GPIO_SetPinMode( ps->psPort, ps->Pin, LL_GPIO_MODE_INPUT );
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   Change pin mode to output with push-pull.
 * @param   ps        Pointer to the pin structure (of type @ref sPin_t).
 * @param   PinState  true/false == set/reset
 */
static void _set_pin_to_output( psPin_t ps, bool PinState ) {
  /**
   * Set initial output state before switching to output mode
   * Configure as output, push-pull, low speed
   */
  if ( ps->psPort ) {
    PinState ? LL_GPIO_SetOutputPin( ps->psPort, ps->Pin ) :
               LL_GPIO_ResetOutputPin( ps->psPort, ps->Pin );
    LL_GPIO_SetPinSpeed( ps->psPort, ps->Pin, LL_GPIO_SPEED_FREQ_LOW );
    LL_GPIO_SetPinOutputType( ps->psPort, ps->Pin, LL_GPIO_OUTPUT_PUSHPULL );
    LL_GPIO_SetPinMode( ps->psPort, ps->Pin, LL_GPIO_MODE_OUTPUT );
  }

  return;
}

/** --------------------------------------------------------------------------
 * @brief   Update signal state with debounce and hysteresis.
 *
 * This function updates the state of a digital input signal by applying
 * debounce filtering and hysteresis. It tracks the raw input state, stable
 * state, and manages a debounce process to ensure reliable state transitions.
 *
 * @param   ps      Pointer to the digital input signal structure.
 * @param   RawNew  The new raw input state (true for high, false for low).
 * @param   Tau     The debounce time constant (number of consecutive samples
 *                  required to change state).
 *
 * @return  The updated stable state of the signal (true for high, false for low).
 */
static bool _signal_update( psDI_Sig_t ps, bool RawNew, uint8_t Tau ) {
  //
  // Apply debounce filter (low-pass)
  ps->FilterOut = _debounce_via_filter( RawNew, ps->FilterOut, Tau );

  // Determine new stable state with hysteresis
  bool _PrevStable = ps->uFlags.StableState;
  bool _NewStable  = ( !_PrevStable && ps->FilterOut >= DI_THRESHOLD_TRUE ) ? true :
                     ( _PrevStable && ps->FilterOut <= DI_THRESHOLD_FALSE ) ? false :
                                                                              _PrevStable;

  // Start of transient process
  if ( RawNew != _PrevStable && !ps->uFlags.IsDebounceOngoing ) {
    ps->uFlags.IsDebounceOngoing = 1;     // Mark debounce process active
    ps->DebounceDuration         = 0;     // Reset debounce time counter
    ps->RawStateChangesCnt       = 0;     // Reset raw state change counter
  }

  // Count duration and raw state changes during debounce process
  if ( ps->uFlags.IsDebounceOngoing ) {
    if ( ps->DebounceDuration < UINT8_MAX )     // Increment duration counter
      ps->DebounceDuration++;
    if ( RawNew != ps->uFlags.RawState ) {
      if ( ps->RawStateChangesCnt < UINT8_MAX )     // Count raw signal changes
        ps->RawStateChangesCnt++;
    }
  }

  // End of transient process when stable state changes
  if ( _NewStable != _PrevStable ) {
    ps->uFlags.IsDebounceOngoing = 0;     // Mark debounce process finished
  }

  ps->uFlags.RawState    = RawNew;         // Save current raw state
  ps->uFlags.StableState = _NewStable;     // Save current stable state

  return _NewStable;
}

/** --------------------------------------------------------------------------
 * @brief   Simple debounce filter implementation with exponential smoothing.
 * @param   Raw   The current raw input signal (true = high, false = low).
 * @param   Prev  The previous filtered value [0..255].
 * @param   Tau   The debounce time constant (1..100 typical).
 *
 * Simple approximation of e^(-1/tau) using an inverse proportion.
 * Tuned for the range tau = 1..100, error up to 20%
 *
 * @return  The updated filtered value [0..255].
 */
__STATIC_FORCEINLINE uint8_t _debounce_via_filter( bool Raw, uint8_t Prev, uint8_t Tau ) {
//
#define DEBOUNCE_SCALE_SHIFT 22     // Max 23
#define DEBOUNCE_SCALE       ( 1 << DEBOUNCE_SCALE_SHIFT )
#define DEBOUNCE_SCALE_DIV2  ( DEBOUNCE_SCALE >> 1 )
  /**
   * Exponential moving average update: result = prev + alpha * (target - prev)
   * All calculations are performed in scaled fixed-point domain.
   */
  int32_t _Alpha = DEBOUNCE_SCALE / ( (int32_t) Tau + 1 );
  if ( _Alpha == 0 ) _Alpha = 1;
  int32_t _Delta = _Alpha * ( (int32_t) ( Raw ? UINT8_MAX : 0 ) - Prev );     // fits in 32-bit
  if ( _Delta / DEBOUNCE_SCALE == 0 ) {
    _Delta = ( _Delta >= 0 ) ? DEBOUNCE_SCALE : -DEBOUNCE_SCALE;     // ensure at least 1 step
  }
  int32_t _Res = ( ( (int32_t) Prev + INT8_MIN ) * DEBOUNCE_SCALE + _Delta ) / DEBOUNCE_SCALE;
  /* Clamp result into [0..255] range after undoing INT8_MIN shift */
  _Res = ( _Res < INT8_MIN ) ? 0 : ( _Res > INT8_MAX ) ? UINT8_MAX : _Res - INT8_MIN;

  return (uint8_t) _Res;
}
