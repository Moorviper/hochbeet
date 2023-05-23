/*
 * creates a LUT for correction of the non linear ADC
 *
 * Uses the VP (gpio 36) pin as an input and DAC_1/ADC1_8 (gpio 25) as an output
 * ADC is 12 Bit DAC is only 8 Bit
 *
 * atten
 * ADC_ATTEN_DB_0    = ~  800 mV
 * ADC_ATTEN_DB_2_5  = ~ 1100 mV
 * ADC_ATTEN_DB_6    = ~ 1350 mV
 * ADC_ATTEN_DB_11   = ~ 2600 mV.
 * 3.3 * val / 255    || 2,6*255/3,3 = 200,9
 *
 *  The ADC can only meassure up to 2.6 Volts so only values up to ~200 (201)
 * are relevant
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
// #include "driver/dac_oneshot.h"
// #include "esp_adc/adc_oneshot.h"
#include "esp_check.h"
#include <driver/adc.h>
#include <driver/dac.h>

#define DAC_PIN ADC_CHANNEL_8      // GPIO25, same as DAC channel 0
#define ADC_PIN ADC_CHANNEL_0      // GPIO36,
#define ADC_WIDTH ADC_WIDTH_BIT_12 // 4095 Values
#define ADC_ATTEN ADC_ATTEN_DB_11  //

// dac_output_enable(DAC_PIN);         //Channel 1 = GPIO25
// dac_output_voltage(DAC_PIN, 200);   // Channel Output = (3.3 * 200 / 255)
// = 2.58V

const TickType_t delay100ms = 100 / portTICK_PERIOD_MS;
const TickType_t delay10ms = 10 / portTICK_PERIOD_MS;
const TickType_t delay15ms = 15 / portTICK_PERIOD_MS;
const TickType_t delay30ms = 30 / portTICK_PERIOD_MS;
const TickType_t delay1ms = 1 / portTICK_PERIOD_MS;
const TickType_t delay2ms = 2 / portTICK_PERIOD_MS;
const TickType_t initwait = 2000 / portTICK_PERIOD_MS;

float calcVolt(int val) {
  float zahl = (float)val;
  float response = 0;
  response = 3.16 * (zahl / 16 / 255);
  return response;
}

float calcVoltFromADC(int val) {
  float zahl = (float)val;
  float response = 0;
  response = 3.16 * (zahl / 4096.00);
  return response;
}

float calcLin(int val) {
  float zahl = (float)val;
  float response = 0;
  response = zahl * (4096.0 / 255.0);
  return response;
}

void voltages() {
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  float val = adc1_get_raw(ADC1_CHANNEL_0);
  dac_output_enable(DAC_CHANNEL_1);
  vTaskDelay(initwait);
  printf("NULLWERT  \n");
  int send = 0;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  val = adc1_get_raw(ADC1_CHANNEL_0);
  // printf("DAC\tADC\tlinear\n");
  printf("config done ...\n  Starte DAC Kalibrierung ... \n");
  printf("\n");
  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (0.094)  \n",
         calcVolt(val));
  vTaskDelay(initwait);

  printf("-----------------------------------------------\n");
  send = 31;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  val = adc1_get_raw(ADC1_CHANNEL_0);

  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (0.46)  \n",
         calcVolt(val));
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  printf("-----------------------------------------------\n");
  send = 63;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  val = adc1_get_raw(ADC1_CHANNEL_0);

  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (0.84)  \n",
         calcVolt(val));
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  //  -------

  printf("-----------------------------------------------\n");

  send = 95;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  val = adc1_get_raw(ADC1_CHANNEL_0);

  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (1.21)  \n",
         calcVolt(val));
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  printf("-----------------------------------------------\n");

  send = 127;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  val = adc1_get_raw(ADC1_CHANNEL_0);

  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (1.60)  \n",
         calcVolt(val));
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  // -------------

  printf("-----------------------------------------------\n");

  send = 159;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  val = adc1_get_raw(ADC1_CHANNEL_0);

  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (2.02)  \n",
         calcVolt(val));
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  printf("-----------------------------------------------\n");

  send = 191;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  val = adc1_get_raw(ADC1_CHANNEL_0);

  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (2.41)  \n",
         calcVolt(val));
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  // ---------------------------------

  printf("-----------------------------------------------\n");

  send = 223;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  val = adc1_get_raw(ADC1_CHANNEL_0);

  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (2.78)  \n",
         calcVolt(val));
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  // 046   084   121   160  202  241   278  316

  printf("-----------------------------------------------\n");

  send = 255;
  dac_output_voltage(DAC_CHANNEL_1, send);
  vTaskDelay(delay15ms);
  val = adc1_get_raw(ADC1_CHANNEL_0);

  printf("Sende %d an DAC - gelesener Wert: %f - Berechneter Volt aus ADC "
         "Wert: %f \n",
         send, val, calcVoltFromADC(val));
  printf("Sollte %2.4f Volt sein. Gemessener Wert mit Multimeter: (3.16)  \n",
         calcVolt(val));
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}
