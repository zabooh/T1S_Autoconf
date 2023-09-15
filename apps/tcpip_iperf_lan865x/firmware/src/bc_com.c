/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    bc_com.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "bc_com.h"
#include "tcpip/tcpip.h"
#include "system/console/sys_console.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

#define __BC_COM_DEBUG_PRINT 
#ifdef __BC_COM_DEBUG_PRINT
#define BC_COM_DEBUG_PRINT(fmt, ...)  SYS_CONSOLE_PRINT(fmt, ##__VA_ARGS__)
#else
#define BC_COM_DEBUG_PRINT(fmt, ...)
#endif

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the BC_COM_Initialize function.

    Application strings and buffers are be defined outside this structure.
 */

BC_COM_DATA bc_com;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void BC_COM_Print_State_Change(void);

/* TODO:  Add any necessary local functions.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void BC_COM_Initialize ( void )

  Remarks:
    See prototype in bc_com.h.
 */

void BC_COM_Initialize(void) {
    /* Place the App state machine in its initial state. */
    bc_com.state = BC_COM_STATE_IDLE;

    memset((void*) &bc_com, 0, sizeof (BC_COM_DATA));

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

/******************************************************************************
  Function:
    void BC_COM_Tasks ( void )

  Remarks:
    See prototype in bc_com.h.
 */

void BC_COM_Tasks(void) {

    BC_COM_Print_State_Change();

    switch (bc_com.state) {

        case BC_COM_STATE_INIT:
        {
            bc_com.state = BC_COM_STATE_IDLE;
            break;
        }

            /************ Server States ************/
        case BC_COM_STATE_SERVER_OPEN:
            bc_com.udp_server_socket = TCPIP_UDP_ServerOpen(IP_ADDRESS_TYPE_IPV4, BC_COM_UDP_SERVER_PORT, 0);
            bc_com.state = BC_COM_STATE_SERVER_WAIT_FOR_CONNECTION;
            break;

        case BC_COM_STATE_SERVER_WAIT_FOR_CONNECTION:
            if (!TCPIP_UDP_IsConnected(bc_com.udp_server_socket)) {
                bc_com.state = BC_COM_STATE_SERVER_WAIT_FOR_GET_IS_READY;
            }
            break;

        case BC_COM_STATE_SERVER_WAIT_FOR_GET_IS_READY:
            if ((bc_com.receive_data_count = TCPIP_UDP_GetIsReady(bc_com.udp_server_socket)) != 0) {
                if (bc_com.receive_data_count >= bc_com.receive_number_of_data_to_read) {
                    bc_com.state = BC_COM_STATE_SERVER_DATA_READ;
                }
            }
            break;

        case BC_COM_STATE_SERVER_DATA_READ:
            TCPIP_UDP_ArrayGet(bc_com.udp_server_socket, (uint8_t*) bc_com.receive_buffer, (uint16_t) bc_com.receive_number_of_data_to_read);
            bc_com.receive_data_has_been_received = true;
            bc_com.state = BC_COM_STATE_SERVER_HOLD;
            break;

        case BC_COM_STATE_SERVER_HOLD:
            if (bc_com.receive_data_has_been_received == false) {
                bc_com.state = BC_COM_STATE_SERVER_WAIT_FOR_GET_IS_READY;
            }
            break;

        case BC_COM_STATE_SERVER_CLOSE:
            TCPIP_UDP_Close(bc_com.udp_server_socket);
            bc_com.state = BC_COM_STATE_IDLE;
            break;




            /************ Client States ************/
        case BC_COM_STATE_CLIENT_OPEN:
            bc_com.ipAddr.Val = 0xFFFFFFFF;
            bc_com.udp_client_socket = TCPIP_UDP_ClientOpen(IP_ADDRESS_TYPE_IPV4, BC_COM_UDP_SERVER_PORT, (IP_MULTI_ADDRESS*) & bc_com.ipAddr);
            bc_com.state = BC_COM_STATE_CLIENT_WAIT_FOR_CONNECTION;
            break;

        case BC_COM_STATE_CLIENT_WAIT_FOR_CONNECTION:
            if (!TCPIP_UDP_IsConnected(bc_com.udp_client_socket)) {
                bc_com.state = BC_COM_STATE_CLIENT_WAIT_FOR_PUT_IS_READY;
            }
            break;

        case BC_COM_STATE_CLIENT_WAIT_FOR_PUT_IS_READY:
            if (TCPIP_UDP_PutIsReady(bc_com.udp_client_socket) == 0) {
                bc_com.state = BC_COM_STATE_CLIENT_DATA_WRITE;
                break;
            }
            break;

        case BC_COM_STATE_CLIENT_DATA_WRITE:
            TCPIP_UDP_ArrayPut(bc_com.udp_client_socket, bc_com.transmit_buffer, bc_com.transmit_count);
            TCPIP_UDP_Flush(bc_com.udp_client_socket);
            bc_com.state = BC_COM_STATE_CLIENT_HOLD;
            break;

        case BC_COM_STATE_CLIENT_HOLD:
            bc_com.transmit_data_has_been_sent = true;
            break;

        case BC_COM_STATE_CLIENT_CLOSE:
            TCPIP_UDP_Close(bc_com.udp_client_socket);
            bc_com.state = BC_COM_STATE_IDLE;
            break;



        case BC_COM_STATE_IDLE:
            break;


        default:
        {
            BC_COM_DEBUG_PRINT("BC_COM: I should be never here: %s %s\n\r", __FILE__, __LINE__);
            while (1);
            break;
        }
    }
}

/*********** Receive Interface *******************/
bool BC_COM_listen(int32_t count) {
    if (bc_com.state != BC_COM_STATE_IDLE) {
        BC_COM_DEBUG_PRINT("BC_COM: already started: %s %s\n\r", __FILE__, __LINE__);
        return false;
    }
    if (bc_com.receive_buffer != 0) {
        BC_COM_DEBUG_PRINT("BC_COM: Receive Buffer already allocated: %s %s\n\r", __FILE__, __LINE__);
        return false;
    }
    bc_com.receive_buffer = malloc(count);
    if (bc_com.receive_buffer == 0) {
        BC_COM_DEBUG_PRINT("BC_COM: Malloc Error: %s %s\n\r", __FILE__, __LINE__);
        bc_com.state = BC_COM_STATE_SERVER_CLOSE;
        return false;
    }
    bc_com.receive_number_of_data_to_read = count;
    bc_com.receive_data_count = 0;
    bc_com.state = BC_COM_STATE_SERVER_OPEN;
    return true;
}

bool BC_COM_is_data_received(void) {
    return bc_com.receive_data_has_been_received;
}

void BC_COM_read_data(uint8_t *buffer) {
    memcpy(buffer, bc_com.receive_buffer, bc_com.receive_number_of_data_to_read);
    free(bc_com.receive_buffer);
    bc_com.receive_buffer = 0;
    bc_com.receive_data_has_been_received = false;
}

void BC_COM_stop_listen(void) {
    bc_com.state = BC_COM_STATE_SERVER_CLOSE;
}

/*********** Transmit Interface *******************/
bool BC_COM_send(uint8_t *buffer, int32_t count) {
    if ((bc_com.state != BC_COM_STATE_IDLE) || (bc_com.state != BC_COM_STATE_CLIENT_HOLD)) {
        BC_COM_DEBUG_PRINT("BC COM send not in idle or hold: %s %s\n\r", __FILE__, __LINE__);
        return true;
    }
    bc_com.transmit_buffer = buffer;
    bc_com.transmit_count = count;
    bc_com.transmit_data_has_been_sent = false;
    if (bc_com.state == BC_COM_STATE_CLIENT_HOLD) {
        bc_com.state = BC_COM_STATE_CLIENT_DATA_WRITE;
        return false;
    }
    bc_com.state = BC_COM_STATE_CLIENT_OPEN;
    return false;
}

bool BC_COM_is_data_send(void) {
    return bc_com.transmit_data_has_been_sent;
}

void BC_COM_stop_send(void) {
    bc_com.state = BC_COM_STATE_CLIENT_CLOSE;
}

bool BC_COM_is_idle(void) {
    if (bc_com.state == BC_COM_STATE_IDLE) {
        return true;
    } else {
        return false;
    }
}


char *bc_com_states_str[] = {
    "BC_COM_STATE_INIT",
    "BC_COM_STATE_SERVER_OPEN",
    "BC_COM_STATE_SERVER_WAIT_FOR_CONNECTION",
    "BC_COM_STATE_SERVER_WAIT_FOR_GET_IS_READY",
    "BC_COM_STATE_SERVER_DATA_READ",
    "BC_COM_STATE_SERVER_HOLD",
    "BC_COM_STATE_SERVER_CLOSE",
    "BC_COM_STATE_CLIENT_OPEN",
    "BC_COM_STATE_CLIENT_WAIT_FOR_CONNECTION",
    "BC_COM_STATE_CLIENT_WAIT_FOR_PUT_IS_READY",
    "BC_COM_STATE_CLIENT_DATA_WRITE",
    "BC_COM_STATE_CLIENT_HOLD",
    "BC_COM_STATE_CLIENT_CLOSE",
    "BC_COM_TIMEOUT",
    "BC_COM_STATE_SERVICE_TASKS",
    "BC_COM_STATE_IDLE",
    "BC_COM_VOID"
};

void BC_COM_Print_State_Change(void) {
    static BC_COM_STATES states = BC_COM_VOID;
    if (states != bc_com.state) {
        states = bc_com.state;
        BC_COM_DEBUG_PRINT("BC_COM State: %s\n\r", bc_com_states_str[states]);
    }
}


/*******************************************************************************
 End of File
 */
