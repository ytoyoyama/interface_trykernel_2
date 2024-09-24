# マイコンカー・リモートプログラム

import socket
import tkinter as tk
from tkinter import messagebox

# サーバのIPアドレスとポートを設定
TCP_IP = '0.0.0.0'
TCP_PORT = 123456

# データの送信
def send_message(message):
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    # ソケットを作成
        client_socket.connect((SERVER_IP, SERVER_PORT)) # サーバに接続
        client_socket.sendall(message.encode('utf-8'))  # データを送信
        client_socket.shutdown(socket.SHUT_WR)          # データの送信を終了
        client_socket.close()                           # ソケットを閉じる
        print(f"Sent: {message}")
        
    except Exception as e:
        messagebox.showerror("Connection Error", f"Unable to connect to server: {e}")

def create_button(frame, text, data):
    button = tk.Button(frame, text=text, command=lambda: send_message(data))
    button.pack(side=tk.LEFT, padx=5, pady=5)
    return button

def main():
    root = tk.Tk()
    root.title("TCP Client")
    
    # ボタンフレームを作成
    frame1 = tk.Frame(root)
    frame2 = tk.Frame(root)
    frame3 = tk.Frame(root)
    
    # 各ボタンを作成
    create_button(frame1, "Left", "left")
    create_button(frame1, "Straight", "stra")
    create_button(frame1, "Right", "righ")
    
    create_button(frame2, "Speed Up", "spup")
    create_button(frame2, "Speed Down", "spdn")
    
    create_button(frame3, "Start", "star")
    create_button(frame3, "Stop", "stop")
    
    # フレームを表示
    frame1.pack(pady=10)
    frame2.pack(pady=10)
    frame3.pack(pady=10)
    
    root.mainloop()

if __name__ == "__main__":
    main()