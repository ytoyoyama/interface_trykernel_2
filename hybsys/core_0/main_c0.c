#include <stdio.h>
#include "pico/stdlib.h"

#define LED_PIN     14              // LEDのGPIO番号

void main_c0(void)
{
    gpio_init(LED_PIN);                 // GPIO初期化
    gpio_set_dir(LED_PIN, GPIO_OUT);    // GPIOを出力に設定
    while (true) {
        gpio_put(LED_PIN, 1);           // LED点灯
        sleep_ms(500);                  // 500ミリ秒間休止
        gpio_put(LED_PIN, 0);           // LED消灯
        sleep_ms(500);                  // 500ミリ秒間休止
    }
}
