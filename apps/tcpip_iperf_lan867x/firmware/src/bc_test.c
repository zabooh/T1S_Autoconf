/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

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


#define __BC_TEST_DEBUG_PRINT 
#ifdef __BC_TEST_DEBUG_PRINT
#define BC_TEST_DEBUG_PRINT(fmt, ...)  SYS_CONSOLE_PRINT(fmt, ##__VA_ARGS__)
#else
#define BC_TEST_DEBUG_PRINT(fmt, ...)
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

typedef struct {
    IPV6_ADDR ip6;
    IPV4_ADDR ip4;
    TCPIP_MAC_ADDR mac;
    uint8_t nodeid;
    uint8_t maxnodeid;
    uint8_t randommssg[20];
} AUTOCONFMSG;

AUTOCONFMSG auto_conf_msg_transmit;
AUTOCONFMSG auto_conf_msg_receive;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the BC_TEST_Initialize function.

    Application strings and buffers are be defined outside this structure.
 */

BC_TEST_DATA bc_test;

extern SYSTEM_OBJECTS sysObj;

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

void BC_TEST_TimerCallback(uintptr_t context);
void BC_TEST_Print_State_Change(void);

/* TODO:  Add any necessary local functions.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void BC_TEST_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void BC_TEST_Initialize(void) {
    bc_test.state = BC_TEST_STATE_INIT_START;
}

/******************************************************************************
  Function:
    void BC_TEST_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void BC_TEST_Tasks(void) {
    TCPIP_NET_HANDLE netH;
    static IPV4_ADDR dwLastIP = {-1};
    SYS_STATUS tcpipStat;

    BC_TEST_Print_State_Change();

    switch (bc_test.state) {


        case BC_TEST_STATE_INIT_START:
            bc_test.timer_client_hdl = SYS_TIME_TimerCreate(0, SYS_TIME_MSToCount(100), &BC_TEST_TimerCallback, (uintptr_t) NULL, SYS_TIME_PERIODIC);
            SYS_TIME_TimerStart(bc_test.timer_client_hdl);
            bc_test.state = BC_TEST_STATE_INIT_TCPIP_WAIT_START;
            break;

        case BC_TEST_STATE_INIT_TCPIP_WAIT_START:
            tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);
            if (tcpipStat < 0) {
                BC_TEST_DEBUG_PRINT("BC_TEST: TCP/IP stack initialization failed!\r\n");
                bc_test.state = BC_TEST_STATE_IDLE;
            } else if (tcpipStat == SYS_STATUS_READY) {
                BC_TEST_DEBUG_PRINT("BC_TEST: TCP/IP stack successful!\r\n");
                bc_test.state = BC_TEST_STATE_INIT_TCPIP_WAIT_FOR_IP;
            }
            break;

        case BC_TEST_STATE_INIT_TCPIP_WAIT_FOR_IP:
            netH = TCPIP_STACK_IndexToNet(0);
            if (!TCPIP_STACK_NetIsReady(netH)) {
                break;
            }
            bc_test.MyIpAddr.Val = TCPIP_STACK_NetAddress(netH);
            bc_test.MyMacAddr = (TCPIP_MAC_ADDR*) TCPIP_STACK_NetAddressMac(netH);
            if (dwLastIP.Val != bc_test.MyIpAddr.Val) {
                dwLastIP.Val = bc_test.MyIpAddr.Val;
                BC_TEST_DEBUG_PRINT("IP Address : %d.%d.%d.%d\r\n", bc_test.MyIpAddr.v[0], bc_test.MyIpAddr.v[1], bc_test.MyIpAddr.v[2], bc_test.MyIpAddr.v[3]);
                BC_TEST_DEBUG_PRINT("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\r\n", bc_test.MyMacAddr->v[0], bc_test.MyMacAddr->v[1], bc_test.MyMacAddr->v[2], bc_test.MyMacAddr->v[3], bc_test.MyMacAddr->v[4], bc_test.MyMacAddr->v[5]);
                BC_COM_Initialize_Runtime();
                bc_test.countdown = 10;
                bc_test.state = BC_TEST_STATE_MEMBER_START_REQUEST;
            }
            break;






        case BC_TEST_STATE_MEMBER_START_REQUEST:
            if (bc_test.countdown == 0) {
                BC_TEST_DEBUG_PRINT("Timeout %s %d\n\r", __FILE__, __LINE__);
#ifdef MY_NODE_0
                BC_TEST_DEBUG_PRINT("MY_NODE_0 %s %s\n\r", __DATE__, __TIME__);
#endif
#ifdef MY_NODE_1
                BC_TEST_DEBUG_PRINT("MY_NODE_1 %s %s\n\r", __DATE__, __TIME__);
#endif
#ifdef MY_NODE_2
                BC_TEST_DEBUG_PRINT("MY_NODE_2 %s %s\n\r", __DATE__, __TIME__);
#endif
#ifdef MY_NODE_3
                BC_TEST_DEBUG_PRINT("MY_NODE_3 %s %s\n\r", __DATE__, __TIME__);
#endif                          
                BC_TEST_DEBUG_PRINT("Timeout %s %d\n\r", __FILE__, __LINE__);
                auto_conf_msg_transmit.ip4.Val = 0x12345678;
                auto_conf_msg_transmit.nodeid = 0xAA;
                auto_conf_msg_transmit.randommssg[19] = 0x55;
                BC_TEST_DEBUG_PRINT("BC_TEST: Data Sent - Member\n\r");
                BC_TEST_DumpMem((uint32_t) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
                BC_COM_send((uint8_t*) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
                while (BC_COM_is_idle() == false);
                BC_COM_listen(sizeof (AUTOCONFMSG));
                bc_test.countdown = 50;
                bc_test.state = BC_TEST_STATE_MEMBER_WAIT_FOR_REQUESTED_ANSWER;
            }
            break;


        case BC_TEST_STATE_MEMBER_WAIT_FOR_REQUESTED_ANSWER:
            if (BC_COM_is_data_received() == true) {
                BC_COM_read_data((uint8_t *) & auto_conf_msg_receive);
                BC_TEST_DEBUG_PRINT("BC_TEST: Data Received - Member\n\r");
                BC_TEST_DumpMem((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));
                bc_test.state = BC_TEST_STATE_MEMBER_PROCESS_REQUESTED_DATA;
                break;
            }
            if (bc_test.countdown == 0) {
                BC_TEST_DEBUG_PRINT("Timeout %s %d\n\r", __FILE__, __LINE__);
                bc_test.state = BC_TEST_STATE_DECIDE_TO_BE_COORDINATOR_NODE;
            }
            break;

        case BC_TEST_STATE_MEMBER_PROCESS_REQUESTED_DATA:
            BC_TEST_DEBUG_PRINT("BC_TEST: Requested data received. Nor further processing\n\r");
            bc_test.state = BC_TEST_STATE_IDLE;
            break;




        case BC_TEST_STATE_DECIDE_TO_BE_COORDINATOR_NODE:
            if (BC_COM_is_idle() == true) {
                BC_COM_listen(sizeof (AUTOCONFMSG));
                bc_test.state = BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST;
            }
            break;

        case BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST:
            if (BC_COM_is_data_received() == true) {
                BC_COM_read_data((uint8_t *) & auto_conf_msg_receive);
                BC_TEST_DEBUG_PRINT("BC_TEST: Data Received - Controller\n\r");
                BC_TEST_DumpMem((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));
                bc_test.state = BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST;
            }
            break;

        case BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST:
            if (BC_COM_is_idle() == true) {
                memcpy((void*) &auto_conf_msg_transmit, (void*) &auto_conf_msg_receive, sizeof (AUTOCONFMSG));
                auto_conf_msg_transmit.ip4.Val = 0x87654321;
                auto_conf_msg_transmit.nodeid = 0x55;
                auto_conf_msg_transmit.randommssg[19] = 0xAA;
                BC_COM_send((uint8_t*) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
                BC_TEST_DEBUG_PRINT("BC_TEST: Data Sent - Controller\n\r");
                BC_TEST_DumpMem((uint32_t) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
                while (BC_COM_is_idle() == false);
                BC_COM_listen(sizeof (AUTOCONFMSG));
                bc_test.state = BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST;
            }
            break;


        case BC_TEST_STATE_IDLE:
            break;

        default:
        {
            BC_TEST_DEBUG_PRINT("APP: I should be never here: %s %d\n\r", __FILE__, __LINE__);
            while (1);
            break;
        }
    }
}

void BC_TEST_TimerCallback(uintptr_t context) {
    if (bc_test.countdown) {
        bc_test.countdown--;
    }
}

char *app_states_str[] = {
    "BC_TEST_STATE_INIT_START",
    "BC_TEST_STATE_INIT_TCPIP_WAIT_START",
    "BC_TEST_STATE_INIT_TCPIP_WAIT_FOR_IP",
    "BC_TEST_STATE_MEMBER_START_REQUEST",
    "BC_TEST_STATE_MEMBER_WAIT_FOR_REQUESTED_ANSWER",
    "BC_TEST_STATE_MEMBER_PROCESS_REQUESTED_DATA",
    "BC_TEST_STATE_DECIDE_TO_BE_COORDINATOR_NODE",
    "BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST",
    "BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST",
    "BC_TEST_STATE_IDLE",
    "BC_TEST_VOID"
};

void BC_TEST_Print_State_Change(void) {
    static BC_TEST_STATES states = BC_TEST_VOID;
    if (states != bc_test.state) {
        states = bc_test.state;
        BC_TEST_DEBUG_PRINT("%s\n\r", app_states_str[states]);
    }
}

void BC_TEST_DumpMem(uint32_t addr, uint32_t count) {
    uint32_t ix, jx;
    uint8_t *puc;
    char str[64];
    int flag = 0;

    puc = (uint8_t *) addr;
    puc = (uint8_t *) addr;

    jx = 0;
    for (ix = 0; ix < count; ix++) {
        if ((ix % 16) == 0) {
            if (flag == 1) {
                str[16] = 0;
                BC_TEST_DEBUG_PRINT("   %s\n\r", str);
            }
            BC_TEST_DEBUG_PRINT("%08x: ", puc);
            flag = 1;
            jx = 0;
        }
        BC_TEST_DEBUG_PRINT(" %02x", *puc);
        if ((*puc > 31) && (*puc < 127))
            str[jx++] = *puc;
        else
            str[jx++] = '.';
        puc++;
    }
    str[jx] = 0;
    BC_TEST_DEBUG_PRINT("   %s", str);
    BC_TEST_DEBUG_PRINT("\n\r");
}


/*******************************************************************************
 End of File
 */
