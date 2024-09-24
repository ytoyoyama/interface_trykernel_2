#include <trykernel.h>
#include <knldef.h>

extern void (* const vector_tbl[])();

#define FIFO_ST                 (0xD0000050)
#define FIFO_ST_RDY             (1<<1)
#define FIFO_ST_VLD             (1<<0)

#define FIFO_WR                 (SIO_BASE+0x54)
#define FIFO_RD                 (SIO_BASE+0x58)

/* コア1起床関数    Core-0 からCore-1 を起床            */
/*  引数    UW *vtbl    例外ベクターテーブルのアドレス   */
/*          UW *sp      スタックポインタの値            */
/*          FP ent      実行開始アドレス                */
void mcore_wup_core1(UW *vtbl, UW *sp, FP ent)
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

void func_core1(void)
{
    UINT    cnt;

    /* ① LEDのポートの初期化 */
    /* GP15端子出力無効 */
    out_w(GPIO_OE_CLR, (1<<15));
   /* GP15端子出力クリア */
    out_w(GPIO_OUT_CLR, (1<<15));
    /* GP15端子 SIO */
    out_w(GPIO_CTRL(15), 5);
    /* GP15出力有効 */
    out_w(GPIO_OE_SET, (1<<15));

    /* ② タイマーの初期化 */
    out_w(SYST_CSR, SYST_CSR_CLKSOURCE);
    out_w(SYST_RVR, (TIMER_PERIOD*TMCLK_KHz)-1);
    out_w(SYST_CVR, (TIMER_PERIOD*TMCLK_KHz)-1);
    out_w(SYST_CSR, SYST_CSR_CLKSOURCE | SYST_CSR_ENABLE);

    /* ③ LEDの点滅 */
    while(1) {
        out_w(GPIO_OUT_XOR, 1<<15);

        cnt = 500/TIMER_PERIOD;
        while(cnt) {
            if((in_w(SYST_CSR) & SYST_CSR_COUNTFLAG) != 0) cnt--;
        }
    }
}

int usermain(void)
{
    /* ① LEDのポートの初期化 */
    out_w(GPIO_OE_CLR, (1<<14));    /* GP14端子出力無効 */
    out_w(GPIO_OUT_CLR, (1<<14));   /* GP14端子出力クリア */
    out_w(GPIO_CTRL(14), 5);        /* GP14端子 SIO */
    out_w(GPIO_OE_SET, (1<<14));    /* GP14出力有効 */

    /* ② CORE1を起床 */
    mcore_wup_core1((UW*)vector_tbl, (UW*)0x20042000, (FP)func_core1);

    /* ③ LEDの点滅 */
    while(1) {
        out_w(GPIO_OUT_XOR, 1<<14);
        tk_dly_tsk(500);
    }

    return 0;
}
