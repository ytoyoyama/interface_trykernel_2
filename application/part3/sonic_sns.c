#include <trykernel.h>

#define GPIO_TRIG               12      // TRIG信号 P12
#define GPIO_ECHO               13      // ECHO信号 P13
#define GPIO_LED                14      // LED P14

/*-------------------------------------------------------------------------*/
/* LED制御タスクの情報 */
ID  tskid_led;                          // タスクID番号
#define STKSZ_LED   1024                // スタックサイズ
UW  tskstk_led[STKSZ_LED/sizeof(UW)];   // スタック領域
void tsk_led(INT stacd, void *exinf);   // タスク実行関数

/* LED制御タスク生成情報 */
T_CTSK ctsk_led = {
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_led,
    .stksz      = STKSZ_LED,
    .itskpri    = 5,
    .bufptr     = tskstk_led,
};

/*-------------------------------------------------------------------------*/
/* センサー制御タスクの情報 */
ID  tskid_sns;                          // タスクID番号
#define STKSZ_SNS   1024                // スタックサイズ
UW  tskstk_sns[STKSZ_SNS/sizeof(UW)];   // スタック領域
void tsk_sns(INT stacd, void *exinf);   // タスク実行関数

T_CTSK ctsk_sns = {                     // センサー制御タスク生成情報
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_sns,
    .stksz      = STKSZ_SNS,
    .itskpri    = 5,
    .bufptr     = tskstk_sns,
};

/*-------------------------------------------------------------------------*/
/* CPUコア間メッセージの情報 */
ID  icmid;                              // CPUコア間メッセージ ID
#define MSG_SZ  sizeof(UINT)            // メッセージサイズ
#define MBF_SZ  ((MSG_SZ+1)*5)          // メッセージバッファサイズ
UB  icmbuf[MBF_SZ];                     // メッセージバッファ領域

T_CICM  cicm = {                        // CPUコア間メッセージ生成情報
    .icmatr = TA_ICM_TO_C0,
    .bufsz  = MBF_SZ,
    .maxmsz = MSG_SZ,
    .bufptr = icmbuf,
};

/*-------------------------------------------------------------------------*/
/* LED制御タスクの実行関数 */
void tsk_led(INT stacd, void *exinf)
{
    UINT    dis;                // 障害物の距離
    ER      err;                // エラーコード

    gpio_enable_output(GPIO_LED, 0);                // LED出力設定 初期値Low

    while(1) {
        err = icc_rcv_msg(icmid, &dis, TMO_FEVR);    // CPUコア間メッセージを受信
        if(err >= E_OK) {
            if(dis < 150) {                         // 障害物が近ければLED点灯
                out_w(GPIO_OUT_SET, 1<<GPIO_LED);
            } else {                                // 障害物が遠ければLED消灯
                out_w(GPIO_OUT_CLR, 1<<GPIO_LED);
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

UW      tim1, tim2;         // 時刻計測用グローバル変数

/* ECHO信号割込みハンドラ */
#define INTNO_ECHO_EDGE_HIGT    21      // ECHO割り込みエッジHigt
#define INTNO_ECHO_EDGE_LOW     20      // ECHO割り込みエッジLow

void echo_inthdr(UW intno)
{
    UW      val;

    for(int i = 0; i <4; i++) {
        val = in_w(INTR(i));                                // 割込み要因の取得
        if(i == 1) {
            if(val&(1<<INTNO_ECHO_EDGE_HIGT)) {             // エッジHigh検出
                tim1 = in_w(TIMER_TIMELR);                      // 開始時刻取得
            } else if (val&(1<<INTNO_ECHO_EDGE_LOW)) {      // エッジLow検出
                tim2 = in_w(TIMER_TIMELR);                      // 終了時刻取得
                tk_wup_tsk(tskid_sns);                      // タスクへの通知
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

/* 障害物に対する動作 */
static void do_action(UW dis)
{
    ER      err;

    err = icc_snd_msg(icmid, &dis, sizeof(dis), 100);       // CPUコア間メッセージの送信 <<< 変更点②
    if(err < E_OK){
        tm_putstring("ERR: icc_snd_msg");
    }
}

/* センサー制御タスクの実行関数 */
void tsk_sns(INT stacd, void *exinf)
{
    /* 割込み登録情報*/
    T_DINT  dint  = {
        .intatr = TA_HLNG,
        .inthdr = echo_inthdr,
    };

    UW      tim_val;            // 計測した時間
    UINT    dis;                // 障害物の距離
    ER      err;                // エラーコード

    /* GPIOの設定 */
    gpio_enable_output(GPIO_TRIG, 0);                       // Trig信号 出力設定 初期値Low
    gpio_enable_input(GPIO_ECHO);                           // Echo信号 入力設定

    gpio_set_intmode(GPIO_ECHO,
            INTE_MODE_EDGE_LOW | INTE_MODE_EDGE_HIGH);      // ECHO信号割込みの設定
    tk_def_int(IRQ_BANK0, &dint);                           // ECHO信号割込みハンドラの登録

    out_w(WDT_TICK, WDT_TICK_ENABLE | 12);                  // ④ Timer初期化 (1MHz)

    while(1) {
        for(int i = 0; i <3; i++) out_w(INTR(i), in_w(INTR(i)));    // 割込み要因の消去
        ClearInt(IRQ_BANK0);
        EnableInt(IRQ_BANK0, 2);                    // ECHO信号割込みを有効（優先度2）

        /* ⑥ TRIG信号の生成 */
        out_w(GPIO_OUT_SET, 1<<GPIO_TRIG);
        wait_10micro();                             // 10マイクロ秒待ち
        out_w(GPIO_OUT_CLR, 1<<GPIO_TRIG);

        err = tk_slp_tsk(1000);                     // 割込みハンドラからの起床待ち
        DisableInt(IRQ_BANK0);                      // ECHO信号割り込みを無効

        if(err >= E_OK) {
            tim_val = tim2 - tim1;
            dis = (tim_val/2) * 340 / 1000;         // ⑩ 障害物の距離を計算
            do_action(dis);
        }
        tk_dly_tsk(500);
    }
    tk_slp_tsk(TMO_FEVR);
}

/*-------------------------------------------------------------------------*/
/* CPU コア0のユーザ・メイン関数 */
int usermain_c0(void)
{
    icmid = icc_cre_msg(&cicm);     // CPUコア間メッセージの生成
    icc_sync_core(CPU_CORE);        // CPUコア間の同期

    /* LED制御タスクの生成・実行 */
    tskid_led = tk_cre_tsk(&ctsk_led);
    tk_sta_tsk(tskid_led, 0);

    tk_slp_tsk(TMO_FEVR);       // 初期タスクを待ち状態に
    return 0;
}

/* CPU コア1のユーザ・メイン関数 */
int usermain_c1(void)
{
    icc_sync_core(CPU_CORE);        /* CPUコア間の同期 */

    /* センサー制御タスクの生成・実行 */
    tskid_sns = tk_cre_tsk(&ctsk_sns);
    tk_sta_tsk(tskid_sns, 0);

	tk_slp_tsk(TMO_FEVR);       // 初期タスクを待ち状態に
    return 0;
}