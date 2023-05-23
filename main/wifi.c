/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "wifi.h"
#include "main.h"

// extern struct wlan wlan;

wifiData wlan;
// extern struct Wifi_data wlan;

// #include "config"

/* The examples use WiFi configuration that you can set via project
   configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
// #define wlan->ssid      CONFIG_ESP_WIFI_SSID
// #define wlan->pw      CONFIG_ESP_WIFI_PASSWORD
// #define WIFI_CONNECT_MAX_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#define CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#define WIFI_CONNECT_MAX_RETRY 5

// #if CONFIG_ESP_WIFI_AUTH_OPEN
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
// #elif CONFIG_ESP_WIFI_AUTH_WEP
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
// #elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
// #elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
// #elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
// #elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
// #elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
// #elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
// #endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about
 * two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG_WIFI = "wifi station w:>";

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < WIFI_CONNECT_MAX_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI(TAG_WIFI, "connect to the AP fail");
    wlan.connected = false;
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    wlan.connected = true;
  }
}

void wifi_init_sta(void) {
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

  //
  // void wifi_init_sta(char* wifiname, char* password)
  // ...
  wifi_config_t wifi_config = {};
  // strncpy(wifi_config.sta, wlan.ssid, sizeof(wifi_config.sta));
  strncpy((char *)wifi_config.sta.ssid, wlan.ssid,
          sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, wlan.pw,
          sizeof(wifi_config.sta.password));
  // strncpy(wifi_config.password, wlan.pw, sizeof(wifi_config.password));
  wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
  wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

  ESP_LOGI(TAG_WIFI, "%d SSID digger. %s", &wifi_config.sta.ssid, wlan.ssid);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG_WIFI, "wifi_init_sta finished.");

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or
   * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT). The
   * bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we
   * can test which event actually happened. */
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s", wlan.ssid,
             wlan.pw);
    wlan.connected = true;
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s, password:%s", wlan.ssid,
             wlan.pw);
    wlan.connected = false;
  } else {
    ESP_LOGE(TAG_WIFI, "UNEXPECTED EVENT");
    wlan.connected = false;
  }
}

void setupWifi() {
  if (HSFULDA == false) {
    ESP_LOGE(TAG_WIFI, "Verbinde mit Heimnetz");
    strcpy(wlan.ssid, "SSID");
    strcpy(wlan.pw, "Password");

  } else {
    ESP_LOGE(TAG_WIFI, "Verbinde mit HSFD-IoT");
    strcpy(wlan.ssid, "HSFD-IoT");
    strcpy(wlan.pw, "IoTonly111");
    // wlan.connected = false;
  }
}

void startWIFI(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG_WIFI, "ESP_WIFI_MODE_STA");
  setupWifi();
  ESP_LOGE(TAG_WIFI, "------ %s", wlan.ssid);
  ESP_LOGE(TAG_WIFI, "------- %s", wlan.pw);
  wifi_init_sta();

  ESP_LOGE(TAG_WIFI, "----wifi_init_sta_-- %s", wlan.ssid);
  ESP_LOGE(TAG_WIFI, "----wifi_init_sta--- %s", wlan.pw);
}
