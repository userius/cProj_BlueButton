#include "main.h"
#include "microtbx.h"
#include "microtbxmodbus.h"
#include "dig_in.h"
#include "dig_mix.h"
#include "dig_out.h"

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
 * @brief Initializes the Modbus RTU slave.
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
    case 40000U: *pVal = phDOM->sProtCtrl.KeepInactive; break;
    case 40001U: *pVal = phDOM->sProtCtrl.KeepActive; break;
    case 40002U: *pVal = phDOM->sProtCtrl.Deactivate; break;
    case 40003U: *pVal = phDOM->sProtCtrl.Activate; break;
    // DIM config registers
    case 40100U:
    case 40101U:
    case 40102U:
    case 40103U: *pVal = phDIM->psCfg->aTau[ Addr - 40100 ]; break;
    case 40116U: *pVal = phDIM->psCfg->MaskForLED; break;
    // MIX config registers
    case 40200U: *pVal = phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.StXOR; break;
    case 40201U: *pVal = phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.State; break;
    case 40202U: *pVal = phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.Rise; break;
    case 40203U: *pVal = phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.Fall; break;
    case 40204U: *pVal = phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.StXOR; break;
    case 40205U: *pVal = phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.State; break;
    case 40206U: *pVal = phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.Rise; break;
    case 40207U: *pVal = phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.Fall; break;
    case 40208U: *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 0 ].MaskUsage )[ 0 ]; break;
    case 40209U: *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 0 ].MaskUsage )[ 1 ]; break;
    case 40210U: *pVal = (uint16_t) phMIX->psCfg->asChCfgs[ 0 ].eLogicOperation; break;
    case 40220U: *pVal = phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.StXOR; break;
    case 40221U: *pVal = phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.State; break;
    case 40222U: *pVal = phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.Rise; break;
    case 40223U: *pVal = phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.Fall; break;
    case 40224U: *pVal = phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.StXOR; break;
    case 40225U: *pVal = phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.State; break;
    case 40226U: *pVal = phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.Rise; break;
    case 40227U: *pVal = phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.Fall; break;
    case 40228U: *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 1 ].MaskUsage )[ 0 ]; break;
    case 40229U: *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 1 ].MaskUsage )[ 1 ]; break;
    case 40230U: *pVal = (uint16_t) phMIX->psCfg->asChCfgs[ 1 ].eLogicOperation; break;
    case 40240U: *pVal = phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.StXOR; break;
    case 40241U: *pVal = phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.State; break;
    case 40242U: *pVal = phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.Rise; break;
    case 40243U: *pVal = phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.Fall; break;
    case 40244U: *pVal = phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.StXOR; break;
    case 40245U: *pVal = phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.State; break;
    case 40246U: *pVal = phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.Rise; break;
    case 40247U: *pVal = phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.Fall; break;
    case 40248U: *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 2 ].MaskUsage )[ 0 ]; break;
    case 40249U: *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 2 ].MaskUsage )[ 1 ]; break;
    case 40250U: *pVal = (uint16_t) phMIX->psCfg->asChCfgs[ 2 ].eLogicOperation; break;
    case 40260U: *pVal = phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.StXOR; break;
    case 40261U: *pVal = phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.State; break;
    case 40262U: *pVal = phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.Rise; break;
    case 40263U: *pVal = phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.Fall; break;
    case 40264U: *pVal = phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.StXOR; break;
    case 40265U: *pVal = phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.State; break;
    case 40266U: *pVal = phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.Rise; break;
    case 40267U: *pVal = phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.Fall; break;
    case 40268U: *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 3 ].MaskUsage )[ 0 ]; break;
    case 40269U: *pVal = ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 3 ].MaskUsage )[ 1 ]; break;
    case 40270U: *pVal = (uint16_t) phMIX->psCfg->asChCfgs[ 3 ].eLogicOperation; break;
    // DOM config registers
    case 40500U: *pVal = phDOM->psCfg->asChCfg[ 0 ].uAct.RegSrcID; break;
    case 40501U: *pVal = phDOM->psCfg->asChCfg[ 0 ].uDeact.RegSrcID; break;
    case 40502U: *pVal = phDOM->psCfg->asChCfg[ 0 ].uCfgTDA.RegTimCgf; break;
    case 40503U: *pVal = phDOM->psCfg->asChCfg[ 0 ].uCfgTHO.RegTimCgf; break;
    case 40504U: *pVal = phDOM->psCfg->asChCfg[ 1 ].uAct.RegSrcID; break;
    case 40505U: *pVal = phDOM->psCfg->asChCfg[ 1 ].uDeact.RegSrcID; break;
    case 40506U: *pVal = phDOM->psCfg->asChCfg[ 1 ].uCfgTDA.RegTimCgf; break;
    case 40507U: *pVal = phDOM->psCfg->asChCfg[ 1 ].uCfgTHO.RegTimCgf; break;
    case 40508U: *pVal = phDOM->psCfg->asChCfg[ 2 ].uAct.RegSrcID; break;
    case 40509U: *pVal = phDOM->psCfg->asChCfg[ 2 ].uDeact.RegSrcID; break;
    case 40510U: *pVal = phDOM->psCfg->asChCfg[ 2 ].uCfgTDA.RegTimCgf; break;
    case 40511U: *pVal = phDOM->psCfg->asChCfg[ 2 ].uCfgTHO.RegTimCgf; break;
    case 40512U: *pVal = phDOM->psCfg->asChCfg[ 3 ].uAct.RegSrcID; break;
    case 40513U: *pVal = phDOM->psCfg->asChCfg[ 3 ].uDeact.RegSrcID; break;
    case 40514U: *pVal = phDOM->psCfg->asChCfg[ 3 ].uCfgTDA.RegTimCgf; break;
    case 40515U: *pVal = phDOM->psCfg->asChCfg[ 3 ].uCfgTHO.RegTimCgf; break;
    case 40516U: *pVal = phDOM->psCfg->OutsMaskXOR; break;

    default:     // Unsupported input register address.
      _Err = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
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
    case 40000U: phDOM->sProtCtrl.KeepInactive = Val; break;
    case 40001U: phDOM->sProtCtrl.KeepActive = Val; break;
    case 40002U: phDOM->sProtCtrl.Deactivate = Val; break;
    case 40003U: phDOM->sProtCtrl.Activate = Val; break;
    // DIM config registers
    case 40100U:
    case 40101U:
    case 40102U:
    case 40103U: phDIM->psCfg->aTau[ Addr - 40100 ] = Val; break;
    case 40116U: phDIM->psCfg->MaskForLED = Val; break;
    // MIX config registers
    case 40200U: phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.StXOR = Val; break;
    case 40201U: phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.State = Val; break;
    case 40202U: phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.Rise = Val; break;
    case 40203U: phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.Fall = Val; break;
    case 40204U: phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.StXOR = Val; break;
    case 40205U: phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.State = Val; break;
    case 40206U: phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.Rise = Val; break;
    case 40207U: phMIX->psCfg->asChCfgs[ 0 ].sMasksDIM.Fall = Val; break;
    case 40208U: ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 0 ].MaskUsage )[ 0 ] = Val; break;
    case 40209U: ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 0 ].MaskUsage )[ 1 ] = Val; break;
    case 40210U: phMIX->psCfg->asChCfgs[ 0 ].eLogicOperation = Val; break;
    case 40220U: phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.StXOR = Val; break;
    case 40221U: phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.State = Val; break;
    case 40222U: phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.Rise = Val; break;
    case 40223U: phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.Fall = Val; break;
    case 40224U: phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.StXOR = Val; break;
    case 40225U: phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.State = Val; break;
    case 40226U: phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.Rise = Val; break;
    case 40227U: phMIX->psCfg->asChCfgs[ 1 ].sMasksDIM.Fall = Val; break;
    case 40228U: ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 1 ].MaskUsage )[ 0 ] = Val; break;
    case 40229U: ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 1 ].MaskUsage )[ 1 ] = Val; break;
    case 40230U: phMIX->psCfg->asChCfgs[ 1 ].eLogicOperation = Val; break;
    case 40240U: phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.StXOR = Val; break;
    case 40241U: phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.State = Val; break;
    case 40242U: phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.Rise = Val; break;
    case 40243U: phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.Fall = Val; break;
    case 40244U: phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.StXOR = Val; break;
    case 40245U: phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.State = Val; break;
    case 40246U: phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.Rise = Val; break;
    case 40247U: phMIX->psCfg->asChCfgs[ 2 ].sMasksDIM.Fall = Val; break;
    case 40248U: ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 2 ].MaskUsage )[ 0 ] = Val; break;
    case 40249U: ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 2 ].MaskUsage )[ 1 ] = Val; break;
    case 40250U: phMIX->psCfg->asChCfgs[ 2 ].eLogicOperation = Val; break;
    case 40260U: phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.StXOR = Val; break;
    case 40261U: phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.State = Val; break;
    case 40262U: phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.Rise = Val; break;
    case 40263U: phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.Fall = Val; break;
    case 40264U: phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.StXOR = Val; break;
    case 40265U: phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.State = Val; break;
    case 40266U: phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.Rise = Val; break;
    case 40267U: phMIX->psCfg->asChCfgs[ 3 ].sMasksDIM.Fall = Val; break;
    case 40268U: ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 3 ].MaskUsage )[ 0 ] = Val; break;
    case 40269U: ( (uint16_t *) &phMIX->psCfg->asChCfgs[ 3 ].MaskUsage )[ 1 ] = Val; break;
    case 40270U: phMIX->psCfg->asChCfgs[ 3 ].eLogicOperation = Val; break;
    // DOM config registers
    case 40500U: phDOM->psCfg->asChCfg[ 0 ].uAct.RegSrcID = Val; break;
    case 40501U: phDOM->psCfg->asChCfg[ 0 ].uDeact.RegSrcID = Val; break;
    case 40502U: phDOM->psCfg->asChCfg[ 0 ].uCfgTDA.RegTimCgf = Val; break;
    case 40503U: phDOM->psCfg->asChCfg[ 0 ].uCfgTHO.RegTimCgf = Val; break;
    case 40504U: phDOM->psCfg->asChCfg[ 1 ].uAct.RegSrcID = Val; break;
    case 40505U: phDOM->psCfg->asChCfg[ 1 ].uDeact.RegSrcID = Val; break;
    case 40506U: phDOM->psCfg->asChCfg[ 1 ].uCfgTDA.RegTimCgf = Val; break;
    case 40507U: phDOM->psCfg->asChCfg[ 1 ].uCfgTHO.RegTimCgf = Val; break;
    case 40508U: phDOM->psCfg->asChCfg[ 2 ].uAct.RegSrcID = Val; break;
    case 40509U: phDOM->psCfg->asChCfg[ 2 ].uDeact.RegSrcID = Val; break;
    case 40510U: phDOM->psCfg->asChCfg[ 2 ].uCfgTDA.RegTimCgf = Val; break;
    case 40511U: phDOM->psCfg->asChCfg[ 2 ].uCfgTHO.RegTimCgf = Val; break;
    case 40512U: phDOM->psCfg->asChCfg[ 3 ].uAct.RegSrcID = Val; break;
    case 40513U: phDOM->psCfg->asChCfg[ 3 ].uDeact.RegSrcID = Val; break;
    case 40514U: phDOM->psCfg->asChCfg[ 3 ].uCfgTDA.RegTimCgf = Val; break;
    case 40515U: phDOM->psCfg->asChCfg[ 3 ].uCfgTHO.RegTimCgf = Val; break;
    case 40516U: phDOM->psCfg->OutsMaskXOR = Val; break;
    default:     // Unsupported holding register address.
      _Res = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return _Res;
}

