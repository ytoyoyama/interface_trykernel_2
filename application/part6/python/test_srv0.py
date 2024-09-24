# センサーノード 実験用簡易サーバープログラム
import socket
import json

# 定数
TCP_IP = '0.0.0.0'  # すべてのインターフェースで接続を待ち受ける
TCP_PORT = 123456   # クライアントと一致するポート番号に設定
BUFFER_SIZE = 1024  # 受信バッファのサイズ

def start_server():
    # ソケットの作成
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((TCP_IP, TCP_PORT))
    server_socket.listen(1)

    print(f'Server started, waiting for connections on {TCP_IP}:{TCP_PORT}')

    while True:
        conn, addr = server_socket.accept()
        with conn:
            print(f'Connection from: {addr}')

            while True:
                data = conn.recv(BUFFER_SIZE)
                if not data:
                    # クライアントが切断した場合
                    print('Client disconnected')
                    break
                # print(f"Received data: {data.decode('utf-8')}")
                try:
                    json_data = json.loads(data.decode('utf-8'))
                    temperature = json_data.get('temp')
                    humidity = json_data.get('humi')
                    brightness = json_data.get('bright')
                        
                    if temperature is not None:
                        print(f"Temperature: {temperature/10}")
                    if humidity is not None:
                        print(f"Humidity: {humidity/10}")
                    if brightness is not None:
                        print(f"Brightness: {brightness}")

                except json.JSONDecodeError:
                        print("Received data is not valid JSON")

if __name__ == '__main__':
    start_server()