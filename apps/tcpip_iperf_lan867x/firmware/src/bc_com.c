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
#include "bc_test.h"
#include "config/FreeRTOS/definitions.h"

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

char *bc_com_states_str[] = {
    "BC_COM_STATE_INIT",
    "BC_COM_STATE_SERVER_OPEN",
    "BC_COM_STATE_SERVER_WAIT_FOR_CONNECTION",
    "BC_COM_STATE_SERVER_WAIT_FOR_GET_IS_READY",
    "BC_COM_STATE_SERVER_DATA_READ",
    "BC_COM_STATE_SERVER_STOP_WAIT",
    "BC_COM_STATE_SERVER_CLOSE",
    "BC_COM_STATE_CLIENT_OPEN",
    "BC_COM_STATE_CLIENT_WAIT_FOR_CONNECTION",
    "BC_COM_STATE_CLIENT_WAIT_FOR_PUT_IS_READY",
    "BC_COM_STATE_CLIENT_DATA_WRITE",
    "BC_COM_STATE_CLIENT_HOLD",
    "BC_COM_STATE_CLIENT_CLOSE",
    "BC_COM_STATE_IDLE",
    "BC_COM_VOID"
};

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
    bc_com.state = BC_COM_STATE_INIT;

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
            BC_COM_DEBUG_PRINT("BC_COM: Server Open: %d\n\r", bc_com.udp_server_socket);
            bc_com.state = BC_COM_STATE_IDLE;
            break;

        case BC_COM_STATE_SERVER_WAIT_FOR_CONNECTION:
            if (TCPIP_UDP_IsConnected(bc_com.udp_server_socket) == true) {
                BC_COM_DEBUG_PRINT("BC_COM: Server is Connected\n\r");
                bc_com.receive_buffer = malloc( bc_com.receive_number_of_data_to_read);
                BC_COM_DEBUG_PRINT("BC_COM: Server Buffer malloc\n\r");
                bc_com.state = BC_COM_STATE_SERVER_WAIT_FOR_GET_IS_READY;
            }
            break;

        case BC_COM_STATE_SERVER_WAIT_FOR_GET_IS_READY:
            if ((bc_com.receive_data_count = TCPIP_UDP_GetIsReady(bc_com.udp_server_socket)) != 0) {
                BC_COM_DEBUG_PRINT("BC_COM: Get is Ready %d\n\r", bc_com.receive_data_count);
                if (bc_com.receive_data_count >= bc_com.receive_number_of_data_to_read) {
                    bc_com.state = BC_COM_STATE_SERVER_DATA_READ;
                }
            }
            break;

        case BC_COM_STATE_SERVER_DATA_READ:
            TCPIP_UDP_ArrayGet(bc_com.udp_server_socket, (uint8_t*) bc_com.receive_buffer, (uint16_t) bc_com.receive_number_of_data_to_read);
            bc_com.receive_data_has_been_received = true;
            BC_COM_DEBUG_PRINT("BC_COM: Server read data\n\r");
            bc_com.state = BC_COM_STATE_IDLE;
            break;

        case BC_COM_STATE_SERVER_STOP_WAIT:
            BC_COM_DEBUG_PRINT("BC_COM: Server Stop Wait\n\r");
            if(bc_com.receive_buffer != 0){
                free(bc_com.receive_buffer);
                bc_com.receive_buffer = 0;
                BC_COM_DEBUG_PRINT("BC_COM: Server Buffer free\n\r");
            }
            bc_com.state = BC_COM_STATE_IDLE;
            break;

        case BC_COM_STATE_SERVER_CLOSE:
            BC_COM_DEBUG_PRINT("BC_COM: Server Close\n\r");
            TCPIP_UDP_Close(bc_com.udp_server_socket);
            bc_com.state = BC_COM_STATE_IDLE;
            break;



            /************ Client States ************/
        case BC_COM_STATE_CLIENT_OPEN:
            bc_com.ipAddr.Val = 0xFFFFFFFF;
            bc_com.udp_client_socket = TCPIP_UDP_ClientOpen(IP_ADDRESS_TYPE_IPV4, BC_COM_UDP_SERVER_PORT, (IP_MULTI_ADDRESS*) & bc_com.ipAddr);
            BC_COM_DEBUG_PRINT("BC_COM: Client Open: %d\n\r", bc_com.udp_client_socket);
            bc_com.state = BC_COM_STATE_CLIENT_WAIT_FOR_CONNECTION;
            break;

        case BC_COM_STATE_CLIENT_WAIT_FOR_CONNECTION:
            if (TCPIP_UDP_IsConnected(bc_com.udp_client_socket) == true) {
                BC_COM_DEBUG_PRINT("BC_COM: is Connected\n\r");
                bc_com.state = BC_COM_STATE_IDLE;
            }
            break;

        case BC_COM_STATE_CLIENT_WAIT_FOR_PUT_IS_READY:
            if (TCPIP_UDP_PutIsReady(bc_com.udp_client_socket) >= bc_com.transmit_count) {
                BC_COM_DEBUG_PRINT("BC_COM: Put is Ready\n\r");
                bc_com.state = BC_COM_STATE_CLIENT_DATA_WRITE;
                break;
            }
            break;

        case BC_COM_STATE_CLIENT_DATA_WRITE:
            BC_COM_DEBUG_PRINT("BC_COM: Put Data %08x %08x %08x\n\r", (int) bc_com.udp_client_socket, (int) bc_com.transmit_buffer, (int) bc_com.transmit_count);
            TCPIP_UDP_ArrayPut(bc_com.udp_client_socket, bc_com.transmit_buffer, bc_com.transmit_count);
            TCPIP_UDP_Flush(bc_com.udp_client_socket);
            bc_com.state = BC_COM_STATE_IDLE;
            break;

        case BC_COM_STATE_CLIENT_CLOSE:
            if(bc_com.receive_buffer != 0){
                free(bc_com.receive_buffer);
                bc_com.receive_buffer = 0;
                BC_COM_DEBUG_PRINT("BC_COM: Server Buffer free\n\r");
            }            
            TCPIP_UDP_Close(bc_com.udp_client_socket);
            BC_COM_DEBUG_PRINT("BC_COM: Client Close\n\r");
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

