/* 
 *** TryKernel-S     初期タスク
*/
#include <trykernel.h>
#include <knldef.h>

/*** メイン関数 & 初期タスク ***/

void initsk(INT stacd, void *exinf);                // 初期タスクの実行プログラム
UW  tskstk_ini[1024*2/sizeof(UW)];                  // 初期タスクのスタック
ID  tskid_ini;                                      // 初期タスクのID番号

T_CTSK  ctsk_ini = {
    .tskatr     = TA_HLNG | TA_RNG0 | TA_USERBUF,   // タスク属性
    .task       = initsk,                           // タスクの実行関数
    .itskpri    = 1,                                // タスク優先度
    .stksz      = sizeof(tskstk_ini),               // スタックサイズ
    .bufptr     = tskstk_ini,                       // スタックへのポインタ
};

void initsk(INT stacd, void *exinf)
{
    usermain();         // アプリケーション・プログラムのユーザ・メイン関数を実行
    tk_ext_tsk();       // タスクの終了
}

int main_c0(void)
{
    init_icc_int();                                 // CPUコア間割り込み初期化
    /* CPUコア間割り込みハンドラ登録 */
    icc_def_int(ICCINT_DISPATCH, iccint_dispatch);
    icc_def_int(ICCINT_ENAINT, iccint_ebableint);
    icc_def_int(ICCINT_DISINT, iccint_disableint);

    tm_com_init();                                  // デバッグ用シリアル通信の初期化
    icc_sync_core(0);                               // 起動処理終了の同期

    tskid_ini = tk_cre_tsk(&ctsk_ini);              // 初期タスク生成
    tk_sta_tsk(tskid_ini, 0);                       // 初期タスク実行

    while(1);      // ここは実行されない
}

int main_c1(void)
{
    init_icc_int();                                 // CPUコア間割り込み初期化
    /* CPUコア間割り込みハンドラ登録 */
    icc_def_int(ICCINT_DISPATCH, iccint_dispatch);
    icc_def_int(ICCINT_ENAINT, iccint_ebableint);
    icc_def_int(ICCINT_DISINT, iccint_disableint);

    icc_sync_core(1);                               // 起動処理終了の同期

    while(1);                                       // 最初のディスパッチまで待つ
}
