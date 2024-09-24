# センサーノード 実験用簡易サーバープログラム GUI版
import socket
import json
import tkinter as tk

# WiFi通信の定義
TCP_IP = '0.0.0.0'  # すべてのインターフェースで接続を待ち受ける
TCP_PORT = 123456   # クライアントと一致するポート番号に設定
BUFFER_SIZE = 1024  # 受信バッファのサイズ

def start_server(display_labels):
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

                try:    # 受信データの処理
                    json_data = json.loads(data.decode('utf-8'))
                    temperature = json_data.get('temp')
                    humidity = json_data.get('humi')
                    brightness = json_data.get('bright')
                    
                    # 受信データのウィンドウへの表示
                    if temperature is not None:
                        display_labels['temperature'].config(text=f"Temperature: {temperature/10}")
                    if humidity is not None:
                        display_labels['humidity'].config(text=f"Humidity: {humidity/10}")
                    if brightness is not None:
                        display_labels['brightness'].config(text=f"Brightness: {brightness}")

                except json.JSONDecodeError:
                    print("Received data is not valid JSON")

def start_server_thread(display_labels):
    import threading
    server_thread = threading.Thread(target=start_server, args=(display_labels,))
    server_thread.daemon = True
    server_thread.start()

def create_window():
    # Tkinterのウィンドウを作成
    window = tk.Tk()
    window.title("Sensor Data Display")

    # ラベルウィジェットの生成
    temperature_label = tk.Label(window, text="Temperature: N/A")
    humidity_label = tk.Label(window, text="Humidity: N/A")
    brightness_label = tk.Label(window, text="Brightness: N/A")

    # ウィジェットの配置
    temperature_label.pack()
    humidity_label.pack()
    brightness_label.pack()

    # ラベルのディクショナリ
    display_labels = {
        'temperature': temperature_label,
        'humidity': humidity_label,
        'brightness': brightness_label
    }

    # サーバーを別スレッドで開始
    start_server_thread(display_labels)

    # ウィンドウのメインループを開始
    window.mainloop()

if __name__ == '__main__':
    create_window()