/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (c) 2006-2018 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "app_event.h"
#include "motor.h"
#include "syslog.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"

/* ----------------------- Defines ------------------------------------------*/
#define FUNC_MOTOR_1 0x01
#define FUNC_MOTOR_2 0x02
#define FUNC_MOTOR_3 0x03
#define FUNC_MOTOR_4 0x04

#define FUNC_VALVE_1 0x05
#define FUNC_VALVE_2 0x06
#define FUNC_VALVE_3 0x07
#define FUNC_VALVE_4 0x08
#define FUNC_VALVE_5 0x09
#define FUNC_VALVE_6 0x0a

#define FUNC_TEMP       0x0b
#define FUNC_CLOSE_ALL  0x0d


#define OPERAT_FWD              0xff
#define OPERAT_REV              0x00
#define OPERAT_RUN              0xa1
#define OPERAT_STOP             0xb1
#define OPERAT_OPEN             0xff
#define OPERAT_CLOSE            0x00
#define OPERAT_TEMP_CONTROL_ON  0xff
#define OPERAT_TEMP_CONTROL_OFF 0x00

#define BIT_ID      2
#define BIT_FUNC    3
#define BIT_OPERAT  4
#define BIT_DATA    5

/* ----------------------- Static functions ---------------------------------*/
eMBException    prveMBError2Exception( eMBErrorCode eErrorCode );

/* ----------------------- Start implementation -----------------------------*/
eMBException
eMBFuncUser( UCHAR * pucFrame, USHORT * usLen )
{
    UCHAR          *pucFrameCur;
    eMBException    eStatus = MB_EX_NONE;
    uint32_t data = 0; 
    data = (pucFrame[BIT_DATA]<<16) | (pucFrame[BIT_DATA+1]<<8) | pucFrame[BIT_DATA+2];
    LOG_I("[MB] frame data:[%d]", data);

    if (pucFrame[BIT_FUNC] >= FUNC_MOTOR_1 &&
        pucFrame[BIT_FUNC] <= FUNC_MOTOR_4) {

        /*motor run stop mode*/
        if (pucFrame[BIT_OPERAT] == OPERAT_RUN ||
            pucFrame[BIT_OPERAT] == OPERAT_STOP) {

            motor_run_stop_t m_run_stop = {0};
            m_run_stop.motor_id = pucFrame[BIT_FUNC];
            if (pucFrame[BIT_OPERAT] == OPERAT_RUN) {
                m_run_stop.state = MOTOR_RUN;
            } else {
                m_run_stop.state = MOTOR_STOP;
            }

            event_callback(EVENT_MOTOR_RUN_STOP, (event_param_t *)&m_run_stop);
        /*motor step mode*/
        } else if (pucFrame[BIT_OPERAT] == OPERAT_FWD ||
                   pucFrame[BIT_OPERAT] == OPERAT_REV) {

            motor_step_t m_step = {0};

            m_step.motor_id = pucFrame[BIT_FUNC];

            if (pucFrame[BIT_OPERAT] == OPERAT_FWD) {
                m_step.dir = DIRECTION_FWD;
            } else {
                m_step.dir = DIRECTION_REV;
            }
            m_step.step = data;

            event_callback(EVENT_MOTOR_STEPS, (event_param_t *)&m_step);
        } else {}

    } else if (pucFrame[BIT_FUNC] >= FUNC_VALVE_1 &&
               pucFrame[BIT_FUNC] <= FUNC_VALVE_6) {

        valve_open_close_t v_open_close = {0};
        v_open_close.valve_id = pucFrame[BIT_FUNC];

        if (pucFrame[BIT_OPERAT] == OPERAT_OPEN) {
            v_open_close.state = VALVE_STATE_OPEN;
        } else {
            v_open_close.state = VALVE_STATE_CLOSE;
        }
        event_callback(EVENT_VALVE_OPEN_CLOSE, (event_param_t *)&v_open_close);

    } else if (pucFrame[BIT_FUNC] == FUNC_CLOSE_ALL) {
        event_callback(EVENT_STOP_ALL, NULL);

    } else if (pucFrame[BIT_FUNC] == FUNC_TEMP) {
        LOG_E("Not support temperature control now");
    } else {}

    return eStatus;
}
