/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    bc_com.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "BC_COM_Initialize" and "BC_COM_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "BC_COM_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _BC_COM_H
#define _BC_COM_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "config/default/system/time/sys_time.h"
#include "config/default/library/tcpip/tcpip.h"
#include "config/default/library/tcpip/udp.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
    // DOM-IGNORE-END

    // *****************************************************************************
    // *****************************************************************************
    // Section: Type Definitions
    // *****************************************************************************
    // *****************************************************************************

#define BC_COM_UDP_SERVER_PORT                 47134
#define BC_COM_SERVER_TIME_SLOT                  100

    // *****************************************************************************

    /* Application states

      Summary:
        Application states enumeration

      Description:
        This enumeration defines the valid application states.  These states
        determine the behavior of the application at various times.
     */

    typedef enum {
        BC_COM_STATE_INIT = 0,
        BC_COM_STATE_SERVER_OPEN,
        BC_COM_STATE_SERVER_WAIT_FOR_CONNECTION,
        BC_COM_STATE_SERVER_WAIT_FOR_GET_IS_READY,
        BC_COM_STATE_SERVER_DATA_READ,
        BC_COM_STATE_SERVER_STOP_WAIT,
        BC_COM_STATE_SERVER_CLOSE,
        BC_COM_STATE_CLIENT_OPEN,
        BC_COM_STATE_CLIENT_WAIT_FOR_CONNECTION,
        BC_COM_STATE_CLIENT_WAIT_FOR_PUT_IS_READY,
        BC_COM_STATE_CLIENT_DATA_WRITE,
        BC_COM_STATE_CLIENT_HOLD,
        BC_COM_STATE_CLIENT_CLOSE,
        BC_COM_STATE_IDLE,
        BC_COM_VOID
    } BC_COM_STATES;



    // *****************************************************************************

    /* Application Data

          Summary:
            Holds application data

          Description:
            This structure holds the application's data.

          Remarks:
            Application strings and buffers are be defined outside this structure.
     */

    typedef struct {
        /* The application's current state */
        volatile BC_COM_STATES state;

        SYS_TIME_HANDLE timer_client_hdl;
        SYS_TIME_HANDLE timer_server_hdl;

        UDP_SOCKET udp_client_socket;
        UDP_SOCKET udp_server_socket;

        IPV4_ADDR ipAddr;

        uint8_t *receive_buffer;
        int32_t receive_data_count;
        int32_t receive_number_of_data_to_read;
        bool receive_data_has_been_received;

        uint8_t *transmit_buffer;
        int32_t transmit_count;
        bool transmit_data_has_been_sent;
                
    } BC_COM_DATA;

    // *****************************************************************************
    // *****************************************************************************
    // Section: Application Callback Routines
    // *****************************************************************************
    // *****************************************************************************
    /* These routines are called by drivers when certain events occur.
     */

    // *****************************************************************************
    // *****************************************************************************
    // Section: Application Initialization and State Machine Functions
    // *****************************************************************************
    // *****************************************************************************

    /*******************************************************************************
      Function:
        void BC_COM_Initialize ( void )

      Summary:
         MPLAB Harmony application initialization routine.

      Description:
        This function initializes the Harmony application.  It places the
        application in its initial state and prepares it to run so that its
        BC_COM_Tasks function can be called.

      Precondition:
        All other system initialization routines should be called before calling
        this routine (in "SYS_Initialize").

      Parameters:
        None.

      Returns:
        None.

      Example:
        <code>
        BC_COM_Initialize();
        </code>

      Remarks:
        This routine must be called from the SYS_Initialize function.
     */

    void BC_COM_Initialize(void);


    /*******************************************************************************
          Function:
            void BC_COM_Tasks ( void )

          Summary:
            MPLAB Harmony Demo application tasks function

          Description:
            This routine is the Harmony Demo application's tasks function.  It
            defines the application's state machine and core logic.

          Precondition:
            The system and application initialization ("SYS_Initialize") should be
            called before calling this.

          Parameters:
            None.

          Returns:
            None.

          Example:
            <code>
            BC_COM_Tasks();
            </code>

          Remarks:
            This routine must be called from SYS_Tasks() routine.
     */
    void BC_COM_Tasks(void);

    /* Interface */
    void BC_COM_Initialize_Runtime(void);
    void BC_COM_DeInitialize_Runtime(void);
    bool BC_COM_listen(int32_t count);
    void BC_COM_listen_stop(void);
    bool BC_COM_is_data_received(void);    
    void BC_COM_read_data(uint8_t *buffer);
    bool BC_COM_send(uint8_t *buffer, int32_t count);
    bool BC_COM_is_idle(void);
    
   
    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _BC_COM_H */

/*******************************************************************************
 End of File
 */

