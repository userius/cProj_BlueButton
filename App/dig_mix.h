#ifndef __DIG_MIX_H__
#define __DIG_MIX_H__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus

#include "main.h"
#include <stdbool.h>
#include "dig_com.h"

#define MIX_QNTT ( 4U )     // Number of MIX cannels, max 16

  typedef enum _mix_logic_operations {
    MIX_LO_NONE = 0,
    MIX_LO_AND,
    MIX_LO_NAND,
    MIX_LO_OR,
    MIX_LO_NOR,
    MIX_LO_XOR,
    MIX_LO_XNOR,
    MIX_LO_PAR,
  } eMIX_LO_t;

  typedef struct _mix_mos_masks {
    uint16_t StXOR;     //
    uint16_t State;     //
    uint16_t Rise;      //
    uint16_t Fall;      //
  } sMIX_MosMasks_t, *psMIX_MosMasks_t;

  typedef struct _mix_channel_config {
    sMIX_MosMasks_t sMasksDIM;
    sMIX_MosMasks_t sMasksMIX;
    // uint32_t        MaskXOR;
    uint32_t        MaskUsage;
    eMIX_LO_t eLogicOperation;
  } sMIX_ChCfg_t, *psMIX_ChCfg_t;

  typedef struct _mix_config {
    psMIX_ChCfg_t asChCfgs;     // array[ MIX_QNTT ]
  } sMIX_Cfg_t, *psMIX_Cfg_t;

  /**
   * @brief   Digital Mixer Module Handle structure
   */
  typedef struct _dig_mix_module_handler {
    psMIX_Cfg_t psCfg;
    uint32_t   *aChannelsInput;     // array[ MIX_QNTT ]
    psMOS_t     psOutsDIM;
    sMOS_t      sOutsMIX;
    uint8_t     ChQntt;     // Total number of channels quantity, max 16
  } hMIX_t, *phMIX_t;

  void MIX_Init( void );
  void MIX_Update( phMIX_t ph );

  extern phMIX_t phMIX;

#ifdef __cplusplus
}
#endif     // __cplusplus
#endif     // __DIG_IN_H__
