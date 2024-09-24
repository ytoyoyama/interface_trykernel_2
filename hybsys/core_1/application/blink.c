#include <trykernel.h>

#define LED_PIN     15              // LEDのGPIO番号

int usermain(void)
{
    out_w(GPIO_OE_CLR, (1<<LED_PIN));    // GPIO端子出力無効
    out_w(GPIO_OUT_CLR, (1<<LED_PIN));   // GPIO端子出力クリア
    out_w(GPIO_CTRL(LED_PIN), 5);        // GPIO端子 SIO
    out_w(GPIO_OE_SET, (1<<LED_PIN));    // GPIO出力有効

    /* LEDの点滅 */
    while(1) {
        out_w(GPIO_OUT_XOR, 1<<LED_PIN);    // LED反転
        tk_dly_tsk(500);                    // 500ミリ秒休止
    }
    return 0;
}