/*********** Initialize and De-Initialize Interface *******************/

void BC_COM_Initialize_Runtime(void) {
    BC_COM_DEBUG_PRINT("BC_COM_Initialize_Runtime()\n\r");
    bc_com.state = BC_COM_STATE_SERVER_OPEN;
    while (bc_com.state != BC_COM_STATE_IDLE);
    bc_com.state = BC_COM_STATE_CLIENT_OPEN;
    while (bc_com.state != BC_COM_STATE_IDLE);
    BC_COM_DEBUG_PRINT("BC_COM_Initialize_Runtime() - Ready\n\r");
}

void BC_COM_DeInitialize_Runtime(void) {
    BC_COM_DEBUG_PRINT("BC_COM_DeInitialize_Runtime()\n\r");
    bc_com.state = BC_COM_STATE_SERVER_CLOSE;
    while (bc_com.state != BC_COM_STATE_IDLE);    
    bc_com.state = BC_COM_STATE_CLIENT_CLOSE;
    while (bc_com.state != BC_COM_STATE_IDLE);
    BC_COM_DEBUG_PRINT("BC_COM_DeInitialize_Runtime() - Ready\n\r");
}

/*********** Receive Interface *******************/
bool BC_COM_listen(int32_t count) {
    BC_COM_DEBUG_PRINT("BC_COM_listen()\n\r");
    bc_com.receive_number_of_data_to_read = count;
    bc_com.receive_data_count = 0;
    bc_com.state = BC_COM_STATE_SERVER_WAIT_FOR_CONNECTION;
    return true;
}

void BC_COM_listen_stop(void) {
    BC_COM_DEBUG_PRINT("BC_COM_listen_stop()\n\r");
    bc_com.state = BC_COM_STATE_IDLE;
}

bool BC_COM_is_data_received(void) {
    return bc_com.receive_data_has_been_received;
}

void BC_COM_read_data(uint8_t *buffer) {
    BC_COM_DEBUG_PRINT("BC_COM_read_data()\n\r");
    if (bc_com.receive_buffer != 0) {
        memcpy(buffer, bc_com.receive_buffer, bc_com.receive_number_of_data_to_read);
        BC_COM_DEBUG_PRINT("BC_COM: Close Server\n\r");
        bc_com.receive_data_has_been_received = false;
        bc_com.state = BC_COM_STATE_SERVER_STOP_WAIT;
    } else {
        BC_COM_DEBUG_PRINT("BC_COM: bc_com.receive_buffer is 0()\n\r");
    }
}

/*********** Transmit Interface *******************/
bool BC_COM_send(uint8_t *buffer, int32_t count) {
    BC_COM_DEBUG_PRINT("BC_COM_send()\n\r");
    if (bc_com.state != BC_COM_STATE_IDLE ) {
        BC_COM_DEBUG_PRINT("BC COM: send not in idle: %s %d\n\r", __FILE__, __LINE__);
        BC_COM_DEBUG_PRINT("BC COM: State: %s\n\r", bc_com_states_str[bc_com.state]);
        return true;
    }
    bc_com.transmit_buffer = buffer;
    bc_com.transmit_count = count;
    bc_com.transmit_data_has_been_sent = false;
    bc_com.state = BC_COM_STATE_CLIENT_WAIT_FOR_PUT_IS_READY;
    return false;
}

//void BC_COM_stop_send(void) {
//    BC_COM_DEBUG_PRINT("BC_COM_stop_send()\n\r");
//    bc_com.state = BC_COM_STATE_CLIENT_CLOSE;
//}

bool BC_COM_is_idle(void) {
    if (bc_com.state == BC_COM_STATE_IDLE) {
        return true;
    } else {
        return false;
    }
}


void BC_COM_Print_State_Change(void) {
    static BC_COM_STATES states = BC_COM_VOID;
    if (states != bc_com.state) {
        states = bc_com.state;
        BC_COM_DEBUG_PRINT("%s\n\r", bc_com_states_str[states]);
    }
}


/*******************************************************************************
 End of File
 */
