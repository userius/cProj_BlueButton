#ifndef __MB_RTU_SLAVE_H__
#define __MB_RTU_SLAVE_H__
#ifdef __cplusplus
extern "C"
{
#endif     // __cplusplus

#include "main.h"
  /** ---------------------------------------------------------------------------
   * @brief Configuration structure for Modbus RTU slave
   * @note  Size must be 8 bytes
   */
  typedef struct _mb_rtu_slv_cgf {
    uint8_t SlaveID;      // Modbus slave ID (1..247)
    uint8_t Reserved;     // Reserved for alignment
    union {
      uint16_t PortCfgReg16;
      struct {
        uint8_t PortID : 3;         // tTbxMbUartPort, UART port to use
        uint8_t BaudrateID : 5;     // tTbxMbUartBaudrate, Communication baudrate
        uint8_t DatabitsID : 2;     // tTbxMbUartDatabits,
        uint8_t StopBitsID : 2;     // tTbxMbUartStopbits, Number of stop bits (1 or 2)
        uint8_t ParityID : 4;       // tTbxMbUartParity, Parity mode (even, odd, none)
      };
    };
  } sMB_RTU_Slv_Cfg_t, *psMB_RTU_Slv_Cfg_t;     // 4 bytes
  
  void MB_RTU_Slave_Init( void );

  extern psMB_RTU_Slv_Cfg_t psMbRtuSlvCfg;

#ifdef __cplusplus
}
#endif     // __cplusplus
#endif     // __MB_RTU_SLAVE_H__
