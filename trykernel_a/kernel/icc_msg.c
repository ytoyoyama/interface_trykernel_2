/* 
 *** TryKernel-A     CPUコア間メッセージ
*/

#include <trykernel.h>
#include <knldef.h>

/* CPUコア間メッセージ管理ブロック(ICMCB) */
ICMCB	icmcb_tbl[CNF_MAX_ICMID];

/* CPUコア間メッセージの生成API */
ID icc_cre_msg( const T_CICM *pk_cicm)
{
    ID      icmid;
    UINT    intsts;

    /* パラメータチェック */
    if((pk_cicm->bufsz <= 0) || (pk_cicm->maxmsz <= 0) || (pk_cicm->maxmsz >= 256)
            || (pk_cicm->bufsz < pk_cicm->maxmsz + sizeof(UB)) || (pk_cicm->bufptr == NULL)) {
        return E_PAR;
    }

    DI(intsts); icc_loc_spin(SPINLOCK_ICM);    // クリティカルセクション開始
    for(icmid = 0; icmcb_tbl[icmid].state != KS_NONEXIST; icmid++);

    if(icmid < CNF_MAX_FLGID) {
        icmcb_tbl[icmid].state  = KS_EXIST;
        icmcb_tbl[icmid].icmatr = pk_cicm->icmatr;
        icmcb_tbl[icmid].bufsz  = pk_cicm->bufsz;
        icmcb_tbl[icmid].maxmsz = pk_cicm->maxmsz;
        icmcb_tbl[icmid].bufptr = pk_cicm->bufptr;
        icmcb_tbl[icmid].freesz = pk_cicm->bufsz;;
        icmcb_tbl[icmid].buf_rp = icmcb_tbl[icmid].buf_wp = pk_cicm->bufptr;
        icmid++;
    } else {
        icmid = E_LIMIT;
    }
    icc_unl_spin(SPINLOCK_ICM); EI(intsts);    // クリティカルセクション終了
    return icmid;
}

/* バッファへのメッセージの格納 */
static void store_msg( ICMCB *icmcb,  const void *msg, INT msgsz)
{
    UB      *src, *dst, *limit;
    INT     i;

    src = (UB*)msg;
    dst = icmcb->buf_wp;
    limit = icmcb->bufptr + icmcb->bufsz;

    *dst++ = (UB)msgsz;
    for( i = msgsz; i > 0; i--) {
        *dst++ = *src++;
        if(dst >= limit) dst = icmcb->bufptr;
    }
    
    icmcb->freesz -= msgsz + sizeof(UB);
    icmcb->buf_wp = dst;
}

/* バッファからメッセージを取得 */
static INT retrieve_msg( ICMCB *icmcb,  const void *msg)
{
    UB      *src, *dst, *limit;
    INT     msgsz;
    INT     i;

    src = icmcb->buf_rp;
    dst = (UB*)msg;
    limit = icmcb->bufptr + icmcb->bufsz;

    msgsz =  *src++;
    for( i = msgsz; i > 0 ; i--) {
        *dst++ = *src++;
        if(src >= limit) src = icmcb->bufptr;
    }

    icmcb->freesz += msgsz + sizeof(UB);
    icmcb->buf_rp = src;

    return msgsz;
}



/* CPUコア間メッセージの送信API */
ER icc_snd_msg( ID icmid, const void *msg, INT msgsz, TMO tmout)
{
    ICMCB   *icmcb;
    UINT    intsts;
    ER      err     = E_OK;

    if(icmid <= 0 || icmid > CNF_MAX_ICMID) return E_ID;

    DI(intsts); icc_loc_spin(SPINLOCK_ICM);    // クリティカルセクション開始
    icmcb = &icmcb_tbl[--icmid];

    /* メッセージ送信方向のチェック */
    if((CPU_CORE == 0 && icmcb->icmatr == TA_ICM_TO_C0)
            || (CPU_CORE == 1 && icmcb->icmatr == TA_ICM_TO_C1)) {
        err = E_CTX;
        goto EXIT;
    }

    if(icmcb->state == KS_EXIST) {
        if(icmcb->freesz > msgsz + sizeof(UB)) {    // バッファに空きがある
            store_msg( icmcb, msg, msgsz);              // バッファへメッセージを格納
            icc_ras_int(ICCINT_ICM_SND<<24 | icmid);    // 他CPUコアへのCPUコア間割り込みの発生
            
        } else {                                    // バッファに空きが無いので送信待ち状態に移行
            tqueue_remove_top(&ready_queue[CPU_CORE][cur_task[CPU_CORE]->itskpri]);     // タスクをレディキューから外す

            /* TCBの各種情報を変更する */
            cur_task[CPU_CORE]->state     = TS_WAIT;      // タスクの状態を待ち状態に変更
            cur_task[CPU_CORE]->waifct    = TWFCT_ICMS;   // 待ち要因を設定
            cur_task[CPU_CORE]->waiobj    = icmid;        // 待ちメッセージバッファIDを設定
            cur_task[CPU_CORE]->waitim    = ((tmout == TMO_FEVR)? tmout: tmout + TIMER_PERIOD);    // 待ち時間を設定
            cur_task[CPU_CORE]->msgsz     = msgsz;        // メッセージのサイズ
            cur_task[CPU_CORE]->msg       = msg;          // メッセージ
            cur_task[CPU_CORE]->waierr    = &err;

            tqueue_add_entry(&wait_queue[CPU_CORE], cur_task[CPU_CORE]);    // タスクをウェイトキューに繋ぐ
            scheduler();                                                    // スケジューラを実行
        }
    } else {
        err = E_NOEXS;
    }

EXIT:
    icc_unl_spin(SPINLOCK_ICM); EI(intsts);    // クリティカルセクション終了
    return err;
}

