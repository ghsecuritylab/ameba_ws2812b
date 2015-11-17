


#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <gpio_api.h>
#include <wlan_fast_connect/example_wlan_fast_connect.h>

#include "ws2812b.h"



#define GPIO_WS2811_PIN     PA_0
#define NUM_LEDS            60



extern wlan_init_done_ptr p_wlan_init_done_callback;



gpio_t gpio_ws2811;






int _wlan_init_done_callback() { printf("%s()\n", __FUNCTION__);

    /* Wlan Set Access Point SSID */
    fATW3("ameba_smart_lighting");

    /* Set Access Point Security Key */
    fATW4("proscendameba");

    /* Wlan Set Access Point Channel */
    fATW5("1");

    /* Wlan Activate Access Point */
    fATWA();

    /* Wlan Start Web Server */
    fATWE();

    return 0;
}














u8 _r, _g, _b;



/*
** data reansfer time(TH + TL = 1.25us +- 600ns)
** 
** +-----+---------------------------+------------+---------+
** | T0H | 0 code, high voltage time | 0.4us      | +-150ns |
** +-----+---------------------------+------------+---------+
** | T1H | 1 code, high voltage time | 0.8us      | +-150ns |
** +-----+---------------------------+------------+---------+
** | T0L | 0 code, low voltage time  | 0.85us     | +-150ns |
** +-----+---------------------------+------------+---------+
** | T1L | 1 code, low voltage time  | 0.45us     | +-150ns |
** +-----+---------------------------+------------+---------+
** | RES | low voltage time          | Above 50us |         |
** +-----+---------------------------+------------+---------+
** 
*/



void Ai_WS2811_sendByte(u8 b) {

    u8 i = 8;

    do {
        if ( (b & 0x80) == 0) { /* send a '0', T0H + T0L */
            /* T0H, 0 code, high voltage time, 0.4us +- 150ns */
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            /* T0L, 0 code, low voltage time, 0.85us +-150ns */
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
        } else { /* send a '1', T1H + T1L */
            /* T1H, 1 code, high voltage time, 0.8us +- 150ns */
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000001); // 96ns
            /* T1L, 1 code, low voltage time, 0.45us +-150ns */
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
            HAL_WRITE32(GPIO_REG_BASE, GPIO_PORTA_DR, 0x00000000); // 96ns
        }
        b = b + b;
    } while (-- i != 0);
}



void Ai_WS2811_send() {
    Ai_WS2811_sendByte(_g);
    Ai_WS2811_sendByte(_r);
    Ai_WS2811_sendByte(_b);
}  



void Ai_WS2811_setColor(u8 r, u8 g, u8 b) {
    printf("%s: (r,g,b) = (%d,%d,%d)\n", __FUNCTION__, r,g,b);

    _r = r;
    _g = g;
    _b = b;
} 






void sendLEDs() {
    u8 i;
    taskENTER_CRITICAL(); 
    for (i = 0; i < NUM_LEDS; ++ i) {
      Ai_WS2811_send();
    }
    taskEXIT_CRITICAL(); 
}










u16 Blink_ms = 500;
u8 IsBlink = 0;

u8 IsMeteor = 0;

u8 Config_r, Config_g, Config_b;

u8 BackgroudColor[NUM_LEDS][3] = {
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
    {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, 
};






void vTaskFunction(void *pvParameters) { 

    static int i = 0;

    static int meteorIdx = 0;

    int meteor_length = 8;
    int meteor_head_idx = 0;
    int meteor_tail_idx = meteor_head_idx - (meteor_length - 1);

    static int blinking = 0;

    portTickType xLastWakeTime = xTaskGetTickCount(); 
     
    for( ;; ) { 



        if (IsBlink) {
            if (blinking) {
                Ai_WS2811_setColor(Config_r, Config_g, Config_b);
                sendLEDs();
            } else {
                Ai_WS2811_setColor(0, 0, 0);
                sendLEDs();
            }
            blinking ^= 1;
        } else if (IsMeteor) {

            taskENTER_CRITICAL(); 

            for (i = 0; i < NUM_LEDS; ++ i) {

              if ( (i <= meteor_head_idx) && (i >= meteor_tail_idx)) {
                  Ai_WS2811_sendByte(Config_g - (meteor_head_idx - i) * (Config_g /meteor_length));
                  Ai_WS2811_sendByte(Config_r - (meteor_head_idx - i) * (Config_r /meteor_length));
                  Ai_WS2811_sendByte(Config_b - (meteor_head_idx - i) * (Config_b /meteor_length));
              } else {
                  Ai_WS2811_sendByte(BackgroudColor[i][0]);
                  Ai_WS2811_sendByte(BackgroudColor[i][1]);
                  Ai_WS2811_sendByte(BackgroudColor[i][2]);
              }

            }

            taskEXIT_CRITICAL(); 

            //meteorIdx = (meteorIdx + 1) % NUM_LEDS;
            meteor_head_idx = (meteor_head_idx + 1) % (NUM_LEDS + (meteor_length - 1));
            meteor_tail_idx = meteor_head_idx - (meteor_length - 1);

        }


        vTaskDelayUntil( &xLastWakeTime, ( Blink_ms / portTICK_RATE_MS ) ); 
    } 

} 









void _init(void) { printf("%s\n", __FUNCTION__);

    gpio_init(&gpio_ws2811, GPIO_WS2811_PIN);
    gpio_dir(&gpio_ws2811, PIN_OUTPUT);
    gpio_mode(&gpio_ws2811, PullNone);



    /* Call back from wlan driver after wlan init done */
    p_wlan_init_done_callback = _wlan_init_done_callback;

    /* wlan intialization */
    wlan_network();



    xTaskCreate( vTaskFunction, "Task", 1000, (void*)vTaskFunction, 1, NULL ); 

    /* Start the scheduler so the tasks start executing. */ 
    vTaskStartScheduler();     



} 



ws2812b_t ws2812b = {
    .init   = _init,
};

