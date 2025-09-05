#include "main.h"
#include "kpb.h"     // TODO: Callbacks
#include "swo_dbg.h"
#include <stdbool.h>
#include "app_ticks.h"
#include "led_ctrl.h"
#include "microtbx.h"
#include "microtbxmodbus.h"
#include "dig_in.h"
#include "dig_mix.h"
#include "dig_out.h"

// #include "EventRecorder.h"
// EventRecorderInitialize( EventID( EventLevelDetail, EvtStatistics_No, 0 ), 1 );

#define LED_ON           GPIO_PIN_SET
#define LED_OFF          GPIO_PIN_RESET
#define LED_Set( state ) HAL_GPIO_WritePin( LD2_GPIO_Port, LD2_Pin, ( state ) )
// Check if the blue button is pressed
#define B1_IsPushed( ) ( GPIO_PIN_RESET == HAL_GPIO_ReadPin( B1_GPIO_Port, B1_Pin ) ? true : false )

extern void MB_RTU_Slave_Init( void);

__STATIC_INLINE void LedTggl( void *pArgs );

static void Button_Serve( void *pArgs );
static void Led_Serve( void *pArgs );
static void DIDO_Serve( void *pArgs );
static void Button_EventCB( bool evt );

/** -------------------------------------------------------------------------
 * @brief   Application main loop.
 */
int app_main( void ) {
  //
  swo_msg( "\n\n%06u: cProj_BlueButton.\n", HAL_GetTick( ) );

  // EventRecorderInitialize( EventID( EventLevelDetail, EvtStatistics_No, 0 ), 1 );

  MX_LC_Init( );
  LC_Background( phLedGreen, &LC_SPD_M1000 );

  MX_KPB_Init( );

  AppTick_Init( );
  AppTick_Add( phAppTicks, 10, Button_Serve, NULL );
  AppTick_Add( phAppTicks, 40, Led_Serve, phLedGreen );
  AppTick_Add( phAppTicks, KPB_TICK_PERIOD, KPB_Tick, phKPB );
  AppTick_Add( phAppTicks, 10, DIDO_Serve, phDIM );
  
  DIM_Init( );
  MIX_Init( ); 
  DOM_Init( );
  MB_RTU_Slave_Init( );

  for ( ;; ) {
    AppTick_Serve( phAppTicks );
    KPB_Serve( phKPB );
		TbxMbEventTask( );
    // EventStartA( 0 );
    // EventStopA( 0 );
  }
}

/** -------------------------------------------------------------------------
 * @brief   Serve the digital input, mixer and output modules.
 */ 
static void DIDO_Serve( void *pArgs ) {
  //
  UNUSED( pArgs );
  DIM_Update( phDIM );
  MIX_Update( phMIX );
  DOM_Update( phDOM );
  return;
}

/** -------------------------------------------------------------------------
 * @brief   Toggle the LED.
 */
__STATIC_INLINE void LedTggl( void *pArgs ) {
  HAL_GPIO_TogglePin( LD2_GPIO_Port, LD2_Pin );
  return;
}

/** -------------------------------------------------------------------------
 * @brief   Blue button event callback function
 * @param   evt:  true - pushed, false - released.
 */
static void Button_EventCB( bool evt ) {
  //
  static uint32_t  id      = 1;
  static sLC_Pat_t asPat[] =                                     //
      { LC_SPD_M1000, LC_SPD_M240,                               //
        LC_SPD_1S,    LC_SPD_2S,   LC_SPD_3S,                    //
        LC_SPD_1S1L,  LC_SPD_2S1L, LC_SPD_1L1S, LC_SPD_1L2S,     //
        LC_SPD_ON,    LC_SPD_OFF };

  if ( !evt ) {
    LC_Background( phLedGreen, &asPat[ id ] );
    if ( ++id >= sizeof( asPat ) / sizeof( asPat[ 0 ] ) ) id = 0;
  }
  else
    LC_Event( phLedGreen, 1, &LC_SPD_M80 );

  return;
}

/** -------------------------------------------------------------------------
 * @brief   AppTick callback fn to serve the button.
 */
static void Button_Serve( void *pArgs ) {
  // static uint16_t MsgNum   = 0;
  static bool BtnState = 0, BtnPrev = 0;
  BtnState = B1_IsPushed( );
  if ( BtnState != BtnPrev ) {
    swo_msg( "%06u: B1 %s!\n", HAL_GetTick( ), BtnState ? "pushed" : "released" );
    BtnPrev = BtnState;
    Button_EventCB( BtnState );
  }
  return;
}

/** -------------------------------------------------------------------------
 * @brief   AppTick callback fn to serve the LED.
 */
static void Led_Serve( void *pArgs ) {
  //
  LC_Serve( (phLC_t) pArgs );
  return;
}

/** -------------------------------------------------------------------------
 * @brief   KPB callback fn.
 */
void KPB_KeyEventCallback( phKPB_t ph, eKPB_Key_t eKey, eKPB_Evt_t eEvt ) {
  //
  switch ( (int32_t) eEvt ) {
    default: break;
    case (int32_t) KPB_EVT_PRESS: {
      LC_Background( phLedGreen, &LC_SPD_OFF );
      LC_Event( phLedGreen, 1, &LC_SPD_M80 );
      break;
    }
    case (int32_t) KPB_EVT_REPEATE: {
      LC_Event( phLedGreen, 1, &LC_SPD_M80 );
      break;
    }
    case (int32_t) KPB_EVT_HELD_TIME_1:
    case (int32_t) KPB_EVT_HELD_TIME_2:
    case (int32_t) KPB_EVT_HELD_TIME_3: {
      LC_Event( phLedGreen, 1, &LC_SPD_OFF );
      break;
    }
    case (int32_t) KPB_EVT_RELEASE: {
      LC_Background( phLedGreen, &LC_SPD_M1000 );
      break;
    }
  }
  return;
}