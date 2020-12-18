/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"
#include "syslog.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbuser.h"
#include "mbframe.h"

#include "mbcrc.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_SER_PDU_SIZE_MIN     14       /*!< Minimum size of a Modbus USER frame. */
#define MB_SER_PDU_SIZE_MAX     14     /*!< Maximum size of a Modbus USER frame. */
#define MB_SER_PDU_SIZE_CRC     0      /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */

#define MB_SER_PDU_PDU_OFF      2      /*!< Offset of Modbus-PDU in Ser-PDU. */

#define MB_SER_PDU_HAED_SIZE     2
#define MB_SER_PDU_TAIL_SIZE     2

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    STATE_RX_INIT,              /*!< Receiver is in initial state. */
    STATE_RX_IDLE,              /*!< Receiver is in idle state. */
    STATE_RX_RCV,               /*!< Frame is beeing received. */
    STATE_RX_ERROR              /*!< If the frame is invalid. */
} eMBRcvState;

typedef enum
{
    STATE_TX_IDLE,              /*!< Transmitter is in idle state. */
    STATE_TX_XMIT               /*!< Transmitter is in transfer state. */
} eMBSndState;

/* ----------------------- Static variables ---------------------------------*/
static volatile eMBSndState eSndState;
static volatile eMBRcvState eRcvState;

volatile UCHAR  ucUSERBuf[MB_SER_PDU_SIZE_MAX];

static volatile UCHAR *pucSndBufferCur;
static volatile USHORT usSndBufferCount;

static volatile USHORT usRcvBufferPos;

/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBUSERInit( UCHAR ucSlaveAddress, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity )
{
    LOG_I("enter eMBUSERInit");
    eMBErrorCode    eStatus = MB_ENOERR;
    ULONG           usTimerT35_50us;

    ( void )ucSlaveAddress;
    ENTER_CRITICAL_SECTION(  );

    /* Modbus USER uses 8 Databits. */
    if( xMBPortSerialInit( ucPort, ulBaudRate, 8, eParity ) != TRUE )
    {
        eStatus = MB_EPORTERR;
    }
    else
    {
        /* If baudrate > 19200 then we should use the fixed timer values
         * t35 = 1750us. Otherwise t35 must be 3.5 times the character time.
         */
        if( ulBaudRate > 19200 )
        {
            usTimerT35_50us = 35;       /* 1800us. */
        }
        else
        {
            /* The timer reload value for a character is given by:
             *
             * ChTimeValue = Ticks_per_1s / ( Baudrate / 10 )
             *             = 10 * Ticks_per_1s / Baudrate
             *             = 200000 / Baudrate
             * The reload for t3.5 is 1.5 times this value and similary
             * for t3.5.
             * current no eParity, so a frame is 10 bit, otherwise 11 bit
             */
            usTimerT35_50us = ( 7UL * 200000UL ) / ( 2UL * ulBaudRate );
        }
        if( xMBPortTimersInit( ( USHORT ) usTimerT35_50us ) != TRUE )
        {
            eStatus = MB_EPORTERR;
        }
    }
    EXIT_CRITICAL_SECTION(  );

    return eStatus;
}

void
eMBUSERStart( void )
{
    ENTER_CRITICAL_SECTION(  );
    /* Initially the receiver is in the state STATE_RX_INIT. we start
     * the timer and if no character is received within t3.5 we change
     * to STATE_RX_IDLE. This makes sure that we delay startup of the
     * modbus protocol stack until the bus is free.
     */
    eRcvState = STATE_RX_INIT;
    vMBPortSerialEnable( TRUE, FALSE );
    vMBPortTimersEnable(  );

    EXIT_CRITICAL_SECTION(  );
}

void
eMBUSERStop( void )
{
    ENTER_CRITICAL_SECTION(  );
    vMBPortSerialEnable( FALSE, FALSE );
    vMBPortTimersDisable(  );
    EXIT_CRITICAL_SECTION(  );
}

BOOL is_correct_frame(UCHAR *frame, uint32_t BufferPos)
{
    uint8_t h0 = frame[0];
    uint8_t h1 = frame[1];
    uint8_t h12 =frame[12];
    uint8_t h13 =frame[13];

    LOG_I("[MBU]BufferPos:%d", BufferPos);
    if (h0  == 0x55 && h1 == 0xaa && h12 == 0x5a && h13 == 0xa5) {
        LOG_I("OK frame");
        return TRUE;
    } else {
        return FALSE;
    }
}

eMBErrorCode
eMBUSERReceive( UCHAR * pucRcvAddress, UCHAR ** pucFrame, USHORT * pusLength )
{
    BOOL            xFrameReceived = FALSE;
    eMBErrorCode    eStatus = MB_ENOERR;

    ENTER_CRITICAL_SECTION(  );
    assert( usRcvBufferPos < MB_SER_PDU_SIZE_MAX );
    hex_dump("ucUSERBuf", ucUSERBuf, 14);
    if( ( usRcvBufferPos >= MB_SER_PDU_SIZE_MIN )
        && ( is_correct_frame((UCHAR *)ucUSERBuf, usRcvBufferPos) == TRUE ))
    {
        /* Save the address field. All frames are passed to the upper layed
         * and the decision if a frame is used is done there.
         */
        //*pucRcvAddress = ucUSERBuf[MB_SER_PDU_ADDR_OFF];

        /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
         * size of address field and CRC checksum.
         */
        *pusLength = ( USHORT )(usRcvBufferPos);

        /* Return the start of the Modbus PDU to the caller. */
        *pucFrame = ( UCHAR * ) & ucUSERBuf[0];
        xFrameReceived = TRUE;
    }
    else
    {
        eStatus = MB_EIO;
    }

    EXIT_CRITICAL_SECTION(  );
    LOG_I("[MB] eMBUSERReceive return %d", eStatus);
    return eStatus;
}

