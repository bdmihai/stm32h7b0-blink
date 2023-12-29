/*_____________________________________________________________________________
 │                                                                            |
 │ COPYRIGHT (C) 2023 Mihai Baneu                                             |
 │                                                                            |
 | Permission is hereby  granted,  free of charge,  to any person obtaining a |
 | copy of this software and associated documentation files (the "Software"), |
 | to deal in the Software without restriction,  including without limitation |
 | the rights to  use, copy, modify, merge, publish, distribute,  sublicense, |
 | and/or sell copies  of  the Software, and to permit  persons to  whom  the |
 | Software is furnished to do so, subject to the following conditions:       |
 |                                                                            |
 | The above  copyright notice  and this permission notice  shall be included |
 | in all copies or substantial portions of the Software.                     |
 |                                                                            |
 | THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS |
 | OR   IMPLIED,   INCLUDING   BUT   NOT   LIMITED   TO   THE  WARRANTIES  OF |
 | MERCHANTABILITY,  FITNESS FOR  A  PARTICULAR  PURPOSE AND NONINFRINGEMENT. |
 | IN NO  EVENT SHALL  THE AUTHORS  OR  COPYRIGHT  HOLDERS  BE LIABLE FOR ANY |
 | CLAIM, DAMAGES OR OTHER LIABILITY,  WHETHER IN AN ACTION OF CONTRACT, TORT |
 | OR OTHERWISE, ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  |
 | THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 |
 |____________________________________________________________________________|
 |                                                                            |
 |  Author: Mihai Baneu                           Last modified: 15.Dec.2023  |
 |                                                                            |
 |___________________________________________________________________________*/

#include <assert.h>

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "gpio.h"
#include "system.h"

void system_init()
{
    SystemInit();

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    assert(HAL_Init() == HAL_OK);

    /*AXI clock gating */
    RCC->CKGAENR = 0xFFFFFFFF;

    /** Supply configuration update enable
     */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 112;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    assert(HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK);


    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                                |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    assert(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) == HAL_OK);

    /* ITM trace configuration */
    //SET_BIT(DBGMCU->CR, DBGMCU_CR_TRACE_IOEN);
    TPI->SPPR = 0x00000002;              /* "Selected PIN Protocol Register": Select which protocol to use for trace output (2: SWO NRZ, 1: SWO Manchester encoding) */
    TPI->ACPR = 100 - 1;                 /* Divisor for Trace Clock is Prescaler + 1: 100 times slower than cpu freq */
    TPI->FFCR = 0x00000100;              /* Formatter and Flush Control Register */
    ITM->LAR  = 0xC5ACCE55;              /* ITM Lock Access Register, C5ACCE55 enables more write access to Control Register 0xE00 :: 0xFFC */
    ITM->TCR  = ITM_TCR_TraceBusID_Msk | 
                ITM_TCR_SWOENA_Msk     | 
                ITM_TCR_SYNCENA_Msk    | 
                ITM_TCR_ITMENA_Msk;      /* ITM Trace Control Register */
    ITM->TPR  = ITM_TPR_PRIVMASK_Msk;    /* ITM Trace Privilege Register */
    ITM->TER  = 0x01;                    /* ITM Trace Enable Register. Enabled tracing on stimulus ports. One bit per stimulus port. */
    DWT->CTRL = 0x400003FE;              /* DWT_CTRL */

    /* set-up the vector table in ram */
    extern char __isr_vector_start;
    SCB->VTOR = (uintptr_t) &__isr_vector_start;
}

/**
 * delay in us (blockig)
 */
extern "C" void delay_us(const uint32_t us)
{
    //SET_BIT(TIM10->EGR, TIM_EGR_UG);
    //do {
    //} while (TIM10->CNT < us );
}

extern "C" unsigned int system_cpu_f()
{
    return SystemCoreClock;
}
