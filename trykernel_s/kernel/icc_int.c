/* 
 *** TryKernel-S     CPUコア間割り込み
*/

#include <trykernel.h>
#include <knldef.h>

FP  icc_int_tbl[CPU_CORE_NUM][16];          //  CPUコア間割り込みハンドラ・テーブル

/*  CPUコア間割り込みハンドラ */
void icc_int_hdr(UINT intno)
{
    UW      data;
    FP      fp;

    while(in_w(FIFO_ST)&FIFO_ST_VLD) {              // Inter-core FIFOにデータのある間
        data = in_w(FIFO_RD);                       // Inter-core FIFOからデータ取得
        fp = icc_int_tbl[CPU_CORE][data>>24];       // CPUコア間割り込みハンドラのアドレス取得
        if(fp) {
            (*fp)(data);                            // CPUコア間割り込みハンドラの実行
        }
    }
    out_w(FIFO_ST,0);                       // Inter-core FIFOの状態クリア
}

/* CPUコア間割り込みハンドラの登録関数 */
ER icc_def_int(UINT intno, FP inthdr)
{
    UINT    intsts;

    if(intno >= 16) return E_ID;            // パラメータチェック

    DI(intsts);                             // 割り込み禁止
    icc_int_tbl[CPU_CORE][intno] = inthdr;  // CPUコア間割り込みハンドラを登録
    EI(intsts);                             // 割り込み許可
    return E_OK;
}

/* CPUコア間割り込み生成関数 */
ER icc_ras_int(UW code)
{
    UINT    intsts;
    ER      err = E_OK;

    DI(intsts);                         // 割り込み禁止
    if(in_w(FIFO_ST)&FIFO_ST_RDY) {     // Inter-core FIFOが書込み可能か？
        out_w(FIFO_WR, code);           // Inter-core FIFOにデータを書込む
    } else {
        err = E_QOVR;                   // 書込み不可ならエラー
    }
    EI(intsts);                         // 割り込み許可

    return err;
}

/* CPUコア間割込み初期化 */
void init_icc_int(void)
{
    T_DINT	dint = { TA_HLNG, icc_int_hdr};
    UINT	imask, intno;
    _UW     dummy;

    dummy = in_w(FIFO_ST);                          // Inter-core FIFOの状態クリア
    (dummy);                                        // ワーニング除去

    tk_def_int(CPU_CORE? IRQ_SIOPR1+100: IRQ_SIOPR0, &dint);      // IRQ_SIOPR*割り込みハンドラの登録

    DI(imask);
    intno = CPU_CORE? IRQ_SIOPR1: IRQ_SIOPR0;
    *(_UB*)(NVIC_IPR(intno)) = (UB)(INTLEVEL_0<<(8-INTPRI_BITWIDTH));   // 割込み優先度の設定
    *(_UW*)(NVIC_ISER(intno)) = (0x01U << (intno % 32));                // 割り込みの有効可
    EI(imask);
}

