
/**  Modbus Mapping for Digital Input/Mixer/Output Module
|==========================================================================================|
| Data Model     | Function Codes  | Address Range   | Access | Description / Mapping      |
|==========================================================================================|
| Coils          | FC01 (Read),    | `00000 – 00003` | R      | `phDOM->OutStates`         |
|                | FC05 (Write),   |                 |        | `phDOM->sProtCtrl`         |
|                | FC15 (Wr.Mult.) | `00000 – 00003` | W      | `.Activate/Deactivate`     |
|                |                 | `00020 – 00023` | R/W    | `.KeepActive`              |
|                |                 | `00040 – 00043` | R/W    | `.KeepInactive`            |
| -------------- | --------------- | --------------- | ------ | -------------------------- |
| Discrete       | FC02 (Read)     | `10000 – 10015` | R      | `phDIM->sOutsDIM.States`   |
| Inputs         |                 |                 |        |                            |
| -------------- | --------------- | --------------- | ------ | -------------------------- |
| Input          | FC04 (Read)     | `30001`         | R      | `phDIM->sOutsDIM.States`   |
| Registers      |                 | `30002`         | R      | `phMIX->sOutsMIX.States`   |
|                |                 | `30003`         | R      | `phDOM->OutStates`         |
| -------------- | --------------- | --------------- | ------ | -------------------------- |
| Holding        | FC03 (Read),    | `40000 - 40003` | R/W    | `phDOM->sProtCtrl`         |
| Registers      | FC06 (Write),   | `40000`         | R/W    | `.KeepInactive`            |
|                | FC16 (Wr.Mult.) | `40001`         | R/W    | `.KeepActive`              |
|                |                 | `40002`         | R/W    | `.Deactivate`              |
|                |                 | `40003`         | R/W    | `.Activate`                |
| -------------- | --------------- | --------------- | ------ | -------------------------- |
| MB RTU         | FC03/FC06/FC16  | `40050`         | R/W    | `sMbRtuSlvCfg.SlaveID`     |
| Slave Cfg      |                 | `40051`         | R      | `sMbRtuSlvCfg.PortID`      |
|                |                 | `40052`         | R/W    | `sMbRtuSlvCfg.BaudrateID`  |
|                |                 | `40053`         | R      | `sMbRtuSlvCfg.DatabitsID`  |
|                |                 | `40054`         | R/W    | `sMbRtuSlvCfg.StopBitsID`  |
|                |                 | `40055`         | R/W    | `sMbRtuSlvCfg.ParityID`    |
| -------------- | --------------- | --------------- | ------ | -------------------------- |
| DIM Block      | FC03/FC06/FC16  | `40100 – 40115` | R/W    | `phDIM->aTau[0..3]`        |
|                |                 | `40116`         | R/W    | `phDIM->MaskForLED`        |
| -------------- | --------------- | --------------- | ------ | -------------------------- |
| MIX Block      |                 |                 |        | `phMIX->psCfg`             |
| MIX Channel 0  | FC03/FC06/FC16  | `40200 – 40210` | R/W    | `->asChCfgs[ 0 ]`          |
|                |                 | `40200`         | R/W    | `.sMasksDIM.StXOR`         |
|                |                 | `40201`         | R/W    | `.sMasksDIM.State`         |
|                |                 | `40202`         | R/W    | `.sMasksDIM.Rise`          |
|                |                 | `40203`         | R/W    | `.sMasksDIM.Fall`          |
|                |                 | `40204`         | R/W    | `.sMasksMIX.StXOR`         |
|                |                 | `40205`         | R/W    | `.sMasksMIX.State`         |
|                |                 | `40206`         | R/W    | `.sMasksMIX.Rise`          |
|                |                 | `40207`         | R/W    | `.sMasksMIX.Fall`          |
|                |                 | `40208`         | R/W    | `.MaskUsage & 0x0000FFFFF` |
|                |                 | `40209`         | R/W    | `.MaskUsage >> 16`         |
|                |                 | `40210`         | R/W    | `.LogicOperation`          |
| MIX Channel 1  | FC03/FC06/FC16  | `40220 – 40230` | R/W    | `same layout as Channel 0` |
| MIX Channel 2  | FC03/FC06/FC16  | `40240 – 40250` | R/W    | `same layout as Channel 0` |
| MIX Channel 3  | FC03/FC06/FC16  | `40260 – 40270` | R/W    | `same layout as Channel 0` |
| MIX Channel 4  | FC03/FC06/FC16  | `40280 – 40290` | R/W    | `same layout as Channel 0` |
| MIX Channel 5  | FC03/FC06/FC16  | `40300 – 40310` | R/W    | `same layout as Channel 0` |
| MIX Channel 6  | FC03/FC06/FC16  | `40320 – 40330` | R/W    | `same layout as Channel 0` |
| MIX Channel 7  | FC03/FC06/FC16  | `40340 – 40350` | R/W    | `same layout as Channel 0` |
| MIX Channel 8  | FC03/FC06/FC16  | `40360 – 40370` | R/W    | `same layout as Channel 0` |
| MIX Channel 9  | FC03/FC06/FC16  | `40380 – 40390` | R/W    | `same layout as Channel 0` |
| MIX Channel 10 | FC03/FC06/FC16  | `40400 – 40410` | R/W    | `same layout as Channel 0` |
| MIX Channel 11 | FC03/FC06/FC16  | `40420 – 40430` | R/W    | `same layout as Channel 0` |
| MIX Channel 12 | FC03/FC06/FC16  | `40440 – 40450` | R/W    | `same layout as Channel 0` |
| MIX Channel 13 | FC03/FC06/FC16  | `40460 – 40470` | R/W    | `same layout as Channel 0` |
| MIX Channel 14 | FC03/FC06/FC16  | `40480 – 40490` | R/W    | `same layout as Channel 0` |
| MIX Channel 15 | FC03/FC06/FC16  | `40500 – 40510` | R/W    | `same layout as Channel 0` |
| -------------- | --------------- | --------------- | ------ | -------------------------- |
| DOM Block      |                 |                 |        | `phDOM->psCfg`             |
| DOM Channel 0  | FC03/FC06/FC16  | `40600 – 40603` | R/W    | `->asChCfg[ 0 ]`           |
|                |                 | `40600`         | R/W    | `.uAct.RegSrcID`           |
|                |                 | `40601`         | R/W    | `.uDeact.RegSrcID`         |
|                |                 | `40602`         | R/W    | `.uCfgTDA.RegTimCgf`       |
|                |                 | `40603`         | R/W    | `.uCfgTHO.RegTimCgf`       |
| DOM Channel 1  | FC03/FC06/FC16  | `40604 – 40607` | R/W    | `same layout as Channel 0` |
| DOM Channel 2  | FC03/FC06/FC16  | `40608 – 40611` | R/W    | `same layout as Channel 0` |
| DOM Channel 3  | FC03/FC06/FC16  | `40612 – 40615` | R/W    | `same layout as Channel 0` |
| DOM Channel 4  | FC03/FC06/FC16  | `40616 – 40619` | R/W    | `same layout as Channel 0` |
| DOM Channel 5  | FC03/FC06/FC16  | `40620 – 40623` | R/W    | `same layout as Channel 0` |
| DOM Channel 6  | FC03/FC06/FC16  | `40624 – 40627` | R/W    | `same layout as Channel 0` |
| DOM Channel 7  | FC03/FC06/FC16  | `40628 – 40631` | R/W    | `same layout as Channel 0` |
| DOM Channel 8  | FC03/FC06/FC16  | `40632 – 40635` | R/W    | `same layout as Channel 0` |
| DOM Channel 9  | FC03/FC06/FC16  | `40636 – 40639` | R/W    | `same layout as Channel 0` |
| DOM Channel 10 | FC03/FC06/FC16  | `40640 – 40643` | R/W    | `same layout as Channel 0` |
| DOM Channel 11 | FC03/FC06/FC16  | `40644 – 40647` | R/W    | `same layout as Channel 0` |
| DOM Channel 12 | FC03/FC06/FC16  | `40648 – 40651` | R/W    | `same layout as Channel 0` |
| DOM Channel 13 | FC03/FC06/FC16  | `40652 – 40655` | R/W    | `same layout as Channel 0` |
| DOM Channel 14 | FC03/FC06/FC16  | `40656 – 40659` | R/W    | `same layout as Channel 0` |
| DOM Channel 15 | FC03/FC06/FC16  | `40660 – 40663` | R/W    | `same layout as Channel 0` |
|                |                 | `40664`         | R/W    | `->OutsMaskXOR[0..3]`      |
| -------------- | --------------- | --------------- | ------ | -------------------------- |
 */

