/*
 *** Try Kernel マイコンカー Autonomous Robotics Platform
 *          障害物センサー制御タスク
 */
#include <trykernel.h>
#include "appli.h"

#define GPIO_ECHO               15      // ECHO信号 P15
#define GPIO_TRIG               14      // TRIG信号 P14

#define INTNO_ECHO_EDGE_HIGT    31      // ECHO割り込みエッジHigt
#define INTNO_ECHO_EDGE_LOW     30      // ECHO割り込みエッジLow

#define TH_DISTANCE                150     // 障害物の距離 15cm

/*-------------------------------------------------------------------------*/
/* 障害物センサータスクの情報 */
ID  tskid_dsns;                          // タスクID番号
#define STKSZ_DSNS   1024                // スタックサイズ
UW  tskstk_dsns[STKSZ_DSNS/sizeof(UW)];  // スタック領域
void tsk_dsns(INT stacd, void *exinf);   // タスク実行関数

T_CTSK ctsk_dsns = {                     // センサー制御タスク生成情報
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_dsns,
    .stksz      = STKSZ_DSNS,
    .itskpri    = 6,
    .bufptr     = tskstk_dsns,
};

/*-------------------------------------------------------------------------*/
/* ECHO信号割込みハンドラの情報 */
void echo_inthdr(UW intno);             // 割り込みハンドラ実行関数

T_DINT  dint  = {                       // ECHO信号割込み登録情報
    .intatr = TA_HLNG,
    .inthdr = echo_inthdr,
};

/*-------------------------------------------------------------------------*/
UW      tim1, tim2;         // 時刻計測用グローバル変数

/* ECHO信号割込みハンドラ */
void echo_inthdr(UW intno)
{
    UW      val;

    for(int i = 0; i <4; i++) {
        val = in_w(INTR(i));                                // 割込み要因の取得
        if(i == 1) {
            if(val&(1<<INTNO_ECHO_EDGE_HIGT)) {             // エッジHigh検出
                tim1 = in_w(TIMER_TIMELR);                      // 開始時刻取得
            } else if (val&(1<<INTNO_ECHO_EDGE_LOW)) {      // エッジLow検出
                tim2 = in_w(TIMER_TIMELR);                      // 開始時刻取得
                tk_wup_tsk(tskid_dsns);                         // タスクへの通知
            }
        }
        out_w(INTR(i), val);                                // 割込み要因のクリア
    }
}

/* 10マイクロ秒待ち */
static void wait_10micro(void)
{
    UW      t0, t;
    t0 = in_w(TIMER_TIMELR);
    do {
        t = in_w(TIMER_TIMELR);
    } while(t-t0 <= 10);
}

/* 障害物センサータスクの実行関数 */
void tsk_dsns(INT stacd, void *exinf)
{
    /* 割込み登録情報*/
    T_DINT  dint  = {
        .intatr = TA_HLNG,
        .inthdr = echo_inthdr,
    };

    UINT    msg, pre_msg;
    UINT    dis;                // 障害物の距離
    ER      err;                // エラーコード

    /* ① GPIOの設定 */
    gpio_enable_output(GPIO_TRIG, 0);                       // Trig信号 出力設定 初期値Low
    gpio_enable_input(GPIO_ECHO);                           // Echo信号 入力設定

    gpio_set_intmode(GPIO_ECHO,
            INTE_MODE_EDGE_LOW | INTE_MODE_EDGE_HIGH);      // ECHO信号割込みの設定
    tk_def_int(IRQ_BANK0, &dint);                           // ECHO信号割込みハンドラの登録

    out_w(WDT_TICK, WDT_TICK_ENABLE | 12);                  // Timer初期化 (1MHz)
    pre_msg = MSG_STOP;

    while(1) {
        for(int i = 0; i <3; i++) out_w(INTR(i), in_w(INTR(i)));    // 割込み要因の消去
        ClearInt(IRQ_BANK0);
        EnableInt(IRQ_BANK0, 0);                    // ECHO信号割込みを有効（優先度2）

        /* ⑥ TRIG信号の生成 */
        out_w(GPIO_OUT_SET, 1<<GPIO_TRIG);
        wait_10micro();                             // 10マイクロ秒待ち
        out_w(GPIO_OUT_CLR, 1<<GPIO_TRIG);

        err = tk_slp_tsk(1000);                     // 割込みハンドラからの起床待ち
        DisableInt(IRQ_BANK0);                      // ECHO信号割り込みを無効

        if(err >= E_OK) {
            dis = ((tim2 - tim1)/2) * 340 / 1000;
        } else {
            dis = 0xFFFFFFFF;
        }

        msg = (dis < TH_DISTANCE)? MSG_STOP: MSG_RUN;   // 距離が近ければ停止
        if(msg != pre_msg) {
            tk_snd_mbf(mbfid, &msg, sizeof(msg), 100);  // メッセージ送信
            pre_msg = msg;
        }
        tk_dly_tsk(100);
    }
    tk_slp_tsk(TMO_FEVR);
}
