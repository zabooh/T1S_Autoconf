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


//#define __BC_TEST_DEBUG_PRINT 
#ifdef __BC_TEST_DEBUG_PRINT
#define BC_TEST_DEBUG_PRINT(fmt, ...)  SYS_CONSOLE_PRINT(fmt, ##__VA_ARGS__)
#else
#define BC_TEST_DEBUG_PRINT(fmt, ...)
#endif

//#define __BC_TEST_DEBUG_DUMP_PRINT 
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
#define TIMEOUT_200_MILLI_SECONDS   2
#define TIMEOUT_1_SECOND            10

#define TIMEOUT_2_SECONDS           10
#define TIMEOUT_3_SECONDS           20
#define TIMEOUT_4_SECONDS           40
#define TIMEOUT_5_SECONDS           50
#define TIMEOUT_6_SECONDS           60
#define TIMEOUT_7_SECONDS           70
#define TIMEOUT_8_SECONDS           80
#define TIMEOUT_9_SECONDS           90
#define TIMEOUT_10_SECONDS          100
#define TIMEOUT_15_SECONDS          150
#define TIMEOUT_20_SECONDS          200
#define TIMEOUT_30_SECONDS          300

#define TIMEOUT_LIVE_REQUEST        TIMEOUT_10_SECONDS
#define TIMEOUT_COORDINATOR         TIMEOUT_15_SECONDS
#define TIMEOUT_WATCHDOG            TIMEOUT_20_SECONDS

#define MEMBER_INIT_REQUEST 1
#define MEMBER_LIVE_REQUEST 2
#define MEMBER_INIT_ANSWER  3
#define MEMBER_LIVE_ANSWER  4

#define RANGE_5_SECONDS      50
#define RANGE_10_SECONDS    100

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

