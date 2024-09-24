/*
 *** Try Kernel マイコンカー Autonomous Robotics Platform
 *          走行制御タスク
 */
#include <trykernel.h>
#include "appli.h"
#include "pwm_rp2040.h"

#define MOTOR_R_F   6           // Motor-2 Forward GP06
#define MOTOR_R_R   7           // Motor-2 Reverse GP07
#define MOTOR_L_F   20          // Motor-1 Forward GP20
#define MOTOR_L_R   19          // Motor-1 Reverse GP19

UINT    pin[] = {MOTOR_R_F, MOTOR_R_R, MOTOR_L_F, MOTOR_L_R};

#define MOTOR_R_SPD_FAST        800
#define MOTOR_R_SPD_LOW         0

#define MOTOR_L_SPD_FAST        800
#define MOTOR_L_SPD_LOW         0

/* 走行制御タスクの情報 */
ID  tskid_run;                          // タスクID番号
#define STKSZ_RUN   1024                // スタックサイズ

UW  tskstk_run[STKSZ_RUN/sizeof(UW)];   // スタック領域
void tsk_run(INT stacd, void *exinf);   // タスク実行関数

T_CTSK ctsk_run = {                     // タスク生成情報
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_run,
    .stksz      = STKSZ_RUN,
    .itskpri    = 7,
    .bufptr     = tskstk_run,
};

/* 走行モータ（PWM）初期化 */
void motor_init(void)
{
    UW      use_pin = 0;
    UINT    i;

    /* PWM 初期化 */
    for(i = 0; i < sizeof(pin)/sizeof(UINT); i++) {
        use_pin |= 1<<pin[i];
    }
    pwm_init(use_pin);

    /* PWM設定 */
    for(i = 0; i < sizeof(pin)/sizeof(UINT); i++) {
        pwm_set_clkdiv(pin[i], 10, 0);      // 10分周
        pwm_set_wrap(pin[i], 1249);         // 10KHz
        pwm_set_cc(pin[i], 0);              // Duty 0 (停止)
        pwm_enable(pin[i], 1);              // 有効
    }
}

/* モータ速度設定 */
void motor_speed(INT right, INT left)
{
    if(right>=1250)   right = 1249;
    if(right<0)     right = 0;
    pwm_set_cc(MOTOR_R_F, right);

    if(left>=1250)   left = 1249;
    if(left<0)     left = 0;
    pwm_set_cc(MOTOR_L_F, left);

}

void tsk_run(INT stacd, void *exinf)
{
    UW      speed[2] = {MOTOR_R_SPD_FAST, MOTOR_R_SPD_FAST};
    UINT    stop = 0;
    UINT    msg;

    motor_init();                                       //  モータ初期化
    motor_speed( MOTOR_R_SPD_FAST, MOTOR_L_SPD_FAST);   // 走行開始

    while(1) {
        tk_rcv_mbf( mbfid, &msg, TMO_FEVR);     // メッセージ受信
        switch(msg) {
        case MSG_STOP:                          // 走行停止
            stop = 1;
            break;
        case MSG_RUN:                           // 走行開始
            stop = 0;
            break;
        case MSG_STRAIGHT:                      // 直進
            speed[0] = MOTOR_R_SPD_FAST;
            speed[1] = MOTOR_L_SPD_FAST;
            break;
        case MSG_RIGHT:                         // 右折
            speed[0] = MOTOR_R_SPD_LOW;
            speed[1] = MOTOR_L_SPD_FAST;
            break;
        case MSG_LEFT:                          // 左折
            speed[0] = MOTOR_R_SPD_FAST;
            speed[1] = MOTOR_L_SPD_LOW;
            break;
        default:
        }

        if(stop)    motor_speed( 0, 0);         // 停止
        else motor_speed( speed[0], speed[1]);  // 走行
    }
}
