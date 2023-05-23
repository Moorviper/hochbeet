/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "sensor.h"
#include "main.h"
#include "relais.h"
// #include "smooth.h"

extern struct general_config config;
// extern struct sensor_config sensorConfig;
extern struct sensor_config sensor;
// struct sensor_data dataSensor;
// printf("----->>>>     %d\n", config.gpioRelais);

int testas[50];

static const char *TAG_SENSOR = "Sensor :  ";
static const char *TAG_MQTT = "MQTT : ";
static TaskHandle_t s_task_handle;

bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle,
                              const adc_continuous_evt_data_t *edata,
                              void *user_data) {
  BaseType_t mustYield = pdFALSE;
  // Notify that ADC continuous driver has done enough number of conversions
  vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

  return (mustYield == pdTRUE);
}

void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num,
                         adc_continuous_handle_t *out_handle) {
  adc_continuous_handle_t handle = NULL;

  adc_continuous_handle_cfg_t adc_config = {
      .max_store_buf_size = 1024,
      .conv_frame_size = EXAMPLE_READ_LEN,
  };
  ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

  adc_continuous_config_t dig_cfg = {
      .sample_freq_hz = 20 * 1000,
      .conv_mode = ADC_CONV_MODE,
      .format = ADC_OUTPUT_TYPE,
  };

  adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
  dig_cfg.pattern_num = channel_num;
  for (int i = 0; i < channel_num; i++) {
    uint8_t unit = GET_UNIT(channel[i]);
    uint8_t ch = channel[i] & 0x7;
    adc_pattern[i].atten = ADC_ATTEN_DB_0;
    // adc_pattern[i].atten = ADC_ATTEN_DB_11;
    adc_pattern[i].channel = ch;
    adc_pattern[i].unit = unit;
    adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

    ESP_LOGI(TAG_SENSOR, "adc_pattern[%d].atten is :%x", i,
             adc_pattern[i].atten);
    ESP_LOGI(TAG_SENSOR, "adc_pattern[%d].channel is :%x", i,
             adc_pattern[i].channel);
    ESP_LOGI(TAG_SENSOR, "adc_pattern[%d].unit is :%x", i, adc_pattern[i].unit);
  }
  dig_cfg.adc_pattern = adc_pattern;
  ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

  *out_handle = handle;
}

