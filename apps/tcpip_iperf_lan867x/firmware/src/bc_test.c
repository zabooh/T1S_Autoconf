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

#define __BC_TEST_DEBUG_DUMP_PRINT 
#ifdef __BC_TEST_DEBUG_DUMP_PRINT
#define BC_TEST_DEBUG_DUMP_PRINT(fmt, ...)  SYS_CONSOLE_PRINT(fmt, ##__VA_ARGS__)
#else
#define BC_TEST_DEBUG_DUMP_PRINT(fmt, ...)
#endif

//#define __BC_TEST_DEBUG_DUMP_PACKET 
#ifdef __BC_TEST_DEBUG_DUMP_PACKET
#define BC_TEST_DEBUG_DUMP_PACKET(fmt, ...)  BC_TEST_DumpMem(fmt, ##__VA_ARGS__)
#else
#define BC_TEST_DEBUG_DUMP_PACKET(fmt, ...)
#endif

#define TIMER_MS_RESOLUTION 100

#define TIMEOUT_100_MILLI_SECONDS   1
#define TIMEOUT_1_SECOND            10
#define TIMEOUT_10_SECONDS          100
#define TIMEOUT_20_SECONDS          200
#define TIMEOUT_30_SECONDS          300

#define MEMBER_INIT_REQUEST 1
#define MEMBER_LIVE_REQUEST 2
#define MEMBER_INIT_ANSWER  3
#define MEMBER_LIVE_ANSWER  4

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

typedef struct {
    TCPIP_MAC_ADDR mac;
    IPV6_ADDR ip6;
    IPV4_ADDR ip4;
    uint8_t origin;
    uint8_t control_code;
    uint8_t node_max;
    uint8_t node_id;
    uint8_t randommssg[20];
    int32_t counter_100ms;
    bool led_state;
    uint32_t random;
} AUTOCONFMSG;

AUTOCONFMSG auto_conf_msg_transmit;
AUTOCONFMSG auto_conf_msg_receive;

AUTOCONFMSG member[8];

#define BC_TEST_MEMBER       12
#define BC_TEST_COORDINATOR  24

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
void BC_TEST_Print_State_Change_And_Trigger_Watchdog(void);

void SYS_Task_Start_TCP(void);
void SYS_Initialization_TCP_Stack(void);

DRV_MIIM_RESULT Write_Phy_Register(LAN867X_REG_OBJ *clientObj, int phyAddress, const uint32_t regAddr, uint16_t wData);
DRV_MIIM_RESULT Write_Bit_Phy_Register(LAN867X_REG_OBJ *clientObj, int phyAddress, const uint32_t regAddr, uint16_t mask, uint16_t wData);
DRV_MIIM_RESULT Read_Phy_Register(LAN867X_REG_OBJ *clientObj, int phyAddress, const uint32_t regAddr, uint16_t *rData);

DRV_MIIM_RESULT BC_TEST_miim_init(void);
void BC_TEST_miim_close(void);

extern uint8_t my_mac_str[];

volatile uint16_t bc_test_node_id;
volatile uint16_t bc_test_node_count;

uint8_t BC_TEST_calculateCRC8(uint8_t *data, int length);

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

char time_result[100];

volatile int tc1_int_counter = 0;

void BC_TEST_TC1_Interrupt_Handler(TC_TIMER_STATUS status, uintptr_t context) {
    tc1_int_counter++;
}

