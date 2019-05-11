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
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "tcpip_adapter.h"

#include "esp_http_client.h"	//かなり高機能なhttpクライアント

#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "https.h"

/* Root cert for howsmyssl.com, taken from howsmyssl_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/

//テストのための公開キーを設定
extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[]   asm("_binary_howsmyssl_com_root_cert_pem_end");

static const char *TAG = "HTTPS";	//ESP_LOGIとかやったときに表示させるヘッダ

EventGroupHandle_t https_task_event_group;	//SetBitsのためのイベントグループ

/*`	httpクライアントのイベントハンドラ	*/
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) 
	{
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            //if (!esp_http_client_is_chunked_response(evt->client)) 
			//{
                // Write out data
                printf("%.*s", evt->data_len, (char*)evt->data);
            //}

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

/*	https get	*/
void https_with_url()
{
    //httpsクライアントの設定変数
    esp_http_client_config_t config = 
	{
        .url = "https://www.howsmyssl.com",
        .event_handler = _http_event_handler,
        .cert_pem = howsmyssl_com_root_cert_pem_start,
	.port = 443,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);	//httpsクライアントを作成、設定を適用
    esp_err_t err = esp_http_client_perform(client);	//httpsクライアント実行

    if (err == ESP_OK) 
    {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    }
    else 
    {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);	//クライアントのバッファを開放
}


void https_test_task(void *pvParameters)
{
    https_with_url();	
    ESP_LOGI(TAG, "Finish http example");
    xEventGroupSetBits(https_task_event_group, GET_TASK_END_BIT);	//終了フラグを立てる
    vTaskDelete(NULL);	//このタスクを終了
}
