#ifndef __SWO_DBG__
#define __SWO_DBG__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus

#ifdef SWODBG
  /** Impotant notes.
   * To use this module, prepare Keil uVision:
   * 1) Project -> Manage -> Run-Time Environment...
   *    set checkbox: Compiler -> I/O -> STDOUT, ITM
   *    set checkbox: Compiler -> I/O -> STDERR, ITM
   * 2) TODO: config keil debugger
   */

#include <stdio.h>
#define swo_msg( ... )                                                                             \
  do {                                                                                             \
    fprintf( stdout, __VA_ARGS__ );                                                                \
    fflush( stdout );                                                                              \
  } while ( 0 )
#define swo_err( ... )                                                                             \
  do {                                                                                             \
    fprintf( stderr, __VA_ARGS__ );                                                                \
    fflush( stderr );                                                                              \
  } while ( 0 )
#else     // SWODBG
#define swo_msg( ... )
#define errmessage( ... )
#endif     // SWODBG

#ifdef __cplusplus
}
#endif     // __cplusplus

#endif     // __SWO_DBG__
