/* 
 *** TryKernel-A     割込み管理
*/

#include <trykernel.h>
#include <knldef.h>

/* 例外ベクタ・テーブル */
void (*vec_tbl_c0[N_SYSVEC + N_INTVEC])() __attribute__((section(".vector_c0")));
void (*vec_tbl_c1[N_SYSVEC + N_INTVEC])() __attribute__((section(".vector_c1")));

/* 割込みハンドラ・テーブル(TA_HLNG属性のみ) */
static FP inthdr_tbl[CPU_CORE_NUM][N_INTVEC];

/* タスク独立部の実行変数 */
_UW knl_taskindp[CPU_CORE_NUM];

/* IPSRレジスタのリード */
static inline UW get_ipsr(void)
{
    UW	ipsr;
    __asm__ volatile("mrs %0, ipsr": "=r"(ipsr));
    return ipsr;
}

/* 低水準割り込みハンドラ*/
void hll_inthdr(void)
{
    FP  inthdr;
    UW  intno;

    intno   = get_ipsr() - N_SYSVEC;        // 割込み番号の取得
    inthdr  = inthdr_tbl[CPU_CORE][intno];  // 割込みハンドラの取得

    knl_taskindp[CPU_CORE]++;
    (*inthdr)(intno);                       // 割込みハンドラの呼び出し
    knl_taskindp[CPU_CORE]--;
}

/* 割込みハンドラの登録API */
ER tk_def_int( UINT intno, const T_DINT *pk_dint )
{
    volatile FP	*intvet;
    FP          inthdr;

    if( intno >= N_INTVEC) return E_ID;

    inthdr = pk_dint->inthdr;
    if(inthdr != NULL) {
        if ( (pk_dint->intatr & TA_HLNG) != 0 ) {   // TA_HLNG属性のハンドラ処理
            inthdr_tbl[CPU_CORE][intno] = inthdr;   // TA_HLNG属性の割り込みハンドラの登録
            inthdr = hll_inthdr;
        }		
    } else 	{
        inthdr = Default_Handler;                   // 割込みハンドラの登録解除
    }

    // 割込みハンドラの登録
    intvet = (CPU_CORE)?(FP*)&vec_tbl_c1[N_SYSVEC]:(FP*)&vec_tbl_c0[N_SYSVEC];
    intvet[intno] = inthdr;

    return E_OK;
}

/* 割込みコントローラ制御 API */
/* 割込みの有効化 */
void EnableInt( UINT intno, INT level )
{
    UINT	imask;
    
    DI(imask);
    *(_UB*)(NVIC_IPR(intno)) = (UB)(level<<(8-INTPRI_BITWIDTH));    // 割込み優先度の設定
    *(_UW*)(NVIC_ISER(intno)) = (0x01U << (intno % 32));            // 割り込みの有効可
    EI(imask);
}

/* 割込みの無効化 */
void DisableInt( UINT intno )
{
	*(_UW*)(NVIC_ICER(intno)) = (0x01U << (intno % 32));
}

/* 割込みのクリア */
void ClearInt( UINT intno )
{
	*(_UW*)(NVIC_ICPR(intno)) = (0x01U << (intno % 32));
}