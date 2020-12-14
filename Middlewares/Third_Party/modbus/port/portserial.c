#include <stdlib.h>
#include "cmsis_os.h"
#include "usart.h"
#include "syslog.h"

#include "mb.h"
#include "port.h"
#include "mbport.h"

extern UART_HandleTypeDef huart3;
UCHAR ch_rx;

void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    ENTER_CRITICAL_SECTION();
    //usart3_enable(xRxEnable, xTxEnable);
    if (xRxEnable) {
        __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
    } else {
        __HAL_UART_DISABLE_IT(&huart3, UART_IT_RXNE);
    }
    if (xTxEnable) {
        __HAL_UART_ENABLE_IT(&huart3, UART_IT_TXE);
    } else {
        __HAL_UART_DISABLE_IT(&huart3, UART_IT_TXE);
    }
    EXIT_CRITICAL_SECTION();
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    (void *)ucPORT;
    (void *)ucDataBits;
    (void *)eParity;

    BOOL bStatus = FALSE;
    MX_USART3_UART_Init(ulBaudRate);
    HAL_UART_Receive_IT(&huart3, &ch_rx, 1);
    bStatus = TRUE;
    LOG_I("enter xMBPortSerialInit");
    return bStatus;
}

void
vMBPortSerialClose( void )
{
    HAL_UART_MspDeInit(&huart3);
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    //huart3.Instance->DR = (ucByte & (uint8_t)0xFF);
    //return TRUE;
    return (HAL_OK == HAL_UART_Transmit(&huart3, (uint8_t*)&ucByte, 1, 10));
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = huart3.Instance->DR;
    return TRUE;
}

void
vUSARTHandler( void )
{
#if 0
    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE)) {
        pxMBFrameCBByteReceived();
    }
    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TXE)) {
        pxMBFrameCBTransmitterEmpty();
    }
#endif

    uint32_t tmp_flag = __HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE);
    uint32_t tmp_it_source = __HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_RXNE);

    if ((tmp_flag != RESET) && (tmp_it_source != RESET)) {
        pxMBFrameCBByteReceived();
        __HAL_UART_CLEAR_PEFLAG(&huart3);
        return;
    }

    if ((__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TXE) != RESET) &&(__HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_TXE) != RESET)) {
        pxMBFrameCBTransmitterEmpty();
        return ;
    }
}