typedef struct {
    uint32_t magic_code;
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

#define BC_TEST_MEMBER       1
#define BC_TEST_COORDINATOR  0

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

extern uint8_t my_mac_str[];

volatile uint16_t bc_test_node_id;
volatile uint16_t bc_test_node_count;

uint8_t BC_TEST_calculateCRC8(uint8_t *data, int length);

void BS_TEST_Check_BC_COM_For_Idle(void);

void BS_TEST_CheckButtons(void);

int32_t tick2_100ms;
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
    bc_test_node_id = 1; //DRV_ETHPHY_PLCA_LOCAL_NODE_ID;
    bc_test_node_count = DRV_ETHPHY_PLCA_NODE_COUNT;
    bc_test.timer_client_hdl = SYS_TIME_TimerCreate(0, SYS_TIME_MSToCount(TIMER_MS_RESOLUTION), &BC_TEST_TimerCallback, (uintptr_t) NULL, SYS_TIME_PERIODIC);
    bc_test.tick_100ms = 0;
    bc_test.led_state = false;
    LED_1_Set();
    LED_2_Set();
    SYS_TIME_TimerStart(bc_test.timer_client_hdl);
    bc_test.init_done = false;
    bc_test.nodeid_ix = 1;
    bc_test.sync = false;

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
    char mac_str[20];


    BC_TEST_Print_State_Change();
    BS_TEST_CheckButtons();
    
    if (bc_test.watchdog == 0) {
        gfx_mono_print_scroll("Soft Watchdog Trg");
        BC_TEST_DEBUG_PRINT("BC_TEST: Soft-Watchdog Triggered\r\n");
        BC_COM_DeInitialize_Runtime();
        BC_COM_Initialize_Runtime();     
        bc_test.timeout = (((TRNG_ReadData() % 0xF) + 1) * RANGE_10_SECONDS) / 16;
        BC_TEST_DEBUG_PRINT("BC_TEST: Watchdog Triggered Restart in %d Ticks\n\r", bc_test.timeout);
        bc_test.watchdog == TIMEOUT_WATCHDOG;
        bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
    }

    switch (bc_test.state) {

            /* ============== Init States ============== */

        case BC_TEST_STATE_INIT_START:

            gfx_mono_ssd1306_init();
            gfx_mono_print_scroll("LAN867x PLCA Node: %d", DRV_ETHPHY_PLCA_LOCAL_NODE_ID);
            gfx_mono_print_scroll("%s", TCPIP_NETWORK_DEFAULT_MAC_ADDR_IDX0);
            gfx_mono_print_scroll("%s", TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0);

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
            memcpy(&mac_ptr->v[0], &bc_test.MyMacAddr.v[0], 6);

            if (dwLastIP.Val != bc_test.MyIpAddr.Val) {

                dwLastIP.Val = bc_test.MyIpAddr.Val;
                BC_TEST_DEBUG_PRINT("BC_TEST: Default IP Address : %d.%d.%d.%d\r\n", bc_test.MyIpAddr.v[0], bc_test.MyIpAddr.v[1], bc_test.MyIpAddr.v[2], bc_test.MyIpAddr.v[3]);
                BC_TEST_DEBUG_PRINT("BC_TEST: Default MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\r\n", bc_test.MyMacAddr.v[0], bc_test.MyMacAddr.v[1], bc_test.MyMacAddr.v[2], bc_test.MyMacAddr.v[3], bc_test.MyMacAddr.v[4], bc_test.MyMacAddr.v[5]);

                bc_test.init_done = true;

                BC_COM_Initialize_Runtime();

                SYS_CONSOLE_PRINT("=============================================\n\r");
                SYS_CONSOLE_PRINT("Build Time %s %s Tag:v3.8.1\n\r", __DATE__, __TIME__);
                gfx_mono_print_scroll("Start System");

                bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * RANGE_10_SECONDS) / 16;
                BC_TEST_NetDown();
                SYS_CONSOLE_PRINT("Start in %d Ticks\n\r", bc_test.countdown);
                bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
            }
            break;




            /* ============== Member Init States ============== */

        case BC_TEST_STATE_MEMBER_INIT_START_REQUEST:
            if (bc_test.countdown) {
                break;
            }
            gfx_mono_print_scroll("Member Init Request");
            BC_TEST_DEBUG_PRINT("BC_TEST: Member Init\n\r");
            if (bc_test.isIperf == true) {
                SERCOM1_USART_Virtual_Receive("iperfk\n");
                bc_test.isIperf = false;
            }
            BC_TEST_NetDown();
            BC_TEST_SetNodeID_and_MAXcount(1, bc_test_node_count);
            BC_TEST_NetUp();
            netH = TCPIP_STACK_NetHandleGet("eth0");
            while (TCPIP_STACK_NetIsReady(netH) == false);

            bc_test.random = TRNG_ReadData();
            memset((void*) &auto_conf_msg_transmit, 0xEE, sizeof (AUTOCONFMSG));
            memcpy(&auto_conf_msg_transmit.mac.v[0], &bc_test.MyMacAddr.v[0], 6);
            
            auto_conf_msg_transmit.magic_code = 0x12345678;
            auto_conf_msg_transmit.random = bc_test.random;
            auto_conf_msg_transmit.origin = BC_TEST_MEMBER;
            auto_conf_msg_transmit.control_code = MEMBER_INIT_REQUEST;

            BC_TEST_DEBUG_PRINT("BC_TEST: Member Init Radom: %s => %08x\n\r", my_mac_str, auto_conf_msg_transmit.random);
            BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));

            BC_COM_send((uint8_t*) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
            BS_TEST_Check_BC_COM_For_Idle();
            BC_COM_listen(sizeof (AUTOCONFMSG));
            bc_test.timeout = TIMEOUT_1_SECOND;
            bc_test.state = BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER;

            break;


        case BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER:
            if (BC_COM_is_data_received() == true) {
                memset((void*) &auto_conf_msg_receive, 0xFF, sizeof (AUTOCONFMSG));
                BC_COM_read_data((uint8_t *) & auto_conf_msg_receive);
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));

                if (auto_conf_msg_receive.origin != BC_TEST_COORDINATOR ||
                        auto_conf_msg_receive.random != auto_conf_msg_transmit.random) {
                    TCPIP_Helper_MACAddressToString(&auto_conf_msg_receive.mac, mac_str, 18);
                    BC_TEST_DEBUG_PRINT("BC_TEST: Member Init Wrong Data Received from %d - %s => %08x\n\r", auto_conf_msg_receive.origin, mac_str, auto_conf_msg_receive.random);
                    bc_test.state = BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER;
                    bc_test.timeout = TIMEOUT_1_SECOND;
                    BS_TEST_Check_BC_COM_For_Idle();
                    BC_COM_listen(sizeof (AUTOCONFMSG));

                    break;
                }

                TCPIP_Helper_MACAddressToString(&auto_conf_msg_receive.mac, mac_str, 18);
                BC_TEST_DEBUG_PRINT("BC_TEST: Member Init Correct Random packet from %d - %s => %0x8\n\r", auto_conf_msg_receive.origin, mac_str, auto_conf_msg_receive.random);
                
                SYS_TIME_TimerStop(bc_test.timer_client_hdl);
                bc_test.tick_100ms = auto_conf_msg_receive.counter_100ms;
                bc_test.led_state = auto_conf_msg_receive.led_state;
                SYS_TIME_TimerStart(bc_test.timer_client_hdl);

                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));

                bc_test.state = BC_TEST_STATE_MEMBER_INIT_PROCESS_REQUESTED_DATA;
                break;
            }
            if (bc_test.timeout == 0) {
                BC_TEST_DEBUG_PRINT("BC_TEST: Member Init Timeout %s %d\n\r", __FILE__, __LINE__);
                BC_COM_listen_stop();
                bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * RANGE_10_SECONDS) / 16;
                BC_TEST_DEBUG_PRINT("BC_TEST: Start as Coordinator in %d Ticks\n\r", bc_test.countdown);
                bc_test.state = BC_TEST_STATE_MEMBER_INIT_DECIDE_TO_BE_COORDINATOR_NODE;
            }
            break;

        case BC_TEST_STATE_MEMBER_INIT_PROCESS_REQUESTED_DATA:
        {
            TCPIP_NET_HANDLE netH;
            IPV4_ADDR ipMask;
            char buff[30];

            bc_test.nodeid_ix = auto_conf_msg_receive.node_id;
            BC_TEST_DEBUG_PRINT("BC_TEST: Member Init Received NodeId:%d\n\r", auto_conf_msg_receive.node_id);
            
            TCPIP_Helper_IPAddressToString(&auto_conf_msg_receive.ip4, buff, 20);
            BC_TEST_DEBUG_PRINT("BC_TEST: Member Init new IP:%s\n\r", buff);
            gfx_mono_print_scroll("New Member IP:");
            gfx_mono_print_scroll("%s", buff);
            bc_test.timeout_live_request = TIMEOUT_LIVE_REQUEST;
            bc_test.sync = true;
            
            BC_TEST_NetDown();
            BC_TEST_SetNodeID_and_MAXcount(bc_test.nodeid_ix, bc_test_node_count);
            BC_TEST_NetUp();
            
            netH = TCPIP_STACK_NetHandleGet("eth0");
            while (TCPIP_STACK_NetIsReady(netH) == false);
            
            ipMask.v[0] = 255;
            ipMask.v[1] = 255;
            ipMask.v[2] = 255;
            ipMask.v[3] = 0;
            TCPIP_STACK_NetAddressSet(netH, &auto_conf_msg_receive.ip4, &ipMask, 1);            
            BC_TEST_DEBUG_PRINT("BC_TEST: New IP Address : %d.%d.%d.%d\r\n", 
                    auto_conf_msg_receive.ip4.v[0], 
                    auto_conf_msg_receive.ip4.v[1], 
                    auto_conf_msg_receive.ip4.v[2], 
                    auto_conf_msg_receive.ip4.v[3]);
                        
            bc_test.state = BC_TEST_STATE_IDLE;
            break;
        }



        case BC_TEST_STATE_MEMBER_INIT_DECIDE_TO_BE_COORDINATOR_NODE:
            if (bc_test.countdown) {
                break;
            }
            if (BC_COM_is_idle() == true) {
                char buff[30];
                sprintf(buff, "%d.%d.%d.%d\r\n", bc_test.MyIpAddr.v[0], bc_test.MyIpAddr.v[1], bc_test.MyIpAddr.v[2], bc_test.MyIpAddr.v[3]);
                gfx_mono_print_scroll("%s", buff);
                gfx_mono_print_scroll("Set to Coordinator");
                BC_TEST_NetDown();
                BC_TEST_SetNodeID_and_MAXcount(0, bc_test_node_count);
                BC_TEST_NetUp();
                netH = TCPIP_STACK_NetHandleGet("eth0");
                while (TCPIP_STACK_NetIsReady(netH) == false);
                //SERCOM1_USART_Virtual_Receive("iperf -u -s\n");
                //bc_test.isIperf = true;
                BC_COM_listen(sizeof (AUTOCONFMSG));
                bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * RANGE_10_SECONDS) / 16;
                bc_test.timeout = TIMEOUT_COORDINATOR;
                bc_test.state = BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST;
            }
            break;



            /* ============== Member Live States ============== */

        case BC_TEST_STATE_MEMBER_LIVE_START_REQUEST:
            if (bc_test.countdown) {
                break;
            }
            netH = TCPIP_STACK_NetHandleGet("eth0");
            while (TCPIP_STACK_NetIsReady(netH) == false);

            bc_test.random = TRNG_ReadData();

            memset((void*) &auto_conf_msg_transmit, 0xEE, sizeof (AUTOCONFMSG));
            memcpy(&auto_conf_msg_transmit.mac.v[0], &bc_test.MyMacAddr.v[0], 6);

            auto_conf_msg_transmit.magic_code = 0x12345678;
            auto_conf_msg_transmit.random = bc_test.random;
            auto_conf_msg_transmit.origin = BC_TEST_MEMBER;
            auto_conf_msg_transmit.control_code = MEMBER_LIVE_REQUEST;

            TCPIP_Helper_MACAddressToString(&auto_conf_msg_transmit.mac, mac_str, 18);
            BC_TEST_DEBUG_PRINT("BC_TEST: Member Live Radom: %s => %08x\n\r", mac_str, auto_conf_msg_transmit.random);
            BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));

            bc_test.state = BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER;

            BC_COM_send((uint8_t*) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
            BS_TEST_Check_BC_COM_For_Idle();
            BC_COM_listen(sizeof (AUTOCONFMSG));
            bc_test.timeout = TIMEOUT_1_SECOND;

            break;

        case BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER:
            if (BC_COM_is_data_received() == true) {

                memset((void*) &auto_conf_msg_receive, 0xFF, sizeof (AUTOCONFMSG));
                BC_COM_read_data((uint8_t *) & auto_conf_msg_receive);
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));

                TCPIP_Helper_MACAddressToString(&auto_conf_msg_receive.mac, mac_str, 18);

                if (auto_conf_msg_receive.origin != BC_TEST_COORDINATOR ||
                        auto_conf_msg_receive.random != auto_conf_msg_transmit.random) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Member Live Wrong Data Received from %d - %s => %08x\n\r", auto_conf_msg_receive.origin, mac_str, auto_conf_msg_receive.random);
                    bc_test.state = BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER;

                    BS_TEST_Check_BC_COM_For_Idle();
                    BC_COM_listen(sizeof (AUTOCONFMSG));
                    bc_test.timeout = TIMEOUT_1_SECOND;
                    break;
                }

                BC_TEST_DEBUG_PRINT("BC_TEST: Member Live Correct Random packet from %d - %s => %08x\n\r", auto_conf_msg_receive.origin, mac_str, auto_conf_msg_receive.random);
                SYS_TIME_TimerStop(bc_test.timer_client_hdl);
                bc_test.tick_100ms = auto_conf_msg_receive.counter_100ms;
                bc_test.led_state = auto_conf_msg_receive.led_state;
                SYS_TIME_TimerStart(bc_test.timer_client_hdl);

                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));

                bc_test.state = BC_TEST_STATE_MEMBER_LIVE_PROCESS_REQUESTED_DATA;
                break;
            }
            if (bc_test.timeout == 0) {
                gfx_mono_print_scroll("Member Live Timeout");
                BC_TEST_DEBUG_PRINT("BC_TEST: Member Live Timeout %s %d\n\r", __FILE__, __LINE__);
                BC_COM_listen_stop();
                bc_test.sync = false;
                bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
            }
            break;


        case BC_TEST_STATE_MEMBER_LIVE_PROCESS_REQUESTED_DATA:
        {
            bc_test.timeout = TIMEOUT_LIVE_REQUEST;
            bc_test.state = BC_TEST_STATE_IDLE;
            break;
        }


            /* ============== Coordinator States ============== */

        case BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST:
            if (bc_test.countdown) {
                break;
            }            
            if (BC_COM_is_data_received() == true) {
                memset((void*) &auto_conf_msg_receive, 0xFF, sizeof (AUTOCONFMSG));
                BC_COM_read_data((uint8_t *) & auto_conf_msg_receive);
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_receive, sizeof (AUTOCONFMSG));

                if (auto_conf_msg_receive.origin == BC_TEST_COORDINATOR) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Data Received from other Coordinator-\n\r");
                    BC_COM_listen_stop();
                    bc_test.timeout = (((TRNG_ReadData() % 0xF) + 1) * RANGE_10_SECONDS) / 16;
                    BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Restart as Member in %d Ticks\n\r", bc_test.timeout);
                    bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
                    break;
                }
                TCPIP_Helper_MACAddressToString(&auto_conf_msg_receive.mac, mac_str, 18);
                BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Data Received from Member: %s => %08x \n\r", mac_str, auto_conf_msg_receive.random);

                BC_COM_listen_stop();
                bc_test.state = BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST;
            }
            if (bc_test.timeout == 0) {
                BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Timeout %s %d\n\r", __FILE__, __LINE__);
                BC_COM_listen_stop();                 
                bc_test.timeout = (((TRNG_ReadData() % 0xF) + 1) * RANGE_10_SECONDS) / 16;
                BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Restart as Member in %d Ticks\n\r", bc_test.timeout);
                bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
            }
            break;

        case BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST:
            if (BC_COM_is_idle() == true) {

                memset((void*) &auto_conf_msg_transmit, 0xCC, sizeof (AUTOCONFMSG));
                memcpy(&auto_conf_msg_transmit.mac.v[0], &bc_test.MyMacAddr.v[0], 6);

                auto_conf_msg_transmit.random = auto_conf_msg_receive.random;
                auto_conf_msg_transmit.origin = BC_TEST_COORDINATOR;

                TCPIP_Helper_MACAddressToString(&auto_conf_msg_receive.mac, mac_str, 18);
                BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Received Radom from Member: %s => %08x\n\r", mac_str, auto_conf_msg_receive.random);

                if (auto_conf_msg_receive.control_code == MEMBER_INIT_REQUEST) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Received Init Request\n\r");

                    auto_conf_msg_transmit.control_code = MEMBER_INIT_ANSWER;
                    if (bc_test.nodeid_ix == (DRV_ETHPHY_PLCA_NODE_COUNT - 2)) { // Node 7 is reserved for an non automatic node
                        bc_test.nodeid_ix = 2;
                    } else {
                        bc_test.nodeid_ix++;
                    }
                    auto_conf_msg_transmit.node_id = bc_test.nodeid_ix;

                    auto_conf_msg_transmit.ip4.v[0] = bc_test.MyIpAddr.v[0];
                    auto_conf_msg_transmit.ip4.v[1] = bc_test.MyIpAddr.v[1];
                    auto_conf_msg_transmit.ip4.v[2] = bc_test.MyIpAddr.v[2];
                    auto_conf_msg_transmit.ip4.v[3] = BC_TEST_calculateCRC8((uint8_t *) & auto_conf_msg_receive.mac, 6);

                    BC_TEST_DEBUG_PRINT("BC_TEST: Member MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
                            auto_conf_msg_receive.mac.v[0], 
                            auto_conf_msg_receive.mac.v[1], 
                            auto_conf_msg_receive.mac.v[2], 
                            auto_conf_msg_receive.mac.v[3], 
                            auto_conf_msg_receive.mac.v[4], 
                            auto_conf_msg_receive.mac.v[5]);
                    
                    BC_TEST_DEBUG_PRINT("BC_TEST: Send IP Address : %d.%d.%d.%d\r\n", 
                            auto_conf_msg_transmit.ip4.v[0], 
                            auto_conf_msg_transmit.ip4.v[1], 
                            auto_conf_msg_transmit.ip4.v[2], 
                            auto_conf_msg_transmit.ip4.v[3]);

                    TCPIP_Helper_MACAddressToString(&auto_conf_msg_transmit.mac, mac_str, 18);
                    BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Data Sent Init - Node Id:%d to %s\n\r", auto_conf_msg_transmit.node_id, mac_str);
                }

                if (auto_conf_msg_receive.control_code == MEMBER_LIVE_REQUEST) {
                    BC_TEST_DEBUG_PRINT("BC_TEST: Coordinator Received Live Request\n\r");
                    auto_conf_msg_transmit.control_code = MEMBER_LIVE_ANSWER;
                }

                bc_test.tick_flag_100ms = false;
                while (bc_test.tick_flag_100ms == false);
                
                auto_conf_msg_transmit.magic_code = 0x12345678;
                auto_conf_msg_transmit.counter_100ms = bc_test.tick_100ms;
                auto_conf_msg_transmit.led_state = bc_test.led_state;
                
                BC_COM_send((uint8_t*) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));
                BC_TEST_DEBUG_DUMP_PACKET((uint32_t) & auto_conf_msg_transmit, sizeof (AUTOCONFMSG));

                BS_TEST_Check_BC_COM_For_Idle();
                BC_COM_listen(sizeof (AUTOCONFMSG));

                bc_test.timeout = TIMEOUT_20_SECONDS;
                bc_test.state = BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST;

            }
            break;


            /* ============== General Idle State ============== */

        case BC_TEST_STATE_IDLE:
            if (bc_test.nodeid_ix > 1) {
                if (bc_test.timeout_live_request == 0) {
                    bc_test.timeout_live_request = TIMEOUT_LIVE_REQUEST;
                    bc_test.countdown = (((TRNG_ReadData() % 0xF) + 1) * RANGE_10_SECONDS) / 16;
                    bc_test.state = BC_TEST_STATE_MEMBER_LIVE_START_REQUEST;
                }
            }
            break;

        default:
        {
            BC_TEST_DEBUG_PRINT("BC_TEST: I should be never here: %s %d\n\r", __FILE__, __LINE__);
            while (1);
            break;
        }
    }
}

