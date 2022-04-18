/*
scan wifi list
retrieve BSSID / signal level

struct wifi_ap_record_t
uint8_t bssid[6]       // MAC address of AP
int8_t rssi            // signal strength of AP
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#define DEFAULT_SCAN_LIST_SIZE 16

typedef struct ap_info
{
    uint8_t bssid[6];       // MAC address of AP
    int8_t rssi;            // signal strength of AP
}AP_info;


static const char *TAG = "scan";

/* Initialize Wi-Fi as sta and set scan method */
static uint16_t wifi_scan(AP_info *AP_list)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        //ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        //printf("BSSID: ");
        for (int j = 0; j < 6; j++) {
            AP_list[i].bssid[j] = ap_info[i].bssid[j];
            //printf("%02x ", ap_info[i].bssid[j]);
        }
        //printf("\n");
        AP_list[i].rssi = ap_info[i].rssi;
    }
    return ap_count;
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    // scan
    AP_info AP_list[DEFAULT_SCAN_LIST_SIZE];
    memset(AP_list, 0, sizeof(AP_list));
    
    uint16_t AP_count = 0;
    AP_count = wifi_scan(AP_list);
    
    // log info
    for (int i = 0; i < AP_count ; i++) {
        //ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        printf("BSSID: ");
        for (int j = 0; j < 6; j++) {
            printf("%02X", AP_list[i].bssid[j]);
        }
        printf(" RSSI: %d\n", AP_list[i].rssi);
    }

    // convert to string (opt.)
    char AP_list_str[DEFAULT_SCAN_LIST_SIZE][32];
    for (int i = 0; i < AP_count ; i++) {
        for (int j = 0; j < 6; j++) {
            sprintf(&AP_list_str[i][2*j], "%02X",  AP_list[i].bssid[j]);
        }
        sprintf(&AP_list_str[i][12], "%d",  AP_list[i].rssi);
    }
    for (int i = 0; i < AP_count ; i++) {
        ESP_LOGI("str", "%s", AP_list_str[i]);
    }
    
}