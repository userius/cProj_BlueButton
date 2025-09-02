#ifndef __APP_TICKS__
#define __APP_TICKS__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus)

#define APP_TICKS_MAX 4

  typedef void ( *AT_CB_t )( void *pArgs );

  typedef enum _eAppTickErrors {
    AT_ERR_NONE,
    AT_ERR_HANDLE,
    AT_ERR_PERIOD,
    AT_ERR_CALLBACK,
    AT_ERR_TICKSMAX,
  } eATE_t;

  typedef struct _hAppTick {
    uint16_t RegFlags;
    uint16_t CntFlags;     // mask of used flags
    uint16_t aPeriods[ APP_TICKS_MAX ];
    AT_CB_t  aCallBackFn[ APP_TICKS_MAX ];
    void    *apArgs[ APP_TICKS_MAX ];
  } hAT_t, *phAT_t;

  phAT_t AppTick_Init( void );
  eATE_t AppTick_Add( phAT_t ph, uint16_t period, AT_CB_t CallBackFn, void *pArgs );
  eATE_t AppTick_Handle( phAT_t ph );
  eATE_t AppTick_Serve( phAT_t ph );

  extern phAT_t phAppTicks;

#ifdef __cplusplus
}
#endif     // __cplusplus

#endif     // __APP_TICKS__
