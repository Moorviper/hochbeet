#ifndef WIFI_H
#define WIFI_H

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdbool.h>
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"

void wifi_init_sta(void);
void startWIFI(void);

#endif
