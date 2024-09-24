#ifndef	PWM_RP2040_H
#define	PWM_RP2040_H

extern void pwm_init(UW use_pin);                          // PWM 初期化
extern void pwm_set_clkdiv(UINT no, UW div_i, UW div_f);   // PWM Clock devision 値設定
extern void pwm_set_wrap(UINT no, UW wrap);                // PWM Counter wrap 値設定
extern void pwm_set_cc(UINT no, UW cc);                    // PWM Counter compare 値設定
extern void pwm_enable(UINT no, BOOL enable);              // PWM Enable

#endif	/* PWM_RP2040_H */