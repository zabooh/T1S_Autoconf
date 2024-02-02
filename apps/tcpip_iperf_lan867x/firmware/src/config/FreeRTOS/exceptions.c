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

void gfx_mono_print_scroll(const char* format, ...);

bool my_SERCOM1_USART_Write(void *buffer, const size_t size) {
    uint8_t *pu8Data = (uint8_t*) buffer;
    uint32_t u32Index = 0U;

    while (u32Index < size) {
        while ((SERCOM1_REGS->USART_INT.SERCOM_INTFLAG & (uint8_t) SERCOM_USART_INT_INTFLAG_DRE_Msk) == 0U) {
        }
        SERCOM1_REGS->USART_INT.SERCOM_DATA = pu8Data[u32Index];
        u32Index++;
    }
        while ((SERCOM1_REGS->USART_INT.SERCOM_INTFLAG & (uint8_t) SERCOM_USART_INT_INTFLAG_DRE_Msk) == 0U) {
        }    
}

/* Brief default interrupt handlers for core IRQs.*/

void __attribute__((noreturn)) NonMaskableInt_Handler(void) {
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
    __builtin_software_breakpoint();
#endif
    while (true) {
    }
}

void __attribute__((noreturn)) HardFault_Handler(void) {
    __asm volatile (
            "   tst    LR, #4           \n"
            "   ite    EQ               \n"
            "   mrseq  R0, MSP          \n"
            "   mrsne  R0, PSP          \n"
            "   b      HardFaultHandler  \n"
            );
    while (1);
}


//void __attribute__((noreturn)) HardFault_Handler(void)
//{
//#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
//   __builtin_software_breakpoint();
//#endif
//   while (true)
//   {
//   }
//}

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
    while (true) {
    }
}

void __attribute__((noreturn)) BusFault_Handler(void) {
    __asm volatile (
            "   tst    LR, #4           \n"
            "   ite    EQ               \n"
            "   mrseq  R0, MSP          \n"
            "   mrsne  R0, PSP          \n"
            "   b      BusFaultHandler  \n"
            );
    while (1);
}

//void __attribute__((noreturn)) BusFault_Handler(void)
//{
//#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
//   __builtin_software_breakpoint();
//#endif
//   while (true)
//   {
//   }
//}

void __attribute__((noreturn)) UsageFault_Handler(void) {
    __asm volatile (
            "   tst    LR, #4           \n"
            "   ite    EQ               \n"
            "   mrseq  R0, MSP          \n"
            "   mrsne  R0, PSP          \n"
            "   b      UsageFaultHandler  \n"
            );
    while (1);
}

//void __attribute__((noreturn)) UsageFault_Handler(void) {
//#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
//    __builtin_software_breakpoint();
//#endif
//    while (true) {
//    }
//}


#define SYSHND_CTRL  (*(volatile unsigned int*)  (0xE000ED24u))  // System Handler Control and State Register
#define NVIC_MFSR    (*(volatile unsigned char*) (0xE000ED28u))  // Memory Management Fault Status Register
#define NVIC_BFSR    (*(volatile unsigned char*) (0xE000ED29u))  // Bus Fault Status Register
#define NVIC_UFSR    (*(volatile unsigned short*)(0xE000ED2Au))  // Usage Fault Status Register
#define NVIC_HFSR    (*(volatile unsigned int*)  (0xE000ED2Cu))  // Hard Fault Status Register
#define NVIC_DFSR    (*(volatile unsigned int*)  (0xE000ED30u))  // Debug Fault Status Register
#define NVIC_BFAR    (*(volatile unsigned int*)  (0xE000ED38u))  // Bus Fault Manage Address Register
#define NVIC_AFSR    (*(volatile unsigned int*)  (0xE000ED3Cu))  // Auxiliary Fault Status Register

