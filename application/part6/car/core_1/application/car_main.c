/*
 *** Try Kernel マイコンカー Autonomous Robotics Platform
 *          アプリケーション・メイン
 */
#include <trykernel.h>
#include "appli.h"

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

int usermain(void)
{
    tm_putstring("Start\n");

    mbfid = tk_cre_mbf( &cmbf);                     // メッセージバッファの生成

    tskid_icc = tk_cre_tsk(&ctsk_icc);              // CPUコア間通信タスクの生成・実行
    tk_sta_tsk(tskid_icc, 0);

    tskid_run = tk_cre_tsk(&ctsk_run);              // 走行制御タスクの生成・実行
    tk_sta_tsk(tskid_run, 0);

    tk_slp_tsk(TMO_FEVR);       // 初期タスクを待ち状態に
    return 0;
}
