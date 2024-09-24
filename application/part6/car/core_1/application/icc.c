/*
 *** Try Kernel マイコンカー Autonomous Robotics Platform
 *          CPUコア間通信タスク
 */
#include <trykernel.h>
#include "appli.h"


/* CPUコア間通信タスクの情報 */
ID  tskid_icc;                          // タスクID番号
#define STKSZ_ICC   1024                // スタックサイズ
UW  tskstk_icc[STKSZ_ICC/sizeof(UW)];   // スタック領域
void tsk_icc(INT stacd, void *exinf);   // タスク実行関数

T_CTSK ctsk_icc = {                     // タスク生成情報
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_icc,
    .stksz      = STKSZ_ICC,
    .itskpri    = 5,
    .bufptr     = tskstk_icc,
};

/* FIFO受信バッファ */
struct {
    UW      buf[16];
    UINT    rp, wp;
} fifo_rdata;

/*  CPUコア間FIFO割り込みハンドラ */
void icc_int_hdr(UINT intno)
{
    UW      data;
    FP      fp;

    while(in_w(FIFO_ST)&FIFO_ST_VLD) {              // Inter-core FIFOにデータのある間
        data = in_w(FIFO_RD);                       // Inter-core FIFOからデータ取得

        fifo_rdata.buf[fifo_rdata.wp++] = data;
        fifo_rdata.wp &= 0x0F;
    }
    out_w(FIFO_ST,0);                       // Inter-core FIFOの状態クリア
    tk_wup_tsk(tskid_icc);
}

const T_DINT	dint = { TA_HLNG, icc_int_hdr};

/* CPUコア間通信タスクの実行関数 */
void tsk_icc(INT stacd, void *exinf)
{
    UINT    cmd;
    UB      tmp[] = "0\n";

    /* 受信データバッファ初期化 */
    fifo_rdata.rp = fifo_rdata.wp = 0;

    /* CPUコア間割込み初期化 */
    _UW dummy = in_w(FIFO_ST);                          // Inter-core FIFOの状態クリア
    (dummy);                                            // ワーニング除去

    tk_def_int(IRQ_SIOPR0 + CPU_CORE, &dint);           // IRQ_SIOPR*割り込みハンドラの登録
    EnableInt(IRQ_SIOPR0 + CPU_CORE, INTLEVEL_0);       // IRQ_SIOPR*割り込みの有効化

    while(1) {
        tk_slp_tsk(TMO_FEVR);                           // 割り込みハンドラからの起床待ち
        DisableInt(IRQ_SIOPR0 + CPU_CORE);              // 割り込み禁止
        while(fifo_rdata.wp != fifo_rdata.rp) {
            cmd = fifo_rdata.buf[fifo_rdata.rp++];              // FIFOバッファからデータを取得
            fifo_rdata.rp &= 0x0F;
            tk_snd_mbf(mbfid, &cmd, sizeof(UINT), TMO_FEVR);    // メッセージバッファに送信
        }
        EnableInt(IRQ_SIOPR0 + CPU_CORE, INTLEVEL_0);   // 割り込みの許可
    }
}