#include "main.h"
#include "app_ticks.h"

hAT_t  hAppTicks;
phAT_t phAppTicks = NULL;

/**
 * @brief   Initializing variables required to use this module.
 */
phAT_t AppTick_Init( void ) {
  //
  phAppTicks = &hAppTicks;
  return phAppTicks;
}

/**
 * @brief   Add application tick period and callback.
 */
eATE_t AppTick_Add( phAT_t ph, uint16_t period, AT_CB_t CallBackFn, void *pArgs ) {
  //
  if ( !ph ) return AT_ERR_HANDLE;
  if ( !period ) return AT_ERR_PERIOD;
  if ( !CallBackFn ) return AT_ERR_CALLBACK;
  if ( ph->CntFlags >= APP_TICKS_MAX ) return AT_ERR_TICKSMAX;

  ph->aPeriods[ ph->CntFlags ]    = period;
  ph->aCallBackFn[ ph->CntFlags ] = CallBackFn;
  ph->apArgs[ ph->CntFlags ]      = pArgs;
  ++ph->CntFlags;

  return AT_ERR_NONE;
}

/**
 * @brief   Set the flags of app ticks.
 * @note    Place AppTick_Handle( ) in function that handles System tick timer.
 */
eATE_t AppTick_Handle( phAT_t ph ) {
  //
  if ( !ph ) return AT_ERR_HANDLE;

  uint32_t SysTickValue = HAL_GetTick( );
  for ( size_t i = 0; i < ph->CntFlags; i++ ) {
    if ( 0 == SysTickValue % ph->aPeriods[ i ] ) SET_BIT( ph->RegFlags, 1U << i );
  }

  return AT_ERR_NONE;
}

/**
 * @brief   Clear the flags of app ticks and call callbacks.
 * @note    Place AppTick_Serve( ) in main loop.
 */
eATE_t AppTick_Serve( phAT_t ph ) {
  //
  if ( !ph ) return AT_ERR_HANDLE;

  if ( ph->RegFlags ) {
    for ( size_t i = 0; i < ph->CntFlags; i++ ) {
      if ( READ_BIT( ph->RegFlags, 1U << i ) ) {
        CLEAR_BIT( ph->RegFlags, 1U << i );
        ph->aCallBackFn[ i ]( ph->apArgs[ i ] );
      }
    }
  }

  return AT_ERR_NONE;
}
