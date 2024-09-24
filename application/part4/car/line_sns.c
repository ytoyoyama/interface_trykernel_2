/*
 *** Try Kernel マイコンカー Autonomous Robotics Platform
 *          走路センサー制御タスク
 */
#include <trykernel.h>
#include "appli.h"

#define ON_LINE     1800

/* 走路センサータスクの情報 */
ID  tskid_lsns;                          // タスクID番号
#define STKSZ_LSNS   1024                // スタックサイズ
UW  tskstk_lsns[STKSZ_LSNS/sizeof(UW)];  // スタック領域
void tsk_lsns(INT stacd, void *exinf);   // タスク実行関数

T_CTSK ctsk_lsns = {                     // タスク生成情報
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_lsns,
    .stksz      = STKSZ_LSNS,
    .itskpri    = 5,
    .bufptr     = tskstk_lsns,
};

/* 走路センサータスクの実行関数 */
void tsk_lsns(INT stacd, void *exinf)
{
    ID		dd_adc;                     // A/DCデバイスディスクリプタ
    UW		data[3];                    // センサー値
    UINT	msg, pre_msg;               // 送信メッセージ
    SZ      asz;
    ER		err;
    
    dd_adc = tk_opn_dev((UB*)"adca", TD_READ);

    pre_msg = MSG_STRAIGHT;
    while(1) {
        /* 走路センサーから値を取得 */
        err = tk_srea_dev(dd_adc, 0, &data[0], 1, &asz);    // 右センサ
        err = tk_srea_dev(dd_adc, 1, &data[1], 1, &asz);    // 中センサ
        err = tk_srea_dev(dd_adc, 2, &data[2], 1, &asz);    // 左センサ

        if(data[1] > ON_LINE) {
            msg = MSG_STRAIGHT;             // 直進
        } else if(data[2]>data[0]) {
            msg = MSG_LEFT;                 // 左折
        } else {
            msg = MSG_RIGHT;                // 右折
        }

        if(msg != pre_msg) {
            tk_snd_mbf(mbfid, &msg, sizeof(msg), 100);  // メッセージ送信
            pre_msg = msg;
        }
        tk_dly_tsk(100);
    }
}
