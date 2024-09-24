/* 
 *** TryKernel-A     CPUコア間スピンロック
*/

#include <trykernel.h>
#include <knldef.h>

static inline void memory_barrier(void) { __asm__ volatile("dmb");}

/* CPUコア間スピンロックのロック */
void icc_loc_spin(UINT no)
{
    UW  data;

    do{
        data = in_w(SPINLOCK(no));      // Hardware SpiLock変数の値の取得
    } while(!data);                     // 変数の値が1になるまでビジーループ
    memory_barrier();                   // メモリバリア
}

/* CPUコア間スピンロックのアンロック */
void icc_unl_spin(UINT no)
{
    memory_barrier();                   // メモリバリア
    out_w(SPINLOCK(no), 0);             // Hardware SpiLock変数に値を書き込む
}

/* CPUコア間スピンロックの初期化 */
void icc_ini_spin(void)
{
    UINT    no;

    for(no = 0; no < 32; no++) {        // すべてのコア間スピンロックをアンロック
        icc_unl_spin(no);
    }
}
