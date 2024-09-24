#include <trykernel.h>
#include "pwm_rp2040.h"

/* GPIOポート番号をPWMスライスに変更 */
#define PWM_PORT_TO_SLICE(port_no)   ((port_no>>1)& 0x07)

/* PWM 初期化 */
void pwm_init(UW use_pin)
{
    UINT    pin;

    clr_w(RESETS_RESET, (1<<14));    /* PWM 動作 */
    while((in_w(RESETS_RESET_DONE) & (1<<14))==0);

    /* PWM出力端子初期化 */
    for(pin = 0; pin <= 29; pin++, use_pin >>= 1) {
        if(use_pin & 1) {
            clr_w(GPIO(pin), GPIO_OD);              // 出力有効
            set_w(GPIO(pin), GPIO_IE);              // 入力有効
            out_w(GPIO_CTRL(pin), 4);               // PWM機能選択
        }
    }
}

/* PWM Clock devision 値設定 */
void pwm_set_clkdiv(UINT no, UW div_i, UW div_f)
{
    out_w(PWM_CH_DIV(PWM_PORT_TO_SLICE(no)), (div_i<<4)|(div_f&0x0F));
}

/* PWM Counter wrap 値設定 */
void pwm_set_wrap(UINT no, UW wrap)
{
    out_w(PWM_CH_TOP(PWM_PORT_TO_SLICE(no)), wrap);
}

/* PWM Counter compare 値設定 */
void pwm_set_cc(UINT no, UW cc)
{
    UINT    slice;
    UW      reg;

    slice = PWM_PORT_TO_SLICE(no);
    reg = in_w(PWM_CH_CC(slice));              
    if(no & 0x01) {     /* Chan B */
        out_w(PWM_CH_CC(slice), (reg & 0x0000FFFF)| (cc<<16));
    } else {            /* Chan A */
        out_w(PWM_CH_CC(slice), (reg & 0xFFFF0000)| cc);
    }
}

void pwm_enable(UINT no, BOOL enable)
{
    if(enable) {
        set_w(PWM_CH_CSR(PWM_PORT_TO_SLICE(no)), PWM_CH_CSR_EN);    // 有効
    } else {
        clr_w(PWM_CH_CSR(PWM_PORT_TO_SLICE(no)), PWM_CH_CSR_EN);    // 無効
    }
}