void sens(void) {
  int errorvalue = 0;
  esp_err_t ret;
  uint32_t ret_num = 0;
  uint8_t result[EXAMPLE_READ_LEN] = {0};
  memset(result, 0xcc, EXAMPLE_READ_LEN);

  s_task_handle = xTaskGetCurrentTaskHandle();

  adc_continuous_handle_t handle = NULL;
  continuous_adc_init(config.channel,
                      sizeof(config.channel) / sizeof(adc_channel_t), &handle);

  adc_continuous_evt_cbs_t cbs = {
      .on_conv_done = s_conv_done_cb,
  };
  ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
  ESP_ERROR_CHECK(adc_continuous_start(handle));

  while (1) {
    /**
     * This is to show you the way to use the ADC continuous mode driver event
     * callback. This `ulTaskNotifyTake` will block when the data processing in
     * the task is fast. However in this example, the data processing (print) is
     * slow, so you barely block here.
     *
     * Without using this event callback (to notify this task), you can still
     * just call `adc_continuous_read()` here in a loop, with/without a certain
     * block timeout.
     */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    while (1) {
      ret = adc_continuous_read(handle, result, EXAMPLE_READ_LEN, &ret_num, 0);
      if (ret == ESP_OK) {
        ESP_LOGI("TASK", "ret is %x,  ret_num is %" PRIu32, ret, ret_num);

        // printf("---->>>>>>   %d\n", *result);
        adc_digi_output_data_t *p = (void *)&result[0]; // 4
        // adc_digi_output_data_t *q = (void*)&result[1];
        adc_digi_output_data_t *k = (void *)&result[2]; // 6
        // adc_digi_output_data_t *l = (void*)&result[3];
        // adc_digi_output_data_t *w = (void*)&result[4];
        // adc_digi_output_data_t *u = (void*)&result[5];
        adc_digi_output_data_t *v = (void *)&result[6]; // 5

        if (p->type1.channel == 4 && k->type1.channel == 6 &&
            v->type1.channel == 5) {
          // while (1) {
          // ESP_LOGI(TAG_SENSOR, "Unit: %d,_Channel: %d, Value: %x",
          // p->type1.unit+1, p->type1.channel, p->type1.data);
          // float converted = p->type1.data;
          // float converted2 = q->type1.data;
          ESP_LOGI(TAG_SENSOR, "RAW CHAN 1: %d ", p->type1.channel);
          // ESP_LOGI(TAG_SENSOR, "RAW CHAN 2: %d ", q->type1.channel);
          ESP_LOGI(TAG_SENSOR, "RAW CHAN 3: %d ", k->type1.channel);
          // ESP_LOGI(TAG_SENSOR, "RAW CHAN 4: %d ", l->type1.channel);
          // ESP_LOGI(TAG_SENSOR, "RAW CHAN 5: %d ", w->type1.channel);
          // ESP_LOGI(TAG_SENSOR, "RAW CHAN 6: %d ", u->type1.channel);
          ESP_LOGI(TAG_SENSOR, "RAW CHAN 7: %d ", v->type1.channel);
          ESP_LOGI(TAG_SENSOR, "RAW DATA 1: %d %d %d ", p->type1.data,
                   sensor.minRead[0], sensor.maxRead[0]);
          // ESP_LOGI(TAG_SENSOR, "RAW DATA 2: %d %%", q->type1.data);
          ESP_LOGI(TAG_SENSOR, "RAW DATA 3: %d %d %d ", k->type1.data,
                   sensor.minRead[1], sensor.maxRead[1]);
          // ESP_LOGI(TAG_SENSOR, "RAW DATA 4: %d %%", l->type1.data);
          // ESP_LOGI(TAG_SENSOR, "RAW DATA 5: %d %%", w->type1.data);
          // ESP_LOGI(TAG_SENSOR, "RAW DATA 6: %d %%", u->type1.data);
          ESP_LOGI(TAG_SENSOR, "RAW DATA 7: %d %d %d ", v->type1.data,
                   sensor.minRead[2], sensor.maxRead[2]);

          if (p->type1.data > sensor.minRead[0] ||
              k->type1.data > sensor.minRead[1] ||
              v->type1.data > sensor.minRead[2]) {
            errorvalue += 1;
          }
          printf("Errors: %d \n", errorvalue);
          printf("4 :   %lu %%\n", map(p->type1.data, sensor.minRead[0],
                                       sensor.maxRead[0], 0, 100));
          printf("6 :   %lu %%\n", map(k->type1.data, sensor.minRead[1],
                                       sensor.maxRead[1], 0, 100));
          printf("5 :   %lu %%\n", map(v->type1.data, sensor.minRead[2],
                                       sensor.maxRead[2], 0, 100));
          config.humidity[0] =
              map(p->type1.data, sensor.minRead[0], sensor.maxRead[0], 0, 100);
          config.humidity[1] =
              map(k->type1.data, sensor.minRead[1], sensor.maxRead[1], 0, 100);
          config.humidity[2] =
              map(v->type1.data, sensor.minRead[2], sensor.maxRead[2], 0, 100);

          // log raw
          // config.humidity[0] = p->type1.data;
          // config.humidity[1] = k->type1.data;
          // config.humidity[2] = v->type1.data;

          int msg_id;
          char sensor4[50];
          snprintf(sensor4, 6, "%f", config.humidity[0]);
          msg_id = esp_mqtt_client_publish(config.client, "beet1/sensor4",
                                           sensor4, 0, 1, 0);
          ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);

          char sensor6[50];
          snprintf(sensor6, 6, "%f", config.humidity[1]);
          msg_id = esp_mqtt_client_publish(config.client, "beet1/sensor6",
                                           sensor6, 0, 1, 0);
          ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);

          char sensor5[50];
          snprintf(sensor5, 6, "%f", config.humidity[2]);
          msg_id = esp_mqtt_client_publish(config.client, "beet1/sensor5",
                                           sensor5, 0, 1, 0);
          ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);

          char stand[50];
          sprintf(stand, "%d", config.standalone);
          printf("========-------->>       STAND  %d\n", config.standalone);
          msg_id = esp_mqtt_client_publish(config.client, "beet1/standalone",
                                           stand, 0, 1, 0);
          ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);

          run();

          vTaskDelay(config.interval * 5000 / portTICK_PERIOD_MS);
          // vTaskDelay(config.interval * 200 / portTICK_PERIOD_MS);
        } else {
          printf("WRONG READING \n");
        }

        // }

        // sensorConfig.range = sensorConfig.minRead - sensorConfig.maxRead;
        // sensorData.humidity[(int)*mqttAdr.number]= (((sensorConfig.minRead -
        // p->type1.data) / range ) * 100 ); sensorData.humidity= (((minRead -
        // converted2) / range ) * 100 );

        // %lu
        // if (verbose) {
        // ESP_LOGI(TAG_SENSOR, "Feuchtigkeit: %f %% (oben) %f %% (unten)",
        // *sensorData.humidity, *sensorData.humidity);
        // }

        // }
        /**
         * Because printing is slow, so every time you call `ulTaskNotifyTake`,
         * it will immediately return. To avoid a task watchdog timeout, add a
         * delay here. When you replace the way you process the data, usually
         * you don't need this delay (as this task will block for a while).
         */
        vTaskDelay(1);
        // } //else
        if (ret == ESP_ERR_TIMEOUT) {
          // We try to read `EXAMPLE_READ_LEN` until API returns timeout, which
          // means there's no available data
          break;
        }
      }
    }
  }
  ESP_ERROR_CHECK(adc_continuous_stop(handle));
  ESP_ERROR_CHECK(adc_continuous_deinit(handle));
}
