/* 
 *** TryKernel-S     タスク付属同期機能
*/

#include <trykernel.h>
#include <knldef.h>

/* タスクの実行遅延 API */
ER tk_dly_tsk( RELTIM dlytim )
{
    TCB     *tcb;
    ER      err = E_OK;

    BEGIN_CRITICAL_SECTION
    if(dlytim > 0) {
        tcb = cur_task[CPU_CORE];
        tqueue_remove_entry(&ready_queue[tcb->itskpri], tcb);   // タスクをレディキューから外す

        /* TCBの各種情報を変更する */
        tcb->state   = TS_WAIT;                     // タスクの状態を待ち状態に変更
        tcb->waifct  = TWFCT_DLY;                   // 待ち要因を設定
        tcb->waitim  = dlytim + TIMER_PERIOD;       // 待ち時間を設定
        tcb->waierr = &err;                         // 待ち解除時のエラーコード

        tqueue_add_entry(&wait_queue, tcb);         // タスクをウェイトキューに繋ぐ
        scheduler();                                // スケジューラの実行
    }
    END_CRITICAL_SECTION
    return err;
}

/* タスク起床待ちAPI */
ER tk_slp_tsk( TMO tmout )
{
    TCB     *tcb;
    ER      err = E_OK;

    BEGIN_CRITICAL_SECTION
    if ( cur_task[CPU_CORE]->wupcnt > 0 ) {    // 起床要求有り
        cur_task[CPU_CORE]->wupcnt--;
	} else {                        // 起床要求無し
        tcb = cur_task[CPU_CORE];
        tqueue_remove_entry(&ready_queue[tcb->itskpri], tcb);    // タスクをレディキューから外す

        /* TCBの各種情報を変更する */
        tcb->state   = TS_WAIT;                             // タスクの状態を待ち状態に変更
        tcb->waifct  = TWFCT_SLP;                           // 待ち要因を設定
        tcb->waitim  = (tmout==TMO_FEVR)?tmout:(tmout+TIMER_PERIOD);   // 待ち時間を設定
        tcb->waierr = &err;

        tqueue_add_entry(&wait_queue, tcb);         // タスクをウェイトキューに繋ぐ
        scheduler();                                // スケジューラの実行
    }
    END_CRITICAL_SECTION
    return err;
}

/* タスクの起床 API */
ER tk_wup_tsk( ID tskid )
{
    TCB	    *tcb;
    ER      err = E_OK;

    if(tskid <= 0 || tskid > CNF_MAX_TSKID) return E_ID;    /* ID番号チェック */

    BEGIN_CRITICAL_SECTION
    tcb = &tcb_tbl[tskid-1];
    if((tcb->state == TS_WAIT) && (tcb->waifct == TWFCT_SLP)) { // tk_slp_tskで待ち状態か？

        tqueue_remove_entry(&wait_queue, tcb);                // タスクをウェイトキューから外す

        /* TCBの各種情報を変更する */
        tcb->state	= TS_READY;
        tcb->waifct	= TWFCT_NON;

        tqueue_add_entry(&ready_queue[tcb->itskpri], tcb);      // タスクをレディキューに繋ぐ
        scheduler();                                            // スケジューラの実行
    } else if(tcb->state == TS_READY || tcb->state == TS_WAIT) {    // 実行できる状態の場合
        tcb->wupcnt++;      // 起床要求数を増やす
    } else {
        err = E_OBJ;       // 起床できるタスクの状態ではない
    }

    END_CRITICAL_SECTION
    return err;
}
