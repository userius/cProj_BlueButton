#include "main.h"
#include "dig_in.h"
#include "dig_out.h"
#include "dig_mix.h"

/** ---------------------------------------------------------------------------
 * @brief Version of the configuration map
 */
typedef union _map_version {
  uint16_t Reg16;      // 0x0100 = version 1.0
  struct {             // little endian
    uint8_t Minor;     // Low byte
    uint8_t Major;     // High byte
  };
} uMapVer_t, *puMapVer_t;     // 2 bytes

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

/**
 * @brief Complete configuration map structure
 * @note  Size must be 8 + 34 + 56 + 98 + 32 + 2 = 230 bytes
 */
typedef struct _cfg_map {
  uMapVer_t         uMapVer;                 // Version of this configuration map
  uint16_t          MapSize;                 // Size of this structure in bytes, including CRC16
  sMB_RTU_Slv_Cfg_t sMbRtuSlvCfg;            //
  sDIM_Cfg_t        sDimCfg;                 // 18 bytes max
  sMIX_Cfg_t        sMixCfg;                 // TODO: change field in this struct
  sDOM_Cfg_t        sDomCfg;                 //
  uint16_t          aDimCnts[ DI_QNTT ];     //
  uint16_t          CRC16;                   // CRC16 of all previous bytes
} sCfgMap_t;

void App_Cfg_Init( void ) {
  //
  sCfgMap_t sMap     = { 0 };
  sMap.uMapVer.Reg16 = 0x0100;     // Version 1.0

  return;
}