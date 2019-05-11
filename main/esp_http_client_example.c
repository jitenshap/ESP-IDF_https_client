/* ESP HTTP Client Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include "wl.h"
#include "https.h"

#define MAX_HTTP_RECV_BUFFER 512
static const char *TAG = "MAIN";

void app_main()
{
    ESP_LOGI(TAG, "app_main() start");
    esp_err_t ret = nvs_flash_init(); //wifi接続設定とか保存する領域初期化
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
      ESP_ERROR_CHECK(nvs_flash_erase()); //初回などはフォーマットが必要
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); //esp_err_tがESP_OKじゃなければエラー表示するマクロ　　
    tcpip_adapter_init(); //TCP接続レイヤの初期化
    ESP_ERROR_CHECK(esp_event_loop_create_default()); //システムイベントハンドラ取得のためのイベントループを回す

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(sta_init());  //STA接続
    https_task_event_group = xEventGroupCreate(); //HTTPSタスクのためのイベントグループ作成
    xTaskCreate(&https_test_task, "https_test_task", 8192, NULL, 5, NULL);  //HTTPSタスク発行
    xEventGroupWaitBits(https_task_event_group, GET_TASK_END_BIT, true, true, portMAX_DELAY); //HTTPSタスクでフラグが立つまで待機
    ESP_ERROR_CHECK(sta_deinit());  //STA切断
}