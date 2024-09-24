#include <trykernel.h>

int usermain_c0(void)
{
    tm_putstring("Start Core-0 Application\n");

    tk_slp_tsk(TMO_FEVR);       // 初期タスクを待ち状態に
    return 0;
}

int usermain_c1(void)
{
    tm_putstring("Start Core-1 Application\n");

    tk_slp_tsk(TMO_FEVR);       // 初期タスクを待ち状態に
    return 0;
}