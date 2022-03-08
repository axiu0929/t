#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <rom/ets_sys.h>

#define GPIO_OUTPUT_IO       GPIO_NUM_3
#define GPIO_OUTPUT_TEST     GPIO_NUM_27
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO) | (1ULL<<GPIO_OUTPUT_TEST))

#define GPIO_INPUT_IO        GPIO_NUM_4
#define GPIO_INPUT_PIN_SEL   (1ULL<<GPIO_OUTPUT_IO)

void send_byte(char data)
{
    uint32_t const delay = 1000000 / 9600;
    
    gpio_set_level(GPIO_OUTPUT_IO, 0); // send_startbit
    gpio_set_level(GPIO_OUTPUT_TEST, 1);
    ets_delay_us(delay);

    for (int i = 0; i < 8; ++i) 
    {
        char bit_HL = data & 1;
        data = data >> 1;
        gpio_set_level(GPIO_OUTPUT_IO, bit_HL); // send_onedatabit
        ets_delay_us(delay);
    }
    
    gpio_set_level(GPIO_OUTPUT_IO, 1); // send_stopbit
    gpio_set_level(GPIO_OUTPUT_TEST, 0);
    ets_delay_us(delay);
    //ets_delay_us(delay);
}

char receive_byte(void)
{
    uint32_t const delay = 1000000 / 9600;
    gpio_set_direction(GPIO_INPUT_IO, GPIO_MODE_INPUT);
    char data = 0;
    
    while (1)
    {
        if (gpio_get_level(GPIO_INPUT_IO) == 0) {
            //gpio_set_level(GPIO_OUTPUT_TEST, 1);
            ets_delay_us(delay*3/2);
            
            for (int i = 0; i < 8; ++i) 
            {
                char bit_HL = gpio_get_level(GPIO_INPUT_IO);
                bit_HL = bit_HL << i;
                data = data | bit_HL;
                ets_delay_us(delay);
            }

            //gpio_set_level(GPIO_OUTPUT_TEST, 0);
            return data;
        }
    }
    return 't';
    
}     

void Test1()
{
    send_byte('S');
    send_byte(receive_byte());
}

void func()
{
    const char* API = "ADD+TEST1";

    const int32_t buffer_size = 20;
    char buffer[buffer_size];

    for (int32_t i = 0; i < buffer_size; i++)
    {
        buffer[i] = receive_byte();
        //send_byte('R');
        //send_byte(buffer[i]);
        
        if ( (buffer[i] == 0x0D) ) {
            buffer[i] = '\0';
            break;
        }
        
    }

    if ( strcmp(buffer, API) == 0 ) {
        Test1();
    }
    else {
        send_byte('F');
    }
    
}

void app_main(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    gpio_set_level(GPIO_OUTPUT_IO, 1);
    vTaskDelay (1);
    
    // send   
    gpio_set_level(GPIO_OUTPUT_TEST, 0);
    
    /*
    for (;;) {
        send_byte(0x78);
        //send_byte(receive_byte());
    } 
    */

    /*
    for (int c = 65; c < 91; ++c) {
        send_byte(c);
    }  
    for (int c = 97; c < 123; ++c) {
        send_byte(c);
    } 
    */

    // receive
    
    /*
    send_byte('T');
    for (;;) {
        send_byte(receive_byte());
    }
    */

   // call API
   for (;;) {
       func();
   }
   //func();
}