void BC_TEST_TimerCallback(uintptr_t context) {

    if (bc_test.countdown) {
        bc_test.countdown--;
    }

    if (bc_test.timeout) {
        bc_test.timeout--;
    }

    if (bc_test.timeout_live_request) {
        bc_test.timeout_live_request--;
    }

    if (bc_test.watchdog) {
        bc_test.watchdog--;
    }

    bc_test.tick_100ms++;
    bc_test.tick_flag_100ms = true;

    tick2_100ms++;
    
    if ((bc_test.tick_100ms % 20) == 0) {
        if (bc_test.led_state == false) {
            bc_test.led_state = true;
            LED_1_Set();
        } else {
            LED_1_Clear();
            bc_test.led_state = false;
        }
    }


    if (bc_test.state == BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST
            || bc_test.state == BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST) {
        if (bc_test.led_state == false) {
            LED_2_Set();
        } else {
            LED_2_Clear();
        }
    } else {
        if (bc_test.sync == false) {
            LED_2_Clear();
        } else {
            LED_2_Set();
        }
    }


}

void convertToTime(uint32_t counter, uint32_t *hours, uint32_t *minutes, uint32_t *seconds, uint32_t *hundredths) {
    uint32_t totalMilliseconds = counter * 100;

    *hundredths = totalMilliseconds / 100;
    *hundredths = *hundredths % 10;
    
    *seconds = totalMilliseconds / 1000;
    *seconds = *seconds % 60;
           
    *minutes = totalMilliseconds / (60*1000);
    *minutes = *minutes % 60;
    *minutes = *minutes % 60;
    
    *hours = totalMilliseconds;

    *hours = totalMilliseconds / (60*60*1000);
    *hours = *hours % (60*60);
    *hours = *hours % 60;

}