#include "main.h"
#include "microtbx.h"
#include "microtbxmodbus.h"
#include "dig_in.h"
#include "dig_mix.h"
#include "dig_out.h"
#include "mb_rtu_slave.h"

sMB_RTU_Slv_Cfg_t sMbRtuSlvCfg = {
    .SlaveID    = 10U,                        //
    .PortID     = TBX_MB_UART_PORT1,          //
    .BaudrateID = TBX_MB_UART_19200BPS,       //
    .DatabitsID = TBX_MB_UART_8_DATABITS,     //
    .StopBitsID = TBX_MB_UART_1_STOPBITS,     //
    .ParityID   = TBX_MB_EVEN_PARITY,         //
};
psMB_RTU_Slv_Cfg_t psMbRtuSlvCfg = &sMbRtuSlvCfg;

typedef tTbxMbServerResult FnRes_t;

/** Function prototypes. ------------------------------------------------------------- */

static FnRes_t _FC01_ReadCoil( tTbxMbServer ph, uint16_t Addr, uint8_t *pVal );
static FnRes_t _FC02_ReadInput( tTbxMbServer channel, uint16_t addr, uint8_t *value );
static FnRes_t _FC03_ReadHoldingRegs( tTbxMbServer ph, uint16_t Addr, uint16_t *pVal );
static FnRes_t _FC04_ReadInputReg( tTbxMbServer channel, uint16_t addr, uint16_t *value );
static FnRes_t _FC05_WriteCoil( tTbxMbServer channel, uint16_t addr, uint8_t value );
static FnRes_t _FC06_WriteHoldingReg( tTbxMbServer channel, uint16_t addr, uint16_t value );

/** Local data declarations. --------------------------------------------------------- */
static tTbxMbTp     phTpMB;      // Modbus RTU transport layer handle.
static tTbxMbServer phSrvMB;     // Modbus server channel handle.

/** -------------------------------------------------------------------------
 * @brief   Initializes the Modbus RTU slave.
 * @details This function initializes the Modbus RTU slave stack, creates the Modbus RTU
 *          transport layer object, creates the Modbus server object, and sets the
 *          callbacks for accessing the Modbus data tables.
 *          Make sure to continuously call the Modbus stack event task function
 *          TbxMbEventTask( ) in your application main loop.
 */
