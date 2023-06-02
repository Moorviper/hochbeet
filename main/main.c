/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "main.h"
#include "mqtt.h"
#include "relais.h"
#include "wifi.h"
// #include "smooth.h"
// #include "sensor.h"
#include "calibrateADC.h"
#include "messureADC.h"
#include "sensorOneshot.h"

// extern wifiData wlan;
extern float rawArray[4096 * 2];
// const TickType_t delay100ms = 100 / portTICK_PERIOD_MS;

configGeneral config = {
    1,   // interval
    100, // standalone init
    10,  // smooth sensor value
    {ADC_CHANNEL_6, ADC_CHANNEL_4, ADC_CHANNEL_5},
    19, // GPIO Relais
    0,  // pump Relais
    // "4",
    10,                // pumpdauert
    "mqttAdr.section", // topic pump
    {0, 0, 0},         // Feuchtigkeit
    0,                 // wifi connected
    0,                 // mqtt handle
    "",                // mqtt adress
    0.0,               // battery
    10,                // Sleep in Minuten
    75,                // Relais schwelle
};
// min read(trocken)   /maxread (nass)
// configSensor sensor = {{4096,4096,4096},{0,0,0}};
//  configSensor sensor = {{3046,3010,3029},{1295,1282,1294}};
configSensor sensor = {{2720, 2930, 3029}, {1407, 1394, 1390}};

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void app_main(void) {
  gpio_set_direction(config.gpioRelais, GPIO_MODE_OUTPUT);

  startWIFI();
  runmqtt();
  // sens();
  one_shot();
  // void run();

  // voltages();

  // vTaskDelay(config.interval * 1000 / portTICK_PERIOD_MS);
}
