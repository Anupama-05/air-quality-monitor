/*
 * Copyright 2016-2026 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    AirQualityMonitor.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_lpadc.h"
#include "fsl_gpio.h"
#include "fsl_lpadc.h"
#include "oled.h"

/* TODO: insert other include files here. */

/* --- Pins configured via Config Tools (see pin_mux.c YAML header for details) --- */
/* LED_GOOD : GPIO2, pin 0  (board header J1[6],  labeled D2) */
/* LED_BAD  : GPIO2, pin 7  (board header J1[12], labeled D5) */
/* BUZZER   : GPIO0, pin 23 (board header J2[2],  labeled D8) */
#define LED_GOOD_GPIO GPIO2
#define LED_GOOD_PIN  0U
#define LED_BAD_GPIO  GPIO2
#define LED_BAD_PIN   7U
#define BUZZER_GPIO   GPIO0
#define BUZZER_PIN    23U
/* FAN_CTRL : GPIO0, pin 21 (board header J1[10], labeled D4) - drives MOSFET gate */
#define FAN_GPIO      GPIO0
#define FAN_PIN       21U


/* Rough starting threshold in millivolts - we'll tune this once we see real
 * MQ-135 readings in clean air vs. disturbed air. Above this = "bad" air. */
#define AIR_QUALITY_THRESHOLD_MV 1500U

static void InitOutputPins(void)
{
    gpio_pin_config_t outputConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 0U, /* start LOW / off */
    };

    GPIO_PinInit(LED_GOOD_GPIO, LED_GOOD_PIN, &outputConfig);
    GPIO_PinInit(LED_BAD_GPIO, LED_BAD_PIN, &outputConfig);
    GPIO_PinInit(BUZZER_GPIO, BUZZER_PIN, &outputConfig);
    GPIO_PinInit(FAN_GPIO, FAN_PIN, &outputConfig);
}

/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
/* Init FSL debug console. */
BOARD_InitDebugConsole();
#endif/* this also runs BOARD_InitPeripherals(), which sets up ADC1 (LPADC_Init, command config, etc.) */



    InitOutputPins();
    OLED_Init();

    OLED_Clear();
    OLED_SetCursor(64,32);
    OLED_Print("Hello");
    OLED_SetFontSize(2);
    OLED_Update();



    PRINTF("Air Quality Monitor starting...\r\n");

    while (1)
    {
        lpadc_conv_result_t result;

        /* Trigger ADC0 Command 1 */
        LPADC_DoSoftwareTrigger(ADC0_PERIPHERAL, 1U);

        /* Wait for conversion result */
        while (!LPADC_GetConvResult(ADC0_PERIPHERAL, &result, 0U))
        {
        }

        /* LPADC result is left-justified */
        uint32_t rawValue16 = result.convValue;
        uint32_t rawValue = rawValue16 >> 4;

        /* Convert ADC reading to voltage */
        uint32_t voltage_mV = (rawValue * 3300U) / 4095U;

        /* Undo the 10k/10k voltage divider */
        uint32_t sensorVoltage_mV = voltage_mV * 2U;

        PRINTF("Raw: %u | ADC: %u mV | Sensor AO: %u mV\r\n",
               rawValue,
               voltage_mV,
               sensorVoltage_mV);

        /* Roughly 500 ms between readings */
        SDK_DelayAtLeastUs(100000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    }




//    while (1)
//    {
//        lpadc_conv_result_t result;
//
//        /* Fire a software trigger (trigger 0) to kick off the conversion command on ADC1 */
//        LPADC_DoSoftwareTrigger(ADC1_PERIPHERAL, 1U);
//
//        /* Wait until the result is ready */
//        uint32_t spins = 0;
//        while (!LPADC_GetConvResult(ADC1_PERIPHERAL, &result, 0U))
//        {
//            spins++;
//        }
//
//        uint32_t rawValue16 = result.convValue;
//        /* LPADC stores the result left-justified in a 16-bit field even at 12-bit
//         * "Standard resolution" - shift right by 4 to get the true 12-bit value. */
//        uint32_t rawValue = rawValue16 >> 4;
//        uint32_t voltage_mV = (rawValue * 3300U) / 4095U;  /* 12-bit resolution, 3.3V reference, in millivolts */
//
//        PRINTF("MQ-135 raw ADC = %u (raw16=%u), voltage = %u mV, cmdSrc=%u, trigSrc=%u, spins=%u\r\n",rawValue, rawValue16, voltage_mV, result.commandIdSource, result.triggerIdSource, spins);
//
//         if (voltage_mV >= AIR_QUALITY_THRESHOLD_MV)
//        {
//            /* Air quality reading is high -> "bad" */
//            GPIO_PinWrite(LED_GOOD_GPIO, LED_GOOD_PIN, 0U);
//            GPIO_PinWrite(LED_BAD_GPIO, LED_BAD_PIN, 1U);
//            GPIO_PinWrite(BUZZER_GPIO, BUZZER_PIN, 1U);
//            GPIO_PinWrite(FAN_GPIO, FAN_PIN, 1U);
//        }
//        else
//        {
//            /* Air quality reading is low -> "good" */
//            GPIO_PinWrite(LED_GOOD_GPIO, LED_GOOD_PIN, 1U);
//            GPIO_PinWrite(LED_BAD_GPIO, LED_BAD_PIN, 0U);
//            GPIO_PinWrite(BUZZER_GPIO, BUZZER_PIN, 0U);
//            GPIO_PinWrite(FAN_GPIO, FAN_PIN, 0U);
//        }
//
//        /* crude delay between readings - replace with a timer later */
//        for (volatile uint32_t i = 0; i < 3000000; i++) { }


//    }

    return 0 ;
}
