/*
    File written by Sylvain LE ROUX, based on a template by Giovanni Di Sirio.
    
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    RTT_streams.c
 * @brief   RTT streams code.
 *
 * @addtogroup rtt_streams
 * @{
 */

#include "hal.h"
#include "RTT_streams.h"
#include "SEGGER_RTT.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static size_t writes(void *ip, const uint8_t *bp, size_t n) {
  RTTStream* rtt_str = ip;
  return SEGGER_RTT_Write(rtt_str->BufferIndex, bp, n);
}

static size_t reads(void *ip, uint8_t *bp, size_t n) {
  RTTStream* rtt_str = ip;
  int ret = SEGGER_RTT_Read(rtt_str->BufferIndex, bp, n);
  /* For the shell, 0 means connection closed, which is not the case here.
     WARNING: This prevents the shell to properly close if the host is
     really deconnected. */
  if (ret == 0) {
      *bp = 0;
      return 1;
  }
  /* RTT Client sends '\n' when pressing "Enter" and the shell wants a '\r'
    instead, so convert it.*/
  #if RTT_CLIENT
  if (bp[0] == '\n')
    bp[0] = '\r';
  #endif // RTT_CLIENT
  return ret;
}

static msg_t put(void *ip, uint8_t b) {
  RTTStream* rtt_str = ip;
  unsigned ret = SEGGER_RTT_Write(rtt_str->BufferIndex, &b, 1);
  if (ret == 1)
    return MSG_OK;
  else
    return MSG_RESET;
}

static msg_t get(void *ip) {
  uint8_t b;
  RTTStream* rtt_str = ip;
  while(!SEGGER_RTT_HasData(rtt_str->BufferIndex))
    chThdSleepMilliseconds(1);
  SEGGER_RTT_Read(rtt_str->BufferIndex, &b, 1);
  return b;
}

static const struct RTTStreamVMT vmt = {writes, reads, put, get};

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Memory stream object initialization.
 *
 * @param[out] rtt_str      pointer to the @p RTTStream object to be initialized
 * @param[in] BufferIndex   Index of the RTT buffer to use
 */
void RTTObjectInit(RTTStream *rtt_str, unsigned BufferIndex) {
    rtt_str->BufferIndex    = BufferIndex;
    rtt_str->vmt            = &vmt;
}

/** @} */
