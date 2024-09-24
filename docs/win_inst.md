# Pico SDK 1.5.1のWindowsへの手動インストール
## 1. はじめに
### 1.1 概要
本ドキュメントでは、Raspberry Pi Pico SDK (以降Pico SDK) バージョン1.5.1の開発環境をWindowsに手動インストールする方法について記載します。  
この方法は公式ドキュメント **Getting Started with Raspberry Pi Pico (Release 2.2)** に記載されていた内容に基づきます。しかしPico SDKがバージョン2に更新した際にドキュメントも変更になり、情報が参照できなくなったようなのでここにまとめておきます。なお、Pico SDK 2.0以降は方法が異なります。  

### 1.2 注意点
本ドキュメントの内容は筆者が動作確認したものですが、PCの環境によっては動作が異なる場合もありますのでご了承ください。  
手動インストールは公式では、手順が複雑なので非推奨となっています。ただ、すでに他の開発環境がインストールされているPCなどでは有用な場合もあるかと思われます。  
ちなみに以下のインストーラによる自動インストールの方が簡単でありトラブルも少ないと思います。  

### 1.3 インストーラによる自動インストール
Pico SDK 1.5.1の開発環境を簡単にインストールするには、公式のインストーラを使用する方法もあります。インストーラは以下から入手できます。Windowsの標準的なインストーラですので、簡単にインストールすることができます。  

