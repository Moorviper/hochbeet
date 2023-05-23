/*
 *
 *
 *   ----------------------  sensors  ------------------------------
 *
 *
 */
#ifndef SENSOR_H
#define SENSOR_H

// #include "config.h"
#include "main.h"

#include "esp_adc/adc_continuous.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>

#define EXAMPLE_READ_LEN 256
#define GET_UNIT(x) ((x >> 3) & 0x1)

// #if CONFIG_IDF_TARGET_ESP32
#define ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_1 // ESP32 only supports ADC1 DMA
                                             // mode
#define ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1 // 12 Bit
// #elif CONFIG_IDF_TARGET_ESP32S2
// #define ADC_CONV_MODE       ADC_CONV_BOTH_UNIT
// #define ADC_OUTPUT_TYPE     ADC_DIGI_OUTPUT_FORMAT_TYPE2
// #elif CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32H2 ||
// CONFIG_IDF_TARGET_ESP32C2 #define ADC_CONV_MODE       ADC_CONV_ALTER_UNIT
// //ESP32C3 only supports alter mode #define ADC_OUTPUT_TYPE
// ADC_DIGI_OUTPUT_FORMAT_TYPE2 #elif CONFIG_IDF_TARGET_ESP32S3 #define
// ADC_CONV_MODE       ADC_CONV_BOTH_UNIT #define ADC_OUTPUT_TYPE
// ADC_DIGI_OUTPUT_FORMAT_TYPE2 #endif

// #if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S3 ||
// CONFIG_IDF_TARGET_ESP32H2 || CONFIG_IDF_TARGET_ESP32C2 static adc_channel_t
// channel[3] = {ADC_CHANNEL_2, ADC_CHANNEL_3, (ADC_CHANNEL_0 | 1 << 3)}; #endif
// #if CONFIG_IDF_TARGET_ESP32S2
// static adc_channel_t channel[3] = {ADC_CHANNEL_2, ADC_CHANNEL_3,
// (ADC_CHANNEL_0 | 1 << 3)}; #endif #if CONFIG_IDF_TARGET_ESP32 static
// adc_channel_t channel[1] = {ADC_CHANNEL_7}; #endif

// bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const
// adc_continuous_evt_data_t *edata, void *user_data);
void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num,
                         adc_continuous_handle_t *out_handle);
bool check_valid_data(const adc_digi_output_data_t *data);
void sens(void);

#endif
