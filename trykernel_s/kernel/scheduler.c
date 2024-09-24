/* 
 *** TryKernel-S     スケジューラ
*/
#include <trykernel.h>
#include <knldef.h>

TCB *ready_queue[CNF_MAX_TSKPRI];    // タスクのレディキュー
TCB *cur_task[CPU_CORE_NUM];         // 実行中のタスク
TCB *sche_task[CPU_CORE_NUM];        // 次に実行するタスク

UW  disp_running[CPU_CORE_NUM];      // ディスパッチャ実行中

/* タスクのスケジューリング */
void scheduler(void)
{
    TCB    *new_sch[CPU_CORE_NUM] = {NULL, NULL};
    UINT    coreno;
    UINT    i, j;

    /* ① 実行するタスクの選択*/
    /* 　実行するタスクをCPUコア数だけ選択し、new_schに登録する */
    for(i = j = 0; i < CNF_MAX_TSKPRI; i++) {
        if( ready_queue[i] != NULL) {
            new_sch[j++] = ready_queue[i];
            if(j > CPU_CORE_NUM-1) break;

            if(ready_queue[i]->next != NULL) {
                new_sch[j] = ready_queue[i]->next;
                break;
            }
        }
    }

    /* ② CPUコアへのタスクの割り当て */
    /* 　実行中のタスクのCPUコアが変わらないように調整する */
    if(new_sch[0]!=NULL) {
        if(new_sch[0] == cur_task[0]) {
            sche_task[0] = new_sch[0];
            sche_task[1] = new_sch[1];
        } else if((new_sch[0] == cur_task[1]) 
            || ((new_sch[1] != NULL) && (new_sch[1] == cur_task[0]))) {
            sche_task[0] = new_sch[1];
            sche_task[1] = new_sch[0];
        } else {
            sche_task[0] = new_sch[0];
            sche_task[1] = new_sch[1];
        }
    } else {
        sche_task[0] = sche_task[1] = NULL;
    }

    /* ③ 自CPUコアのディスパッチの呼び出し */
    coreno = CPU_CORE;
    if((sche_task[coreno] != cur_task[coreno]) && !disp_running[coreno]) {
        dispatch();         // ディスパッチャを実行
    }

    /* ④ 他CPUコアのディスパッチの呼び出し */
    coreno = coreno?0:1;
    if(sche_task[coreno] != cur_task[coreno]) {
        icc_ras_int(ICCINT_DISPATCH<<24);       // 他コアにディスパッチ要求 (CPUコア間割り込み)
    }
}

/* CPUコア間割り込みハンドラ */
void iccint_dispatch(UW data)
{
    BEGIN_CRITICAL_SECTION
    if((sche_task[CPU_CORE] != cur_task[CPU_CORE])
        && !disp_running[CPU_CORE])	dispatch();     // ディスパッチ
    END_CRITICAL_SECTION
}