char *app_states_str[] = {
    "BC_TEST_STATE_INIT_START",
    "BC_TEST_STATE_INIT_TCPIP_WAIT_START",
    "BC_TEST_STATE_INIT_TCPIP_WAIT_FOR_IP",
    "BC_TEST_STATE_MEMBER_INIT_START_REQUEST",
    "BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER",
    "BC_TEST_STATE_MEMBER_INIT_PROCESS_REQUESTED_DATA",
    "BC_TEST_STATE_MEMBER_INIT_DECIDE_TO_BE_COORDINATOR_NODE",
    "BC_TEST_STATE_MEMBER_LIVE_START_REQUEST",
    "BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER",
    "BC_TEST_STATE_MEMBER_LIVE_PROCESS_REQUESTED_DATA",
    "BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST",
    "BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST",
    "BC_TEST_STATE_IDLE",
    "BC_TEST_VOID"
};

void BC_TEST_Print_State_Change(void) {
    static BC_TEST_STATES states = BC_TEST_VOID;
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
    uint32_t ms_100;
    char time_str[30];
    int32_t diff;
    static int32_t old = 0;

    if (states != bc_test.state) {
        states = bc_test.state;
        bc_test.watchdog = TIMEOUT_WATCHDOG;
        diff = tick2_100ms - old;
        old = tick2_100ms;
        convertToTime(diff, &hours, &minutes, &seconds, &ms_100);
        sprintf(time_str, "%02d:%02d:%02d.%01d", hours, minutes, seconds, ms_100);
        SYS_CONSOLE_PRINT("%s %s\n\r", time_str, app_states_str[states]);
        // BC_TEST_DEBUG_PRINT("%s\n\r", app_states_str[states]);
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
                SYS_CONSOLE_PRINT("   %s\n\r", str);
            }
            SYS_CONSOLE_PRINT("%08x: ", puc);
            flag = 1;
            jx = 0;
        }
        SYS_CONSOLE_PRINT(" %02x", *puc);
        if ((*puc > 31) && (*puc < 127))
            str[jx++] = *puc;
        else
            str[jx++] = '.';
        puc++;
    }
    str[jx] = 0;
    SYS_CONSOLE_PRINT("   %s", str);
    SYS_CONSOLE_PRINT("\n\r");
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
    BC_TEST_DEBUG_PRINT("BC_TEST: state -> BC_TEST_STATE_MEMBER_START_REQUEST\r\n");
    bc_test.timeout = (((TRNG_ReadData() % 0xF) + 1) * 10) / 16;
    BC_TEST_DEBUG_PRINT("BC_TEST: Random Delay %d ms\r\n", bc_test.timeout * TIMER_MS_RESOLUTION);
    bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
}

