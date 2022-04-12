#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <rom/ets_sys.h>

#define GPIO_INPUT_PIN_SEL   ((1ULL<<UART_RXD) | (1ULL<<UART_CTS))
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<UART_TXD) | (1ULL<<UART_RTS))

#define UART_TXD GPIO_NUM_4
#define UART_RXD GPIO_NUM_5
#define UART_RTS GPIO_NUM_18
#define UART_CTS GPIO_NUM_19

#define CAPACITY             20

#define ESP_INTR_FLAG_DEFAULT 0

#define ATCOMMAND_NUM    32
#define ATCOMMAND_LEN    64

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

void get_res(char *res)
{
    for (int32_t i = 0; i < ATCOMMAND_LEN; i++)
    {
        res[i] = receive_byte();
        
        if ( (res[i] == 0x0D) ) {
            res[i] = '\0';
            break;
        }
        
    }
}
void send_ATcommand(char *command)
{
    size_t len = strlen(command);
    for (size_t i = 0; i < len; i++) {
        send_byte(command[i]);
    }
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

/*
send
- wait CTS low
- send by TXD

receive
- get RI (urc)
- RTS pulldown
- uart buffer 1 rec
- host buffer rec
- parser
*/

/*
void app_main(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pin_bit_mask = (GPIO_INPUT_PIN_SEL | GPIO_OUTPUT_PIN_SEL);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    gpio_set_direction(UART_RXD, GPIO_MODE_INPUT);
    gpio_set_direction(UART_CTS, GPIO_MODE_INPUT);
    gpio_set_direction(UART_TXD, GPIO_MODE_OUTPUT);  
    gpio_set_direction(UART_RTS, GPIO_MODE_OUTPUT);  

    gpio_set_level(UART_TXD, 1);
    vTaskDelay (1);
    
    // send  
    char command[ATCOMMAND_NUM][ATCOMMAND_LEN];
    char res[ATCOMMAND_LEN];

    while (gpio_get_level(UART_CTS) != 0)
    {
        continue;
    }
    send_ATcommand(command);

    // receive
    // RI
    gpio_set_level(UART_RTS, 0);

}
*/