/* 
 *** TryKernel-A     イベントフラグ
*/

#include <trykernel.h>
#include <knldef.h>

FLGCB   flgcb_tbl[CPU_CORE_NUM][CNF_MAX_FLGID];     /* イベントフラグ管理ブロック(FLGCB) */

/* イベントフラグの生成API */
ID tk_cre_flg( const T_CFLG *pk_cflg )
{
    ID      flgid;
    UINT    intsts;

    DI(intsts);     // 割込み禁止
    for(flgid = 0; flgcb_tbl[CPU_CORE][flgid].state != KS_NONEXIST; flgid++);

    if(flgid < CNF_MAX_FLGID) {
        flgcb_tbl[CPU_CORE][flgid].state = KS_EXIST;
        flgcb_tbl[CPU_CORE][flgid].flgptn = pk_cflg->iflgptn;
        flgid++;
    } else {
        flgid = E_LIMIT;
    }
    EI(intsts);      // 割込み許可
    return flgid;
}

/* イベントフラグ待ちの条件チェック */
static BOOL check_flag(UINT flgptn, UINT waiptn, UINT wfmode)
{
    if(wfmode & TWF_ORW) {
        return ((flgptn & waiptn) != 0);
    } else {
        return ((flgptn &waiptn) == waiptn);
    }
}

/* イベントフラグのセットAPI */
ER tk_set_flg( ID flgid, UINT setptn )
{
    FLGCB   *flgcb;
    TCB     *tcb;
    ER      err = E_OK;
    UINT    intsts;

    if(flgid <= 0 || flgid > CNF_MAX_FLGID) return E_ID;

    DI(intsts);     // 割込み禁止
    flgcb = &flgcb_tbl[CPU_CORE][--flgid];
    if(flgcb->state == KS_EXIST) {
        flgcb->flgptn |= setptn;            // フラグのセット

        for( tcb = wait_queue[CPU_CORE]; tcb != NULL; tcb = tcb->next) {
            if((tcb->waifct == TWFCT_FLG) && (tcb->waiobj == flgid)) {
                if(check_flag(flgcb->flgptn, tcb->waiptn, tcb->wfmode)) {   // 条件成立の確認
                    tqueue_remove_entry( &wait_queue[CPU_CORE], tcb);                 // タスクをウェイトキューから外す
                    tcb->state	= TS_READY;
                    tcb->waifct	= TWFCT_NON;
                    *tcb->p_flgptn = flgcb->flgptn;
                    tqueue_add_entry( &ready_queue[CPU_CORE][tcb->itskpri], tcb);     // タスクをレディキューへつなぐ
                    scheduler();                                            // スケジューラを実行

                    if ((tcb->wfmode & TWF_BITCLR) != 0 ) {
                        if ( (flgcb->flgptn &= ~(tcb->waiptn)) == 0 ) {     // 対象フラグのクリア
                            break;
                        }
                    }
                    if ((tcb->wfmode & TWF_CLR) != 0 ) {
                        flgcb->flgptn = 0;                                  // 全フラグのクリア
                        break;
                    }
                }
            }
        }
    } else {
        err = E_NOEXS;
    }

    EI(intsts);     // 割込み許可
    return err;
}

/* イベントフラグのクリアAPI */
ER tk_clr_flg( ID flgid, UINT clrptn )
{
    FLGCB   *flgcb;
    ER      err = E_OK;
    UINT    intsts;

    if(flgid <= 0 || flgid > CNF_MAX_FLGID) return E_ID;

    DI(intsts);     // 割込み禁止
    flgcb = &flgcb_tbl[CPU_CORE][--flgid];
    if(flgcb->state == KS_EXIST) {
        flgcb->flgptn &= clrptn;        // フラグのクリア
    } else {
        err = E_NOEXS;
    }
    EI(intsts);     // 割込み許可
    return err;
}

/* イベントフラグ待ちAPI */
ER tk_wai_flg( ID flgid, UINT waiptn, UINT wfmode, UINT *p_flgptn, TMO tmout )
{
    FLGCB   *flgcb;
    ER      err = E_OK;
    UINT    intsts;

    if(flgid <= 0 || flgid > CNF_MAX_FLGID) return E_ID;

    DI(intsts);     // 割込み禁止
    flgcb = &flgcb_tbl[CPU_CORE][--flgid];
    if(flgcb->state == KS_EXIST) {
        if(check_flag(flgcb->flgptn, waiptn, wfmode)) {     // 待ち条件が成立している場合
            *p_flgptn = flgcb->flgptn;                      // 条件成立時のフラグ値を返す
            if ( (wfmode & TWF_BITCLR) != 0 ) {
                flgcb->flgptn &= ~waiptn;                   // 該当フラグのクリア
            }
            if ( (wfmode & TWF_CLR) != 0 ) {
                flgcb->flgptn = 0;                          // 全フラグのクリア
            }
        } else if(tmout == TMO_POL) {                       // 待ち条件不成立、かつ、待ち時間0の場合
            err = E_TMOUT;
        } else {                                            // 待ち条件不成立、待ち状態に移行
            tqueue_remove_top(&ready_queue[CPU_CORE][cur_task[CPU_CORE]->itskpri]);     // タスクをレディキューから外す

            /* TCBの各種情報を変更する */
            cur_task[CPU_CORE]->state     = TS_WAIT;      // タスクの状態を待ち状態に変更
            cur_task[CPU_CORE]->waifct    = TWFCT_FLG;    // 待ち要因を設定
            cur_task[CPU_CORE]->waiobj    = flgid;        // 待ちイベントフラグIDを設定
            cur_task[CPU_CORE]->waitim    = ((tmout == TMO_FEVR)? tmout: tmout + TIMER_PERIOD);    // 待ち時間を設定
            cur_task[CPU_CORE]->waiptn    = waiptn;
            cur_task[CPU_CORE]->wfmode    = wfmode;
            cur_task[CPU_CORE]->p_flgptn  = p_flgptn;
            cur_task[CPU_CORE]->waierr    = &err;

            tqueue_add_entry(&wait_queue[CPU_CORE], cur_task[CPU_CORE]);      // タスクをウェイトキューに繋ぐ
            scheduler();                                            // スケジューラを実行
        }
    } else {
        err = E_NOEXS;      // 未登録のイベントフラグ
    }

    EI(intsts);     // 割込み許可
    return err;
}
