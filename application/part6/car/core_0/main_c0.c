#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

/* WiFi通信の定義 ！実際のネットワークの設定に合わせて指定します！ */
#define SSID           "##########"     // WiFiのSSID
#define PASSWORD       "#########"      // WiFiのパスワード 
#define SERVER_PORT    123456

static struct tcp_pcb *server_pcb;  // サーバのTCPプロトコル制御ブロック
static struct tcp_pcb *client_pcb;  // クライアントのTCPプロトコル制御ブロック

const char *cmdstr[] = {"stop", "star", "stra", "righ", "left", "spup", "spdn" };

/* クライアントからのデータ受信コールバック関数 */
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    char buf[1024];     // 受信データ
    int cmdno;          // コマンド番号


    if (p == NULL) {
        tcp_close(client_pcb);                          // 接続終了
        printf("Client disconnected\n");
        return ERR_OK;
    }
    
    pbuf_copy_partial(p, buf, p->tot_len, 0);           // 受信データの取り出し
    buf[p->tot_len] = '\0';
    printf("Received data: %s\n", buf);
    
    for(cmdno = 0; cmdno < sizeof(cmdstr)/sizeof(char*); cmdno++) {
        if(!strcmp(buf, cmdstr[cmdno])) break;
    }
    if(cmdno < sizeof(cmdstr)/sizeof(char*)) {
        printf("Command No: %d\n", cmdno);
        if(sio_hw->fifo_st & SIO_FIFO_ST_RDY_BITS) {    // コマンド番号をFIFOに送信
            sio_hw->fifo_wr = cmdno;
        }
    }
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    
    return ERR_OK;
}

/* 接続要求の受付コールバック関数 */
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    client_pcb = newpcb;                // クライアントのTCPプロトコル制御ブロック
    printf("Client connected\n");
    
    tcp_recv(client_pcb, tcp_server_recv);  // クライアントからのデータ受信コールバック関数の設定
    
    return ERR_OK;
}

/* WiFi通信制御メイン関数 */
void main_c0(void)
{
    err_t err;

    if (cyw43_arch_init()) {            // WiFi初期化
        printf("Wi-Fi init failed\n");
        return;
    }
    cyw43_arch_enable_sta_mode();

    /* WiFi接続 */
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(SSID, PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect.\n");
        goto ERROR;
    }
    printf("Connected.\n");             // 接続成功

    server_pcb = tcp_new();             // サーバのTCPプロトコル制御ブロック生成
    if (!server_pcb) {
        printf("tcp_new failed\n");
        goto ERROR;
    }
    err = tcp_bind(server_pcb, IP_ADDR_ANY, SERVER_PORT);
    if (err != ERR_OK) {
        printf("tcp_bind failed %d\n", err);
        goto ERROR;
    }

    server_pcb = tcp_listen(server_pcb);            // サーバをリッスン状態に設定
    tcp_accept(server_pcb, tcp_server_accept);      // サーバの接続要求の受付コールバック関数を設定
    printf("Server is listening on port %d\n", SERVER_PORT);

    while (1) {                // プロトコルスタックのポーリング・ループ
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(100));
    }

ERROR:
    if (server_pcb) tcp_close(server_pcb);
    cyw43_arch_deinit();
}
