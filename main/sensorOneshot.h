/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "relais.h"
#include "sensor.h"
// #include "smooth.h"

extern struct general_config config;
// extern struct sensor_config sensorConfig;
extern struct sensor_config sensor;
// struct sensor_data dataSensor;
// printf("----->>>>     %d\n", config.gpioRelais);

// static const char *TAG_SENSOR = "Sensor :  ";
// static const char *TAG_MQTT = "MQTT : ";

/*---------------------------------------------------------------
        ADC General Macros
---------------------------------------------------------------*/
// ADC1 Channels

#define EXAMPLE_ADC1_CHAN4 ADC_CHANNEL_4
#define EXAMPLE_ADC1_CHAN5 ADC_CHANNEL_5
#define EXAMPLE_ADC1_CHAN6 ADC_CHANNEL_6
#define EXAMPLE_ADC1_CHAN3 ADC_CHANNEL_3
#define EXAMPLE_ADC1_CHAN1 ADC_CHANNEL_1

#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_11

// long map(long x, long in_min, long in_max, long out_min, long out_max);
void one_shot(void);

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten,
                          adc_cali_handle_t *out_handle);
void adc_calibration_deinit(adc_cali_handle_t handle);
