#include <trykernel.h>
#define STKSZ_SNS   1024                // スタックサイズ

ID  tskid_a;                            // タスクID番号
UW  tskstk_a[STKSZ_SNS/sizeof(UW)];     // スタック領域

void tsk_a(INT stacd, void *exinf)
{
    /* LEDのポートの初期化 */
    out_w(GPIO_OE_CLR, (1<<14));    /* GP14端子出力無効 */
    out_w(GPIO_OUT_CLR, (1<<14));   /* GP14端子出力クリア */
    out_w(GPIO_CTRL(14), 5);        /* GP14端子 SIO */
    out_w(GPIO_OE_SET, (1<<14));    /* GP14出力有効 */

    /* LEDの点滅 */
    while(1) {
        if(CPU_CORE) tm_putstring("A-1\n");
        else tm_putstring("A-0\n");

        out_w(GPIO_OUT_XOR, 1<<14);
        tk_dly_tsk(500);
        // while(1);               // ①ここのコメントを外す
    }
}

ID  tskid_b;                            // タスクID番号
UW  tskstk_b[STKSZ_SNS/sizeof(UW)];     // スタック領域

void tsk_b(INT stacd, void *exinf)
{
    /* LEDのポートの初期化 */
    out_w(GPIO_OE_CLR, (1<<15));    /* GP15端子出力無効 */
    out_w(GPIO_OUT_CLR, (1<<15));   /* GP15端子出力クリア */
    out_w(GPIO_CTRL(15), 5);        /* GP15端子 SIO */
    out_w(GPIO_OE_SET, (1<<15));    /* GP15出力有効 */

    /* LEDの点滅 */
    while(1) {
        if(CPU_CORE) tm_putstring("B-1\n");
        else tm_putstring("B-0\n");

        out_w(GPIO_OUT_XOR, 1<<15);
        tk_dly_tsk(500);
    }
}

int usermain(void)
{
    T_CTSK ctsk = {
        .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
        .stksz      = STKSZ_SNS,
        .itskpri    = 5,
    };

    ctsk.task       = tsk_a;
    ctsk.bufptr     = tskstk_a;
    tskid_a = tk_cre_tsk(&ctsk);
    tk_sta_tsk(tskid_a, 0);

    ctsk.task       = tsk_b;
    ctsk.bufptr     = tskstk_b;
    tskid_b = tk_cre_tsk(&ctsk);
    tk_sta_tsk(tskid_b, 0);

    tk_slp_tsk(TMO_FEVR);
    return 0;
}