/*==============================================================================*/
/** @file       MePort.c
    @brief      This file holds all interface functions to the MeComAPI
    @author     Meerstetter Engineering GmbH: Thomas Braun, Marc Luethi

    Please only modify these functions to implement the MeComAPI into
    your system. It should not be necessary to modify any files in the
    private folder.

*/

/*==============================================================================*/
/*                          IMPORT                                              */
/*==============================================================================*/
#include "MePort.h"
#include "private/MeFrame.h"
#ifndef ARDUINO
#include "../ComPort/ComPort.h"
#include "../TcpSocket/TcpSocket.h"
#else
#include "../ArduinoSerial/ArduinoSerial.h"
#endif

//These include files can be removed, depending on the target system
#ifdef _WIN32 // Includes both 32 bit and 64 bit
#include <windows.h> //Used for Sleep function
#elif !defined(ARDUINO)
//Some of these include files can be removed, depending on the target system
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#endif

#ifndef ARDUINO
#include <stdio.h>   //Used for printf function
#endif

/*==============================================================================*/
/*                          DEFINITIONS/DECLARATIONS                            */
/*==============================================================================*/


/*==============================================================================*/
/*                          STATIC FUNCTION PROTOTYPES                          */
/*==============================================================================*/

/*==============================================================================*/
/*                          EXTERN VARIABLES                                    */
/*==============================================================================*/
#ifndef ARDUINO
extern MePort_InterfaceType InterfaceType; // Variable is defined and set in "main.c"
#endif

/*==============================================================================*/
/*                          STATIC VARIABLES                                    */
/*==============================================================================*/
#ifdef _WIN32
static volatile uint8_t SemaphorHandler = 0;
#elif !defined(ARDUINO)
static pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t Condition = PTHREAD_COND_INITIALIZER;
#endif

