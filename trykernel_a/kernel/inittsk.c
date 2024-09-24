/* 
 *** TryKernel-A     初期タスク
*/
#include <trykernel.h>
#include <knldef.h>

/*** Core 0 メイン関数 & 初期タスク ***/

void initsk_c0(INT stacd, void *exinf);             // 初期タスクの実行プログラム
UW  tskstk_ini_c0[1024/sizeof(UW)];                 // 初期タスクのスタック
ID  tskid_ini_c0;                                   // 初期タスクのID番号

T_CTSK  ctsk_ini_c0 = {
    .tskatr     = TA_HLNG | TA_RNG0 | TA_USERBUF,   // タスク属性
    .task       = initsk_c0,                        // タスクの実行関数
    .itskpri    = 1,                                // タスク優先度
    .stksz      = sizeof(tskstk_ini_c0),            // スタックサイズ
    .bufptr     = tskstk_ini_c0,                    // スタックへのポインタ
};

void initsk_c0(INT stacd, void *exinf)
{
    usermain_c0();                                  // ユーザ・メイン関数の実行
    tk_ext_tsk();
}

int main_c0(void)
{
    init_icc_int();                                 // ① CPUコア間割り込み初期化
    init_icc_msg();                                 // ② CPUコア間メッセージ初期化
    tm_com_init();                                  // ③ デバッグ用シリアル通信の初期化

    /* ④ 初期タスクの生成と実行 */
    tskid_ini_c0 = tk_cre_tsk(&ctsk_ini_c0);
    tk_sta_tsk(tskid_ini_c0, 0);

    while(1);      // ここは実行されない
}

/*** Core 1 メイン関数 & 初期タスク ***/

void initsk_c1(INT stacd, void *exinf);             // 初期タスクの実行プログラム
UW  tskstk_ini_c1[1024/sizeof(UW)];                 // 初期タスクのスタック
ID  tskid_ini_c1;                                   // 初期タスクのID番号

T_CTSK  ctsk_ini_c1 = {
    .tskatr     = TA_HLNG | TA_RNG0 | TA_USERBUF,   // タスク属性
    .task       = initsk_c1,                        // タスクの実行関数
    .itskpri    = 1,                                // タスク優先度
    .stksz      = sizeof(tskstk_ini_c1),            // スタックサイズ
    .bufptr     = tskstk_ini_c1,                    // スタックへのポインタ
};

void initsk_c1(INT stacd, void *exinf)
{
    usermain_c1();                                  // ユーザ・メイン関数の実行
    tk_ext_tsk();
}

int main_c1(void)
{
    init_icc_int();                                 // CPUコア間割り込み初期化
    init_icc_msg();                                 // CPUコア間メッセージ初期化

    /* 初期タスクの生成と実行 */
    tskid_ini_c1 = tk_cre_tsk(&ctsk_ini_c1);
    tk_sta_tsk(tskid_ini_c1, 0);

    while(1);      // ここは実行されない
}
