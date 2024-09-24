# マイコンカー・リモートプログラム

import socket
from kivy.app import App
from kivy.uix.button import Button
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.textinput import TextInput
from kivy.uix.label import Label
from kivy.uix.popup import Popup
from kivy.core.window import Window

class TcpClientApp(App):
    def build(self):
        # ウィンドウサイズを取得
        window_width, window_height = Window.size

        self.server_ip = '192.168.0.0'  # 初期IPアドレス（必要に応じて変更）
        self.server_port = 123456       # サーバプログラムで設定したポート番号

        root = BoxLayout(orientation='vertical', spacing=20, padding=20)

        # IPアドレス入力フィールド
        ip_layout = BoxLayout(size_hint=(1, None), height=80)
        ip_layout.add_widget(Label(text='Server IP:', size_hint=(None, None), size=(window_width * 0.25, 80)))
        self.ip_input = TextInput(text=self.server_ip, multiline=False, size_hint=(None, None), size=(window_width * 0.55, 80))
        ip_layout.add_widget(self.ip_input)
        root.add_widget(ip_layout)

        # ボタンフレームを作成
        frame1 = BoxLayout(size_hint=(1, .3))
        frame2 = BoxLayout(size_hint=(1, .3))
        frame3 = BoxLayout(size_hint=(1, .3))

        # 各ボタンを作成
        self.create_button(frame1, window_width*0.28 , "Left", "left")
        self.create_button(frame1, window_width*0.28 , "Straight", "stra")
        self.create_button(frame1, window_width*0.28 , "Right", "righ")
        
        self.create_button(frame2, window_width*0.35, "Speed Up", "spup")
        self.create_button(frame2, window_width*0.35, "Speed Down", "spdn")
        
        self.create_button(frame3, window_width*0.28, "Start", "star")
        self.create_button(frame3, window_width*0.28, "Stop", "stop")

        # レイアウトの設定
        button_layout = BoxLayout(orientation='vertical',size_hint=(1, .5))
        root.add_widget(button_layout)
        root.add_widget(BoxLayout(size_hint=(1, .5)))
        button_layout.add_widget(frame1)
        button_layout.add_widget(frame2)
        button_layout.add_widget(frame3)

        return root

    # ボタンの作成
    def create_button(self, layout, button_width, text, data):
        button = Button(text=text, size_hint=(None, None), size=(button_width, 120))
        button.bind(on_press=lambda instance: self.send_message(data))
        layout.add_widget(button)
        return button

    # メッセージの送信
    def send_message(self, message):
        self.server_ip = self.ip_input.text.strip()
        if not self.server_ip:
            popup = Popup(title='IP Address Error',
                          content=Label(text="Please enter a valid IP address."),
                          size_hint=(None, None), size=(400, 200))
            popup.open()
            return

        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect((self.server_ip, self.server_port))
            client_socket.sendall(message.encode('utf-8'))
            client_socket.shutdown(socket.SHUT_WR)
            client_socket.close()
            print(f"Sent: {message}")

        except Exception as e:
            popup = Popup(title='Connection Error',
                          content=Label(text=f"Unable to connect to server: {e}"),
                          size_hint=(None, None), size=(400, 200))
            popup.open()

if __name__ == '__main__':
    TcpClientApp().run()
