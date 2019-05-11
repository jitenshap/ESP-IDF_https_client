#pragma once

#include "esp_system.h"
#include "freertos/event_groups.h"

#define GOT_IPV4_BIT  BIT(0)
#define CONNECTED_BITS  (GOT_IPV4_BIT)

extern EventGroupHandle_t s_connect_event_group;
extern esp_err_t sta_init();
extern esp_err_t sta_deinit();
