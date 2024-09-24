/* 
 *** Try Kernel v2  起動処理
*/

#include <typedef.h>
#include <sysdef.h>
#include <syslib.h>
#include <knldef.h>

/* 初期スタック */
UW  knl_stack[2048/sizeof(UW)];

/*** システムタイマの初期化 ***/
static void init_systim(void)
{
    out_w(SYST_CSR ,SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT);     /* SysTick動作停止 */
    out_w(SYST_RVR, (TIMER_PERIOD*TMCLK_KHz)-1);                /* リロード値設定 */
    out_w(SYST_CVR, (TIMER_PERIOD*TMCLK_KHz)-1);                /* カウント値設定 */
    out_w(SYST_CSR, SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE);   /* SysTick動作開始 */
}

/*** CPUコア1の起動処理 ****/
void main_c1(void)
{
    out_w(SCB_SHPR3, (INTLEVEL_0<<24)|(INTLEVEL_3<<16));    // PendSVC例外とSysTick例外の優先度設定 */
    init_systim();      // システムタイマの初期化
    set_primask(0);     // 割り込みを有効化

    knl_main();         // OSのメイン関数を実行
    while(1);
}
