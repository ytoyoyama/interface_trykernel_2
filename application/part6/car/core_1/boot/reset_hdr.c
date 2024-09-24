/* 
 *** Try Kernel v2  リセットハンドラ
*/

#include <typedef.h>
#include <sysdef.h>
#include <syslib.h>
#include <knldef.h>

/* 初期スタック */
UW  knl_stack[2048/sizeof(UW)];

/*** 例外・割込みの初期化 ***/
static void init_int(void)
{
    *(_UW*)SCB_VTOR = (UW)knl_vec_tbl;

    /* PendSVC例外とSysTick例外の優先度設定 */
    out_w(SCB_SHPR3, (INTLEVEL_0<<24)|(INTLEVEL_3<<16));
}

/*** ペリフェラルの有効化 ***/

static void init_peri(void)
{
    /* GPIOの有効化*/
    clr_w(RESETS_RESET, (1<<5));    /* IO_BANK0 */
    while((in_w(RESETS_RESET_DONE) & (1<<5))==0);

    clr_w(RESETS_RESET, (1<<8));    /* PADS_BANK0 */
    while((in_w(RESETS_RESET_DONE) & (1<<8))==0);

    /* UART0の有効化 */
    clr_w(RESETS_RESET, (1<<22));
    while((in_w(RESETS_RESET_DONE) & (1<<22))==0);

    /* 端子設定 */
#if (USE_PICO_W == 0)       /* オンボードLED初期化(Pico Wを除く) */
    out_w(GPIO_OE_CLR, (1<<25));    /* P25端子出力無効 */
    out_w(GPIO_OUT_CLR, (1<<25));   /* P25端子出力クリア */
    out_w(GPIO_CTRL(25), 5);        /* P25端子 SIO */
    out_w(GPIO_OE_SET, (1<<25));    /* P25出力有効 */
#endif

    out_w(GPIO_CTRL(0), 2);         /* P0端子 UART0-TX */
    out_w(GPIO_CTRL(1), 2);         /* P1端子 UART0-RX */

}

/*** システムタイマの初期化 ***/

static void init_systim(void)
{
    out_w(SYST_CSR ,SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT);     /* SysTick動作停止 */
    out_w(SYST_RVR, (TIMER_PERIOD*TMCLK_KHz)-1);                /* リロード値設定 */
    out_w(SYST_CVR, (TIMER_PERIOD*TMCLK_KHz)-1);                /* カウント値設定 */
    out_w(SYST_CSR, SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE);   /* SysTick動作開始 */
}

/*** リセットハンドラ ****/

void reset_core1(void)
{
    // UINT    intsts;
    
    // DI(intsts);     /* 割込みを無効化 */

    init_int();     /* 割込み・例外の初期化 */
    init_peri();    /* ペリフェラルの有効化 */
    init_systim();  /* システムタイマの初期化 */

    // EI(intsts);     /* 割込みを有効化 */
    set_primask(0);

    knl_main();         /* main関数を実行 */
    while(1);
}