/*******************************************************************************
  Function:
    void BC_TEST_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void BC_TEST_Initialize(void) {
    BC_TEST_Command_Init();
    //    SYS_Initialization_TCP_Stack();
    //    SYS_Task_Start_TCP();
    bc_test_node_id = 1; //DRV_ETHPHY_PLCA_LOCAL_NODE_ID;
    bc_test_node_count = DRV_ETHPHY_PLCA_NODE_COUNT;
    bc_test.timer_client_hdl = SYS_TIME_TimerCreate(0, SYS_TIME_MSToCount(TIMER_MS_RESOLUTION), &BC_TEST_TimerCallback, (uintptr_t) NULL, SYS_TIME_PERIODIC);
    bc_test.tick_100ms = 0;
    bc_test.led_state = false;
    LED_1_Set();
    LED_2_Set();
    SYS_TIME_TimerStart(bc_test.timer_client_hdl);
    bc_test.init_done = false;


    bc_test.MyMacAddr.v[0] = 0x00;
    bc_test.MyMacAddr.v[1] = 0x04;
    bc_test.MyMacAddr.v[2] = 0x25;
    bc_test.MyMacAddr.v[3] = (uint8_t) TRNG_ReadData();
    bc_test.MyMacAddr.v[4] = (uint8_t) TRNG_ReadData();
    bc_test.MyMacAddr.v[5] = (uint8_t) TRNG_ReadData();
    TCPIP_Helper_MACAddressToString(&bc_test.MyMacAddr, my_mac_str, 18);

    TC1_TimerCallbackRegister(BC_TEST_TC1_Interrupt_Handler, (uintptr_t) NULL);

    bc_test.watchdog = TIMEOUT_30_SECONDS;

    bc_test.state = BC_TEST_STATE_INIT_START;
}

void BC_TEST_Time_Measure_Start(void) {
    TC1_TimerStop();
    TC1_Timer16bitCounterSet(0);
    tc1_int_counter = 0;
    TC1_TimerStart();
}

void BC_Test_Time_Measure_Stop_And_Get_Result(char *str) {
    float f_result;
    float f_frequency;
    float f_period;
    float f_value;

    TC1_TimerStop();
    f_result = (float) TC1_Timer16bitCounterGet();
    f_frequency = (float) TC1_TimerFrequencyGet();
    f_period = 1.0 / f_frequency;
    f_value = ((float) tc1_int_counter * f_period) + f_result / f_frequency;
    f_value *= 1000.0;

    sprintf(str, "%f", f_value);
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
    bool result;

    BC_TEST_Print_State_Change_And_Trigger_Watchdog();

    if (bc_test.watchdog == 0) {
        BC_TEST_DEBUG_PRINT("BC_TEST: Soft-Watchdog Triggered\r\n");
        BC_COM_DeInitialize_Runtime();
        BC_COM_Initialize_Runtime();
        bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * 100) / 16;
        BC_TEST_DEBUG_PRINT("BC_TEST: Watchdog Triggered Restart in %d Ticks\n\r", bc_test.countdown);
        bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
    }

    switch (bc_test.state) {


        case BC_TEST_STATE_INIT_START:
            BC_TEST_DEBUG_PRINT("BC_TEST: Build Time "__DATE__" "__TIME__"\n\r");
            BC_TEST_Time_Measure_Start();
            SYS_Initialization_TCP_Stack();
            SYS_Task_Start_TCP();
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

            TCPIP_MAC_ADDR * mac_ptr;
            mac_ptr = (TCPIP_MAC_ADDR*) TCPIP_STACK_NetAddressMac(netH);
            memcpy(&bc_test.MyMacAddr.v[0], &mac_ptr->v[0], 6);

            if (dwLastIP.Val != bc_test.MyIpAddr.Val) {

                dwLastIP.Val = bc_test.MyIpAddr.Val;
                BC_TEST_DEBUG_PRINT("BC_TEST: IP Address : %d.%d.%d.%d\r\n", bc_test.MyIpAddr.v[0], bc_test.MyIpAddr.v[1], bc_test.MyIpAddr.v[2], bc_test.MyIpAddr.v[3]);
                BC_TEST_DEBUG_PRINT("BC_TEST: MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\r\n", bc_test.MyMacAddr.v[0], bc_test.MyMacAddr.v[1], bc_test.MyMacAddr.v[2], bc_test.MyMacAddr.v[3], bc_test.MyMacAddr.v[4], bc_test.MyMacAddr.v[5]);

                bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * 100) / 16;
                BC_TEST_DEBUG_PRINT("BC_TEST: Start in %d Ticks\n\r", bc_test.countdown);

                bc_test.init_done = true;

                BC_COM_Initialize_Runtime();
                BC_Test_Time_Measure_Stop_And_Get_Result(time_result);
                BC_TEST_DEBUG_PRINT("BC_TEST: Time to Init TCP Stack: %s secs\n\r", time_result);
                BC_TEST_DEBUG_PRINT("BC_TEST: =============================================\n\r");
                BC_TEST_DEBUG_PRINT("BC_TEST: Build Time %s %s\n\r", __DATE__, __TIME__);

                bc_test.state = BC_TEST_STATE_IDLE;
            }
            break;






        case BC_TEST_STATE_MEMBER_INIT_START_REQUEST:
            if (bc_test.countdown == 0) {

                BC_TEST_NetDown();
                BC_TEST_DEBUG_PRINT("BC_TEST: bc_test_node_count: %d\n\r", bc_test_node_count);
                BC_TEST_SetNodeID_and_MAXcount(1, bc_test_node_count);
                BC_TEST_NetUp();
                netH = TCPIP_STACK_NetHandleGet("eth0");
                while (TCPIP_STACK_NetIsReady(netH) == false);

                bc_test.random = TRNG_ReadData();

                memset((void*) &auto_conf_msg_transmit, 0xEE, sizeof (AUTOCONFMSG));
                memcpy(&auto_conf_msg_transmit.mac.v[0], &bc_test.MyMacAddr.v[0], 6);

                auto_conf_msg_transmit.random = bc_test.random;
                auto_conf_msg_transmit.origin = BC_TEST_MEMBER;
                auto_conf_msg_transmit.control_code = MEMBER_INIT_REQUEST;

                BC_TEST_DEBUG_PRINT("BC_TEST: Radom - Member: %08x\n\r", bc_test.random);
                BC_TEST_DEBUG_PRINT("BC_TEST: Data Sent - Member Init\n\r");
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));

                BC_TEST_Time_Measure_Start();
                BC_COM_send((uint8_t*) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
                while (BC_COM_is_idle() == false);
                BC_COM_listen(sizeof (AUTOCONFMSG));
                bc_test.countdown = TIMEOUT_1_SECOND;
                bc_test.state = BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER;
            }
            break;


        case BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER:
            if (BC_COM_is_data_received() == true) {

                BC_COM_read_data((uint8_t *) & auto_conf_msg_receive);

                if (auto_conf_msg_receive.origin != BC_TEST_COORDINATOR ||
                        auto_conf_msg_receive.random != auto_conf_msg_transmit.random) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Wrong Data Received\n\r");
                    BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));
                    bc_test.countdown = TIMEOUT_1_SECOND;
                    while (BC_COM_is_idle() == false);
                    BC_COM_listen(sizeof (AUTOCONFMSG));
                    bc_test.state = BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER;
                    BC_Test_Time_Measure_Stop_And_Get_Result(time_result);
                    BC_TEST_DEBUG_PRINT("BC_TEST: Time to Restart Receiving: %s secs\n\r", time_result);
                    break;
                }


                BC_TEST_DEBUG_PRINT("BC_TEST: Correct Random, process packet\n\r");

                SYS_TIME_TimerStop(bc_test.timer_client_hdl);
                bc_test.tick_100ms = auto_conf_msg_receive.counter_100ms;
                bc_test.led_state = auto_conf_msg_receive.led_state;
                SYS_TIME_TimerStart(bc_test.timer_client_hdl);

                BC_TEST_DEBUG_PRINT("BC_TEST: Data Received - Member\n\r");
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));

                BC_Test_Time_Measure_Stop_And_Get_Result(time_result);
                BC_TEST_DEBUG_PRINT("BC_TEST: Time to start process received data: %s secs\n\r", time_result);

                bc_test.state = BC_TEST_STATE_MEMBER_INIT_PROCESS_REQUESTED_DATA;
                break;
            }
            if (bc_test.countdown == 0) {
                BC_TEST_DEBUG_PRINT("BC_TEST: Timeout %s %d\n\r", __FILE__, __LINE__);
                BC_COM_listen_stop();

                BC_Test_Time_Measure_Stop_And_Get_Result(time_result);
                BC_TEST_DEBUG_PRINT("BC_TEST: Time to Timeout and start Coordinator: %s secs\n\r", time_result);

                bc_test.state = BC_TEST_STATE_MEMBER_INIT_DECIDE_TO_BE_COORDINATOR_NODE;
            }
            break;

        case BC_TEST_STATE_MEMBER_INIT_PROCESS_REQUESTED_DATA:
        {
            TCPIP_NET_HANDLE netH;
            IPV4_ADDR ipMask;
            char buff[30];

            bc_test.nodeid_ix = auto_conf_msg_receive.node_id;
            BC_TEST_DEBUG_PRINT("BC_TEST: Received NodeId:%d\n\r", auto_conf_msg_receive.node_id);

            BC_TEST_Time_Measure_Start();
            BC_TEST_NetDown();
            BC_TEST_SetNodeID_and_MAXcount(bc_test.nodeid_ix, bc_test_node_count);
            BC_TEST_NetUp();
            netH = TCPIP_STACK_NetHandleGet("eth0");
            while (TCPIP_STACK_NetIsReady(netH) == false);
            BC_Test_Time_Measure_Stop_And_Get_Result(time_result);
            BC_TEST_DEBUG_PRINT("BC_TEST: Time to Re-Init TCP Stack: %s secs\n\r", time_result);

            netH = TCPIP_STACK_NetHandleGet("eth0");
            ipMask.v[0] = 255;
            ipMask.v[1] = 255;
            ipMask.v[2] = 255;
            ipMask.v[3] = 0;
            TCPIP_STACK_NetAddressSet(netH, &auto_conf_msg_receive.ip4, &ipMask, 1);
            TCPIP_Helper_IPAddressToString(&auto_conf_msg_receive.ip4, buff, 20);
            BC_TEST_DEBUG_PRINT("BC_TEST: new IP:%s\n\r", buff);

            BC_TEST_DEBUG_PRINT("BC_TEST: Requested data received. Nor further processing\n\r");

            bc_test.countdown = TIMEOUT_10_SECONDS;
            bc_test.state = BC_TEST_STATE_IDLE;
            break;
        }



        case BC_TEST_STATE_MEMBER_INIT_DECIDE_TO_BE_COORDINATOR_NODE:
            if (BC_COM_is_idle() == true) {

                BC_TEST_NetDown();
                BC_TEST_SetNodeID_and_MAXcount(0, bc_test_node_count);
                BC_TEST_NetUp();
                netH = TCPIP_STACK_NetHandleGet("eth0");
                while (TCPIP_STACK_NetIsReady(netH) == false);

                bc_test.nodeid_ix = 1;
                LED_2_Clear();

                BC_COM_listen(sizeof (AUTOCONFMSG));
                bc_test.countdown = TIMEOUT_20_SECONDS;
                bc_test.state = BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST;
            }
            break;


        case BC_TEST_STATE_MEMBER_LIVE_START_REQUEST:
            if (bc_test.countdown == 0) {

                netH = TCPIP_STACK_NetHandleGet("eth0");
                while (TCPIP_STACK_NetIsReady(netH) == false);

                bc_test.random = TRNG_ReadData();

                memset((void*) &auto_conf_msg_transmit, 0xEE, sizeof (AUTOCONFMSG));
                memcpy(&auto_conf_msg_transmit.mac.v[0], &bc_test.MyMacAddr.v[0],6);

                auto_conf_msg_transmit.random = bc_test.random;
                auto_conf_msg_transmit.origin = BC_TEST_MEMBER;
                auto_conf_msg_transmit.control_code = MEMBER_LIVE_REQUEST;

                BC_TEST_DEBUG_PRINT("BC_TEST: Radom - Member: %08x\n\r", bc_test.random);
                BC_TEST_DEBUG_PRINT("BC_TEST: Data Sent - Member Live\n\r");
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));

                BC_TEST_Time_Measure_Start();
                BC_COM_send((uint8_t*) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
                while (BC_COM_is_idle() == false);
                BC_COM_listen(sizeof (AUTOCONFMSG));
                bc_test.countdown = TIMEOUT_1_SECOND;
                bc_test.state = BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER;
            }
            break;

        case BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER:
            if (BC_COM_is_data_received() == true) {

                BC_COM_read_data((uint8_t *) & auto_conf_msg_receive);

                if (auto_conf_msg_receive.origin != BC_TEST_COORDINATOR ||
                        auto_conf_msg_receive.random != auto_conf_msg_transmit.random) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Wrong Data Received\n\r");
                    BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));
                    bc_test.countdown = TIMEOUT_1_SECOND;
                    while (BC_COM_is_idle() == false);
                    BC_COM_listen(sizeof (AUTOCONFMSG));
                    bc_test.state = BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER;
                    BC_Test_Time_Measure_Stop_And_Get_Result(time_result);
                    BC_TEST_DEBUG_PRINT("BC_TEST: Time to Restart Live Request Receiving: %s secs\n\r", time_result);
                    break;
                }


                BC_TEST_DEBUG_PRINT("BC_TEST: Correct Random, process packet\n\r");

                SYS_TIME_TimerStop(bc_test.timer_client_hdl);
                bc_test.tick_100ms = auto_conf_msg_receive.counter_100ms;
                bc_test.led_state = auto_conf_msg_receive.led_state;
                SYS_TIME_TimerStart(bc_test.timer_client_hdl);

                BC_TEST_DEBUG_PRINT("BC_TEST: Data Received - Member\n\r");
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));

                BC_Test_Time_Measure_Stop_And_Get_Result(time_result);
                BC_TEST_DEBUG_PRINT("BC_TEST: Time to start process received data: %s secs\n\r", time_result);

                bc_test.state = BC_TEST_STATE_MEMBER_LIVE_PROCESS_REQUESTED_DATA;
                break;
            }
            if (bc_test.countdown == 0) {
                BC_TEST_DEBUG_PRINT("BC_TEST: Timeout %s %d\n\r", __FILE__, __LINE__);
                BC_COM_listen_stop();

                BC_Test_Time_Measure_Stop_And_Get_Result(time_result);
                BC_TEST_DEBUG_PRINT("BC_TEST: Time to Timeout and start Member Init: %s secs\n\r", time_result);

                bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
            }
            break;


        case BC_TEST_STATE_MEMBER_LIVE_PROCESS_REQUESTED_DATA:
        {
            bc_test.countdown = TIMEOUT_10_SECONDS;
            bc_test.state = BC_TEST_STATE_IDLE;
            break;
        }


        case BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST:
            if (BC_COM_is_data_received() == true) {
                BC_COM_read_data((uint8_t *) & auto_conf_msg_receive);
                if (auto_conf_msg_receive.origin == BC_TEST_COORDINATOR) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Data Received - Coordinator\n\r");
                    BC_TEST_DEBUG_PRINT("BC_TEST: Packet dropped\n\r");
                    break;
                }
                BC_TEST_DEBUG_PRINT("BC_TEST: Data Received - Member\n\r");
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));
                BC_COM_listen_stop();
                bc_test.state = BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST;
            }
            if (bc_test.countdown == 0) {
                BC_COM_listen_stop();
                bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * 100) / 16;
                BC_TEST_DEBUG_PRINT("BC_TEST: Restart in %d Ticks\n\r", bc_test.countdown);
                bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
            }
            break;

        case BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST:
            if (BC_COM_is_idle() == true) {

                memset((void*) &auto_conf_msg_transmit, 0xCC, sizeof (AUTOCONFMSG));
                memcpy(&auto_conf_msg_transmit.mac.v[0], &bc_test.MyMacAddr.v[0],6);

                auto_conf_msg_transmit.random = auto_conf_msg_receive.random;
                auto_conf_msg_transmit.origin = BC_TEST_COORDINATOR;

                BC_TEST_DEBUG_PRINT("BC_TEST: Received Radom - Member: %08x\n\r", auto_conf_msg_receive.random);

                if (auto_conf_msg_receive.control_code == MEMBER_INIT_REQUEST) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Received Init Request\n\r");

                    auto_conf_msg_transmit.control_code = MEMBER_INIT_ANSWER;
                    if (bc_test.nodeid_ix == (DRV_ETHPHY_PLCA_NODE_COUNT - 1)) {
                        bc_test.nodeid_ix = 2;
                    } else {
                        bc_test.nodeid_ix++;
                    }
                    auto_conf_msg_transmit.node_id = bc_test.nodeid_ix;

                    auto_conf_msg_transmit.ip4.v[0] = bc_test.MyIpAddr.v[0];
                    auto_conf_msg_transmit.ip4.v[1] = bc_test.MyIpAddr.v[1];
                    auto_conf_msg_transmit.ip4.v[2] = bc_test.MyIpAddr.v[2];
                    auto_conf_msg_transmit.ip4.v[3] = BC_TEST_calculateCRC8((uint8_t *) & auto_conf_msg_receive.mac, 6);

                    BC_TEST_DEBUG_PRINT("BC_TEST: Data Sent Init - Node Id:%d\n\r", auto_conf_msg_transmit.node_id);
                }

                if (auto_conf_msg_receive.control_code == MEMBER_LIVE_REQUEST) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Received Live Request\n\r");
                    auto_conf_msg_transmit.control_code = MEMBER_LIVE_ANSWER;
                    BC_TEST_DEBUG_PRINT("BC_TEST: Data Sent Live \n\r");
                }

                bc_test.tick_flag_100ms = false;
                while (bc_test.tick_flag_100ms == false);

                auto_conf_msg_transmit.counter_100ms = bc_test.tick_100ms;
                auto_conf_msg_transmit.led_state = bc_test.led_state;

                BC_COM_send((uint8_t*) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
                BC_TEST_DEBUG_PRINT("BC_TEST: Data Sent - Controller\n\r");
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));

                while (BC_COM_is_idle() == false);
                BC_COM_listen(sizeof (AUTOCONFMSG));

                bc_test.countdown = TIMEOUT_20_SECONDS;
                bc_test.state = BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST;
            }
            break;


        case BC_TEST_STATE_IDLE:
            if (bc_test.nodeid_ix > 1) {
                if (bc_test.countdown == 0) {
                    bc_test.countdown = TIMEOUT_1_SECOND;
                    bc_test.state = BC_TEST_STATE_MEMBER_LIVE_START_REQUEST;
                }
            }
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

    if (bc_test.watchdog) {
        bc_test.watchdog--;
    }

    bc_test.tick_100ms++;
    bc_test.tick_flag_100ms = true;

    if ((bc_test.tick_100ms % 20) == 0) {
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
    "BC_TEST_STATE_MEMBER_INIT_START_REQUEST",
    "BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER",
    "BC_TEST_STATE_MEMBER_INIT_PROCESS_REQUESTED_DATA",
    "BC_TEST_STATE_MEMBER_LIVE_START_REQUEST",
    "BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER",
    "BC_TEST_STATE_MEMBER_LIVE_PROCESS_REQUESTED_DATA",
    "BC_TEST_STATE_MEMBER_INIT_DECIDE_TO_BE_COORDINATOR_NODE",
    "BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST",
    "BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST",
    "BC_TEST_STATE_IDLE",
    "BC_TEST_VOID"
};

void BC_TEST_Print_State_Change_And_Trigger_Watchdog(void) {
    static BC_TEST_STATES states = BC_TEST_VOID;
    if (states != bc_test.state) {
        states = bc_test.state;
        bc_test.watchdog = TIMEOUT_30_SECONDS;
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
                BC_TEST_DEBUG_DUMP_PRINT("   %s\n\r", str);
            }
            BC_TEST_DEBUG_DUMP_PRINT("%08x: ", puc);
            flag = 1;
            jx = 0;
        }
        BC_TEST_DEBUG_DUMP_PRINT(" %02x", *puc);
        if ((*puc > 31) && (*puc < 127))
            str[jx++] = *puc;
        else
            str[jx++] = '.';
        puc++;
    }
    str[jx] = 0;
    BC_TEST_DEBUG_DUMP_PRINT("   %s", str);
    BC_TEST_DEBUG_DUMP_PRINT("\n\r");
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

    //    if (bc_test.init_done == false) {
    //        BC_TEST_DEBUG_PRINT("BC_TEST: state -> BC_TEST_STATE_INIT_START\r\n");
    //        SYS_Initialization_TCP_Stack();
    //        SYS_Task_Start_TCP();        
    //        bc_test.state = BC_TEST_STATE_INIT_START;
    //    } else {
    BC_TEST_DEBUG_PRINT("BC_TEST: state -> BC_TEST_STATE_MEMBER_START_REQUEST\r\n");
    bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * 10) / 16;
    BC_TEST_DEBUG_PRINT("BC_TEST: Random Delay %d ms\r\n", bc_test.countdown * TIMER_MS_RESOLUTION);
    bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
    //    }
}

static void my_reinit(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    BC_COM_DeInitialize_Runtime();
    BC_COM_Initialize_Runtime();
    bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * 10) / 16;
    bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
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
        BC_TEST_DEBUG_PRINT("BC_TEST: Local miim open: failed!\r\n");
        bc_test.MiimObj.miimHandle = 0;
        res = DRV_MIIM_RES_OP_INTERNAL_ERR;
    } else {

        miimSetup.hostClockFreq = (uint32_t) TCPIP_INTMAC_PERIPHERAL_CLK;
        miimSetup.maxBusFreq = 2000000;
        miimSetup.setupFlags = 0;

        /*  Setup the miim driver instance. */
        res = bc_test.MiimObj.miimBase->DRV_MIIM_Setup(bc_test.MiimObj.miimHandle, &miimSetup);
        if (res < 0) {
            BC_TEST_DEBUG_PRINT("BC_TEST: Local miim setup: failed!\r\n");
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

static void my_plca_set_config(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    uint16_t node_id;
    uint16_t node_count;
    DRV_MIIM_RESULT opRes = DRV_MIIM_RES_OK;
    uint16_t data;

    if (argc != 3) {
        BC_TEST_DEBUG_PRINT("Usage: nds <node_id> <node_count>\n\r");
        return;
    }

    bc_test_node_id = strtoul(argv[1], NULL, 10);
    bc_test_node_count = strtoul(argv[2], NULL, 10);

}

void BC_TEST_write_miim(uint16_t reg, uint16_t value) {
    DRV_MIIM_RESULT opRes = DRV_MIIM_RES_OK;
    uint16_t data;

    BC_TEST_miim_init();

    do {
        opRes = Write_Phy_Register(&bc_test.MiimObj, 0, reg, value);
        vTaskDelay(10U / portTICK_PERIOD_MS);
    } while (opRes == DRV_MIIM_RES_PENDING);

    if (opRes < 0) {
        /* In case of an error, report and close miim instance. */
        BC_TEST_DEBUG_PRINT("BC_TEST: Register Write Error occurred:%d\r\n", opRes);
    } else if (opRes == DRV_MIIM_RES_OK) /* Check operation is completed. */ {
        //BC_TEST_DEBUG_PRINT("BC_TEST:  Register set, Node Id: %d, Node count: %d. \r\n", R2F(data, PHY_PLCA_CTRL1_ID), R2F(data, PHY_PLCA_CTRL1_NCNT));
    } else {
        BC_TEST_DEBUG_PRINT("BC_TEST: Register Write opRes: %d\n\r", opRes);
    }

    BC_TEST_miim_close();
}

void BC_TEST_plca_write_config(uint16_t node_id, uint16_t node_count) {

    DRV_MIIM_RESULT opRes = DRV_MIIM_RES_OK;
    uint16_t data;

    BC_TEST_miim_init();

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
    {"nds", (SYS_CMD_FNC) my_plca_set_config, ": Node set Write"},
    {"ndr", (SYS_CMD_FNC) my_plca_read_config, ": Node Config Read: ndr"},
    {"reinit", (SYS_CMD_FNC) my_reinit, ": Re-Initialze Server and Client"}
};

bool BC_TEST_Command_Init(void) {
    bool ret = false;

    if (!SYS_CMD_ADDGRP(msd_cmd_tbl, sizeof (msd_cmd_tbl) / sizeof (*msd_cmd_tbl), "test", ": Test Commands")) {
        ret = true;
    }
    return ret;
}

void BC_TEST_NetDown(void) {
    TCPIP_NET_HANDLE netH;

    netH = TCPIP_STACK_NetHandleGet("eth0");
    TCPIP_STACK_NetDown(netH);
    BC_TEST_DEBUG_PRINT("BC_TEST: Net Down\n\r");
}

bool BC_TEST_NetUp(void) {
    TCPIP_NET_HANDLE netH;
    SYS_MODULE_OBJ tcpipStackObj;
    TCPIP_STACK_INIT tcpip_init_data = {
        {0}
    };
    TCPIP_NETWORK_CONFIG ifConf, *pIfConf;
    uint16_t net_ix = 0;
    bool res = false;

    netH = TCPIP_STACK_NetHandleGet("eth0");
    net_ix = TCPIP_STACK_NetIndexGet(netH);

    // get the data passed at initialization
    tcpipStackObj = TCPIP_STACK_Initialize(0, 0);
    TCPIP_STACK_InitializeDataGet(tcpipStackObj, &tcpip_init_data);

    pIfConf = &ifConf;
    memcpy(pIfConf, tcpip_init_data.pNetConf + net_ix, sizeof (*pIfConf));

    // change the power mode to FULL
    pIfConf->powerMode = TCPIP_STACK_IF_POWER_FULL;
    res = TCPIP_STACK_NetUp(netH, pIfConf);
    BC_TEST_DEBUG_PRINT("BC_TEST: Net Up\n\r");
}

void BC_TEST_SetNodeID_and_MAXcount(uint16_t NodeId, uint16_t MaxCount) {

    bc_test_node_id = NodeId;
    bc_test_node_count = MaxCount;

}

#define BC_TEST_POLYNOMIAL_16 0x8005 

uint16_t BC_TEST_calculateCRC16(uint8_t *data, int length) {
    uint16_t crc = 0;

    for (int i = 0; i < length; i++) {
        crc ^= (uint16_t) data[i] << 8;

        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ BC_TEST_POLYNOMIAL_16;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

#define BC_TEST_POLYNOMIAL_8 0x07 

uint8_t BC_TEST_calculateCRC8(uint8_t *data, int length) {
    uint8_t crc = 0;

    for (int i = 0; i < length; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ BC_TEST_POLYNOMIAL_8;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/*
void BC_TEST_CopyMAC(uint8_t *target,uint8_t *origin){

    target[0] = origin[0];

}



                auto_conf_msg_transmit.mac.v[0] = bc_test.MyMacAddr.v[0];
                auto_conf_msg_transmit.mac.v[1] = bc_test.MyMacAddr.v[1];
                auto_conf_msg_transmit.mac.v[2] = bc_test.MyMacAddr.v[2];
                auto_conf_msg_transmit.mac.v[3] = bc_test.MyMacAddr.v[3];
                auto_conf_msg_transmit.mac.v[4] = bc_test.MyMacAddr.v[4];
                auto_conf_msg_transmit.mac.v[5] = bc_test.MyMacAddr.v[5];

 */
/*******************************************************************************
 End of File
 */
