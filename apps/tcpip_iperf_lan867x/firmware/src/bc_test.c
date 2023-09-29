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
    TCPIP_MAC_ADDR mac;
    IPV6_ADDR ip6;
    IPV4_ADDR ip4;
    uint8_t nodeid;
    uint8_t maxnodeid;
    uint8_t randommssg[20];
    int32_t counter_100ms;
    bool led_state;
    uint32_t random;
} AUTOCONFMSG;

AUTOCONFMSG auto_conf_msg_transmit;
AUTOCONFMSG auto_conf_msg_receive;

AUTOCONFMSG member[8];

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

void SYS_Task_Start_TCP(void);
void SYS_Initialization_TCP_Stack(void);

DRV_MIIM_RESULT Write_Phy_Register(LAN867X_REG_OBJ *clientObj, int phyAddress, const uint32_t regAddr, uint16_t wData);
DRV_MIIM_RESULT Write_Bit_Phy_Register(LAN867X_REG_OBJ *clientObj, int phyAddress, const uint32_t regAddr, uint16_t mask, uint16_t wData);
DRV_MIIM_RESULT Read_Phy_Register(LAN867X_REG_OBJ *clientObj, int phyAddress, const uint32_t regAddr, uint16_t *rData);

DRV_MIIM_RESULT BC_TEST_miim_init(void);
void BC_TEST_miim_close(void);

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
    BC_TEST_Command_Init();
    SYS_Initialization_TCP_Stack();
    SYS_Task_Start_TCP();
    bc_test.timer_client_hdl = SYS_TIME_TimerCreate(0, SYS_TIME_MSToCount(100), &BC_TEST_TimerCallback, (uintptr_t) NULL, SYS_TIME_PERIODIC);
    bc_test.counter_100ms = 0;
    bc_test.led_state = false;    
    LED_1_Set();
    LED_2_Set();
    SYS_TIME_TimerStart(bc_test.timer_client_hdl);
    bc_test.init_done = false;
    bc_test.state = BC_TEST_STATE_IDLE;
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
                BC_TEST_DEBUG_PRINT("BC_TEST: IP Address : %d.%d.%d.%d\r\n", bc_test.MyIpAddr.v[0], bc_test.MyIpAddr.v[1], bc_test.MyIpAddr.v[2], bc_test.MyIpAddr.v[3]);
                BC_TEST_DEBUG_PRINT("BC_TEST: MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\r\n", bc_test.MyMacAddr->v[0], bc_test.MyMacAddr->v[1], bc_test.MyMacAddr->v[2], bc_test.MyMacAddr->v[3], bc_test.MyMacAddr->v[4], bc_test.MyMacAddr->v[5]);
                
                bc_test.countdown = 10;
                bc_test.init_done = true;
                bc_test.MyMacAddr = (TCPIP_MAC_ADDR*) TCPIP_STACK_NetAddressMac(netH);
              
                BC_COM_Initialize_Runtime();
                
                bc_test.state = BC_TEST_STATE_MEMBER_START_REQUEST;
            }
            break;






        case BC_TEST_STATE_MEMBER_START_REQUEST:
            if (bc_test.countdown == 0) {
                BC_TEST_DEBUG_PRINT("BC_TEST: =============================================\n\r");
                BC_TEST_DEBUG_PRINT("BC_TEST: Timeout %s %d\n\r", __FILE__, __LINE__);
                
                bc_test.random = TRNG_ReadData();               
                
#ifdef MY_NODE_0
                BC_TEST_DEBUG_PRINT("BC_TEST: MY_NODE_0 %s %s\n\r", __DATE__, __TIME__);
#endif
#ifdef MY_NODE_1
                BC_TEST_DEBUG_PRINT("BC_TEST: MY_NODE_1 %s %s\n\r", __DATE__, __TIME__);
#endif
#ifdef MY_NODE_2
                BC_TEST_DEBUG_PRINT("BC_TEST: MY_NODE_2 %s %s\n\r", __DATE__, __TIME__);
#endif
#ifdef MY_NODE_3
                BC_TEST_DEBUG_PRINT("BC_TEST: MY_NODE_3 %s %s\n\r", __DATE__, __TIME__);
#endif   
                
                memset((void*) &auto_conf_msg_transmit, 0xEE, sizeof (AUTOCONFMSG));

                auto_conf_msg_transmit.mac.v[0] = bc_test.MyMacAddr->v[0];
                auto_conf_msg_transmit.mac.v[1] = bc_test.MyMacAddr->v[1];
                auto_conf_msg_transmit.mac.v[2] = bc_test.MyMacAddr->v[2];
                auto_conf_msg_transmit.mac.v[3] = bc_test.MyMacAddr->v[3];
                auto_conf_msg_transmit.mac.v[4] = bc_test.MyMacAddr->v[4];
                auto_conf_msg_transmit.mac.v[5] = bc_test.MyMacAddr->v[5];

                auto_conf_msg_transmit.random = bc_test.random;

                BC_TEST_DEBUG_PRINT("BC_TEST: Radom - Member: %08x\n\r", bc_test.random);

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

                if (auto_conf_msg_receive.random != auto_conf_msg_transmit.random) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Data Received - Member\n\r");
                    BC_TEST_DumpMem((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));
                    BC_TEST_DEBUG_PRINT("BC_TEST: Wrong Random, skip packet\n\r");
                    bc_test.countdown = 50;
                    while (BC_COM_is_idle() == false);
                    BC_COM_listen(sizeof (AUTOCONFMSG));                    
                    bc_test.state = BC_TEST_STATE_MEMBER_WAIT_FOR_REQUESTED_ANSWER;
                    break;
                }
                BC_TEST_DEBUG_PRINT("BC_TEST: Correct Random, process packet\n\r");
                
                SYS_TIME_TimerStop(bc_test.timer_client_hdl);
                bc_test.counter_100ms = auto_conf_msg_receive.counter_100ms;
                bc_test.led_state = auto_conf_msg_receive.led_state;
                SYS_TIME_TimerStart(bc_test.timer_client_hdl);

                BC_TEST_DEBUG_PRINT("BC_TEST: Data Received - Member\n\r");
                BC_TEST_DumpMem((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));

                bc_test.state = BC_TEST_STATE_MEMBER_PROCESS_REQUESTED_DATA;
                break;
            }
            if (bc_test.countdown == 0) {
                BC_TEST_DEBUG_PRINT("BC_TEST: Timeout %s %d\n\r", __FILE__, __LINE__);
                BC_COM_listen_stop();
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

                memset((void*) &auto_conf_msg_transmit, 0xCC, sizeof (AUTOCONFMSG));

                auto_conf_msg_transmit.mac.v[0] = bc_test.MyMacAddr->v[0];
                auto_conf_msg_transmit.mac.v[1] = bc_test.MyMacAddr->v[1];
                auto_conf_msg_transmit.mac.v[2] = bc_test.MyMacAddr->v[2];
                auto_conf_msg_transmit.mac.v[3] = bc_test.MyMacAddr->v[3];
                auto_conf_msg_transmit.mac.v[4] = bc_test.MyMacAddr->v[4];
                auto_conf_msg_transmit.mac.v[5] = bc_test.MyMacAddr->v[5];

                auto_conf_msg_transmit.random = auto_conf_msg_receive.random;

                bc_test.counter_flag_100ms = false;
                while (bc_test.counter_flag_100ms == false);

                auto_conf_msg_transmit.counter_100ms = bc_test.counter_100ms;
                auto_conf_msg_transmit.led_state = bc_test.led_state;

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
            BC_TEST_DEBUG_PRINT("BC_TEST:  I should be never here: %s %d\n\r", __FILE__, __LINE__);
            while (1);
            break;
        }
    }
}

