#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "tcpip_adapter.h"

#include "esp_http_client.h"

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
#include "wl.h"

static ip4_addr_t s_ip_addr;
static const char* s_connection_name;
static const char *TAG = "WL";

EventGroupHandle_t s_connect_event_group;

/*	IPアドレスを取得したときのハンドラ	*/
static void on_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr));
    xEventGroupSetBits(s_connect_event_group, GOT_IPV4_BIT);	//接続完了ビットを立てる
}

/*	Wi-Fiが切断されたときのハンドラ	*/
static void on_wifi_disconnect(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    xEventGroupClearBits(s_connect_event_group, GOT_IPV4_BIT);	//接続完了ビットを下げる
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

/*	Wi-Fi接続		*/
esp_err_t sta_init()
{
    if (s_connect_event_group != NULL) 
    {
        return ESP_ERR_INVALID_STATE;
    }
    s_connect_event_group = xEventGroupCreate();	//WaitBitsとかのためにイベントグループを作成
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();	//とりあえず標準の設定でWi-Fiインターフェース初期化
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /*	Wi-Fi関連のイベントハンドラ作成	*/
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));	//Wi-Fi設定はメモリにのみ保存
    /*	接続先設定	*/
    wifi_config_t wifi_config =
    {
        .sta =
        {
            .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
        },
    };
    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));	//STAモードにする
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));	//接続先設定適用
    ESP_ERROR_CHECK(esp_wifi_start());	//物理層初期化
    ESP_ERROR_CHECK(esp_wifi_connect());	//接続開始
    s_connection_name = CONFIG_EXAMPLE_WIFI_SSID;	//接続名称をSSIDにする

    /*
    	裏で動いているタスクを待ちたい場合xEventGroupWaitBitsする。
    	xEventGroupSetBitsされるまで待つ。
	反対にxEventGroupClearBitsすると解除。
    */
    xEventGroupWaitBits(s_connect_event_group, CONNECTED_BITS, true, true, portMAX_DELAY);	//接続待ち
    ESP_LOGI(TAG, "Connected to %s", s_connection_name);
    ESP_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&s_ip_addr));
    return ESP_OK;
}

/*	切断	*/
esp_err_t sta_deinit()
{
    if (s_connect_event_group == NULL) 
	{
        return ESP_ERR_INVALID_STATE;
    }
    vEventGroupDelete(s_connect_event_group);
    s_connect_event_group = NULL;

    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip));
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    ESP_LOGI(TAG, "Disconnected from %s", s_connection_name);
    s_connection_name = NULL;
    return ESP_OK;
}
