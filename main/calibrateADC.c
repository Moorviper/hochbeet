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

int samples = 10;
float rawArray[4096 * 2]; // double size to prevent task errors

float calcVolt(int val) {
  float response = 0;
  response = 3.3 * val / 255;
  return response;
}

float calcLin(int val) {
  float zahl = (float)val;
  float response = 0;
  response = zahl * (4096.0 / 255.0);
  return response;
}

void printRecords() {
  printf("\n \n Ausgabe Daten: \n\n");
  for (int i = 0; i <= 4096; i++) {
    if (i % 16 == 0) {
      printf("<--| \n %6d :%4d: %7.2f ", i, i / 16,
             rawArray[i]); // zeilenumbruch vor der Ausgabe

      // if (i % 1024 == 0) {
      //   vTaskDelay(delay100ms); // delay wegen Task bei langer Ausgabe
      // }
      // printf("%5.2f \n", rawArray[i] );
    } else {
      printf("%7  .2f, ", rawArray[i]);
    }
  }
}

void getSamples() {
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  float val = adc1_get_raw(ADC1_CHANNEL_0);
  dac_output_enable(DAC_CHANNEL_1);
  vTaskDelay(initwait);
  // printf("DAC\tADC\tlinear\n");
  printf("config done ...\n  Starte ADC Kalibrierung ... \n");
  vTaskDelay(initwait);

  // initMiddle();

  for (int samples = 0; samples <= 15; samples++) {
    for (int i = 0; i <= 256; i++) {
      dac_output_voltage(DAC_CHANNEL_1, i);
      vTaskDelay(delay15ms);
      val = adc1_get_raw(ADC1_CHANNEL_0);
      rawArray[i * 16 + samples] = val;
      // vTaskDelay(delay15ms);
    }
    // vTaskDelay(delay15ms);
    printf("    Durchgang Nummer %2d/15 beended !!!\n", samples + 1);
  }
}

void sumUp() {
  printf("\n\n\n");
  // for (int samples = 0; samples <= 15; samples++) {   // Spalten
  for (int i = 0; i <= 256; i++) { // zeilen
    // printf("ZEILE  %d\n", i);          //
    for (int v = 1; v <= 15; v++) { // Spalten
      // printf("SPALTE %d\n", v);
      rawArray[i * 16] = rawArray[i * 16] + rawArray[i * 16 + v];
      rawArray[i * 16 + v] = 0;
      // printf("%f - %f   \n", i*16, i*16+v);
      // printf("WERT : %8.2f \n", rawArray[i*16+v]);
    }
    rawArray[i * 16] = rawArray[i * 16] / 16;
    // rawArray[i+samples] = 666;
  }
  printf("Ausgabe für LaTex \n");
  for (int i = 0; i <= 255; i++) { // zeilen
    // printf("ZEILE  %d\n", i);          //
    float step = (rawArray[i * 16 + 16] - rawArray[i * 16]) / 16;
    // printf("- %3d ---------      STEP  %f\n", i, step); // normale Ausgabe
    printf("%3d %f\n", i, step); // normale Ausgabe
    //      for (int v = 1; v <= 15; v++) {    // Spalten
    // printf("SPALTE %d\n", v);

    // rawArray[i*16] = rawArray[i*16] + rawArray[i*16+v];
    // rawArray[i*16+v] = 0;
    // printf("%f - %f   \n", i*16, i*16+v);
    // printf("WERT : %8.2f \n", rawArray[i*16+v]);
    //      }
    // rawArray[i*16] = rawArray[i*16]/16;
  }
}
// }

void evenOut() {
  printf(
      "\n\n\n\n\n --------------- EVEN OUT _____________----------\n\n\n\n\n");
  float step = 0.0;
  for (int i = 0; i <= 256; i++) {
    step = (rawArray[i * 16 + 16] - rawArray[i * 16]) / 16;
    printf("%10.4f step \n", step);
    if (rawArray[i * 16] == 4095.00) {
      // printf("evenOut \n");
      for (int collum = 1; collum <= 15; collum++) {
        rawArray[i * 16 + collum] = 4095.00;
      }
    } else {
      for (int collum = 1; collum <= 15; collum++) {
        rawArray[i * 16 + collum] = rawArray[i * 16] + (collum * step);
      }
    }
  }
}

void calibrateRUN() {
  // printf("running ADC calibration \n" );
  // xTaskCreate(&adc_task,"adc_task",4096,NULL,1,NULL);
  // xTaskCreate(&dac_task,"dac_task",4096,NULL,1,NULL);

  getSamples();
  // printf("sd sd \n");
  // printf("middle array funtion start \n");
  // middleArray();
  // printf("DAC,DAC\n");
  // printf("done ADC calibration \n" );

  vTaskDelay(delay100ms);
  // printRecords();
  vTaskDelay(delay100ms);
  sumUp();
  vTaskDelay(delay100ms);
  printf("\n\n\n Printing results \n "
         "-------------------------------------------------------\n");
  vTaskDelay(delay100ms);
  // printRecords();
  vTaskDelay(delay100ms);
  evenOut();
  vTaskDelay(delay100ms);
  printRecords();
  vTaskDelay(delay100ms);
  printf(" \n");
}
