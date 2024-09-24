/* 
 *** TryKernel-A     スケジューラ
*/
#include <trykernel.h>
#include <knldef.h>

TCB *ready_queue[CPU_CORE_NUM][CNF_MAX_TSKPRI];    // タスクのレディキュー
TCB *cur_task[CPU_CORE_NUM];                       // 実行中のタスク
TCB *sche_task[CPU_CORE_NUM];                      // 次に実行するタスク

UW  disp_running[CPU_CORE_NUM];                    // ディスパッチャ実行中

/* タスクのスケジューリング */
void scheduler(void)
{
    INT		i;

    for(i = 0; i < CNF_MAX_TSKPRI; i++) {
        if( ready_queue[CPU_CORE][i] != NULL) break;
    }

    if(i < CNF_MAX_TSKPRI) {
        sche_task[CPU_CORE] = ready_queue[CPU_CORE][i];
    } else {
        sche_task[CPU_CORE] = NULL;    // 実行できるタスクは無い
    }
    if(sche_task[CPU_CORE] != cur_task[CPU_CORE] && !disp_running[CPU_CORE]) {
        dispatch();         // ディスパッチャを実行
    }
}