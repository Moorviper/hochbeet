/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sensorOneshot.h"
#include "driver/temperature_sensor.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "main.h"
#include "relais.h"
// #include "smooth.h"

extern struct general_config config;
extern struct sensor_config sensor;

static const char *TAG_SENSOR_ONE_SHOT = "Sensor :  ";
static const char *TAG_MQTT_ONE_SHOT = "MQTT : ";
// static TaskHandle_t s_task_handle;

static int adc_raw[2][10];
static int voltage[2][10];
// static float evenout[5][10];
bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten,
                          adc_cali_handle_t *out_handle);
void adc_calibration_deinit(adc_cali_handle_t handle);
//
// long map(long x, long in_min, long in_max, long out_min, long out_max) {
//   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
// }

static void schlafen(int i) {
  printf("Erde ist feucht genug. Schlafe für %d \n", i);
  // esp_sleep_enable_timer_wakeup(3600 * 1000000); // Sleep for an hour
  esp_sleep_enable_timer_wakeup(i * 60 * 1000000); // Sleep for an 1minute
  esp_deep_sleep_start();
}

void one_shot(void) {
  ESP_LOGI(TAG_SENSOR_ONE_SHOT, "KANAL 1------------ > %d", ADC_CHANNEL_1);
  ESP_LOGI(TAG_SENSOR_ONE_SHOT, "KANAL 4------------ > %d", ADC_CHANNEL_4);
  ESP_LOGI(TAG_SENSOR_ONE_SHOT, "KANAL 5------------ > %d", ADC_CHANNEL_5);
  ESP_LOGI(TAG_SENSOR_ONE_SHOT, "KANAL 6------------ > %d", ADC_CHANNEL_6);
  ESP_LOGI(TAG_SENSOR_ONE_SHOT, "KANAL 3------------ > %d", ADC_CHANNEL_3);
  ESP_LOGI(TAG_SENSOR_ONE_SHOT, "KANAL ------------ > %d", ADC_CHANNEL_3);

  //-------------ADC1 Init---------------//
  adc_oneshot_unit_handle_t adc1_handle;
  adc_oneshot_unit_init_cfg_t init_config1 = {
      .unit_id = ADC_UNIT_1,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

  //-------------ADC1 Config---------------//
  adc_oneshot_chan_cfg_t configadc1 = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = EXAMPLE_ADC_ATTEN,
  };
  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &configadc1));
  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &configadc1));
  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_5, &configadc1));
  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &configadc1));
  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &configadc1));

  //-------------ADC1 Calibration Init---------------//
  adc_cali_handle_t adc1_cali_handle = NULL;
  bool do_calibration1 =
      adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC_ATTEN, &adc1_cali_handle);

  for (size_t k = 0; k < 5; k++) {
    // while (1) {    // keep for debuging without sleep
    ESP_ERROR_CHECK(
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_1, &adc_raw[0][1]));
    vTaskDelay(pdMS_TO_TICKS(250));
    ESP_ERROR_CHECK(
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &adc_raw[0][3]));
    vTaskDelay(pdMS_TO_TICKS(250));
    ESP_ERROR_CHECK(
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &adc_raw[0][4]));
    vTaskDelay(pdMS_TO_TICKS(250));
    ESP_ERROR_CHECK(
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_5, &adc_raw[0][5]));
    vTaskDelay(pdMS_TO_TICKS(250));
    ESP_ERROR_CHECK(
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &adc_raw[0][6]));
    vTaskDelay(pdMS_TO_TICKS(250));
    ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Raw Data: %d",
             ADC_UNIT_1 + 1, ADC_CHANNEL_1, adc_raw[0][1]);
    ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Raw Data: %d",
             ADC_UNIT_1 + 1, ADC_CHANNEL_3, adc_raw[0][3]);
    ESP_LOGE(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Raw Data: %d",
             ADC_UNIT_1 + 1, ADC_CHANNEL_4, adc_raw[0][4]);
    ESP_LOGE(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Raw Data: %d",
             ADC_UNIT_1 + 1, ADC_CHANNEL_5, adc_raw[0][5]);
    ESP_LOGE(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Raw Data: %d",
             ADC_UNIT_1 + 1, ADC_CHANNEL_6, adc_raw[0][6]);
    if (do_calibration1) {
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][1],
                                              &voltage[0][1]));
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][3],
                                              &voltage[0][3]));

      printf("minread %d\n", sensor.minRead[0]);
      printf("minread %d\n", sensor.maxRead[0]);
      printf("adcraw %D\n", adc_raw[0][4]);
      if (sensor.minRead[0] < adc_raw[0][4]) {
        printf("Zu trocken \n");
        adc_raw[0][4] = sensor.minRead[0];
        printf("neuer adc raw wert %d\n", adc_raw[0][4]);
      }
      if (sensor.maxRead[0] > adc_raw[0][4]) {
        printf("zu nass\n");
        adc_raw[0][4] = sensor.maxRead[0];
        printf("neuer adc raw wert %d\n", adc_raw[0][4]);
      }
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][4],
                                              &voltage[0][4]));

      printf("minread %d\n", sensor.minRead[1]);
      printf("minread %d\n", sensor.maxRead[1]);
      printf("adcraw %D\n", adc_raw[0][5]);
      if (sensor.minRead[1] < adc_raw[0][5]) {
        printf("Zu trocken \n");
        adc_raw[0][5] = sensor.minRead[1];
        printf("neuer adc raw wert %d\n", adc_raw[0][5]);
      }
      if (sensor.maxRead[1] > adc_raw[0][5]) {
        printf("zu nass\n");
        adc_raw[0][5] = sensor.maxRead[1];
        printf("neuer ADC raw Wert %d\n", adc_raw[0][5]);
      }
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][5],
                                              &voltage[0][5]));

      printf("minread %d\n", sensor.minRead[2]);
      printf("minread %d\n", sensor.maxRead[2]);
      printf("ADC RAW %D\n", adc_raw[0][6]);
      if (sensor.minRead[2] < adc_raw[0][6]) {
        printf("Zu trocken \n");
        adc_raw[0][6] = sensor.minRead[2];
        printf("neuer ADC RAW Wert %d\n", adc_raw[0][6]);
      }
      if (sensor.maxRead[2] > adc_raw[0][6]) {
        printf("zu nass\n");
        adc_raw[0][6] = sensor.maxRead[2];
        printf("neuer ADC RAW Wert %d\n", adc_raw[0][6]);
      }
      ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][6],
                                              &voltage[0][6]));
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Cali Voltage: %d mV",
               ADC_UNIT_1 + 1, ADC_CHANNEL_1, voltage[0][1]);
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Cali Voltage: %d mV",
               ADC_UNIT_1 + 1, ADC_CHANNEL_3, voltage[0][3]);
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Cali Voltage: %d mV",
               ADC_UNIT_1 + 1, ADC_CHANNEL_4, voltage[0][4]);
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Cali Voltage: %d mV",
               ADC_UNIT_1 + 1, ADC_CHANNEL_5, voltage[0][5]);
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Cali Voltage: %d mV",
               ADC_UNIT_1 + 1, ADC_CHANNEL_6, voltage[0][6]);
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "4 %lu %%",
               map(adc_raw[0][4], sensor.minRead[0], sensor.maxRead[0], 0,
                   100)); // min max
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "5 %lu %%",
               map(adc_raw[0][5], sensor.minRead[1], sensor.maxRead[1], 0,
                   100)); // min max
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "6 %lu %%",
               map(adc_raw[0][6], sensor.minRead[2], sensor.maxRead[2], 0,
                   100)); // min max
      // float bat = (((((float)voltage[0][3])/1000))*6.5488565489); 12,5
      float bat = (((((float)voltage[0][3]) / 1000)) * 6.6536382537);
      ESP_LOGI(TAG_SENSOR_ONE_SHOT, "Batterie Cali Voltage: %f mV", bat);
      // 27,6*
      printf("4 :   %lu %%\n",
             map(adc_raw[0][4], sensor.minRead[0], sensor.maxRead[0], 0, 100));
      printf("5 :   %lu %%\n",
             map(adc_raw[0][5], sensor.minRead[1], sensor.maxRead[1], 0, 100));
      printf("6 :   %lu %%\n",
             map(adc_raw[0][6], sensor.minRead[2], sensor.maxRead[2], 0, 100));
      config.humidity[0] =
          map(adc_raw[0][4], sensor.minRead[0], sensor.maxRead[0], 0, 100);
      config.humidity[1] =
          map(adc_raw[0][5], sensor.minRead[1], sensor.maxRead[1], 0, 100);
      config.humidity[2] =
          map(adc_raw[0][6], sensor.minRead[2], sensor.maxRead[2], 0, 100);
      config.battery = bat;

      int msg_id;
      char sensor4[50];
      snprintf(sensor4, 6, "%f", config.humidity[0]);
      msg_id = esp_mqtt_client_publish(config.client, "beet1/sensor4", sensor4,
                                       0, 1, 0);
      ESP_LOGI(TAG_MQTT_ONE_SHOT, "sent publish successful, msg_id=%d", msg_id);

      char sensor6[50];
      snprintf(sensor6, 6, "%f", config.humidity[1]);
      msg_id = esp_mqtt_client_publish(config.client, "beet1/sensor6", sensor6,
                                       0, 1, 0);
      ESP_LOGI(TAG_MQTT_ONE_SHOT, "sent publish successful, msg_id=%d", msg_id);

      char sensor5[50];
      snprintf(sensor5, 6, "%f", config.humidity[2]);
      msg_id = esp_mqtt_client_publish(config.client, "beet1/sensor5", sensor5,
                                       0, 1, 0);
      ESP_LOGI(TAG_MQTT_ONE_SHOT, "sent publish successful, msg_id=%d", msg_id);

      char stand[50];
      sprintf(stand, "%d", config.standalone);
      printf("========-------->>       STAND  %d\n", config.standalone);
      msg_id = esp_mqtt_client_publish(config.client, "beet1/standalone", stand,
                                       0, 1, 0);
      ESP_LOGI(TAG_MQTT_ONE_SHOT, "sent publish successful, msg_id=%d", msg_id);

      char batt[50];
      sprintf(batt, "%.2f", config.battery);
      printf("========-------->>       Battery  %.2f\n", config.battery);
      msg_id = esp_mqtt_client_publish(config.client, "beet1/batterie", batt, 0,
                                       1, 0);
      ESP_LOGI(TAG_MQTT_ONE_SHOT, "sent publish successful, msg_id=%d", msg_id);

      if (config.standalone < 100) {
        if (config.humidity[1] <= config.standalone &&
            config.humidity[2] <= config.standalone) {
          printf("-------- NODE RED\n");
          run();
          vTaskDelay(pdMS_TO_TICKS(5000));
        }
      } else {
        if (config.humidity[1] <= 40 && config.humidity[2] <= 40) {
          printf("-------- STAND ALONE MODE\n");
          run();
          vTaskDelay(pdMS_TO_TICKS(5000));
        }
      }
    }

    // vTaskDelay(pdMS_TO_TICKS(1000));
    //
    // ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_3,
    // &adc_raw[0][3])); ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d Channel[%d] Raw
    // Data: %d", ADC_UNIT_1 + 1, ADC_CHANNEL_3, adc_raw[0][3]); if
    // (do_calibration1) {
    //     ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle,
    //     adc_raw[0][3], &voltage[0][1])); ESP_LOGI(TAG_SENSOR_ONE_SHOT, "ADC%d
    //     Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC_CHANNEL_3,
    //     voltage[0][3]); ESP_LOGI(TAG_SENSOR_ONE_SHOT, " %lu %%",
    //     map(voltage[0][3], 4095, 0, 0, 100)); // min max
    // }
    vTaskDelay(pdMS_TO_TICKS(5000));
    printf("--------------------------------------\n");
    printf("\n");
  }
  schlafen(config.ruhe);

  // Tear Down
  ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
  if (do_calibration1) {
    adc_calibration_deinit(adc1_cali_handle);
  }
}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten,
                          adc_cali_handle_t *out_handle) {
  adc_cali_handle_t handle = NULL;
  esp_err_t ret = ESP_FAIL;
  bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  if (!calibrated) {
    ESP_LOGI(TAG_SENSOR_ONE_SHOT, "calibration scheme version is %s",
             "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
    if (ret == ESP_OK) {
      calibrated = true;
    }
  }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  if (!calibrated) {
    ESP_LOGI(TAG_SENSOR_ONE_SHOT, "calibration scheme version is %s",
             "Line Fitting");
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
    if (ret == ESP_OK) {
      calibrated = true;
    }
  }
#endif

  *out_handle = handle;
  if (ret == ESP_OK) {
    ESP_LOGI(TAG_SENSOR_ONE_SHOT, "Calibration Success");
  } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
    ESP_LOGW(TAG_SENSOR_ONE_SHOT, "eFuse not burnt, skip software calibration");
  } else {
    ESP_LOGE(TAG_SENSOR_ONE_SHOT, "Invalid arg or no memory");
  }

  return calibrated;
}

void adc_calibration_deinit(adc_cali_handle_t handle) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  ESP_LOGI(TAG_SENSOR_ONE_SHOT, "deregister %s calibration scheme",
           "Curve Fitting");
  ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  ESP_LOGI(TAG_SENSOR_ONE_SHOT, "deregister %s calibration scheme",
           "Line Fitting");
  ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}