void MB_RTU_Slave_Init( void ) {
  /**
   * Construct a Modbus RTU transport layer object.
   * Make sure the transport layer was created successfully.
   * Construct a Modbus server object.
   * Set the callbacks for accessing the Modbus data tables.
   */
  phTpMB = TbxMbRtuCreate( 10U, TBX_MB_UART_PORT1, TBX_MB_UART_19200BPS,     //
                           TBX_MB_UART_1_STOPBITS, TBX_MB_EVEN_PARITY );
  TBX_ASSERT( phTpMB );
  if ( !phTpMB ) return;

  phSrvMB = TbxMbServerCreate( phTpMB );
  TBX_ASSERT( phSrvMB );
  if ( !phSrvMB ) return;

  TbxMbServerSetCallbackReadCoil( phSrvMB, _FC01_ReadCoil );
  TbxMbServerSetCallbackReadInput( phSrvMB, _FC02_ReadInput );
  TbxMbServerSetCallbackReadHoldingReg( phSrvMB, _FC03_ReadHoldingRegs );
  TbxMbServerSetCallbackReadInputReg( phSrvMB, _FC04_ReadInputReg );
  TbxMbServerSetCallbackWriteCoil( phSrvMB, _FC05_WriteCoil );
  TbxMbServerSetCallbackWriteHoldingReg( phSrvMB, _FC06_WriteHoldingReg );

  return;
}

/** -------------------------------------------------------------------------------------
 * @brief     Reads a data element from the coils data table.
 * @details   Note that the element is specified by its zero-based address in the range
 *            0 - 65535, not its element number (1 - 65536).
 * @param     ph    Handle to the Modbus server channel object that triggered the callback.
 * @param     Addr  Element address (0..65535).
 * @param     pVal  Pointer to write the value of the coil to. Use TBX_ON if the
 *                  coils is on, TBX_OFF otherwise.
 * @return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
 *            specific data element address is not supported by this server,
 *            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
 */
