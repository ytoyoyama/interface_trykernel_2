/* 
 *** TryKernel-A     ディスパッチャ
*/
    .align 2
    .thumb_func
    .globl dispatch_entry

dispatch_entry:
    mov		r0, #1
    msr		primask, r0         // 割込み禁止

    ldr     r1, =0xD0000000
    ldr     r3,[r1]
    cmp     r3, #0
    beq     disp_005
    mov     r3, #4

disp_005:
    ldr		r1, =disp_running
    add     r1, r3
    str		r0, [r1]

    /* ① 実行中の関数の実行コンテキスト情報をスタックに退避 */
    push    {r4-r7}
    mov     r4, r8
    mov     r5, r9
    mov     r6, r10
    mov     r7, r11
    push    {r4-r7}

    /* ② 現在実行中のタスクの確認 */
    ldr     r0, =cur_task
    add     r0, r3
    ldr     r1, [r0]
    cmp     r1, #0
    beq     disp_010    // 実行中のタスク無し(cur_task[CPU_CORE] == NULL) ならば disp_010へ

    /* ③ 実行コンテキスト情報へのポインタを実行中のタスクのTCBに格納 */
    mov     r2, sp
    str     r2, [r1]

disp_010:
    /* ④ 次に実行するタスクの確認 */
    ldr     r1, =sche_task
    add     r1, r3
    ldr     r2, [r1]
    cmp     r2, #0
    bne     disp_030

    /* ⑤ 次に実行するタスクが無かった場合の処理 */
    str     r2, [r0]
disp_020:
    mov		r5, #0
    msr		primask, r5     // 割込み許可
    mov		r5, #1
    msr		primask, r5     // 割込み禁止

    ldr     r2, [r1]
    cmp     r2, #0
    beq     disp_020

    /* ⑥ 実行するタスクの切り替え */
disp_030:
    str     r2, [r0]
    ldr     r0, [r2]
    mov     sp, r0

    /* ⑦ スタック上のコンテキス情報の復元 */
    pop     {r4-r7}
    mov     r11, r7
    mov     r10, r6
    mov     r9, r5
    mov     r8, r4
    pop     {r4-r7}

    /* 割込み許可 */
    ldr     r0, =disp_running
    add     r0, r3
    mov     r1, #0
    str     r1, [r0]
    msr     primask, r1     // 割込み許可

    bx	lr
