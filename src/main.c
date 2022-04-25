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

#include "response.h"
#include "ringbuffer.h"
//#include "str.h"

#define DEFAULT_SCAN_LIST_SIZE      16

#define DEFAULT_CELL_LIST_SIZE       32
#define DEFAULT_CELL_LIST_INFO_SIZE 32
#define OK                          "OK"
#define ERROR                       "ERROR"


// AT+QOPSCFG="scancontrol",<RAT>
char* scancontrol(int RAT, char *read_ptr, Ringbuffer *buffer) // return new read_ptr 
{
    //const char *TAG = "scancontrol";

    // send ATcommand
    printf("AT+QOPSCFG=\"scancontrol\",%d\n", RAT-2);
    
    // echo
    //ESP_LOGI(TAG, "start");
    read_ptr = ringbuffer_produce(buffer, read_ptr);
    /*
    char byte = ringbuffer_del(buffer);
    ESP_LOGI("y", "start");
    while (byte != 0) {
        printf("%c", byte);
        byte = ringbuffer_del(buffer);
    }
    */
    ringbuffer_consume(buffer, NULL, 0);
    
    // 0x0A
    read_ptr = ringbuffer_produce(buffer, read_ptr);
    ringbuffer_consume(buffer, NULL, 0);
    
    // res
    char res[16] = {0};
    read_ptr = ringbuffer_produce(buffer, read_ptr);
    ringbuffer_consume(buffer, res, 0x0A);
    printf("%s\n", res);
    //ESP_LOGI(TAG, "end");
    if ( strncmp(res, OK, 2) == 0) {     // OK
        return read_ptr;
    }
    else {                        // ERROR
        return NULL;
    }
}

void get_oper(Ringbuffer *buffer, char *oper)
{
    ringbuffer_consume(buffer, NULL, ',');
    ringbuffer_consume(buffer, NULL, ',');
    ringbuffer_del(buffer);
    
    // mcc
    for (int i = 0; i < 3; i++)
        oper[i] = ringbuffer_del(buffer);
    oper[3] = ',';
    
    // mnc
    ringbuffer_consume(buffer, &oper[4], '"');
    int len = strlen(oper);
    oper[len] = ',';
    oper[len+1] = '\0';

    ringbuffer_del(buffer);
    return;
}

void get_list(int RAT, Ringbuffer *buffer, char *str)
{
    char tmp[DEFAULT_CELL_LIST_INFO_SIZE] = {0};
    char *split = ",";
    
    switch (RAT)
    {
    case 2: 
        for (int i = 0; i < 3; i++)
            ringbuffer_consume(buffer, NULL,',');
        
        // <lac>
        ringbuffer_consume(buffer, tmp, ','); 
        strcat(str, tmp);
        strcat(str, split);
        
        // <ci>
        ringbuffer_consume(buffer, tmp, ','); 
        strcat(str, tmp);
        strcat(str, split);

        ringbuffer_consume(buffer, NULL,',');

        // <rxlev>
        ringbuffer_consume(buffer, tmp, ','); 
        strcat(str, tmp);

        ringbuffer_consume(buffer, NULL, 0);
        break;
    
    case 3: case 4:
        for (int i = 0; i < 4; i++)
            ringbuffer_consume(buffer, NULL,',');
        
        // <lac> / <tac>
        ringbuffer_consume(buffer, tmp, ',');
        strcat(str, tmp);
        strcat(str, split);

        // <ci>
        ringbuffer_consume(buffer, tmp, ',');
        strcat(str, tmp);
        strcat(str, split);
        
        // <rscp> / <rsrp>
        ringbuffer_consume(buffer, tmp, ',');
        strcat(str, tmp);

        ringbuffer_consume(buffer, NULL, 0);
        break;
    
    default:
        ringbuffer_consume(buffer, NULL, 0);
        break;
    }
    
    return;
}

int32_t make_cell_list(int RAT, char *read_ptr, char list[DEFAULT_CELL_LIST_SIZE][DEFAULT_CELL_LIST_INFO_SIZE])
{
    Ringbuffer buffer;
    ringbuffer_set(&buffer);

    read_ptr = scancontrol(RAT, read_ptr, &buffer);
    if (read_ptr == NULL) {
        ESP_LOGE("scancontrol", "ERROR");
        return -1;
    }

    // send ATcommand
    printf("AT+QOPS\n");
    
    // echo
    read_ptr = ringbuffer_produce(&buffer, read_ptr);
    ringbuffer_consume(&buffer, NULL, 0);
    
    // 0x0A
    read_ptr = ringbuffer_produce(&buffer, read_ptr);
    ringbuffer_consume(&buffer, NULL, 0);

    // res
    char byte = *read_ptr;
    read_ptr = ringbuffer_produce(&buffer, read_ptr);
    
    if ( strncmp(buffer.thebuffer, ERROR, 5) == 0 ) {  // res ERROR
        ESP_LOGE("AT+QOPS", "ERROR");
        return -1;
    }

    char oper[16] = {0};
    int32_t list_len = 0;

    while (byte != 0x0A) {
        //printf("byte: %c\n", byte);
        if (byte == '+') {
            get_oper(&buffer, oper);
            //printf("%s\n", oper);
        }
        else {
            strcpy(list[list_len], oper);
            get_list(RAT, &buffer, list[list_len]);
            list_len++;
        }  
        
        byte = *read_ptr;
        read_ptr = ringbuffer_produce(&buffer, read_ptr); 
        
    }

    // 0x0A
    ringbuffer_consume(&buffer, NULL, 0);
    
    // OK
    char res[16] = {0};
    read_ptr = ringbuffer_produce(&buffer, read_ptr);
    ringbuffer_consume(&buffer, res, 0x0A);
    if ( strncmp(res, OK, 2) != 0) {     // 不是收到OK
        ESP_LOGE("AT+QOPS", "ERROR");
    }
    //ESP_LOGI("AT+QOPS", "OK");
    return list_len;
}

void app_main(void)
{
    //make_wifi_list();
    
    char *read_ptr;
    char cell_list[DEFAULT_CELL_LIST_SIZE][DEFAULT_CELL_LIST_INFO_SIZE] = {0};
    int32_t list_len = 0;

    printf("2G:\n");
    read_ptr = gsm;
    list_len = make_cell_list(2, read_ptr, cell_list);
    for (int i = 0; i < list_len; i++) {
        printf("%s\n", cell_list[i]);
    }
    
    printf("3G:\n");
    read_ptr = wcdma;
    list_len = make_cell_list(3, read_ptr, cell_list);
    for (int i = 0; i < list_len; i++) {
        printf("%s\n", cell_list[i]);
    }
    
    printf("4G:\n");
    read_ptr = lte;
    list_len = make_cell_list(4, read_ptr, cell_list);
    
    for (int i = 0; i < list_len; i++) {
        printf("%s\n", cell_list[i]);
    }
    
}