/*==============================================================================*/
/** @brief      Interface Function: Send Byte
 *
 *  For the example target system, this function collects all the given bytes
 *  and generates a string. If the frame send function sends the last byte
 *  to this function, the string is being given to the Comport function.
 *
 *  For example in case of a microcontroller, it is also possible to
 *  pass every single byte directly to the Comport function.
 *
 *  In case of an RS485 Interface it can be helpful to use the
 *  "MePort_SB_IsFirstByte" case to enable the RS485 TX Signal
 *  and "MePort_SB_IsLastByte" to disable the TX Signal
 *  after the last byte has been sent.
 *
*/
void MePort_SendByte(int8_t in, MePort_SB FirstLast)
{
    static char Buffer[MEPORT_MAX_TX_BUF_SIZE];
    static int Ctr;

    switch (FirstLast)
    {
    case MePort_SB_IsFirstByte:
        //This is the first Byte of the Message String 
        Ctr = 0;
        Buffer[Ctr] = in;
        Ctr++;
        break;
    case MePort_SB_Normal:
        //These are some middle Bytes
        if (Ctr < MEPORT_MAX_TX_BUF_SIZE - 1)
        {
            Buffer[Ctr] = in;
            Ctr++;
        }
        break;
    case MePort_SB_IsLastByte:
        //This is the last Byte of the Message String
        if (Ctr < MEPORT_MAX_TX_BUF_SIZE - 1)
        {
            Buffer[Ctr] = in;
            Ctr++;
            Buffer[Ctr] = 0;
            Ctr++;

#ifndef ARDUINO
            if (InterfaceType == TcpSocket)
            {
                Socket_Send(Buffer);

                char RcvBuffer[1000] = ""; // Initialize an empty string
                Socket_Receive(RcvBuffer, sizeof(RcvBuffer));
            }
            else // InterfaceType == ComPort or nothing defined
            {
                ComPort_Send(Buffer);
            }
#else
            ArduinoSerial_Send(Buffer);
#endif
        }
        break;
    }
}
/*==============================================================================*/
/** @brief      Interface Function: Receive Byte
 *
 *  For the example target system, this function just calls the function
 *  "MeFrame_Receive" for every received char in the given string.
 *
 *  It is also possible to modify the function prototype of this function,
 *  to just receive one single byte. (For example in case of an MCU)
*/
void MePort_ReceiveByte(int8_t* arr)
{
    while (*arr)
    {
#ifndef _WIN32
        if (*arr == '\n') *arr = '\r';
#endif
        MeFrame_Receive(*arr);
        arr++;
    }
}
/*==============================================================================*/
/** @brief      Interface Function: SemaphorTake
 *
 *  This function is being called by the Query and Set functions,
 *  while these functions are waiting for an answer of the connected device.
 *
 *  A timeout variable in milliseconds is passed to this function. The user
 *  implementation has to make sure that after this timeout has ran out,
 *  the function ends, even if no data has been received, otherwise the
 *  system will be stuck forever.
 *
 *  For the example target System a very simple Semaphore functionality has
 *  been implemented. It is strongly recommended to use the proper Operating
 *  System functions to reach optimal system performance.
 *
 *  It is also possible to run this API without an operating system:
 *  - Use a simple delay or timer function of an MCU and the
 *    data receiving function is being called by an interrupt
 *    routine of the UART interface.
 *  - Use a simple delay or timer function of an MCU to have a time base
 *    and poll the UART interface to check if some bytes have been received.
 *
*/
#ifndef ARDUINO
void MePort_SemaphorTake(uint32_t TimeoutMs)
{
#ifdef _WIN32
    while (SemaphorHandler == 0)
    {
        Sleep(10);
        if (TimeoutMs > 0) TimeoutMs -= 10; else return;
    }
    SemaphorHandler = 0;
#else
    struct timespec Timeout;

    // Set Timeout
    clock_gettime(CLOCK_REALTIME, &Timeout);
    Timeout.tv_sec += TimeoutMs / 1000;
    Timeout.tv_nsec += (TimeoutMs % 1000) * 1000000;
    if (Timeout.tv_nsec >= 1000000000L)
    {
        Timeout.tv_sec++;
        Timeout.tv_nsec = Timeout.tv_nsec - 1000000000L;
    }

    // Wait for Data
    pthread_mutex_lock(&Mutex);
    pthread_cond_timedwait(&Condition, &Mutex, &Timeout);
    pthread_mutex_unlock(&Mutex);
#endif
}
#endif
/*==============================================================================*/
/** @brief      Interface Function: SemaphorGive
 *
 *  This function is being called by the Frame receiving function, as soon as a
 *  complete frame has been received.
 *
 *  For the example target System a very simple Semaphore functionality has
 *  been implemented. It is strongly recommended to use the proper Operating
 *  System functions to reach optimal system performance.
 *
*/
#ifndef ARDUINO
void MePort_SemaphorGive(void)
{
#ifdef _WIN32
    SemaphorHandler = 1;
#else
    pthread_mutex_lock(&Mutex);
    pthread_cond_signal(&Condition);
    pthread_mutex_unlock(&Mutex);
#endif
}
#endif

/*==============================================================================*/
/** @brief      Interface Function: ErrorThrow
 *
 *  This function is being called by the Query and Set functions when
 *  something went wrong.
 *
 *  For the example target System a simple console print out has been added.
 *
 *  It is recommended to forward this error Numbers to your error Management system.
 *
*/
#ifndef ARDUINO
void MePort_ErrorThrow(int32_t ErrorNr)
{
    switch (ErrorNr)
    {
    case MEPORT_ERROR_CMD_NOT_AVAILABLE:
        printf("MePort Error: Command not available\n");
        break;

    case MEPORT_ERROR_DEVICE_BUSY:
        printf("MePort Error: Device is Busy\n");
        break;

    case MEPORT_ERROR_GENERAL_COM:
        printf("MePort Error: General Error\n");
        break;

    case MEPORT_ERROR_FORMAT:
        printf("MePort Error: Format Error\n");
        break;

    case MEPORT_ERROR_PAR_NOT_AVAILABLE:
        printf("MePort Error: Parameter not available\n");
        break;

    case MEPORT_ERROR_PAR_NOT_WRITABLE:
        printf("MePort Error: Parameter not writable\n");
        break;

    case MEPORT_ERROR_PAR_OUT_OF_RANGE:
        printf("MePort Error: Parameter out of Range\n");
        break;

    case MEPORT_ERROR_PAR_INST_NOT_AVAILABLE:
        printf("MePort Error: Parameter Instance not available\n");
        break;

    case MEPORT_ERROR_SET_TIMEOUT:
        printf("MePort Error: Set Timeout\n");
        break;

    case MEPORT_ERROR_QUERY_TIMEOUT:
        printf("MePort Error: Query Timeout\n");
        break;
    }
}
#endif
