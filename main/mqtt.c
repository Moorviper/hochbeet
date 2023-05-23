/* MQTT (over TCP) Example

This example code is in the Public Domain (or CC0 licensed, at your option.)

Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.
*/

#include "mqtt.h"
#include "main.h"

static const char *TAG_MQTT = "MQTT : ";

extern struct general_config config;

#define MQTTDEBUG false
char rel[200];

void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG_MQTT, "Last error %s: 0x%x", message, error_code);
  }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                        int32_t event_id, void *event_data) {
  ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d",
           base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  config.client = event->client;
  int msg_id = 0;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    // char buffer[50];
    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
    }

    msg_id = esp_mqtt_client_subscribe(config.client, "beet1/sensor4", 0);
    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d sensor4",
               msg_id);
    }

    msg_id = esp_mqtt_client_subscribe(config.client, "beet1/sensor5", 1);
    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d sensor5",
               msg_id);
    }

    msg_id = esp_mqtt_client_subscribe(config.client, "beet1/sensor6", 1);
    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d sensor6",
               msg_id);
    }

    msg_id = esp_mqtt_client_subscribe(config.client, "beet1/standalone", 1);
    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d standalone",
               msg_id);
    }

    msg_id = esp_mqtt_client_subscribe(config.client, "beet1/relais", 1);
    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d relais", msg_id);
    }

    msg_id = esp_mqtt_client_subscribe(config.client, "beet1/batterie", 1);
    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d Batterie",
               msg_id);
    }
    break;

  case MQTT_EVENT_DISCONNECTED:

    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
    }
    break;

  case MQTT_EVENT_SUBSCRIBED:

    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    }
    // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);

    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);
    }
    break;
  case MQTT_EVENT_UNSUBSCRIBED:

    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    }
    break;
  case MQTT_EVENT_PUBLISHED:

    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    }
    break;
  case MQTT_EVENT_DATA:
    printf("something was sent \n");

    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
    }
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);

    char topicTest[60];
    char standData[60];
    snprintf(topicTest, event->topic_len + 1, "%s\r\n", event->topic);
    snprintf(standData, event->data_len + 1, "%s\r\n", event->data);

    if (strcmp(topicTest, "beet1/standalone") == 0) {
      printf("%s   %s\n", topicTest, standData);
      if (strcmp(standData, "100") == 0) {
        config.standalone = 100;
      } else {
        config.standalone = atoi(standData);
      }
    }
    break;
  case MQTT_EVENT_ERROR:

    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
    }
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      log_error_if_nonzero("reported from esp-tls",
                           event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack",
                           event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno",
                           event->error_handle->esp_transport_sock_errno);

      if (MQTTDEBUG == true) {
        ESP_LOGI(TAG_MQTT, "Last errno string (%s)",
                 strerror(event->error_handle->esp_transport_sock_errno));
      }
    }
    break;
  default:

    if (MQTTDEBUG == true) {
      ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
    }
    break;
  }
}

void mqtt_app_start(void) {
  ESP_LOGI(TAG_MQTT, "[MQTT] Startup..");
  esp_mqtt_client_config_t mqtt_cfg = {
      // .broker.address.uri = rel,
      // .broker.address.uri = CONFIG_BROKER_URL,
      // .broker.address.uri = "mqtt://user:pass@192.168.86.21:1883",
      .broker.address.uri = "mqtt://user:pass@192.168.178.153:1883",
  };
  printf("mqtt\n");
  // esp_mqtt_client_handle_t
  config.client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this
   * example mqtt_event_handler */
  esp_mqtt_client_register_event(config.client, ESP_EVENT_ANY_ID,
                                 mqtt_event_handler, NULL);
  esp_mqtt_client_start(config.client);
}

void runmqtt(void) {
  // if (config.wifiConnected == true) {
  ESP_LOGI(TAG_MQTT, "[MQTT] Setup..");
  // setBroker();

  // ESP_LOGI(TAG_MQTT, "[APP] Free memory: %d bytes",
  // esp_get_free_heap_size()); ESP_LOGI(TAG_MQTT, "[APP] IDF version: %s",
  // esp_get_idf_version());
  //
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
  esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("outbox", ESP_LOG_VERBOSE);
  //
  // ESP_ERROR_CHECK(nvs_flash_init());
  // ESP_ERROR_CHECK(esp_netif_init());
  // ESP_ERROR_CHECK(esp_event_loop_create_default());

  /* This helper function configures Wi-Fi or Ethernet, as selected in
   * menuconfig. Read "Establishing Wi-Fi or Ethernet Connection" section in
   * examples/protocols/README.md for more information about this function.
   */
  // ESP_ERROR_CHECK(example_connect());

  mqtt_app_start();
  // } else {
  //   ESP_LOGI(TAG_MQTT, "[MQTT] ====== Wifi not connected !!! / not starting
  //   !!!");
  // }
}
