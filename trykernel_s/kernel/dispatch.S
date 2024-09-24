/* 
 *** Try Kernel-S       ディスパッチャ
*/
    .align 2
    .thumb_func
    .globl dispatch_entry

#define SIO_CPUID               (0xD0000000)
#define SPINLOCK_KERNEL         (0xD000010C)
#define FIFO_ST                 (0xD0000000+0x50)
#define FIFO_WR                 (0xD0000000+0x54)
#define FIFO_ST_RDY             (1<<1)
#define SRAM4_END               (0x2003F000)
#define SRAM5_END               (0x20040000)

dispatch_entry:
    mov		r0, #1
    msr		primask, r0         // 割込み禁止
    
    /* SpinLock ロック */
    ldr     r0, =SPINLOCK_KERNEL
disp_005:
    ldr     r1, [r0]
    cmp     r1, #0
    beq     disp_005
    dmb

    /* 実行中の関数の実行コンテキスト情報をスタックに退避 */
    push    {r4-r7}
    mov     r0, r8
    mov     r1, r9
    mov     r2, r10
    mov     r3, r11
    push    {r0-r3}

    ldr     r0, =SIO_CPUID
    ldr     r3, [r0]
    lsl     r3, #2                  // R3 = CPU_ID * 4

    mov		r0, #1
    ldr		r1, =disp_running       // disp_running[CPU_CORE] = 1
    str		r0, [r1, r3]

    /* 現在実行中のタスクの確認 */
    ldr     r0, =cur_task           // R0 = CUR_TASK
    ldr     r1, [r0, r3]
    cmp     r1, #0
    beq     disp_010    // 実行中のタスク無し(cur_task[CPU_CORE] == NULL) ならば disp_010へ

    /* 実行コンテキスト情報へのポインタを実行中のタスクのTCBに格納 */
    mov     r2, sp
    str     r2, [r1]

disp_010:
    /* 次に実行するタスクの確認 */
    ldr     r1, =sche_task      // R1 = SCHE_TASK
    ldr     r2, [r1, r3]
    cmp     r2, #0
    bne     disp_030

    /* 次に実行するタスクが無かった場合の処理 */
    str     r2, [r0, r3]        // CUR_TSK = NULL
disp_020:
    cmp     r3, #0
    beq     disp_022
    ldr     r2, =SRAM5_END
    b       disp_024
disp_022:
    ldr     r2, =SRAM4_END
disp_024:
    mov     sp, r2              // 専用スタックに変更

disp_026:
    mov	    r2, #0
    dmb                         // スピンロック解除
    ldr     r4, =SPINLOCK_KERNEL
    str     r2, [r4]
    msr		primask, r2         // 割込み許可

    mov		r2, #1
    msr		primask, r2         // 割込み禁止
disp_028:
    ldr     r2, [r4]            // スピンロック取得
    cmp     r2, #0
    beq     disp_028
    dmb

    ldr     r2, [r1, r3]
    cmp     r2, #0
    beq     disp_026

    /* 実行するタスクの切り替え */
disp_030:
    str     r2, [r0, r3]
    ldr     r0, [r2]
    mov     sp, r0

    mov		r0, #0
    ldr		r1, =disp_running   // disp_running[CPU_CORE] = 0
    str		r0, [r1, r3]

    /* スタック上のコンテキス情報の復元 */
    pop     {r0-r3}
    mov     r11, r3
    mov     r10, r2
    mov     r9, r1
    mov     r8, r0
    pop     {r4-r7}

    /* SpinLock アンロック */
    dmb
    ldr     r0, =SPINLOCK_KERNEL
    mov     r1, #0
    str     r1, [r0]

    msr     primask, r1     // 割込み許可

    bx	lr