/**  Modbus Mapping for Digital Input/Output Module
───────────────────────────────────────────────────────────────────────────────
 Function  | Address Range | Access | Description
───────────┼───────────────┼────────┼──────────────────────────────────────────
 FC01/05   | 00000..00003  | R/W    | Digital outputs (coils)
           |               |        | Read: phDOM->OutStates
           |               |        | Write: Activate/Deactivate
           |               |        | bits 0..3
───────────┼───────────────┼────────┼──────────────────────────────────────────
 FC05 only | 00020..00023  | W      | KeepActive control bits
           |               |        | (phDOM->sProtCtrl.KeepActive)
───────────┼───────────────┼────────┼──────────────────────────────────────────
 FC05 only | 00040..00043  | W      | KeepInactive control bits
           |               |        | (phDOM->sProtCtrl.KeepInactive)
───────────┼───────────────┼────────┼──────────────────────────────────────────
 FC02      | 10000..10015  | R      | Digital inputs:
           |               |        | phDIM->sOutsDIM.States
           |               |        | bits 0..15
───────────┼───────────────┼────────┼──────────────────────────────────────────
 FC04      | 30000         | R      | Input reg: phDIM->sOutsDIM.States
           | 30001         | R      | Input reg: phMIX->sOutsMIX.States
           | 30002         | R      | Input reg: phDOM->OutStates
───────────┼───────────────┼────────┼──────────────────────────────────────────
 FC03/06   | 40000         | R/W    | Holding reg: phDOM->sProtCtrl.KeepInactive
           | 40001         | R/W    | Holding reg: phDOM->sProtCtrl.KeepActive
           | 40002         | R/W    | Holding reg: phDOM->sProtCtrl.Deactivate
           | 40003         | R/W    | Holding reg: phDOM->sProtCtrl.Activate
───────────────────────────────────────────────────────────────────────────────
 */
