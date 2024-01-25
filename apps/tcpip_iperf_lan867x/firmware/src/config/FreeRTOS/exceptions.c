/*******************************************************************************
  System Exceptions File

  File Name:
    exceptions.c

  Summary:
    This file contains a function which overrides the default _weak_ exception
    handlers provided by the interrupt.c file.

  Description:
    This file redefines the default _weak_  exception handler with a more debug
    friendly one. If an unexpected exception occurs the code will stop in a
    while(1) loop.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "configuration.h"
#include "interrupts.h"
#include "definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: Exception Handling Routine
// *****************************************************************************
// *****************************************************************************

/* Brief default interrupt handlers for core IRQs.*/



bool my_SERCOM1_USART_Write(void *buffer, const size_t size) {
    uint8_t *pu8Data = (uint8_t*) buffer;
    uint32_t u32Index = 0U;

    /* Blocks while buffer is being transferred */
    while (u32Index < size) {
        /* Check if USART is ready for new data */
        while ((SERCOM1_REGS->USART_INT.SERCOM_INTFLAG & (uint8_t) SERCOM_USART_INT_INTFLAG_DRE_Msk) == 0U) {
            /* Do nothing */
        }
        SERCOM1_REGS->USART_INT.SERCOM_DATA = pu8Data[u32Index];
        /* Increment index */
        u32Index++;
    }
}

void __attribute__((noreturn)) NonMaskableInt_Handler(void) {
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    while (true) {
    }
}

void __attribute__((noreturn)) HardFault_Handler(void) {
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    char eBuff[200];
    sprintf(eBuff, "\n\rEXCEPTION: Hard Fault\n\r");
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));
    while (true) {
    }
}

void __attribute__((noreturn)) DebugMonitor_Handler(void) {
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    while (true) {
    }
}

void __attribute__((noreturn)) MemoryManagement_Handler(void) {
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    char eBuff[200];
    sprintf(eBuff, "\n\rEXCEPTION: Memory Management\n\r");
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));
    while (true) {
    }
}

void __attribute__((noreturn)) BusFault_Handler(void) {
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    char eBuff[200];
    sprintf(eBuff, "\n\rEXCEPTION: Bus Fault\n\r");
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));
    while (true) {
    }
}

void __attribute__((noreturn)) UsageFault_Handler(void) {
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    char eBuff[200];
    sprintf(eBuff, "\n\rEXCEPTION: Usage Fault\n\r");
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));
    while (true) {
    }
}
/*******************************************************************************
 End of File
 */
