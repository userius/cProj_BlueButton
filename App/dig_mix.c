/***************************************************************************
 * @file  dig_mix.c
 * @note  Digital signal mixer module.
 * ************************************************************************* */

#include "dig_mix.h"
#include "dig_in.h"

__STATIC_FORCEINLINE bool     _mix_channel_calc( phMIX_t ph, uint8_t ChId );
__STATIC_FORCEINLINE void     _mix_inputs_update( phMIX_t ph );
__STATIC_FORCEINLINE void     _mix_outputs_update( phMIX_t ph );
__STATIC_FORCEINLINE uint16_t _mix_apply_masks( psMOS_t psOuts, psMIX_MosMasks_t psMasks );

hMIX_t  hMIX;
phMIX_t phMIX;

sMIX_ChCfg_t asChannelCfg[ MIX_QNTT ];

sMIX_Cfg_t sCfgMIX = {
    .asChCfgs = asChannelCfg,
};
uint32_t aChannelsInput[ MIX_QNTT ];

/** ---------------------------------------------------------------------------
 * @brief   Initialize the digital mixer module.
 */
void MIX_Init( void ) {
  //
  for ( size_t i = 0; i < sizeof( asChannelCfg ); i++ ) ( (uint8_t *) asChannelCfg )[ i ] = 0;
  for ( size_t i = 0; i < MIX_QNTT; i++ ) {
    asChannelCfg[ i ].eLogicOperation = MIX_LO_AND;
    asChannelCfg[ i ].sMasksDIM.State = 1U << i;
    asChannelCfg[ i ].MaskUsage       = 1U << i;
    aChannelsInput[ i ]               = 0;
  }

  hMIX.psCfg          = &sCfgMIX;
  hMIX.aChannelsInput = aChannelsInput;
  hMIX.psOutsDIM      = &phDIM->sOutsDIM;
  // Clear output signals
  uint16_t *_ptr = (uint16_t *) &hMIX.sOutsMIX;
  for ( uint8_t i; i < 4; i++ ) _ptr[ i ] = 0;
  hMIX.ChQntt = MIX_QNTT;
  phMIX       = &hMIX;

  return;
}

/** ---------------------------------------------------------------------------
 * @brief Update the digital mixer outputs and edge states.
 *
 * This function processes the new input states, recalculates each output
 * channel based on its configured logical operation (@ref _mix_chanel_calc),
 * and updates the edge detection fields.
 *
 * @param[in,out] ph         Pointer to the mixer handle structure (@ref hMIX_t).
 *
 * The function performs:
 * - Updates @ref hMIX_t::Inputs
 * - Calls @ref _mix_chanel_calc for each channel (0..15) to determine the new output bit
 * - Stores the calculated outputs in a temporary variable
 * - Calculates edge transitions:
 *   - @ref hMIX_t::EdgeRise : bits changing from 0 to 1
 *   - @ref hMIX_t::EdgeFall : bits changing from 1 to 0
 *   - @ref hMIX_t::EdgeAny  : bits changing in either direction
 * - Updates @ref hMIX_t::Outputs with the new state
 *
 * @note Edge detection is performed by comparing the previous and new outputs.
 */
void MIX_Update( phMIX_t ph ) {
  //
  _mix_inputs_update( ph );
  _mix_outputs_update( ph );

  return;
}

/** ---------------------------------------------------------------------------
 * @brief   Apply configured masks to module output signals.
 * @param   psOuts  Pointer to the module output signals structure (psMOS_t).
 * @param   psMasks Pointer to the mixer MOS masks structure (psMIX_MosMasks_t).
 * @return  Masked output signals as a 16-bit value.
 */
__STATIC_FORCEINLINE uint16_t _mix_apply_masks( psMOS_t psOuts, psMIX_MosMasks_t psMasks ) {
  //
  return ( ( psOuts->States ^ psMasks->StXOR ) & psMasks->State ) |     //
         ( psOuts->EdgesRise & psMasks->Rise ) |                        //
         ( psOuts->EdgesFall & psMasks->Fall );                         //
}

