/* 
 *** TryKernel-A     CPUコア間FIFO
*/

#include <trykernel.h>
#include <knldef.h>

/* CPUコア1起床関数    Core-0 からCore-1 を起床         */
/*  引数    UW *vtbl    例外ベクターテーブルのアドレス   */
/*          UW *sp      スタックポインタの値            */
/*          FP ent      実行開始アドレス                */
void icc_wup_core1(UW *vtbl, UW *sp, FP ent)
{
    UW      cmd[] = {0, 0, 1, (UW)vtbl, (UW)sp, (UW)ent};
    UW      res;
    UINT    seq = 0;

    do {
        if(!cmd[seq]) {
            while(in_w(FIFO_ST)&FIFO_ST_VLD) in_w(FIFO_RD); // FIFOを空にする
            __asm__ volatile("sev");    // SEV命令発行
        }

        while((in_w(FIFO_ST)&FIFO_ST_RDY) == 0);    // FIFOが送信可能になるまで待つ
        out_w(FIFO_WR, cmd[seq]);                   // FIFOにデータを書き込む
        __asm__ volatile("sev");                    // SEV命令発行

        while((in_w(FIFO_ST)&FIFO_ST_VLD) == 0) {   // FIFOのデータを待つ
             __asm__ volatile("wfe");               // WFE命令実行
        }
        res = in_w(FIFO_RD);

        if(cmd[seq] == res) seq++;                  // Core-1からの応答の確認
        else seq = 0;                               // エラー 最初からやり直し
    } while(seq < sizeof(cmd)/sizeof(UW));
}

/* CPUコア間の同期処理 */
void icc_sync_core(UINT coreno)
{
    if(coreno) {
        icc_unl_spin(SPINLOCK_SYNC_C0);             // 他CPUコアのスピンロック解除
        icc_loc_spin(SPINLOCK_SYNC_C1);             // 自CPUコアのスピンロック解除
    } else {
        icc_unl_spin(SPINLOCK_SYNC_C1);             // 他CPUコアのスピンロック解除
        icc_loc_spin(SPINLOCK_SYNC_C0);             // 自CPUコアのスピンロック解除
    }
}