#include "main.h"
#include "microtbx.h"
#include "microtbxmodbus.h"

typedef tTbxMbServerResult FnRes_t;

/** Function prototypes. ------------------------------------------------------------- */
static FnRes_t _WriteCoil( tTbxMbServer channel, uint16_t addr, uint8_t value );
static FnRes_t _ReadInput( tTbxMbServer channel, uint16_t addr, uint8_t *value );
static FnRes_t _ReadInputReg( tTbxMbServer channel, uint16_t addr, uint16_t *value );
static FnRes_t _WriteHoldingReg( tTbxMbServer channel, uint16_t addr, uint16_t value );

/** Local data declarations. --------------------------------------------------------- */
static tTbxMbTp     phTpMB;      // Modbus RTU transport layer handle.
static tTbxMbServer phSrvMB;     // Modbus server channel handle.

/** -------------------------------------------------------------------------
 * @brief
 */
void MB_RTU_Slave_Init( void ) {
  /**
   * Construct a Modbus RTU transport layer object.
   * Make sure the transport layer was created successfully.
   * Construct a Modbus server object.
   */
  phTpMB = TbxMbRtuCreate( 10U, TBX_MB_UART_PORT1, TBX_MB_UART_19200BPS,     //
                           TBX_MB_UART_1_STOPBITS, TBX_MB_EVEN_PARITY );
  TBX_ASSERT( phTpMB );
  if ( !phTpMB ) return;

  phSrvMB = TbxMbServerCreate( phTpMB );
  TBX_ASSERT( phSrvMB );
  if ( !phSrvMB ) return;

  /* Set the callbacks for accessing the Modbus data tables. */
  TbxMbServerSetCallbackWriteCoil( phSrvMB, _WriteCoil );
  TbxMbServerSetCallbackReadInput( phSrvMB, _ReadInput );
  TbxMbServerSetCallbackReadInputReg( phSrvMB, _ReadInputReg );
  TbxMbServerSetCallbackWriteHoldingReg( phSrvMB, _WriteHoldingReg );

  /* Continuously call the Modbus stack event task function. */
  // for ( ;; ) { TbxMbEventTask( ); }

  return;
}

/****************************************************************************************
** \brief     Writes a data element to the coils data table.
** \details   Note that the element is specified by its zero-based address in the range
**            0 - 65535, not its element number (1 - 65536).
** \param     channel Handle to the Modbus server channel object that triggered the
**            callback.
** \param     addr Element address (0..65535).
** \param     value Coil value. Use TBX_ON to activate the coil, TBX_OFF otherwise.
** \return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
**            specific data element address is not supported by this server,
**            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
**
****************************************************************************************/
static FnRes_t _WriteCoil( tTbxMbServer channel, uint16_t addr, uint8_t value ) {
  //
  FnRes_t result = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( channel );
  switch ( addr ) {
    case 0U: break;     // BspDigitalOut(BSP_DIGITAL_OUT2, value);
    case 1U: break;     //
    default:            // Unsupported coil address.
      result = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return result;
}

/****************************************************************************************
** \brief     Reads a data element from the discrete inputs data table.
** \details   Note that the element is specified by its zero-based address in the range
**            0 - 65535, not its element number (1 - 65536).
** \param     channel Handle to the Modbus server channel object that triggered the
**            callback.
** \param     addr Element address (0..65535).
** \param     value Pointer to write the value of the input to. Use TBX_ON if the input
**            is on, TBX_OFF otherwise.
** \return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
**            specific data element address is not supported by this server,
**            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
**
****************************************************************************************/
static FnRes_t _ReadInput( tTbxMbServer channel, uint16_t addr, uint8_t *value ) {
  //
  FnRes_t result = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( channel );
  switch ( addr ) {
    case 10000U: break;     // *value = BspDigitalIn(BSP_DIGITAL_IN1);
    case 10001U: break;     // *value = BspDigitalIn(BSP_DIGITAL_IN2);
    default:                // Unsupported discrete input address.
      result = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return result;
}

/****************************************************************************************
** \brief     Reads a data element from the input registers data table.
** \details   Write the value of the input register in your CPUs native endianess. The
**            MicroTBX-Modbus stack will automatically convert this to the big endianess
**            that the Modbus protocol requires.
**            Note that the element is specified by its zero-based address in the range
**            0 - 65535, not its element number (1 - 65536).
** \param     channel Handle to the Modbus server channel object that triggered the
**            callback.
** \param     addr Element address (0..65535).
** \param     value Pointer to write the value of the input register to.
** \return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
**            specific data element address is not supported by this server,
**            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
**
****************************************************************************************/
static FnRes_t _ReadInputReg( tTbxMbServer channel, uint16_t addr, uint16_t *value ) {
  //
  FnRes_t result = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( channel );
  switch ( addr ) {
    case 30000U: *value = 0; break;
    case 30001U: *value = 0; break;
    default:     // Unsupported input register address.
      result = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return result;
}

/****************************************************************************************
** \brief     Writes a data element to the holding registers data table.
** \details   Note that the element is specified by its zero-based address in the range
**            0 - 65535, not its element number (1 - 65536).
**            The value of the holding register in already in your CPUs native endianess.
** \param     channel Handle to the Modbus server channel object that triggered the
**            callback.
** \param     addr Element address (0..65535).
** \param     value Value of the holding register.
** \return    TBX_MB_SERVER_OK if successful, TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR if the
**            specific data element address is not supported by this server,
**            TBX_MB_SERVER_ERR_DEVICE_FAILURE otherwise.
**
****************************************************************************************/
static FnRes_t _WriteHoldingReg( tTbxMbServer channel, uint16_t addr, uint16_t value ) {
  //
  FnRes_t result = TBX_MB_SERVER_OK;
  TBX_UNUSED_ARG( channel );
  switch ( addr ) {
    case 40000U: result = TBX_MB_SERVER_ERR_DEVICE_FAILURE; break;
    case 40001U: result = TBX_MB_SERVER_ERR_DEVICE_FAILURE; break;
    default:     // Unsupported holding register address.
      result = TBX_MB_SERVER_ERR_ILLEGAL_DATA_ADDR;
      break;
  }

  return result;
}