#define REG_R0  0
#define REG_R1  1
#define REG_R2  2
#define REG_R3  3
#define REG_R12 4
#define REG_LR  5
#define REG_PC  6
#define REG_PSR 7

void BusFaultHandler(unsigned int* pStack) {

    char eBuff[200];
    sprintf(eBuff, "\n\rEXCEPTION: Bus Fault %01X\n\r", NVIC_BFSR);
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    sprintf(eBuff, "\n\rR0=%08X R1=%08X R2=%08X R3=%08X\n\r",
            pStack[REG_R0],
            pStack[REG_R1],
            pStack[REG_R2],
            pStack[REG_R3]);
    
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    sprintf(eBuff, "\n\rR12=%08X LR=%08X PC=%08X PSR=%08X SP=%08X\n\r",
            pStack[REG_R12],
            pStack[REG_LR],
            pStack[REG_PC],
            pStack[REG_PSR],
            pStack);

    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    gfx_mono_print_scroll("BusFault:LR,PC,R0-R3");
    gfx_mono_print_scroll("%08X %08X", pStack[REG_LR], pStack[REG_PC]);
    gfx_mono_print_scroll("%08X %08X", pStack[REG_R0], pStack[REG_R1]);
    gfx_mono_print_scroll("%08X %08X", pStack[REG_R2], pStack[REG_R3]);

    // NVIC_SystemReset();
    
    while (true) {
    }

}

void HardFaultHandler(unsigned int* pStack) {

    char eBuff[200];
    sprintf(eBuff, "\n\rEXCEPTION: Hard Fault %01x\n\r", NVIC_HFSR);
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    sprintf(eBuff, "\n\rR0=%08X R1=%08X R2=%08X R3=%08X\n\r",
            pStack[REG_R0],
            pStack[REG_R1],
            pStack[REG_R2],
            pStack[REG_R3]);

    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    sprintf(eBuff, "\n\rR12=%08X LR=%08X PC=%08X PSR=%08X SP=%08X\n\r",
            pStack[REG_R12],
            pStack[REG_LR],
            pStack[REG_PC],
            pStack[REG_PSR],
            pStack);

    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    gfx_mono_print_scroll("HrdFault:LR,PC,R0-R3");
    gfx_mono_print_scroll("%08X %08X", pStack[REG_LR], pStack[REG_PC]);
    gfx_mono_print_scroll("%08X %08X", pStack[REG_R0], pStack[REG_R1]);
    gfx_mono_print_scroll("%08X %08X", pStack[REG_R2], pStack[REG_R3]);

    // NVIC_SystemReset();();    
    
    while (true) {
    }

}

void UsageFaultHandler(unsigned int* pStack) {

    char eBuff[200];
    sprintf(eBuff, "\n\rEXCEPTION: Usage Fault %01X\n\r", NVIC_UFSR);
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    sprintf(eBuff, "\n\rR0=%08X R1=%08X R2=%08X R3=%08X\n\r",
            pStack[REG_R0],
            pStack[REG_R1],
            pStack[REG_R2],
            pStack[REG_R3]);
    
    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    sprintf(eBuff, "\n\rR12=%08X LR=%08X PC=%08X PSR=%08X SP=%08X\n\r",
            pStack[REG_R12],
            pStack[REG_LR],
            pStack[REG_PC],
            pStack[REG_PSR],
            pStack);

    my_SERCOM1_USART_Write(eBuff, sizeof (eBuff));

    gfx_mono_print_scroll("UsgFault:LR,PC,R0-R3");
    gfx_mono_print_scroll("%08X %08X", pStack[REG_LR], pStack[REG_PC]);
    gfx_mono_print_scroll("%08X %08X", pStack[REG_R0], pStack[REG_R1]);
    gfx_mono_print_scroll("%08X %08X", pStack[REG_R2], pStack[REG_R3]);

    // NVIC_SystemReset();();
    
    while (true) {
    }

}
/*******************************************************************************
 End of File
 */
