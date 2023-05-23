#include "driver/gpio.h"
#include "main.h"
#include "sensor.h"
extern struct general_config config;
extern struct sensor_config sensor;

int relais_msg_id = 0;
static const char *TAG_RELAIS = "RELAIS : ";

void standaloneMode() {
  if (config.humidity[1] <= config.pumphumidity &&
      config.humidity[2] <= config.pumphumidity) {
    /* pumpe an  wenn Sensor 5+6 unter 40% sind */
    printf("Pumpe an\n");
    gpio_set_level(GPIO_RELAIS, 1);

    char stand[50];
    snprintf(stand, 10, "%d", config.standalone);
    relais_msg_id = esp_mqtt_client_publish(config.client, "beet1/standalone",
                                            stand, 0, 1, 0);
    ESP_LOGI(TAG_RELAIS, "sent publish successful, msg_id=%d", relais_msg_id);

    config.pumpRelais = 100;

    relais_msg_id =
        esp_mqtt_client_publish(config.client, "beet1/relais", "100", 0, 1, 0);
    ESP_LOGI(TAG_RELAIS, "sent publish successful, msg_id=%d", relais_msg_id);

    vTaskDelay(config.relaisLaenge * 1000 /
               portTICK_PERIOD_MS); // pumpe für x sekunden
    gpio_set_level(GPIO_RELAIS, 0);

    relais_msg_id =
        esp_mqtt_client_publish(config.client, "beet1/relais", "0", 0, 1, 0);
    ESP_LOGI(TAG_RELAIS, "sent publish successful, msg_id=%d", relais_msg_id);

  } else {
    /* prumpe aus */
    printf("pumpe aus\n");
    config.pumpRelais = 0;
    gpio_set_level(GPIO_RELAIS, 0);
    // char pumpoff[50];
    // snprintf(pumpoff, 10, "%f", config.pumpRelais);
    relais_msg_id =
        esp_mqtt_client_publish(config.client, "beet1/relais", "0", 0, 1, 0);
    ESP_LOGI(TAG_RELAIS, "sent publish successful, msg_id=%d", relais_msg_id);
    // gpio_set_level(GPIO_RELAISA, 0);
  }
}

void nodeRED() {
  if (config.humidity[1] <= config.standalone &&
      config.humidity[2] <= config.standalone) {
    /* prumpe an */
    printf("Pumpe an\n");
    gpio_set_level(GPIO_RELAIS, 1);

    char stand[50];
    snprintf(stand, 10, "%d", config.standalone);
    relais_msg_id = esp_mqtt_client_publish(config.client, "beet1/standalone",
                                            stand, 0, 1, 0);
    ESP_LOGI(TAG_RELAIS, "sent publish successful, msg_id=%d", relais_msg_id);

    config.pumpRelais = 100;

    relais_msg_id =
        esp_mqtt_client_publish(config.client, "beet1/relais", "100", 0, 1, 0);
    ESP_LOGI(TAG_RELAIS, "sent publish successful, msg_id=%d", relais_msg_id);

    vTaskDelay(config.relaisLaenge * 1000 /
               portTICK_PERIOD_MS); // pumpe für x sekunden
    gpio_set_level(GPIO_RELAIS, 0);

    relais_msg_id =
        esp_mqtt_client_publish(config.client, "beet1/relais", "0", 0, 1, 0);
    ESP_LOGI(TAG_RELAIS, "sent publish successful, msg_id=%d", relais_msg_id);

  } else {
    /* prumpe aus */
    printf("pumpe aus\n");
    config.pumpRelais = 0;
    gpio_set_level(GPIO_RELAIS, 0);
    // char pumpoff[50];
    // snprintf(pumpoff, 10, "%f", config.pumpRelais);
    relais_msg_id =
        esp_mqtt_client_publish(config.client, "beet1/relais", "0", 0, 1, 0);
    ESP_LOGI(TAG_RELAIS, "sent publish successful, msg_id=%d", relais_msg_id);
    // gpio_set_level(GPIO_RELAISA, 0);
  }
}

void run() {
  if (config.standalone < 100) {
    nodeRED();
  } else {
    standaloneMode();
  }
}