static FnRes_t _FC01_ReadCoil( tTbxMbServer ph, uint16_t Addr, uint8_t *pVal ) {
  //
  FnRes_t _Err = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( ph );
  switch ( Addr ) {
    case 0U:
    case 1U:
    case 2U:
    case 3U: *pVal = ( 0 != READ_BIT( phDOM->OutStates, 1U << ( Addr ) ) ); break;
    default:     // Unsupported coil address.
      _Err = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return _Err;
}

/** -------------------------------------------------------------------------------------
 * @brief     Writes a data element to the coils data table.
 * @details   Note that the element is specified by its zero-based address in the range
 *            0 - 65535, not its element number (1 - 65536).
 * @param     channel Handle to the Modbus server channel object that triggered the
 *            callback.
 * @param     addr Element address (0..65535).
 * @param     value Coil value. Use TBX_ON to activate the coil, TBX_OFF otherwise.
 * @return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
 *            specific data element address is not supported by this server,
 *            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
 */
static FnRes_t _FC05_WriteCoil( tTbxMbServer ph, uint16_t Addr, uint8_t Val ) {
  //
  FnRes_t _Err = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( ph );
  switch ( Addr ) {
    case 0U:
    case 1U:
    case 2U:
    case 3U:
      Val ? SET_BIT( phDOM->sProtCtrl.Activate, 1U << ( Addr ) ) :
            SET_BIT( phDOM->sProtCtrl.Deactivate, 1U << ( Addr ) );
      break;
    case 20U:
    case 21U:
    case 22U:
    case 23U:
      Val ? SET_BIT( phDOM->sProtCtrl.KeepActive, 1U << ( Addr - 20 ) ) :
            CLEAR_BIT( phDOM->sProtCtrl.KeepActive, 1U << ( Addr - 20 ) );
      break;
    case 40U:
    case 41U:
    case 42U:
    case 43U:
      Val ? SET_BIT( phDOM->sProtCtrl.KeepInactive, 1U << ( Addr - 40 ) ) :
            CLEAR_BIT( phDOM->sProtCtrl.KeepInactive, 1U << ( Addr - 40 ) );
      break;
    default:     // Unsupported coil address.
      _Err = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return _Err;
}

/** -------------------------------------------------------------------------------------
 * @brief     Reads a data element from the discrete inputs data table.
 * @details   Note that the element is specified by its zero-based address in the range
 *            0 - 65535, not its element number (1 - 65536).
 * @param     ph    Handle to the Modbus server channel object that triggered the callback.
 * @param     Addr  Element address (0..65535).
 * @param     pVal  Pointer to write the value of the input to. Use TBX_ON if the input
 *                  is on, TBX_OFF otherwise.
 * @return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
 *            specific data element address is not supported by this server,
 *            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
 */
static FnRes_t _FC02_ReadInput( tTbxMbServer ph, uint16_t Addr, uint8_t *pVal ) {
  //
  FnRes_t _Err = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( ph );
  switch ( Addr ) {
    case 10000U:
    case 10001U:
    case 10002U:
    case 10003U:
    case 10004U:
    case 10005U:
    case 10006U:
    case 10007U:
    case 10008U:
    case 10009U:
    case 10010U:
    case 10011U:
    case 10012U:
    case 10013U:
    case 10014U:
    case 10015U:
      *pVal = ( 0 != READ_BIT( phDIM->sOutsDIM.States, 1U << ( Addr - 10000U ) ) );
      break;
    default:     // Unsupported discrete input address.
      _Err = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return _Err;
}

/** -------------------------------------------------------------------------------------
 * @brief     Reads a data element from the input registers data table.
 * @details   Write the value of the input register in your CPUs native endianess. The
 *            MicroTBX-Modbus stack will automatically convert this to the big endianess
 *            that the Modbus protocol requires.
 *            Note that the element is specified by its zero-based address in the range
 *            0 - 65535, not its element number (1 - 65536).
 * @param     channel Handle to the Modbus server channel object that triggered the
 *            callback.
 * @param     addr Element address (0..65535).
 * @param     value Pointer to write the value of the input register to.
 * @return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
 *            specific data element address is not supported by this server,
 *            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
 */
static FnRes_t _FC04_ReadInputReg( tTbxMbServer ph, uint16_t Addr, uint16_t *pVal ) {
  //
  FnRes_t _Err = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( ph );
  switch ( Addr ) {
    case 30000U: *pVal = phDIM->sOutsDIM.States; break;
    case 30001U: *pVal = phMIX->sOutsMIX.States; break;
    case 30002U: *pVal = phDOM->OutStates; break;
    default:     // Unsupported input register address.
      _Err = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return _Err;
}

/** -------------------------------------------------------------------------------------
 * @brief     Reads a data element from the holding registers data table.
 * @details   Write the value of the holding register in your CPUs native endianess. The
 *            MicroTBX-Modbus stack will automatically convert this to the big endianess
 *            that the Modbus protocol requires.
 *            Note that the element is specified by its zero-based address in the range
 *            0 - 65535, not its element number (1 - 65536).
 * @param     ph    Handle to the Modbus server channel object that triggered the callback.
 * @param     Addr  Element address (0..65535).
 * @param     pVal  Pointer to write the value of the holding register to.
 * @return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
 *            specific data element address is not supported by this server,
 *            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
 */
static FnRes_t _FC03_ReadHoldingRegs( tTbxMbServer ph, uint16_t Addr, uint16_t *pVal ) {
  FnRes_t _Err = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( ph );
  switch ( Addr ) {
    /* Modbus control registers -------------------------------------------- */
    case 40000U: *pVal = phDOM->sProtCtrl.KeepInactive; break;
    case 40001U: *pVal = phDOM->sProtCtrl.KeepActive; break;
    case 40002U: *pVal = phDOM->sProtCtrl.Deactivate; break;
    case 40003U: *pVal = phDOM->sProtCtrl.Activate; break;

    /* Modbus config registers --------------------------------------------- */
    case 40050U: *pVal = psMbRtuSlvCfg->SlaveID; break;
    case 40051U: *pVal = psMbRtuSlvCfg->PortID; break;
    case 40052U: *pVal = psMbRtuSlvCfg->BaudrateID; break;
    case 40053U: *pVal = psMbRtuSlvCfg->DatabitsID; break;
    case 40054U: *pVal = psMbRtuSlvCfg->StopBitsID; break;
    case 40055U: *pVal = psMbRtuSlvCfg->ParityID; break;

    /* DIM config registers ------------------------------------------------ */
    case 40100U:     // DIM channel 0
    case 40101U:     // DIM channel 1
    case 40102U:     // DIM channel 2
    case 40103U:     // DIM channel 3
    case 40104U:     // DIM channel 4
    case 40105U:     // DIM channel 5
    case 40106U:     // DIM channel 6
    case 40107U:     // DIM channel 7
    case 40108U:     // DIM channel 8
    case 40109U:     // DIM channel 9
    case 40110U:     // DIM channel 10
    case 40111U:     // DIM channel 11
    case 40112U:     // DIM channel 12
    case 40113U:     // DIM channel 13
    case 40114U:     // DIM channel 14
    case 40115U:     // DIM channel 15
      *pVal = phDIM->psCfg->aTau[ Addr - 40100 ];
      break;
    case 40116U: *pVal = phDIM->psCfg->MaskForLED; break;

    /* MIX config registers ------------------------------------------------ */
    case 40200U:     // MIX channel 0
    case 40220U:     // MIX channel 1
    case 40240U:     // MIX channel 2
    case 40260U:     // MIX channel 3
    case 40280U:     // MIX channel 4
    case 40300U:     // MIX channel 5
    case 40320U:     // MIX channel 6
    case 40340U:     // MIX channel 7
    case 40360U:     // MIX channel 8
    case 40380U:     // MIX channel 9
    case 40400U:     // MIX channel 10
    case 40420U:     // MIX channel 11
    case 40440U:     // MIX channel 12
    case 40460U:     // MIX channel 13
    case 40480U:     // MIX channel 14
    case 40500U:     // MIX channel 15
      *pVal = phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksDIM.StXOR;
      break;
    case 40201U:     // MIX channel 0
    case 40221U:     // MIX channel 1
    case 40241U:     // MIX channel 2
    case 40261U:     // MIX channel 3
    case 40281U:     // MIX channel 4
    case 40301U:     // MIX channel 5
    case 40321U:     // MIX channel 6
    case 40341U:     // MIX channel 7
    case 40361U:     // MIX channel 8
    case 40381U:     // MIX channel 9
    case 40401U:     // MIX channel 10
    case 40421U:     // MIX channel 11
    case 40441U:     // MIX channel 12
    case 40461U:     // MIX channel 13
    case 40481U:     // MIX channel 14
    case 40501U:     // MIX channel 15
      *pVal = phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksDIM.State;
      break;
    case 40202U:     // MIX channel 0
    case 40222U:     // MIX channel 1
    case 40242U:     // MIX channel 2
    case 40262U:     // MIX channel 3
    case 40282U:     // MIX channel 4
    case 40302U:     // MIX channel 5
    case 40322U:     // MIX channel 6
    case 40342U:     // MIX channel 7
    case 40362U:     // MIX channel 8
    case 40382U:     // MIX channel 9
    case 40402U:     // MIX channel 10
    case 40422U:     // MIX channel 11
    case 40442U:     // MIX channel 12
    case 40462U:     // MIX channel 13
    case 40482U:     // MIX channel 14
    case 40502U:     // MIX channel 15
      *pVal = phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksDIM.Rise;
      break;
    case 40203U:     // MIX channel 0
    case 40223U:     // MIX channel 1
    case 40243U:     // MIX channel 2
    case 40263U:     // MIX channel 3
    case 40283U:     // MIX channel 4
    case 40303U:     // MIX channel 5
    case 40323U:     // MIX channel 6
    case 40343U:     // MIX channel 7
    case 40363U:     // MIX channel 8
    case 40383U:     // MIX channel 9
    case 40403U:     // MIX channel 10
    case 40423U:     // MIX channel 11
    case 40443U:     // MIX channel 12
    case 40463U:     // MIX channel 13
    case 40483U:     // MIX channel 14
    case 40503U:     // MIX channel 15
      *pVal = phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksDIM.Fall;
      break;
    case 40204U:     // MIX channel 0
    case 40224U:     // MIX channel 1
    case 40244U:     // MIX channel 2
    case 40264U:     // MIX channel 3
    case 40284U:     // MIX channel 4
    case 40304U:     // MIX channel 5
    case 40324U:     // MIX channel 6
    case 40344U:     // MIX channel 7
    case 40364U:     // MIX channel 8
    case 40384U:     // MIX channel 9
    case 40404U:     // MIX channel 10
    case 40424U:     // MIX channel 11
    case 40444U:     // MIX channel 12
    case 40464U:     // MIX channel 13
    case 40484U:     // MIX channel 14
    case 40504U:     // MIX channel 15
      *pVal = phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksMIX.StXOR;
      break;
    case 40205U:     // MIX channel 0
    case 40225U:     // MIX channel 1
    case 40245U:     // MIX channel 2
    case 40265U:     // MIX channel 3
    case 40285U:     // MIX channel 4
    case 40305U:     // MIX channel 5
    case 40325U:     // MIX channel 6
    case 40345U:     // MIX channel 7
    case 40365U:     // MIX channel 8
    case 40385U:     // MIX channel 9
    case 40405U:     // MIX channel 10
    case 40425U:     // MIX channel 11
    case 40445U:     // MIX channel 12
    case 40465U:     // MIX channel 13
    case 40485U:     // MIX channel 14
    case 40505U:     // MIX channel 15
      *pVal = phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksMIX.State;
      break;
    case 40206U:     // MIX channel 0
    case 40226U:     // MIX channel 1
    case 40246U:     // MIX channel 2
    case 40266U:     // MIX channel 3
    case 40286U:     // MIX channel 4
    case 40306U:     // MIX channel 5
    case 40326U:     // MIX channel 6
    case 40346U:     // MIX channel 7
    case 40366U:     // MIX channel 8
    case 40386U:     // MIX channel 9
    case 40406U:     // MIX channel 10
    case 40426U:     // MIX channel 11
    case 40446U:     // MIX channel 12
    case 40466U:     // MIX channel 13
    case 40486U:     // MIX channel 14
    case 40506U:     // MIX channel 15
      *pVal = phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksMIX.Rise;
      break;
    case 40207U:     // MIX channel 0
    case 40227U:     // MIX channel 1
    case 40247U:     // MIX channel 2
    case 40267U:     // MIX channel 3
    case 40287U:     // MIX channel 4
    case 40307U:     // MIX channel 5
    case 40327U:     // MIX channel 6
    case 40347U:     // MIX channel 7
    case 40367U:     // MIX channel 8
    case 40387U:     // MIX channel 9
    case 40407U:     // MIX channel 10
    case 40427U:     // MIX channel 11
    case 40447U:     // MIX channel 12
    case 40467U:     // MIX channel 13
    case 40487U:     // MIX channel 14
    case 40507U:     // MIX channel 15
      *pVal = phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksMIX.Fall;
      break;
    case 40208U:     // MIX channel 0
    case 40228U:     // MIX channel 1
    case 40248U:     // MIX channel 2
    case 40268U:     // MIX channel 3
    case 40288U:     // MIX channel 4
    case 40308U:     // MIX channel 5
    case 40328U:     // MIX channel 6
    case 40348U:     // MIX channel 7
    case 40368U:     // MIX channel 8
    case 40388U:     // MIX channel 9
    case 40408U:     // MIX channel 10
    case 40428U:     // MIX channel 11
    case 40448U:     // MIX channel 12
    case 40468U:     // MIX channel 13
    case 40488U:     // MIX channel 14
    case 40508U:     // MIX channel 15
      *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].MaskUsage )[ 0 ];
      break;
    case 40209U:     // MIX channel 0
    case 40229U:     // MIX channel 1
    case 40249U:     // MIX channel 2
    case 40269U:     // MIX channel 3
    case 40289U:     // MIX channel 4
    case 40309U:     // MIX channel 5
    case 40329U:     // MIX channel 6
    case 40349U:     // MIX channel 7
    case 40369U:     // MIX channel 8
    case 40389U:     // MIX channel 9
    case 40409U:     // MIX channel 10
    case 40429U:     // MIX channel 11
    case 40449U:     // MIX channel 12
    case 40469U:     // MIX channel 13
    case 40489U:     // MIX channel 14
    case 40509U:     // MIX channel 15
      *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].MaskUsage )[ 1 ];
      break;
    case 40210U:     // MIX channel 0
    case 40230U:     // MIX channel 1
    case 40250U:     // MIX channel 2
    case 40270U:     // MIX channel 3
    case 40290U:     // MIX channel 4
    case 40310U:     // MIX channel 5
    case 40330U:     // MIX channel 6
    case 40350U:     // MIX channel 7
    case 40370U:     // MIX channel 8
    case 40390U:     // MIX channel 9
    case 40410U:     // MIX channel 10
    case 40430U:     // MIX channel 11
    case 40450U:     // MIX channel 12
    case 40470U:     // MIX channel 13
    case 40490U:     // MIX channel 14
    case 40510U:     // MIX channel 15
      *pVal = (uint16_t) phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].eLogicOperation;
      break;

    /* DOM config registers ------------------------------------------------ */
    case 40600U:     // DOM channel 0
    case 40604U:     // DOM channel 1
    case 40608U:     // DOM channel 2
    case 40612U:     // DOM channel 3
    case 40616U:     // DOM channel 4
    case 40620U:     // DOM channel 5
    case 40624U:     // DOM channel 6
    case 40628U:     // DOM channel 7
    case 40632U:     // DOM channel 8
    case 40636U:     // DOM channel 9
    case 40640U:     // DOM channel 10
    case 40644U:     // DOM channel 11
    case 40648U:     // DOM channel 12
    case 40652U:     // DOM channel 13
    case 40656U:     // DOM channel 14
    case 40660U:     // DOM channel 15
      *pVal = phDOM->psCfg->asChCfg[ ( Addr - 40500 ) / 4 ].uAct.RegSrcID;
      break;
    case 40601U:     // DOM channel 0
    case 40605U:     // DOM channel 1
    case 40609U:     // DOM channel 2
    case 40613U:     // DOM channel 3
    case 40617U:     // DOM channel 4
    case 40621U:     // DOM channel 5
    case 40625U:     // DOM channel 6
    case 40629U:     // DOM channel 7
    case 40633U:     // DOM channel 8
    case 40637U:     // DOM channel 9
    case 40641U:     // DOM channel 10
    case 40645U:     // DOM channel 11
    case 40649U:     // DOM channel 12
    case 40653U:     // DOM channel 13
    case 40657U:     // DOM channel 14
    case 40661U:     // DOM channel 15
      *pVal = phDOM->psCfg->asChCfg[ ( Addr - 40500 ) / 4 ].uDeact.RegSrcID;
      break;
    case 40602U:     // DOM channel 0
    case 40606U:     // DOM channel 1
    case 40610U:     // DOM channel 2
    case 40614U:     // DOM channel 3
    case 40618U:     // DOM channel 4
    case 40622U:     // DOM channel 5
    case 40626U:     // DOM channel 6
    case 40630U:     // DOM channel 7
    case 40634U:     // DOM channel 8
    case 40638U:     // DOM channel 9
    case 40642U:     // DOM channel 10
    case 40646U:     // DOM channel 11
    case 40650U:     // DOM channel 12
    case 40654U:     // DOM channel 13
    case 40658U:     // DOM channel 14
    case 40662U:     // DOM channel 15
      *pVal = phDOM->psCfg->asChCfg[ ( Addr - 40500 ) / 4 ].uCfgTDA.RegTimCgf;
      break;
    case 40603U:     // DOM channel 0
    case 40607U:     // DOM channel 1
    case 40611U:     // DOM channel 2
    case 40615U:     // DOM channel 3
    case 40619U:     // DOM channel 4
    case 40623U:     // DOM channel 5
    case 40627U:     // DOM channel 6
    case 40631U:     // DOM channel 7
    case 40635U:     // DOM channel 8
    case 40639U:     // DOM channel 9
    case 40643U:     // DOM channel 10
    case 40647U:     // DOM channel 11
    case 40651U:     // DOM channel 12
    case 40655U:     // DOM channel 13
    case 40659U:     // DOM channel 14
    case 40663U:     // DOM channel 15
      *pVal = phDOM->psCfg->asChCfg[ ( Addr - 40500 ) / 4 ].uCfgTHO.RegTimCgf;
      break;
    // DOM out pins mask
    case 40664U: *pVal = phDOM->psCfg->OutsMaskXOR; break;

    /* Unsupported input register address. --------------------------------- */
    default: _Err = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR; break;
  }

  return _Err;
}