/** ---------------------------------------------------------------------------
 * @brief   Update the combined input signals for the mixer module.
 *
 * This function reads the output signals from both the Digital Input Module (DIM)
 * and the Mixer Module (MIX) itself, applies the configured masks, and combines
 * them into a single 32-bit input field stored in @ref hMIX_t::Inputs.
 *
 * The lower 16 bits of @ref hMIX_t::Inputs contain the masked signals from DIM,
 * while the upper 16 bits contain the masked signals from MIX.
 *
 * @param[in,out] ph   Pointer to the mixer handle structure (@ref hMIX_t).
 *
 * The function performs:
 * - Reads DIM outputs from @ref hMIX_t::psOutsDIM and applies @ref sMIX_Cfg_t::sMasksDIM
 * - Reads MIX outputs from @ref hMIX_t::sOutsMIX and applies @ref sMIX_Cfg_t::sMasksMIX
 * - Combines the results into @ref hMIX_t::Inputs
 *
 * This combined input field is then used by each mixer channel to evaluate its
 * logical operation based on its configuration.
 */
__STATIC_FORCEINLINE void _mix_inputs_update( phMIX_t ph ) {
  //
  psMIX_MosMasks_t psMasks;
  psMOS_t          psOuts;
  uint16_t         _InDIM, _InMIX;

  /* Loop through all configured mixer channels */
  for ( size_t _Ch = 0; _Ch < ph->ChQntt; _Ch++ ) {
    // get inputs from DIM
    psMasks = &ph->psCfg->asChCfgs[ _Ch ].sMasksDIM;
    psOuts  = ph->psOutsDIM;
    _InDIM  = _mix_apply_masks( psOuts, psMasks );

    // get inputs from MIX
    psMasks = &ph->psCfg->asChCfgs[ _Ch ].sMasksMIX;
    psOuts  = &ph->sOutsMIX;
    _InMIX  = _mix_apply_masks( psOuts, psMasks );

    // combine inputs
    ph->aChannelsInput[ _Ch ] = ( (uint32_t) _InMIX << 16 ) | _InDIM;
  }

  return;
}

/** ---------------------------------------------------------------------------
 * @brief   Update the mixer outputs and edge states.
 * @param   ph      Pointer to the digital mixer module handler (phMIX_t).
 */
__STATIC_FORCEINLINE void _mix_outputs_update( phMIX_t ph ) {
  //
  uint16_t _NewOut = 0;
  for ( uint8_t _Ch = 0; _Ch < ph->ChQntt; _Ch++ ) {
    if ( _mix_channel_calc( ph, _Ch ) )     //
      SET_BIT( _NewOut, 1U << _Ch );
  }

  ph->sOutsMIX.EdgesRise = ( ~ph->sOutsMIX.States ) & _NewOut;     // 0 -> 1
  ph->sOutsMIX.EdgesFall = ph->sOutsMIX.States & ( ~_NewOut );     // 1 -> 0
  ph->sOutsMIX.EdgesAny  = _NewOut ^ ph->sOutsMIX.States;          // 0 -> 1 or 1 -> 0
  ph->sOutsMIX.States    = _NewOut;

  return;
}

/** ---------------------------------------------------------------------------
 * @brief   Calculates the output of a single digital mixer channel.
 * @param   ph      Pointer to the digital mixer module handler (phMIX_t).
 * @param   ChId    Channel index (0..15).
 * @return  Result of the logical operation for this channel.
 */
__STATIC_FORCEINLINE bool _mix_channel_calc( phMIX_t ph, uint8_t ChId ) {
  psMIX_ChCfg_t _ps  = &ph->psCfg->asChCfgs[ ChId ];
  bool          _Out = false;
  uint32_t      _In  = ph->aChannelsInput[ ChId ];

  // _In ^= _ps->MaskXOR;     // Apply XOR mask (invert selected input bits)
  switch ( _ps->eLogicOperation ) {
    default: break;
    case MIX_LO_AND: _Out = ( ( _In & _ps->MaskUsage ) == _ps->MaskUsage ); break;
    case MIX_LO_NAND: _Out = !( ( _In & _ps->MaskUsage ) == _ps->MaskUsage ); break;
    case MIX_LO_OR: _Out = ( ( _In & _ps->MaskUsage ) != 0 ); break;
    case MIX_LO_NOR: _Out = !( ( _In & _ps->MaskUsage ) != 0 ); break;
    case MIX_LO_XOR:
    case MIX_LO_XNOR:
    case MIX_LO_PAR: {                                      // PAR = XOR (odd number of bits)
      _In &= _ps->MaskUsage;     // clang-format off
      while ( _In ) { _Out = !_Out; _In &= ( _In - 1 ); }          // clear lowest set bit
      if ( _ps->eLogicOperation == MIX_LO_XNOR ) _Out = !_Out;     // clang-format on
    } break;
  }

  return _Out;
}
