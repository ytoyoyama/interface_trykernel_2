/*
 *** Try Kernel マイコンカー Autonomous Robotics Platform
 *          アプリケーション・メイン
 */
#include <trykernel.h>
#include "appli.h"

/*-------------------------------------------------------------------------*/
/* CPUコア間メッセージの情報 */
ID  icmid;                              // CPUコア間メッセージ ID
#define MSG_SZ  sizeof(UINT)            // メッセージサイズ
#define MBF_SZ  ((MSG_SZ+1)*5)          // メッセージバッファサイズ
UB  icmbuf[MBF_SZ];                     // メッセージバッファ領域

T_CICM  cicm = {                        // メッセージバッファ生成情報
    .icmatr = TA_ICM_TO_C0,
    .bufsz  = MBF_SZ,
    .maxmsz = MSG_SZ,
    .bufptr = icmbuf,
};

/*-------------------------------------------------------------------------*/
/* CPUコア1 ユーザメイン関数 */
int usermain_c1(void)
{
    icc_sync_core(CPU_CORE);                // CPUコア間の同期

    tskid_dsns = tk_cre_tsk(&ctsk_dsns);    // 障害物センサータスクの生成・実行
    tk_sta_tsk(tskid_dsns, 0);

    tskid_lsns = tk_cre_tsk(&ctsk_lsns);    // 走路センサータスクの生成・実行
    tk_sta_tsk(tskid_lsns, 0);

    tk_slp_tsk(TMO_FEVR);                   // 初期タスクを待ち状態に
    return 0;
}

/* CPUコア0 ユーザメイン関数 */
int usermain_c0(void)
{
    icmid = icc_cre_msg(&cicm);             // CPUコア間メッセージの生成
    icc_sync_core(CPU_CORE);                // CPUコア間の同期

    tskid_run = tk_cre_tsk(&ctsk_run);      // 走行制御タスクの生成・実行
    tk_sta_tsk(tskid_run, 0);

    tk_slp_tsk(TMO_FEVR);                   // 初期タスクを待ち状態に
    return 0;
}