Windows用インストーラ  
[pico-setup-windows (GitHub)](https://github.com/raspberrypi/pico-setup-windows)  

## 2. 開発ツールのインストール
### 2.1 インストールする開発ツール
以下の開発ツールをインストールします。すでにインストール済みのツールは再度インストールは不要です。  

|名称|機能|ダウンロードページのURL|
|-|-|-|
|Arm GNU Toolchain|Arm用Cクロスコンパイラ・ツールチェン|https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads|
|CMake|ビルド管理ツール|https://cmake.org/download/|
|Build Tools for Visual Studio 2022|WindowsネイティブCコンパイラ|https://visualstudio.microsoft.com/ja/downloads/|
|Python 3| Windows用Python3|https://www.python.org/downloads/windows/|
|Git|ソースコード管理ツール|https://git-scm.com/download/win|

### 2.2 インストーラとの相違点
前述の公式インストーラと、ここで説明する手動インストールでは、インストールされる開発ツールに若干の相違があります。たとえばCMakeから使用されるビルドツールは、インストーラではNinjaですが、手動インストールではBuild Tools for Visual Studio 2022のnmakeを使用します。  

### 2.3 インストール手順
以下の手順で各開発ツールをインストールしていきます。

#### ① Arm用Cクロスコンパイラ・ツールチェンのインストール
前述のURLのArm GNU Toolchainのダウンロードページから、Windows用のAArch32 bare-metal target (arm-none-eabi)のインストーラをダウンロードし実行します。  
インストール中に聞かれたら、ArmコンパイラへのパスをWindowsの環境変数として登録を有効とします。  

#### ② CMakeのインストール
前述のURLのCMakeのダウンロードページから、Windows用のCMakeのインストーラをダウンロードし実行します。  
インストール中に聞かれたら、すべてのユーザーのシステムのPATHにCMakeを追加を有効とします。

#### ③ Build Tools for Visual Studio 2022のインストール
前述のURLのVisual Studio 2022のダウンロードページから、Build Tools for Visual Studio 2022のインストーラを探してダウンロードします。Webページ下の方のTools for Visual Studioの中にあります。  
インストーラを実行し、C++ Build toolsをインストールします。  

#### ④ Python 3のインストール
前述のURLのPython for WindowsのWebページから、Python 3のインストーラをダウンロードし実行します。  
インストール中に聞かれたら以下のように設定します。  
- すべてのユーザー向けにインストール
- Python 3 をシステムの PATH に追加
- MAX_PATHの長さ制限を無効

#### ⑤ Gitのインストール
前述のURLのWindows用Gitのダウンロードページから、Gitのインストーラをダウンロードし実行します。  
インストール中に聞かれたら以下のように設定します。 
- デフォルトエディタを変更
- 「Git from the command line and also from 3rd-party software」を選択
- 「Checkout as-is, commit a-is」を選択
- 「Use Windows' default console windows」を選択
- 「Enable exprimental suppport for pseudo consoles」を選択

※上記は公式ドキュメントに記載の設定ですが、各自の環境に合わせての変更はできると思います。  

以上で開発ツールのインストールは完了です。  

## 3. Pico-SDKの準備
### 3.1 Pico SDKのソースコード取得
Pico SDKとpico-examplesのソースコードをgitを使用して取得します。pico-examplesはサンプルのプロジェクトです。  
仮に作業を行うディレクトリをpico_workとします。pico_workディレクトリをカレントとして、以下のgitコマンドを実行します。    

```
 git clone https://github.com/raspberrypi/pico-sdk.git -b 1.5.1
 cd pico-sdk
 git submodule update --init
 cd ..
 git clone https://github.com/raspberrypi/pico-examples.git -b sdk-1.5.1
 ```

 pico_workディレクトリ内にpico-sdkとpico-examplesの二つのディレクトリができます。この二つのディレクトリは必ず同一のディレクトリに置いてください。  

### 3.2 サンプルプログラムのビルド
開発環境の動作確認のためにpico-examplesを以下の手順でビルドしてみます。  

① Windowsのスタートメニューから、[Visual Studio 2022]->[Developer Command Prompt for VS 2022]を選択し、コマンドプロンプトを実行します。  

② pico_workディレクトリをカレントとして、環境変数 PICO_SDK_PATHにPico SDKのパスを以下のコマンドで設定します。  

`setx PICO_SDK_PATH "..\..\pico-sdk"`  

環境変数を設定するために、コマンドプロンプトをいったん終了し、再度実行します。なお、環境変数の設定は1回のみです。  

③ pico_workディレクトリをカレントとして、以下のコマンドで実行します。

```
cd pico-examples
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```

最後のコマンドでpico-examplesの全サンプルのビルドが開始します。1回目のビルドは非常に時間がかかります。  
成功すると、buildディレクトリ内の各プロジェクトのserialおよびusbディレクトリに、elf、bin、uf2の実行プログラムのファイルが生成されます。これで開発環境の動作は確認できました。  

## 4. Visual Studio Codeを使った開発環境
### 4.1 事前準備
Visual Studio Code(以降VSCode)からプログラムをビルドする環境を作ります。
本項までの手順は終わって、Pico-SDKのサンプルがビルドできる環境が出来ているとします。  
VSCodeがインストールされてなければインストールし、VSCodeに「CMake Tools」機能拡張をインストールします。  

### 4.2 VSCodeによるサンプルのビルド
VSCodeを使ってpico-examplesをビルドする手順を以下に記します。  

① VSCodeを起動します。  

③ VSCodeの左側にあるナビゲーションバー下部の歯車をクリックし[設定]を選択します。  
続いて[設定]ペインで[拡張機能]をクリックし、[CMake Tools]をクリックします。  

④ 「Cmake: Configure Environment」まで下にスクロールし、「項目の追加」をクリックしてPICO_SDK_PATHに`..\..\pico-sdk`を設定します。  

⑤ 「Cmake:Generator」まで下にスクロールし、ボックスに`NMake Makefiles`と入力します。  以上で設定ページを閉じます。  

⑥ [ファイル]メニューから[フォルダを開く]を選択し、pico-examplesディレクトリを開きます。プロジェクトを構成するか聞かれますので、コンパイラに "GCC for arm-none-eabi" を選択します。

⑦ ウィンドウの下部バーにある[ビルド]ボタン(歯車付き)をクリックします。これにより、CMake が実行され、サンプルプログラムがビルドされます。  

## 5. さいごに
以上で、Windowsのコマンドプロンプト、またはVSCodeからPico-SDKを使用したプログラムの構築ができました。インストーラによる自動インストールとほぼ同じ環境が出来上がっています（一部使用するツールは違います）。  
新規プロジェクトの作成は、Raspberry Pi PicoのVSCode機能拡張やProject Generatorを使用して行うことができます。詳しくは「ゼロから作るマルチコアOS 第6部 第1章」の「プロジェクト作成ツール」以降をご覧ください。  

以上

## 履歴
2024.09.24  初版