static void my_reinit(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    BC_COM_DeInitialize_Runtime();
    BC_COM_Initialize_Runtime();
    bc_test.timeout = (((TRNG_ReadData() % 0xF) + 1) * 10) / 16;
    bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
}

void BS_TEST_Check_BC_COM_For_Idle(void) {
    bc_test.timeout = TIMEOUT_5_SECONDS;
    do {
        if (bc_test.timeout == 0) {
            BC_COM_DeInitialize_Runtime();
            BC_COM_Initialize_Runtime();
            bc_test.timeout = (((TRNG_ReadData() % 0xF) + 1) * 10) / 16;
            bc_test.state = BC_TEST_STATE_MEMBER_INIT_START_REQUEST;
        }
    } while (BC_COM_is_idle() == false);
}

static void my_ex(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    volatile uint32_t *ptr=(volatile uint32_t *)0xFFFFFFFF;
    *ptr=0xAFFE;
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    
}

static void my_heap_overflow(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    volatile int *p;
    p = pvPortMalloc(100000);
}

void my_heap(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    HeapStats_t xHeapStats;
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "\n\rHeap Statistics\r\n");

    vPortGetHeapStats(&xHeapStats);
    
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "configTOTAL_HEAP_SIZE \t: %d\r\n", configTOTAL_HEAP_SIZE);        
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "xAvailableHeapSpaceInBytes \t: %d\r\n", xHeapStats.xAvailableHeapSpaceInBytes);
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "xSizeOfLargestFreeBlockInBytes \t: %d\r\n", xHeapStats.xSizeOfLargestFreeBlockInBytes);
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "xSizeOfSmallestFreeBlockInBytes \t: %d\r\n", xHeapStats.xSizeOfSmallestFreeBlockInBytes);
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "xNumberOfFreeBlocks \t: %d\r\n", xHeapStats.xNumberOfFreeBlocks);
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "xMinimumEverFreeBytesRemaining \t: %d\r\n", xHeapStats.xMinimumEverFreeBytesRemaining);
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "xNumberOfSuccessfulAllocations \t: %d\r\n", xHeapStats.xNumberOfSuccessfulAllocations);
    (*pCmdIO->pCmdApi->print)(cmdIoParam, "xNumberOfSuccessfulFrees \t: %d\r\n", xHeapStats.xNumberOfSuccessfulFrees);
 //   (*pCmdIO->pCmdApi->print)(cmdIoParam, "xNumberOfFaileddAllocations     : %d\r\n", xHeapStats.xNumberOfFaileddAllocations);

}