void BC_TEST_TimerCallback(uintptr_t context) {

    if (bc_test.countdown) {
        bc_test.countdown--;
    }

    bc_test.counter_100ms++;
    bc_test.counter_flag_100ms = true;

    if ((bc_test.counter_100ms % 20) == 0) {
        if (bc_test.led_state == false) {
            bc_test.led_state = true;
            LED_1_Set();
        } else {
            LED_1_Clear();
            bc_test.led_state = false;
        }
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

static void my_dump(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    //const void* cmdIoParam = pCmdIO->cmdIoParam;
    uint32_t addr;
    uint32_t count;

    addr = strtoul(argv[1], NULL, 16);
    count = strtoul(argv[2], NULL, 16);
    BC_TEST_DumpMem(addr, count);

}

static void my_run(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {

    //    SYS_Initialization_TCP_Stack();
    //    SYS_Task_Start_TCP();

    if (bc_test.init_done == false) {
        bc_test.state = BC_TEST_STATE_INIT_START;
    } else {
        bc_test.state = BC_TEST_STATE_MEMBER_START_REQUEST;
    }



    //    vTaskDelay(3000U / portTICK_PERIOD_MS);
    //    SERCOM1_USART_Virtual_Receive("iperf -u -s\n");

}

DRV_MIIM_RESULT BC_TEST_miim_init(void) {
    DRV_MIIM_SETUP miimSetup;
    DRV_MIIM_RESULT res;
    static DRV_MIIM_OPERATION_HANDLE opHandle;
    static SYS_MODULE_INDEX miimObjIx = 0; // MIIM object index

    opHandle = 0;
    bc_test.MiimObj.miimOpHandle = &opHandle;
    bc_test.MiimObj.miimBase = &DRV_MIIM_OBJECT_BASE_Default;
    miimObjIx = DRV_MIIM_DRIVER_INDEX_0;

    /*  Open the MIIM driver and get an instance to it. */
    bc_test.MiimObj.miimHandle = bc_test.MiimObj.miimBase->DRV_MIIM_Open(miimObjIx, DRV_IO_INTENT_SHARED);
    if ((bc_test.MiimObj.miimHandle == DRV_HANDLE_INVALID) || (bc_test.MiimObj.miimHandle == 0)) {
        SYS_CONSOLE_PRINT("BC_TEST: Local miim open: failed!\r\n");
        bc_test.MiimObj.miimHandle = 0;
        res = DRV_MIIM_RES_OP_INTERNAL_ERR;
    } else {

        miimSetup.hostClockFreq = (uint32_t) TCPIP_INTMAC_PERIPHERAL_CLK;
        miimSetup.maxBusFreq = 2000000;
        miimSetup.setupFlags = 0;

        /*  Setup the miim driver instance. */
        res = bc_test.MiimObj.miimBase->DRV_MIIM_Setup(bc_test.MiimObj.miimHandle, &miimSetup);
        if (res < 0) {
            SYS_CONSOLE_PRINT("BC_TEST: Local miim setup: failed!\r\n");
        } else {
            //SYS_CONSOLE_PRINT("> Miim Successfully opened. \r\n");
        }
    }

    return res;
}

void BC_TEST_miim_close(void) {
    bc_test.MiimObj.miimBase->DRV_MIIM_Close(bc_test.MiimObj.miimHandle);
    bc_test.MiimObj.miimHandle = 0;
    //SYS_CONSOLE_PRINT("> Miim closed. \r\n");
}

static void my_plca_write_config(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    uint16_t node_id;
    uint16_t node_count;
    DRV_MIIM_RESULT opRes = DRV_MIIM_RES_OK;
    uint16_t data;

    if (argc != 3) {
        BC_TEST_DEBUG_PRINT("Usage: ndw <node_id> <node_count>\n\r");
        return;
    }

    BC_TEST_miim_init();

    node_id = strtoul(argv[1], NULL, 16);
    node_count = strtoul(argv[2], NULL, 16);

    /* Set the Node id as 0 and Node count as 5*/
    data = F2R_(node_id, PHY_PLCA_CTRL1_ID) | F2R_(node_count, PHY_PLCA_CTRL1_NCNT);

    do {
        opRes = Write_Phy_Register(&bc_test.MiimObj, 0, PHY_PLCA_CTRL1, data);
        vTaskDelay(10U / portTICK_PERIOD_MS);
    } while (opRes == DRV_MIIM_RES_PENDING);

    if (opRes < 0) {
        /* In case of an error, report and close miim instance. */
        BC_TEST_DEBUG_PRINT("BC_TEST: Register Write Error occurred:%d\r\n", opRes);
    } else if (opRes == DRV_MIIM_RES_OK) /* Check operation is completed. */ {
        BC_TEST_DEBUG_PRINT("BC_TEST:  Register set, Node Id: %d, Node count: %d. \r\n", R2F(data, PHY_PLCA_CTRL1_ID), R2F(data, PHY_PLCA_CTRL1_NCNT));
    } else {
        BC_TEST_DEBUG_PRINT("BC_TEST: Register Write opRes: %d\n\r", opRes);
    }

    BC_TEST_miim_close();

}

static void my_plca_read_config(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    DRV_MIIM_RESULT opRes = DRV_MIIM_RES_OK;
    uint16_t data;

    BC_TEST_miim_init();

    do {
        opRes = Read_Phy_Register(&bc_test.MiimObj, 0, PHY_PLCA_CTRL1, &data);
        vTaskDelay(10U / portTICK_PERIOD_MS);
    } while (opRes == DRV_MIIM_RES_PENDING);

    if (opRes < 0) {
        /* In case of an error, report and close miim instance. */
        BC_TEST_DEBUG_PRINT("BC_TEST: Register Read Error occurred:%d\r\n", opRes);
    } else if (opRes == DRV_MIIM_RES_OK) /* Check operation is completed. */ {
        BC_TEST_DEBUG_PRINT("BC_TEST:  Node Id: %d, Node count: %d. \r\n", R2F(data, PHY_PLCA_CTRL1_ID), R2F(data, PHY_PLCA_CTRL1_NCNT));
    } else {
        BC_TEST_DEBUG_PRINT("BC_TEST: Register Read opRes: %d\n\r", opRes);
    }

    BC_TEST_miim_close();
}

const SYS_CMD_DESCRIPTOR msd_cmd_tbl[] = {
    {"dump", (SYS_CMD_FNC) my_dump, ": dump memory"},
    {"run", (SYS_CMD_FNC) my_run, ": start application"},
    {"ndw", (SYS_CMD_FNC) my_plca_write_config, ": Node Config Write"},
    {"ndr", (SYS_CMD_FNC) my_plca_read_config, ": Node Config Read: ndr"}
};

bool BC_TEST_Command_Init(void) {
    bool ret = false;

    if (!SYS_CMD_ADDGRP(msd_cmd_tbl, sizeof (msd_cmd_tbl) / sizeof (*msd_cmd_tbl), "test", ": Test Commands")) {
        ret = true;
    }
    return ret;
}


/*******************************************************************************
 End of File
 */
