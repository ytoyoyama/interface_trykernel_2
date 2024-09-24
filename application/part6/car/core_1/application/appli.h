/*
 *** Try Kernel マイコンカー Autonomous Robotics Platform
 *          アプリケーション共通定義
 */

#define MSG_STOP        0
#define MSG_RUN         1
#define	MSG_STRAIGHT    2
#define	MSG_RIGHT       3
#define MSG_LEFT        4
#define MSG_SPDUP       5
#define MSG_SPDDWN      6

extern T_CTSK ctsk_icc;                    // CPUコア間通信タスク生成情報
extern ID  tskid_icc;                      // タスクID番号

extern T_CTSK ctsk_run;                   	// 走行制御タスク生成情報
extern ID  tskid_run;                     	// タスクID番号

extern ID  mbfid;                           // メッセージバッファ ID