/** -------------------------------------------------------------------------------------
 * @brief     Writes a data element to the holding registers data table.
 * @details   Note that the element is specified by its zero-based address in the range
 *            0 - 65535, not its element number (1 - 65536).
 *            The value of the holding register in already in your CPUs native endianess.
 * @param     ph    Handle to the Modbus server channel object that triggered the callback.
 * @param     Addr  Element address (0..65535).
 * @param     Val   Value of the holding register.
 * @return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
 *            specific data element address is not supported by this server,
 *            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
 *
 */
static FnRes_t _FC06_WriteHoldingReg( tTbxMbServer ph, uint16_t Addr, uint16_t Val ) {
  //
  FnRes_t _Res = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( ph );
  switch ( Addr ) {
    /* Modbus control registers -------------------------------------------- */
    case 40000U: phDOM->sProtCtrl.KeepInactive = Val; break;
    case 40001U: phDOM->sProtCtrl.KeepActive = Val; break;
    case 40002U: phDOM->sProtCtrl.Deactivate = Val; break;
    case 40003U: phDOM->sProtCtrl.Activate = Val; break;

    /* Modbus config registers --------------------------------------------- */
    case 40050U: psMbRtuSlvCfg->SlaveID = Val; break;
    case 40051U: psMbRtuSlvCfg->PortID = Val; break;
    case 40052U: psMbRtuSlvCfg->BaudrateID = Val; break;
    case 40053U: psMbRtuSlvCfg->DatabitsID = Val; break;
    case 40054U: psMbRtuSlvCfg->StopBitsID = Val; break;
    case 40055U: psMbRtuSlvCfg->ParityID = Val; break;

    /* DIM config registers ------------------------------------------------ */
    case 40100U:     // DIM channel 0
    case 40101U:     // DIM channel 1
    case 40102U:     // DIM channel 2
    case 40103U:     // DIM channel 3
    case 40104U:     // DIM channel 4
    case 40105U:     // DIM channel 5
    case 40106U:     // DIM channel 6
    case 40107U:     // DIM channel 7
    case 40108U:     // DIM channel 8
    case 40109U:     // DIM channel 9
    case 40110U:     // DIM channel 10
    case 40111U:     // DIM channel 11
    case 40112U:     // DIM channel 12
    case 40113U:     // DIM channel 13
    case 40114U:     // DIM channel 14
    case 40115U:     // DIM channel 15
      phDIM->psCfg->aTau[ Addr - 40100 ] = Val;
      break;
    case 40116U: phDIM->psCfg->MaskForLED = Val; break;

    /* MIX config registers ------------------------------------------------ */
    case 40200U:     // MIX channel 0
    case 40220U:     // MIX channel 1
    case 40240U:     // MIX channel 2
    case 40260U:     // MIX channel 3
    case 40280U:     // MIX channel 4
    case 40300U:     // MIX channel 5
    case 40320U:     // MIX channel 6
    case 40340U:     // MIX channel 7
    case 40360U:     // MIX channel 8
    case 40380U:     // MIX channel 9
    case 40400U:     // MIX channel 10
    case 40420U:     // MIX channel 11
    case 40440U:     // MIX channel 12
    case 40460U:     // MIX channel 13
    case 40480U:     // MIX channel 14
    case 40500U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksDIM.StXOR = Val;
      break;
    case 40201U:     // MIX channel 0
    case 40221U:     // MIX channel 1
    case 40241U:     // MIX channel 2
    case 40261U:     // MIX channel 3
    case 40281U:     // MIX channel 4
    case 40301U:     // MIX channel 5
    case 40321U:     // MIX channel 6
    case 40341U:     // MIX channel 7
    case 40361U:     // MIX channel 8
    case 40381U:     // MIX channel 9
    case 40401U:     // MIX channel 10
    case 40421U:     // MIX channel 11
    case 40441U:     // MIX channel 12
    case 40461U:     // MIX channel 13
    case 40481U:     // MIX channel 14
    case 40501U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksDIM.State = Val;
      break;
    case 40202U:     // MIX channel 0
    case 40222U:     // MIX channel 1
    case 40242U:     // MIX channel 2
    case 40262U:     // MIX channel 3
    case 40282U:     // MIX channel 4
    case 40302U:     // MIX channel 5
    case 40322U:     // MIX channel 6
    case 40342U:     // MIX channel 7
    case 40362U:     // MIX channel 8
    case 40382U:     // MIX channel 9
    case 40402U:     // MIX channel 10
    case 40422U:     // MIX channel 11
    case 40442U:     // MIX channel 12
    case 40462U:     // MIX channel 13
    case 40482U:     // MIX channel 14
    case 40502U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksDIM.Rise = Val;
      break;
    case 40203U:     // MIX channel 0
    case 40223U:     // MIX channel 1
    case 40243U:     // MIX channel 2
    case 40263U:     // MIX channel 3
    case 40283U:     // MIX channel 4
    case 40303U:     // MIX channel 5
    case 40323U:     // MIX channel 6
    case 40343U:     // MIX channel 7
    case 40363U:     // MIX channel 8
    case 40383U:     // MIX channel 9
    case 40403U:     // MIX channel 10
    case 40423U:     // MIX channel 11
    case 40443U:     // MIX channel 12
    case 40463U:     // MIX channel 13
    case 40483U:     // MIX channel 14
    case 40503U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksDIM.Fall = Val;
      break;
    case 40204U:     // MIX channel 0
    case 40224U:     // MIX channel 1
    case 40244U:     // MIX channel 2
    case 40264U:     // MIX channel 3
    case 40284U:     // MIX channel 4
    case 40304U:     // MIX channel 5
    case 40324U:     // MIX channel 6
    case 40344U:     // MIX channel 7
    case 40364U:     // MIX channel 8
    case 40384U:     // MIX channel 9
    case 40404U:     // MIX channel 10
    case 40424U:     // MIX channel 11
    case 40444U:     // MIX channel 12
    case 40464U:     // MIX channel 13
    case 40484U:     // MIX channel 14
    case 40504U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksMIX.StXOR = Val;
      break;
    case 40205U:     // MIX channel 0
    case 40225U:     // MIX channel 1
    case 40245U:     // MIX channel 2
    case 40265U:     // MIX channel 3
    case 40285U:     // MIX channel 4
    case 40305U:     // MIX channel 5
    case 40325U:     // MIX channel 6
    case 40345U:     // MIX channel 7
    case 40365U:     // MIX channel 8
    case 40385U:     // MIX channel 9
    case 40405U:     // MIX channel 10
    case 40425U:     // MIX channel 11
    case 40445U:     // MIX channel 12
    case 40465U:     // MIX channel 13
    case 40485U:     // MIX channel 14
    case 40505U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksMIX.State = Val;
      break;
    case 40206U:     // MIX channel 0
    case 40226U:     // MIX channel 1
    case 40246U:     // MIX channel 2
    case 40266U:     // MIX channel 3
    case 40286U:     // MIX channel 4
    case 40306U:     // MIX channel 5
    case 40326U:     // MIX channel 6
    case 40346U:     // MIX channel 7
    case 40366U:     // MIX channel 8
    case 40386U:     // MIX channel 9
    case 40406U:     // MIX channel 10
    case 40426U:     // MIX channel 11
    case 40446U:     // MIX channel 12
    case 40466U:     // MIX channel 13
    case 40486U:     // MIX channel 14
    case 40506U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksMIX.Rise = Val;
      break;
    case 40207U:     // MIX channel 0
    case 40227U:     // MIX channel 1
    case 40247U:     // MIX channel 2
    case 40267U:     // MIX channel 3
    case 40287U:     // MIX channel 4
    case 40307U:     // MIX channel 5
    case 40327U:     // MIX channel 6
    case 40347U:     // MIX channel 7
    case 40367U:     // MIX channel 8
    case 40387U:     // MIX channel 9
    case 40407U:     // MIX channel 10
    case 40427U:     // MIX channel 11
    case 40447U:     // MIX channel 12
    case 40467U:     // MIX channel 13
    case 40487U:     // MIX channel 14
    case 40507U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].sMasksMIX.Fall = Val;
      break;
    case 40208U:     // MIX channel 0
    case 40228U:     // MIX channel 1
    case 40248U:     // MIX channel 2
    case 40268U:     // MIX channel 3
    case 40288U:     // MIX channel 4
    case 40308U:     // MIX channel 5
    case 40328U:     // MIX channel 6
    case 40348U:     // MIX channel 7
    case 40368U:     // MIX channel 8
    case 40388U:     // MIX channel 9
    case 40408U:     // MIX channel 10
    case 40428U:     // MIX channel 11
    case 40448U:     // MIX channel 12
    case 40468U:     // MIX channel 13
    case 40488U:     // MIX channel 14
    case 40508U:     // MIX channel 15
      ( (uint16_t *) &phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].MaskUsage )[ 0 ] = Val;
      break;
    case 40209U:     // MIX channel 0
    case 40229U:     // MIX channel 1
    case 40249U:     // MIX channel 2
    case 40269U:     // MIX channel 3
    case 40289U:     // MIX channel 4
    case 40309U:     // MIX channel 5
    case 40329U:     // MIX channel 6
    case 40349U:     // MIX channel 7
    case 40369U:     // MIX channel 8
    case 40389U:     // MIX channel 9
    case 40409U:     // MIX channel 10
    case 40429U:     // MIX channel 11
    case 40449U:     // MIX channel 12
    case 40469U:     // MIX channel 13
    case 40489U:     // MIX channel 14
    case 40509U:     // MIX channel 15
      ( (uint16_t *) &phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].MaskUsage )[ 1 ] = Val;
      break;
    case 40210U:     // MIX channel 0
    case 40230U:     // MIX channel 1
    case 40250U:     // MIX channel 2
    case 40270U:     // MIX channel 3
    case 40290U:     // MIX channel 4
    case 40310U:     // MIX channel 5
    case 40330U:     // MIX channel 6
    case 40350U:     // MIX channel 7
    case 40370U:     // MIX channel 8
    case 40390U:     // MIX channel 9
    case 40410U:     // MIX channel 10
    case 40430U:     // MIX channel 11
    case 40450U:     // MIX channel 12
    case 40470U:     // MIX channel 13
    case 40490U:     // MIX channel 14
    case 40510U:     // MIX channel 15
      phMIX->psCfg->asChCfgs[ ( Addr - 40200 ) / 20 ].eLogicOperation = Val;
      break;

    /* DOM config registers ------------------------------------------------ */
    case 40600U:     // DOM channel 0
    case 40604U:     // DOM channel 1
    case 40608U:     // DOM channel 2
    case 40612U:     // DOM channel 3
    case 40616U:     // DOM channel 4
    case 40620U:     // DOM channel 5
    case 40624U:     // DOM channel 6
    case 40628U:     // DOM channel 7
    case 40632U:     // DOM channel 8
    case 40636U:     // DOM channel 9
    case 40640U:     // DOM channel 10
    case 40644U:     // DOM channel 11
    case 40648U:     // DOM channel 12
    case 40652U:     // DOM channel 13
    case 40656U:     // DOM channel 14
    case 40660U:     // DOM channel 15
      phDOM->psCfg->asChCfg[ ( Addr - 40500 ) / 4 ].uAct.RegSrcID = Val;
      break;
    case 40601U:     // DOM channel 0
    case 40605U:     // DOM channel 1
    case 40609U:     // DOM channel 2
    case 40613U:     // DOM channel 3
    case 40617U:     // DOM channel 4
    case 40621U:     // DOM channel 5
    case 40625U:     // DOM channel 6
    case 40629U:     // DOM channel 7
    case 40633U:     // DOM channel 8
    case 40637U:     // DOM channel 9
    case 40641U:     // DOM channel 10
    case 40645U:     // DOM channel 11
    case 40649U:     // DOM channel 12
    case 40653U:     // DOM channel 13
    case 40657U:     // DOM channel 14
    case 40661U:     // DOM channel 15
      phDOM->psCfg->asChCfg[ ( Addr - 40500 ) / 4 ].uDeact.RegSrcID = Val;
      break;
    case 40602U:     // DOM channel 0
    case 40606U:     // DOM channel 1
    case 40610U:     // DOM channel 2
    case 40614U:     // DOM channel 3
    case 40618U:     // DOM channel 4
    case 40622U:     // DOM channel 5
    case 40626U:     // DOM channel 6
    case 40630U:     // DOM channel 7
    case 40634U:     // DOM channel 8
    case 40638U:     // DOM channel 9
    case 40642U:     // DOM channel 10
    case 40646U:     // DOM channel 11
    case 40650U:     // DOM channel 12
    case 40654U:     // DOM channel 13
    case 40658U:     // DOM channel 14
    case 40662U:     // DOM channel 15
      phDOM->psCfg->asChCfg[ ( Addr - 40500 ) / 4 ].uCfgTDA.RegTimCgf = Val;
      break;
    case 40603U:     // DOM channel 0
    case 40607U:     // DOM channel 1
    case 40611U:     // DOM channel 2
    case 40615U:     // DOM channel 3
    case 40619U:     // DOM channel 4
    case 40623U:     // DOM channel 5
    case 40627U:     // DOM channel 6
    case 40631U:     // DOM channel 7
    case 40635U:     // DOM channel 8
    case 40639U:     // DOM channel 9
    case 40643U:     // DOM channel 10
    case 40647U:     // DOM channel 11
    case 40651U:     // DOM channel 12
    case 40655U:     // DOM channel 13
    case 40659U:     // DOM channel 14
    case 40663U:     // DOM channel 15
      phDOM->psCfg->asChCfg[ ( Addr - 40500 ) / 4 ].uCfgTHO.RegTimCgf = Val;
      break;
    // DOM out pins mask
    case 40664U: phDOM->psCfg->OutsMaskXOR = Val; break;

    /* Unsupported holding register address. ------------------------------- */
    default: _Res = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR; break;
  }

  return _Res;
}
