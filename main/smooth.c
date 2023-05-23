// #include "smooth.h"
// #include "main.h"
#include <stdio.h> // for test

static const char *TAG_SMOOTH = "smooth : ";

float readings[10][4];
// extern struct general_config config;
extern struct general_config config;

void addReading(int pos, int sensor, float value) {
  readings[pos][sensor - 1] = value;
}

void test(void) {
  printf("Smoothing Values ->    %d\n", config.smoothSensorValue);
}

float getMiddle(int sensor) {
  float sum = 0;
  for (size_t i = 0; i < config.smoothSensorValue; i++) {
    // ESP_LOGW(TAG, "-------------smoothing  ------- Wert: %d ", i);
    sum += readings[i][sensor];
  }
  // if (ver0boseMQ) {
  ESP_LOGW(TAG_SMOOTH, "-------------smoothing  ------- Summe: %f", sum);
  // }
  return (sum / config.smoothSensorValue);
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
