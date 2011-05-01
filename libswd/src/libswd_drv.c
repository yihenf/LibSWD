/*
 * $Id$
 *
 * Serial Wire Debug Open Library.
 * Library Body File.
 *
 * Copyright (C) 2010-2011, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the Tomasz Boleslaw CEDRO nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.*
 *
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010-2011;
 *
 */

/** \file libswd_drv.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup swd_drv SWD Bus and Interface Driver Transfer Functions that
 * executes command queue.
 * @{
 ******************************************************************************/

extern int swd_drv_mosi_8(swd_ctx_t *swdctx, swd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_32(swd_ctx_t *swdctx, swd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
extern int swd_drv_miso_8(swd_ctx_t *swdctx, swd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
extern int swd_drv_miso_32(swd_ctx_t *swdctx, swd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_trn(swd_ctx_t *swdctx, int bits);
extern int swd_drv_miso_trn(swd_ctx_t *swdctx, int bits);

/** Transmit selected command from the command queue to the interface driver.
 * \param *swdctx swd context pointer.
 * \param *cmd pointer to the command to be sent.
 * \return number of commands transmitted (1), or SWD_ERROR_CODE on failure.
 */ 
int swd_drv_transmit(swd_ctx_t *swdctx, swd_cmd_t *cmd){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (cmd==NULL) return SWD_ERROR_NULLPOINTER;
  
 int res=SWD_ERROR_BADCMDTYPE;

 switch (cmd->cmdtype){
  case SWD_CMDTYPE_MOSI:
  case SWD_CMDTYPE_MISO:
   swd_log(swdctx, SWD_LOGLEVEL_WARNING, "This command does not contain payload.");
   break;

  case SWD_CMDTYPE_MOSI_CONTROL:
   // 8 clock cycles.
   if (cmd->bits!=8) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, cmd, &cmd->control, 8, SWD_DIR_MSBFIRST);
   swdctx->log.write.control=cmd->control;
   break;

  case SWD_CMDTYPE_MOSI_BITBANG:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, cmd, &cmd->mosibit, 1, SWD_DIR_LSBFIRST);
   swdctx->log.write.control=cmd->mosibit;
   break;

  case SWD_CMDTYPE_MOSI_PARITY:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, cmd, &cmd->parity, 1, SWD_DIR_LSBFIRST);
   swdctx->log.write.parity=cmd->parity;
   break;

  case SWD_CMDTYPE_MOSI_TRN:
   // 1..4-bit clock cycle.
   if (cmd->bits<SWD_TURNROUND_MIN_VAL && cmd->bits>SWD_TURNROUND_MAX_VAL)
    return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_trn(swdctx, cmd->bits);
   break;

  case SWD_CMDTYPE_MOSI_REQUEST:
   // 8 clock cycles.
   if (cmd->bits!=SWD_REQUEST_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, cmd, &cmd->request, 8, SWD_DIR_MSBFIRST);
   swdctx->log.write.request=cmd->request;
   break;

  case SWD_CMDTYPE_MOSI_DATA:
   // 32 clock cycles.
   if (cmd->bits!=SWD_DATA_BITLEN) return SWD_ERROR_BADCMDDATA; 
   res=swd_drv_mosi_32(swdctx, cmd, &cmd->mosidata, 32, SWD_DIR_LSBFIRST);
   swdctx->log.write.data=cmd->mosidata;
   break;

  case SWD_CMDTYPE_MISO_ACK:
   // 3 clock cycles.
   if (cmd->bits!=SWD_ACK_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, cmd, &cmd->ack, cmd->bits, SWD_DIR_LSBFIRST);
   swdctx->log.read.ack=cmd->ack;
   break;

  case SWD_CMDTYPE_MISO_BITBANG:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, cmd, &cmd->misobit, 1, SWD_DIR_LSBFIRST);
   swdctx->log.read.control=cmd->misobit;
   break;

  case SWD_CMDTYPE_MISO_PARITY:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, cmd, &cmd->parity, 1, SWD_DIR_LSBFIRST);
   swdctx->log.read.parity=cmd->parity;
   break;

  case SWD_CMDTYPE_MISO_TRN:
   // 1..4 clock cycles
   if (cmd->bits<SWD_TURNROUND_MIN_VAL && cmd->bits>SWD_TURNROUND_MAX_VAL)
    return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_trn(swdctx, cmd->bits);
   break;

  case SWD_CMDTYPE_MISO_DATA:
   // 32 clock cycles
   if (cmd->bits!=SWD_DATA_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_32(swdctx, cmd, &cmd->misodata, cmd->bits, SWD_DIR_LSBFIRST);
   swdctx->log.read.data=cmd->misodata;
   break;

  case SWD_CMDTYPE_UNDEFINED:
   res=0;
   break; 

  default:
   return SWD_ERROR_BADCMDTYPE;
 } 

 swd_log(swdctx, SWD_LOGLEVEL_DEBUG,
  "SWD_D: swd_drv_transmit(0x%x, 0x%x) bits=%-2d cmdtype=%-12s returns=%-3d payload=0x%08x (%s)\n",
  swdctx, cmd, cmd->bits, swd_cmd_string_cmdtype(cmd), res,
  (cmd->bits>8)?cmd->data32:cmd->data8,
  (cmd->bits<=8)?swd_bin8_string(&cmd->data8):swd_bin32_string(&cmd->data32));

 if (res<0) return res;
 cmd->done=1;
 return res;
}

/** @} */