const SYS_CMD_DESCRIPTOR msd_cmd_tbl[] = {
    {"dump", (SYS_CMD_FNC) my_dump, ": dump memory"},
    {"run", (SYS_CMD_FNC) my_run, ": start application"},
    {"heap", (SYS_CMD_FNC) my_heap, ": show heap status"},
    {"reinit", (SYS_CMD_FNC) my_reinit, ": Re-Initialze Server and Client"},
    {"ex", (SYS_CMD_FNC) my_ex, ": force exception"},
    {"mf", (SYS_CMD_FNC) my_heap_overflow, ": force malloc fail"},
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

    gfx_mono_print_scroll("Id:%d Max:%d", NodeId, MaxCount);
    bc_test_node_id = NodeId;
    bc_test_node_count = MaxCount;

}

uint16_t BC_TEST_calculateCRC16(uint8_t *data, int length) {
    uint16_t crc = 0;

#define BC_TEST_POLYNOMIAL_16 0x8005 

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

uint8_t BC_TEST_calculateCRC8(uint8_t *data, int length) {
    uint8_t crc = 0;

#define BC_TEST_POLYNOMIAL_8 0x07 

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

void BS_TEST_CheckButtons(void) {
    static int old_but1 = 0;
    int temp_but1 = BUTTON1_Get();
    if (temp_but1 && !old_but1) {
        LED1_Set();
    }
    if (!temp_but1 && old_but1) {
        LED1_Clear();
        //SERCOM1_USART_Virtual_Receive("iperf -u -s\n");
        //gfx_mono_print_scroll("iperf TCP server");
    }
    old_but1 = temp_but1;

    static int old_but2 = 0;
    int temp_but2 = BUTTON2_Get();
    if (temp_but2 && !old_but2) {
        LED2_Set();
    }
    if (!temp_but2 && old_but2) {
        LED2_Clear();
        SERCOM1_USART_Virtual_Receive("iperf -u -s\n");
        gfx_mono_print_scroll("iperf UDP server");
    }
    old_but2 = temp_but2;

    static int old_but3 = 0;
    int temp_but3 = BUTTON3_Get();
    if (temp_but3 && !old_but3) {
        LED3_Set();
    }
    if (!temp_but3 && old_but3) {
        LED3_Clear();
        SERCOM1_USART_Virtual_Receive("iperf -u -c 192.168.0.150\n");
        gfx_mono_print_scroll("iperf UDP client");
    }
    old_but3 = temp_but3;
}

void* BC_Test_Calloc(size_t nElems, size_t elemSize) {
    size_t nBytes = nElems * elemSize;

    void* ptr = pvPortMalloc(nBytes);
    if (ptr) {
        memset(ptr, 0, nBytes);
    }

    return ptr;
}

/*******************************************************************************
 End of File
 */
