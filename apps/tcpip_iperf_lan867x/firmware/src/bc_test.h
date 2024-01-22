/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    bc_test.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "BC_TEST_Initialize" and "BC_TEST_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "BC_TEST_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

#ifndef _BC_TEST_H
#define _BC_TEST_H

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
#include "driver/ethphy/src/dynamic/drv_extphy_lan867x.h"

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

    // *****************************************************************************

    /* Application states

      Summary:
        Application states enumeration

      Description:
        This enumeration defines the valid application states.  These states
        determine the behavior of the application at various times.
     */

    typedef enum {
        /* Application's state machine's initial state. */
        BC_TEST_STATE_INIT_START = 0,
        BC_TEST_STATE_INIT_TCPIP_WAIT_START,
        BC_TEST_STATE_INIT_TCPIP_WAIT_FOR_IP,
        BC_TEST_STATE_MEMBER_INIT_START_REQUEST,
        BC_TEST_STATE_MEMBER_INIT_WAIT_FOR_REQUESTED_ANSWER,
        BC_TEST_STATE_MEMBER_INIT_PROCESS_REQUESTED_DATA,
        BC_TEST_STATE_MEMBER_INIT_DECIDE_TO_BE_COORDINATOR_NODE,
        BC_TEST_STATE_MEMBER_LIVE_START_REQUEST,
        BC_TEST_STATE_MEMBER_LIVE_WAIT_FOR_REQUESTED_ANSWER,
        BC_TEST_STATE_MEMBER_LIVE_PROCESS_REQUESTED_DATA,       
        BC_TEST_STATE_COORDINATOR_WAIT_FOR_REQUEST,
        BC_TEST_STATE_COORDINATOR_ANSWER_REQUEST,
        BC_TEST_STATE_IDLE,
        BC_TEST_VOID
    } BC_TEST_STATES;


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
        BC_TEST_STATES state;
        SYS_TIME_HANDLE timer_client_hdl;
        volatile uint32_t countdown;
        volatile uint32_t timeout;
        volatile uint32_t timeout_live_request;
        volatile uint32_t watchdog;
        IPV4_ADDR MyIpAddr;
        TCPIP_MAC_ADDR MyMacAddr;
        LAN867X_REG_OBJ MiimObj;
        int32_t tick_100ms;
        volatile bool tick_flag_100ms;
        bool led_state;
        bool init_done;
        uint32_t random;
        uint8_t nodeid_ix;
    } BC_TEST_DATA;

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
        void BC_TEST_Initialize ( void )

      Summary:
         MPLAB Harmony application initialization routine.

      Description:
        This function initializes the Harmony application.  It places the
        application in its initial state and prepares it to run so that its
        BC_TEST_Tasks function can be called.

      Precondition:
        All other system initialization routines should be called before calling
        this routine (in "SYS_Initialize").

      Parameters:
        None.

      Returns:
        None.

      Example:
        <code>
        BC_TEST_Initialize();
        </code>

      Remarks:
        This routine must be called from the SYS_Initialize function.
     */

    void BC_TEST_Initialize(void);


    /*******************************************************************************
      Function:
        void BC_TEST_Tasks ( void )

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
        BC_TEST_Tasks();
        </code>

      Remarks:
        This routine must be called from SYS_Tasks() routine.
     */

    void BC_TEST_Tasks(void);

    void BC_TEST_DumpMem(uint32_t addr, uint32_t count);
    bool BC_TEST_Command_Init(void);
    void BC_TEST_plca_write_config(uint16_t node_id, uint16_t node_count);
    void BC_TEST_write_miim(uint16_t reg, uint16_t value);
    void BC_TEST_SetNodeID_and_MAXcount(uint16_t NodeId, uint16_t MaxCount);
    void BC_TEST_NetDown(void);
    bool BC_TEST_NetUp(void);

    //DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _BC_TEST_H */

/*******************************************************************************
 End of File
 */

