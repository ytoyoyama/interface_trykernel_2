﻿
#ifndef KNLDEF_H
#define KNLDEF_H
/*
 *** TryKernel-S     カーネル内部共通定義
 */

/* タスク状態 */
typedef enum {
    TS_NONEXIST = 0,    // 未登録
    TS_READY    = 1,    // 実行状態 or 実行可能状態
    TS_WAIT     = 2,    // 待ち状態
    TS_DORMANT  = 8     // 休止状態
} TSTAT;

/* タスクの待ち要因 */
typedef enum {
    TWFCT_NON   = 0,    // 無し
    TWFCT_DLY   = 1,    // tk_dly_tskによる時間待ち
    TWFCT_SLP   = 2,    // tk_slp_tskによる起床待ち
    TWFCT_FLG   = 3,    // tk_wai_flgによるフラグ待ち
    TWFCT_SEM   = 4,    // tk_wai_semによる資源待ち
    TWFCT_MBFS  = 5,    // tk_snd_mbfによる送信待ち
    TWFCT_MBFR  = 6,    // tk_rcv_mbfによる受信待ち
} TWFCT;

/* TCB(Task Control Block)定義 */
typedef struct st_tcb {
    void    *context;           // コンテキスト情報へのポインタ

    /* タスクキュー用ポインタ */
    struct st_tcb   *pre;       // 一つ前の要素
    struct st_tcb   *next;      // 一つ後の要素

    /* タスク情報 */
    TSTAT   state;              // タスク状態
    FP      tskadr;             // 実行開始アドレス
    PRI     itskpri;            // 実行優先度
    void    *stkadr;            // スタックのアドレス
    SZ      stksz;              // スタックのサイズ
    INT     wupcnt;             // 起床要求数

    /* 時間待ち情報 */
    TWFCT   waifct;             // 待ち要因
    ID      waiobj;             // 待ち対象オブジェクト
    RELTIM  waitim;             // 待ち時間
    ER      *waierr;            // 待ち解除のエラーコード

    /* イベントフラグ待ち情報 */
    UINT    waiptn;             // 待ちフラグパターン
    UINT    wfmode;             // 待ちモード
    UINT    *p_flgptn;          // 待ち解除時のフラグパターン

    /* セマフォ待ち情報 */
    INT     waisem;             // セマフォ資源要求数

    /* メッセージバッファ待ち情報 */
    INT             msgsz;      // メッセージのサイズ
    const void      *msg;       // メッセージを格納する領域
} TCB;

extern TCB	tcb_tbl[CNF_MAX_TSKID];         // TCBテーブル
extern TCB	*ready_queue[CNF_MAX_TSKPRI];   // タスクの実行待ち行列（優先度毎）
extern TCB	*cur_task[CPU_CORE_NUM];        // 実行中のタスク
extern TCB	*sche_task[CPU_CORE_NUM];       // 次に実行するタスク
extern TCB  *wait_queue;                    // タスクの時間待ち行列

/* グローバル関数 */
extern void Reset_Handler_c0(void);     // CPUコア0 リセットハンドラ
extern void Reset_Handler_c1(void);     // CPUコア1 リセットハンドラ
extern void dispatch_entry(void);       // ディスパッチャ
extern void systimer_handler(void);     // システムタイマ割込みハンドラ

/* ディスパッチャの呼出し */
#define SCB_ICSR        0xE000ED04      // 割込み制御ステートレジスタのアドレス
#define ICSR_PENDSVSET  (1<<28)         // PendSV set-pending ビット
static inline void dispatch( void )
{
    out_w(SCB_ICSR, ICSR_PENDSVSET);    // PendSV例外を発生
}

extern void scheduler(void);            // スケジューラ

extern void *make_context( UW *sp, UINT ssize, void (*fp)());   // タスクコンテキストの作成

/* タスクの待ち行列操作関数 */
extern void tqueue_add_entry(TCB **queue, TCB *tcb);        // エントリ追加関数
extern void tqueue_remove_top(TCB **queue);                 // 先頭エントリ削除関数
extern void tqueue_remove_entry(TCB **queue, TCB *tcb);     // エントリ削除関数

/* カーネルオブジェクト状態 */
typedef enum {
    KS_NONEXIST = 0,    // 未登録
    KS_EXIST    = 1     // 登録済み
} KSSTAT;

/* イベントフラグ管理情報(FLGCB) */
typedef struct st_flgcb {
    KSSTAT  state;      // イベントフラグ状態
    UINT    flgptn;     // イベントフラグ値
} FLGCB;

/* セマフォ管理情報(SEMCB) */
typedef struct semaphore_control_block {
    KSSTAT  state;      // セマフォ状態
    INT     semcnt;     // セマフォ値
    INT     maxsem;     // セマフォ最大値
} SEMCB;

/* メッセージバッファ管理情報(MBFCB) */
typedef struct st_mbfcb {
    KSSTAT  state;          // メッセージバッファ状態
    SZ      bufsz;          // メッセージバッファのサイズ
    INT     maxmsz;         // メッセージの最大サイズ
    void    *bufptr;        // メッセージバッファ領域のアドレス

    SZ      freesz;         // バッファ・サイズ
    void    *buf_rp;        // 読み込み位置
    void    *buf_wp;        // 書き込み位置
} MBFCB;

/* 例外・割込み関連 */
extern void (* const vector_tbl[])();   // 例外・割込みベクタテーブル
extern void (*vec_tbl_c0[])();          // 例外・割込みベクタテーブル(Core0/RAM)
extern void (*vec_tbl_c1[])();          // 例外・割込みベクタテーブル(Core1/RAM)

extern void Default_Handler(void);      // デフォルトハンドラ

/* マルチコア制御 */
/* CPUコア1の起床 */
extern void icc_wup_core1(UW *vtbl, UW *sp, FP ent);

/* CPUコア間スピンロック制御関数 */
extern void icc_ini_spin(void);         // CPUコア間スピンロックの初期化
extern void icc_loc_spin(UINT no);      // CPUコア間スピンロックのロック
extern void icc_unl_spin(UINT no);      // CPUコア間スピンロックのアンロック

#define SPINLOCK_SYNC_C0    0           // CPUコア同期用スピンロック(Core0)
#define SPINLOCK_SYNC_C1    1           // CPUコア同期用スピンロック(Core1)
#define SPINLOCK_DEBUG      2           // デバッグ用シリアル通信用スピンロック
#define SPINLOCK_KERNEL     3           // OSクリティカルセクション用スピンロック

/* CPUコア間割込み制御関数 */
extern void init_icc_int(void);         // CPUコア間割込み初期化
ER icc_def_int(UINT intno, FP inthdr);  // CPUコア間割り込みハンドラの登録
ER icc_ras_int(UW code);                // CPUコア間割込み生成関数

#define ICCINT_DISPATCH     1           // CPUコア間のディスパッチ要求
extern void iccint_dispatch(UW data);
#define ICCINT_ENAINT       2           // CPUコア間のEnableInt
extern void iccint_ebableint(UW data);
#define ICCINT_DISINT       3           // CPUコア間のDisableInt
extern void iccint_disableint(UW data);

/* APIクリティカルセクション */
#define BEGIN_CRITICAL_SECTION  UINT intsts=get_primask();set_primask(1); icc_loc_spin(SPINLOCK_KERNEL);
#define END_CRITICAL_SECTION    icc_unl_spin(SPINLOCK_KERNEL); set_primask(intsts);

/* OSメイン関数 */
extern int main_c0(void);
extern int main_c1(void);

/* ユーザメイン関数 */
extern int usermain(void);

#endif  /* KNLDEF_H */