#include <trykernel.h>

void task_lsns(INT stacd, void *exinf);             // タスクの実行関数
UW  tskstk_lsns[1024/sizeof(UW)];                   // タスクのスタック領域
ID  tskid_lsns;                                     // タスクID

/* タスク生成情報 */
T_CTSK  ctsk_lsns = {
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,   // タスク属性
    .task       = task_lsns,                        // タスクの実行関数
    .itskpri    = 10,                               // タスク優先度
    .stksz      = sizeof(tskstk_lsns),              // スタックサイズ
    .bufptr     = tskstk_lsns,                      // スタックへのポインタ
};

int     g_temp, g_humi, g_bright;

/* 光センサ制御タスク */
void task_lsns(INT stacd, void *exinf)
{
    ID  dd_adc;             // デバイスディスクリプタ
    UW  data;               // 受信データ
    SZ  asz;
    ER  err;

    dd_adc = tk_opn_dev("adca", TD_READ);           // A/DCデバイスのオープン

    while(1) {
        err = tk_srea_dev(dd_adc, 0, &data, 1, &asz);   // データの取得
        g_bright = data;                            // グローバル変数に保存
        tk_dly_tsk(500);                            // 0.5秒待ち
    }
}

void task_tsns(INT stacd, void *exinf);             // タスクの実行関数
UW  tskstk_tsns[1024/sizeof(UW)];                   // タスクのスタック領域
ID  tskid_tsns;                                     // タスクID

/* タスク生成情報 */
T_CTSK  ctsk_tsns = {
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,   // タスク属性
    .task       = task_tsns,                        // タスクの実行関数
    .itskpri    = 10,                               // タスク優先度
    .stksz      = sizeof(tskstk_tsns),              // スタックサイズ
    .bufptr     = tskstk_tsns,                      // スタックへのポインタ
};

/* 温度湿度センサー(SHT35)情報定義*/
#define	I2C_SADR			(0x45)

/* 温度湿度センサー制御タスク */
void task_tsns(INT stacd, void *exinf)
{
	ID	dd_i2c;             // デバイスディスクリプタ
	UB	cmd[2], data[6];    // 送信および受信データ
	UW	temp, humi;         // 温度および湿度
	SZ	asz;
	ER	err;

    dd_i2c = tk_opn_dev("iica", TD_UPDATE); // I2Cデバイスのオープン
    cmd[0] = 0x23; cmd[1] = 0x34;
    err = tk_swri_dev(dd_i2c, I2C_SADR, cmd, sizeof(cmd), &asz);    // コマンドの送信

	while(1) {
		err = tk_srea_dev(dd_i2c, I2C_SADR, data, sizeof(data), &asz);  // データの受信 
		temp = ((UW)data[0]<<8) | data[1];
		temp = (temp*1750 >> 16) - 450;     // 温度データの補正
		humi = ((UW)data[3]<<8) | data[4];
		humi = humi*1000 >> 16;             // 湿度データの補正

		g_temp = temp;                      // グローバル変数に保存
		g_humi = humi;

		tk_dly_tsk(500);			        // 0.5秒間待ち
	}    
}

int usermain(void)
{
    tm_putstring("Start\n");

    tskid_lsns = tk_cre_tsk(&ctsk_lsns);
    tk_sta_tsk(tskid_lsns, 0);

    tskid_tsns = tk_cre_tsk(&ctsk_tsns);
    tk_sta_tsk(tskid_tsns, 0);

    tk_slp_tsk(TMO_FEVR);       // 初期タスクを待ち状態に
    return 0;
}