eMBErrorCode
eMBUSERSend( UCHAR ucSlaveAddress, const UCHAR * pucFrame, USHORT usLength )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          usCRC16;

    LOG_I("[MB] eMBUSERSend before critical");
    ENTER_CRITICAL_SECTION(  );

    /* Check if the receiver is still in idle state. If not we where to
     * slow with processing the received frame and the master sent another
     * frame on the network. We have to abort sending the frame.
     */
    if( eRcvState == STATE_RX_IDLE )
    {
        /* First byte before the Modbus-PDU is the slave address. */
        pucSndBufferCur = ( UCHAR * ) pucFrame;
        usSndBufferCount = 0;

        /* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
        usSndBufferCount += usLength;

        /* Activate the transmitter. */
        eSndState = STATE_TX_XMIT;
        vMBPortSerialEnable( FALSE, TRUE );
    }
    else
    {
        eStatus = MB_EIO;
    }
    EXIT_CRITICAL_SECTION(  );
    LOG_I("[MB] eMBUSERSend exit critical");
    return eStatus;
}

BOOL
xMBUSERReceiveFSM( void )
{
    BOOL            xTaskNeedSwitch = FALSE;
    UCHAR           ucByte;

    assert( eSndState == STATE_TX_IDLE );

    /* Always read the character. */
    ( void )xMBPortSerialGetByte( ( CHAR * ) & ucByte );

    switch ( eRcvState )
    {
        /* If we have received a character in the init state we have to
         * wait until the frame is finished.
         */
    case STATE_RX_INIT:
        vMBPortTimersEnable(  );
        break;

        /* In the error state we wait until all characters in the
         * damaged frame are transmitted.
         */
    case STATE_RX_ERROR:
        vMBPortTimersEnable(  );
        break;

        /* In the idle state we wait for a new character. If a character
         * is received the t1.5 and t3.5 timers are started and the
         * receiver is in the state STATE_RX_RECEIVCE.
         */
    case STATE_RX_IDLE:
        usRcvBufferPos = 0;
        ucUSERBuf[usRcvBufferPos++] = ucByte;
        eRcvState = STATE_RX_RCV;

        /* Enable t3.5 timers. */
        vMBPortTimersEnable(  );
        break;

        /* We are currently receiving a frame. Reset the timer after
         * every character received. If more than the maximum possible
         * number of bytes in a modbus frame is received the frame is
         * ignored.
         */
    case STATE_RX_RCV:
        if( usRcvBufferPos < MB_SER_PDU_SIZE_MAX )
        {
            ucUSERBuf[usRcvBufferPos++] = ucByte;
        }
        else
        {
            eRcvState = STATE_RX_ERROR;
        }
        vMBPortTimersEnable(  );
        break;
    }
    return xTaskNeedSwitch;
}

BOOL
xMBUSERTransmitFSM( void )
{
    BOOL            xNeedPoll = FALSE;

    assert( eRcvState == STATE_RX_IDLE );

    switch ( eSndState )
    {
        /* We should not get a transmitter event if the transmitter is in
         * idle state.  */
    case STATE_TX_IDLE:
        /* enable receiver/disable transmitter. */
        vMBPortSerialEnable( TRUE, FALSE );
        break;

    case STATE_TX_XMIT:
        /* check if we are finished. */
        if( usSndBufferCount != 0 )
        {
            xMBPortSerialPutByte( ( CHAR )*pucSndBufferCur );
            pucSndBufferCur++;  /* next byte in sendbuffer. */
            usSndBufferCount--;
        }
        else
        {
            xNeedPoll = xMBPortEventPost( EV_FRAME_SENT );
            /* Disable transmitter. This prevents another transmit buffer
             * empty interrupt. */
            vMBPortSerialEnable( TRUE, FALSE );
            eSndState = STATE_TX_IDLE;
        }
        break;
    }

    return xNeedPoll;
}

BOOL
xMBUSERTimerT35Expired( void )
{
    BOOL            xNeedPoll = FALSE;
    LOG_I("xMBUSERTimerT35Expired");
    switch ( eRcvState )
    {
        /* Timer t35 expired. Startup phase is finished. */
    case STATE_RX_INIT:
        xNeedPoll = xMBPortEventPost( EV_READY );
        break;

        /* A frame was received and t35 expired. Notify the listener that
         * a new frame was received. */
    case STATE_RX_RCV:
        xNeedPoll = xMBPortEventPost( EV_FRAME_RECEIVED );
        break;

        /* An error occured while receiving the frame. */
    case STATE_RX_ERROR:
        break;

        /* Function called in an illegal state. */
    default:
        assert( ( eRcvState == STATE_RX_INIT ) ||
                ( eRcvState == STATE_RX_RCV ) || ( eRcvState == STATE_RX_ERROR ) );
    }

    vMBPortTimersDisable(  );
    eRcvState = STATE_RX_IDLE;

    return xNeedPoll;
}