/* CPUコア間メッセージの受信API */
INT icc_rcv_msg( ID icmid, void *msg, TMO tmout)
{
    ICMCB   *icmcb;
    UINT    intsts;
    INT     msgsz   = 0;
    ER      err     = E_OK;

    if(icmid <= 0 || icmid > CNF_MAX_ICMID) return E_ID;

    DI(intsts); icc_loc_spin(SPINLOCK_ICM);    // クリティカルセクション開始
    icmcb = &icmcb_tbl[--icmid];

    /* メッセージ送信方向のチェック */
    if((CPU_CORE == 0 && icmcb->icmatr == TA_ICM_TO_C1)
            || (CPU_CORE == 1 && icmcb->icmatr == TA_ICM_TO_C0)) {
        err = E_CTX;
        goto EXIT;
    }

    if(icmcb->state == KS_EXIST) {
        if(icmcb->buf_rp != icmcb->buf_wp) {    // メッセージが格納されている
            msgsz = retrieve_msg( icmcb,  msg);         // バッファからメッセージを取得
            icc_ras_int(ICCINT_ICM_RCV<<24 | icmid);    // 他CPUコアへのCPUコア間割り込みの発生

        } else {                                // メッセージが無いので受信待ち状態に移行
            tqueue_remove_top(&ready_queue[CPU_CORE][cur_task[CPU_CORE]->itskpri]);     // タスクをレディキューから外す

            /* TCBの各種情報を変更する */
            cur_task[CPU_CORE]->state     = TS_WAIT;      // タスクの状態を待ち状態に変更
            cur_task[CPU_CORE]->waifct    = TWFCT_ICMR;   // 待ち要因を設定
            cur_task[CPU_CORE]->waiobj    = icmid;        // 待ちメッセージバッファIDを設定
            cur_task[CPU_CORE]->waitim    = ((tmout == TMO_FEVR)? tmout: tmout + TIMER_PERIOD);    // 待ち時間を設定
            cur_task[CPU_CORE]->msg       = msg;          // メッセージを格納する領域
            cur_task[CPU_CORE]->waierr    = &err;

            tqueue_add_entry(&wait_queue[CPU_CORE], cur_task[CPU_CORE]);      // タスクをウェイトキューに繋ぐ
            scheduler();                                            // スケジューラを実行
        }
    } else {
        err = E_NOEXS;
    }

EXIT:
    icc_unl_spin(SPINLOCK_ICM); EI(intsts);    // クリティカルセクション終了
    return (msgsz? msgsz: (INT)err);
}

/* CPUコア間割り込みハンドラ  CPUコア間メッセージ送信 */
void icm_snd_inthdr(UW intdat)
{
    ICMCB   *icmcb;
    TCB     *tcb;
    ID      icmid;
    INT     msgsz;
    UINT    intsts;

    DI(intsts); icc_loc_spin(SPINLOCK_ICM);    // クリティカルセクション開始

    icmid = intdat & 0x0FFF;
    icmcb = &icmcb_tbl[icmid];
    for( tcb = wait_queue[CPU_CORE]; tcb != NULL; tcb = tcb->next) {    // ウェイトキューの検索
        if((tcb->waifct == TWFCT_ICMR) && (tcb->waiobj == icmid)) {     // 受信待ちタスクはあるか？
            msgsz = retrieve_msg( icmcb,  tcb->msg);                        // メッセージの受信
                    
            tqueue_remove_entry( &wait_queue[CPU_CORE], tcb);               // タスクをウェイトキューから外す
            tcb->state	= TS_READY;                                         // タスクを実行可能状態へ遷移
            tcb->waifct	= TWFCT_NON;
            *(tcb->waierr) = msgsz; 
            tqueue_add_entry( &ready_queue[CPU_CORE][tcb->itskpri], tcb);   // タスクをレディキューへつなぐ
            scheduler();                                            // スケジューラを実行
            break;
        }
    }

    icc_unl_spin(SPINLOCK_ICM); EI(intsts);    // クリティカルセクション終了
}

/* CPUコア間割り込みハンドラ  CPUコア間メッセージ受信 */
void icm_rcv_inthdr(UW intdat)
{
    ICMCB   *icmcb;
    TCB     *tcb;
    ID      icmid;
    UINT    intsts;

    DI(intsts); icc_loc_spin(SPINLOCK_ICM);    // クリティカルセクション開始

    icmid = intdat & 0x0FFF;
    icmcb = &icmcb_tbl[icmid];
    for( tcb = wait_queue[CPU_CORE]; tcb != NULL; tcb = tcb->next) {    // ウェイトキューの検索
        if((tcb->waifct == TWFCT_MBFR) && (tcb->waiobj == icmid)) {     // 受信待ちタスクはあるか？
            if(tcb->msgsz <= icmcb->freesz) {                           // バッファに空きはあるか？
                store_msg(icmcb, tcb->msg, tcb->msgsz);                         // メッセージの送信

                tqueue_remove_entry( &wait_queue[CPU_CORE], tcb);               // タスクをウェイトキューから外す
                tcb->state	= TS_READY;                                         // タスクを実行可能状態へ
                tcb->waifct	= TWFCT_NON;
                tqueue_add_entry( &ready_queue[CPU_CORE][tcb->itskpri], tcb);   // タスクをレディキューへつなぐ
                scheduler();                                            // スケジューラを実行
            }
            break;
        }
    }
    
    icc_unl_spin(SPINLOCK_ICM); EI(intsts);    // クリティカルセクション終了
}

/* CPUコア間メッセージの初期化 */
void init_icc_msg(void)
{
    icc_def_int(ICCINT_ICM_SND, (FP)icm_snd_inthdr);
    icc_def_int(ICCINT_ICM_RCV, (FP)icm_rcv_inthdr);
}
