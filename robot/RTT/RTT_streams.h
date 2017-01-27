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
 * @file    RTT_streams.h
 * @brief   RTT streams structures and macros.

 * @addtogroup rtt_streams
 * @{
 */

#ifndef _RTT_STREAMS_H_
#define _RTT_STREAMS_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief Indicates if connection is made with RTT_CLIENT.
 *    0 -> Connection is made with TELNET
 *    1 -> Connection is made with RTT_CLIENT
 */
#define RTT_CLIENT 1

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   @p RTTStream specific data.
 */
#define _RTT_stream_data                                                \
  _base_sequential_stream_data                                          \
  /* Index of the communication buffer to use */                        \
  unsigned BufferIndex;

/**
 * @brief   @p MemStream virtual methods table.
 */
struct RTTStreamVMT {
  _base_sequential_stream_methods
};

/**
 * @extends BaseSequentialStream
 *
 * @brief RTT stream object.
 */
typedef struct {
  /** @brief Virtual Methods Table.*/
  const struct RTTStreamVMT *vmt;
  _RTT_stream_data
} RTTStream;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void RTTObjectInit(RTTStream *rtt_str, unsigned BufferIndex);
#ifdef __cplusplus
}
#endif

#endif /* _RTT_STREAMS_H_ */

/** @} */
