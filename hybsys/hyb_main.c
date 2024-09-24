#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
              
void main_c1(void);                 // CPUコア1 メイン関数
#define STKSZ   (1024/sizeof(int))      // CPUコア1 初期スタックサイズ
extern int      knl_stack[STKSZ];       // CPUコア1 初期スタック
extern void     (*knl_vec_tbl[])();     // CPUコア1 例外ベクタテーブル

void main_c0(void);                 // CPUkコア0 メイン関数

int main()
{
    stdio_init_all();               // Pico SDK 標準入出力初期化

    /* CPUコア1 実行開始*/
    multicore_launch_core1_raw(main_c1, 
            (uint32_t*)&knl_stack[STKSZ], (uint32_t)knl_vec_tbl);

    /* CPUコア0 アプリケーション実行開始 */
    main_c0();      

    return 0;
}
