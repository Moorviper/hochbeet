#ifndef MAIN_H
#define MAIN_H

#include "driver/gpio.h"
#include "esp_adc/adc_continuous.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "sdkconfig.h"
#include <stdbool.h>
#include <stdio.h>

#define HSFULDA true
// #define HSFULDA false
#define GPIO_RELAIS 19

typedef struct general_config {
  int interval;
  int standalone;
  int smoothSensorValue;
  adc_channel_t channel[3];
  int gpioRelais;
  int pumpRelais;
  int relaisLaenge;
  // char beet[3];
  char topicPump[60];
  float humidity[3];
  bool wifiConnected;
  esp_mqtt_client_handle_t client;
  char mqttAdress[200];
  float battery;
  int ruhe;
  int pumphumidity;
} configGeneral;

typedef struct sensor_config {
  const int minRead[3];
  const int maxRead[3];
} configSensor;

struct mqtt_adress {
  char section[14];
  char topic[9];
  char number[3];
  char device[14];
};

typedef struct Wifi_data {
  char ssid[33];
  char pw[64];
  bool connected;
} wifiData;

long map(long x, long in_min, long in_max, long out_min, long out_max);

#endif
