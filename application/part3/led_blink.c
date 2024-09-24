#include <trykernel.h>

int usermain_c0(void)
{
    UW      data = 0;

    icc_sync_core(CPU_CORE);        /* CPUコア間の同期 */

    /* LEDのポートの初期化 */
    out_w(GPIO_OE_CLR, (1<<14));    /* GP14端子出力無効 */
    out_w(GPIO_OUT_CLR, (1<<14));   /* GP14端子出力クリア */
    out_w(GPIO_CTRL(14), 5);        /* GP14端子 SIO */
    out_w(GPIO_OE_SET, (1<<14));    /* GP14出力有効 */

    /* LEDの点滅 */
    while(1) {
        out_w(GPIO_OUT_XOR, 1<<14);
        tk_dly_tsk(500);
    }
    return 0;
}

int usermain_c1(void)
{
    UW      data;

    icc_sync_core(CPU_CORE);        /* CPUコア間の同期 */

    /* LEDのポートの初期化 */
    out_w(GPIO_OE_CLR, (1<<15));    /* GP15端子出力無効 */
    out_w(GPIO_OUT_CLR, (1<<15));   /* GP15端子出力クリア */
    out_w(GPIO_CTRL(15), 5);        /* GP15端子 SIO */
    out_w(GPIO_OE_SET, (1<<15));    /* GP15出力有効 */

    /* LEDの点滅 */
    while(1) {
        out_w(GPIO_OUT_XOR, 1<<15);
        tk_dly_tsk(500);
    }
    return 0;
}