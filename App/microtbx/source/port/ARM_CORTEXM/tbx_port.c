/************************************************************************************/
/**
 * \file         port/ARM_CORTEXM/tbx_port.c
 * \brief        Port specifics source file.
 * \internal
 *----------------------------------------------------------------------------------------
 *                          C O P Y R I G H T
 *----------------------------------------------------------------------------------------
 *   Copyright (c) 2019 by Feaser     www.feaser.com     All rights reserved
 *
 *----------------------------------------------------------------------------------------
 *                            L I C E N S E
 *----------------------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * \endinternal
 ****************************************************************************************/

#include "microtbx.h"     // MicroTBX global header

/* At this point nothing is implemented here. The TbxPortInterruptsXxx functions were
 * implemented in assembly for MISRA compliance. MISRA requires that where assembly
 * language instructions are required, it is recommended that they be encapsulated and
 * isolated in either: (a) assembler functions, (b) C functions or (c) macros.
 * Recommendation (a) was chosen for the TbxPortInterruptsXxx functions. They are located
 * in the compiler specific part of the port.
 */

#include "main.h"
// or #include "cmsis_gcc.h" or "cmsis_armclang.h" залежно від тулчейну
// CPU status register type (PRIMASK fits in 32-bit)
// typedef uint32_t tTbxPortCpuSR;     // def @tbx_types.h

/**
 * @brief   Disable interrupts and return the previous PRIMASK state.
 *          Called when entering a critical section.
 */
tTbxPortCpuSR TbxPortInterruptsDisable( void ) {
  //
  tTbxPortCpuSR prev = __get_PRIMASK( );     // Read current interrupt mask
  __disable_irq( );                          // Disable global interrupts

  return prev;
}

/**
 * @brief   Restore interrupts based on saved PRIMASK state.
 *          Called when leaving a critical section.
 */
void TbxPortInterruptsRestore( tTbxPortCpuSR prevCpuSr ) {
  //
  if ( ( prevCpuSr & 0x1u ) == 0u )     // If interrupts were enabled before,
    __enable_irq( );                    // re-enable them

  return;
}

/*********************************** end of tbx_port.c *********************************/
