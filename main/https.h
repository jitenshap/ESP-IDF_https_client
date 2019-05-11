#pragma once
/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_system.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
#define GET_TASK_END_BIT BIT(1)

extern EventGroupHandle_t https_task_event_group;
static esp_err_t _http_event_handler(esp_http_client_event_t *evt);

extern void https_with_url();
extern void https_test_task(void *pvParameters);