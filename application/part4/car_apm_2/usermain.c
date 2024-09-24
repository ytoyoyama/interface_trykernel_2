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
/* メッセージバッファの情報*/
ID  mbfid;                              // メッセージバッファ ID
#define MSG_SZ  sizeof(UINT)            // メッセージサイズ
#define MBF_SZ  ((MSG_SZ+1)*5)          // メッセージバッファサイズ
UB  mbfbuf[MBF_SZ];                     // メッセージバッファ領域

T_CMBF  cmbf = {                        // メッセージバッファ生成情報
    .bufptr = mbfbuf,
    .bufsz  = MBF_SZ,
    .maxmsz = MSG_SZ,
    .mbfatr = 0,
};

/*-------------------------------------------------------------------------*/
/* メッセージ中継タスクの情報 */
ID  tskid_msg;                          // タスクID番号
#define STKSZ_MSG   1024                // スタックサイズ

UW  tskstk_msg[STKSZ_MSG/sizeof(UW)];   // スタック領域
void tsk_msg(INT stacd, void *exinf);   // タスク実行関数

T_CTSK ctsk_msg = {                     // タスク生成情報
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_msg,
    .stksz      = STKSZ_MSG,
    .itskpri    = 7,
    .bufptr     = tskstk_msg,
};

/* メッセージ中継タスクの実行関数 */
void tsk_msg(INT stacd, void *exinf)
{
    UINT    msg;

    while(1) {
        icc_rcv_msg(icmid, &msg, TMO_FEVR);        // CPUコア間メッセージ受信
        tk_snd_mbf(mbfid, &msg, sizeof(msg), 100);  // メッセージ送信
    }
}

/*-------------------------------------------------------------------------*/
/* CPUコア1 ユーザメイン関数 */
int usermain_c1(void)
{
    icc_sync_core(CPU_CORE);                // CPUコア間の同期

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

    mbfid = tk_cre_mbf( &cmbf);             // メッセージバッファの生成

    tskid_msg = tk_cre_tsk(&ctsk_msg);      //メッセージ中継タスクの生成・実行
    tk_sta_tsk(tskid_msg, 0);

    tskid_dsns = tk_cre_tsk(&ctsk_dsns);    // 障害物センサータスクの生成・実行
    tk_sta_tsk(tskid_dsns, 0);

    tskid_run = tk_cre_tsk(&ctsk_run);      // 走行制御タスクの生成・実行
    tk_sta_tsk(tskid_run, 0);

    tk_slp_tsk(TMO_FEVR);                   // 初期タスクを待ち状態に
    return 0;
}
