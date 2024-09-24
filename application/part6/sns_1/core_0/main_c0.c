#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

/* WiFi通信の定義 */
#define SSID           "##########"         // WiFiのSSID
#define PASSWORD        "########"          // WiFiのパスワード
#define SERVER_IP       "192.168.xxx.xxx"   // サーバーのIPアドレス
#define SERVER_PORT     123456              // 通信ポート

#define POLL_INTERVAL_S 2                   // ポーリング間隔（秒単位）

static struct tcp_pcb   *client_pcb;    
static ip_addr_t        server_ip;
static int              do_loop;

/* センサーデータ (CPUコア1のグローバル変数) */
extern int              g_temp, g_humi, g_bright;

/* TCPクライアントのポーリングコールバック関数 */
static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t err;
    char buf[100];

    snprintf(buf, sizeof(buf), "{\"temp\": %d, \"humi\": %d, \"bright\": %d}",
        g_temp, g_humi, g_bright);      // 送信データの作成
    
    err = tcp_write(tpcb, buf, strlen(buf), TCP_WRITE_FLAG_COPY);   // データの送信
    if (err != ERR_OK) {
        printf("Failed to write data %d\n", err);
        do_loop = 0;
        return ERR_ABRT;
    }
    tcp_output(tpcb);

    return ERR_OK;
}

/* エラーコールバック関数 */
static void tcp_client_err(void *arg, err_t err) {
    printf("tcp_client_err %d\n", err);
    do_loop = 0;
}

/* 接続完了コールバック関数 */
static err_t connect_callback(void *arg, struct tcp_pcb *pcb, err_t err) {
    if (err != ERR_OK) {
        printf("Connection failed\n");
        return err;
    }
    printf("Connected to server\n");
    return ERR_OK;
}

/* WiFi通信制御メイン関数 */
void main_c0(void)
{
    err_t   err;

    if (cyw43_arch_init()) {            // WiFi初期化
        printf("Wi-Fi init failed\n");
        return;
    }
    cyw43_arch_enable_sta_mode();

    /* WiFi接続 */
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(SSID, PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        goto ERROR;
    }
    printf("Connected.\n");             // 接続成功
    
    client_pcb = tcp_new();             // TCPプロトコル制御ブロック生成
    if (!client_pcb) {
        printf("tcp_new failed\n");
        goto ERROR;
    }

    if (ipaddr_aton(SERVER_IP, &server_ip)) {       // サーバIPアドレスの変換
        printf("The numeric IP representation is: %u\n", server_ip.addr);   // 変換成功
    } else {
        printf("Invalid IP address format.\n");                             // 変換失敗
        goto ERROR;
    }

    /* コールバック関数設定 */
    tcp_poll(client_pcb, tcp_client_poll, POLL_INTERVAL_S * 2);             // ポーリング
    tcp_err(client_pcb, tcp_client_err);                                    // エラー

    /* TCP接続 */
    err = tcp_connect(client_pcb, &server_ip, SERVER_PORT, connect_callback);
    if(err != ERR_OK) {
        printf("tcp connect error %d", (int)err);
        goto ERROR;
    }
    printf("tcp connect\n");        // サーバーに接続成功
    do_loop = 1;
    while (do_loop) {                // プロトコルスタックのポーリング・ループ
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(100));
    }

ERROR:
    if (client_pcb) tcp_close(client_pcb);
    cyw43_arch_deinit();
}