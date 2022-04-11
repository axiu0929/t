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

#define CAPACITY             20

#define ESP_INTR_FLAG_DEFAULT 0

void send_byte(char data)
{
    uint32_t const delay = 1000000 / 9600;
    
    gpio_set_level(GPIO_OUTPUT_IO, 0); // send_startbit
    //gpio_set_level(GPIO_OUTPUT_TEST, 1);
    ets_delay_us(delay);

    for (int i = 0; i < 8; ++i) 
    {
        char bit_HL = data & 1;
        data = data >> 1;
        gpio_set_level(GPIO_OUTPUT_IO, bit_HL); // send_onedatabit
        ets_delay_us(delay);
    }
    
    gpio_set_level(GPIO_OUTPUT_IO, 1); // send_stopbit
    //gpio_set_level(GPIO_OUTPUT_TEST, 0);
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

void get_res(char *res, int32_t res_size)
{

    for (int32_t i = 0; i < res_size; i++)
    {
        res[i] = receive_byte();
        
        if ( (res[i] == 0x0D) ) {
            res[i] = '\0';
            break;
        }
        
    }

}
void send_ATcommand(char *command, char *res, int32_t res_size)
{
    size_t len = strlen(command);
    for (size_t i = 0; i < len; i++) {
        send_byte(command[i]);
    }
    get_res(res, res_size);
}


typedef struct ringbuffer
{
    int32_t head;
    int32_t tail;
    char thebuffer[CAPACITY+1];
}Ringbuffer;
void ringbuffer_set(Ringbuffer *buffer)
{
    buffer->head = -1;
    buffer->tail = -1;
}
int32_t ringbuffer_add(Ringbuffer *buffer, char byte)
{
    // if full return 0
    buffer->tail = (buffer->tail + 1) % CAPACITY;
    (buffer->thebuffer)[buffer->tail] = byte;
    return 1;
}
char ringbuffer_del(Ringbuffer *buffer)
{
    if (buffer->head == buffer->tail) {
        return 0; //buffer is empty
    }
    else {
        buffer->head = (buffer->head + 1) % CAPACITY;
        return (buffer->thebuffer)[buffer->head];
    }
}

char ringbuffer_produce(Ringbuffer *buffer)
{
    char byte = receive_byte();
    ringbuffer_add(buffer, byte);
    return byte;
}
void ringbuffer_consume(Ringbuffer *buffer)
{
    char byte = ringbuffer_del(buffer);
    while ( (byte != 0) && (byte != 0x0D) )
    {
        send_byte(byte);
        byte = ringbuffer_del(buffer);
    }
    
}

void receive_handler(void* arg)
{
    arg++;
    //gpio_uninstall_isr_service();
    //send_byte('R');
    //receive_byte();
    //gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
}

void app_main(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pin_bit_mask = (GPIO_OUTPUT_PIN_SEL | GPIO_INPUT_PIN_SEL);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    gpio_set_direction(GPIO_INPUT_IO, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_OUTPUT_IO, GPIO_MODE_OUTPUT);
    //gpio_set_direction(GPIO_OUTPUT_TEST, GPIO_MODE_OUTPUT);    

    gpio_set_level(GPIO_OUTPUT_IO, 1);
    vTaskDelay (1);
    
    // send  
    char command[64];
    char res[64];
    int32_t res_size = 64;
    send_ATcommand(command, res, res_size);
}