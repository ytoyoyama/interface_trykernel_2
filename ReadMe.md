# Interface 2024年11月号 特集「ゼロから作るマルチコアOS」配布プログラム

## 概要

本リポジトリは、CQ出版(株) Interface 2024年11月号の特集「ゼロから作るマルチコアOS」の配布プログラムです。  
特集記事にて記載した自作OS「Try Kernel / Try Kernel-A / Try Kernel-S」のプログラムです。ラズパイPico/Pico Wにて動作します。プログラムの内容、開発環境や手順については記事をご覧ください。  
特集の各部、各章で作成した完成プログラムのソースコードを以下のディレクトリに格納しています。

├ trykernel_2　　Try Kernel 2.0 （シングルコア版）  
├ trykernel_a　　Try Kernel-A　（AMP版）  
├ trykernel_s　　Try Kernel-S　（SMP版）  
├ hybsys　　　　Try Kernel + PicoSDK ハイブリッド  
└ application　　アプリケーションなど  
　├ part1 第1部　OS作りの準備①…マルチコアを理解する  
　├ part2 第2部　OS作りの準備②…割り込みを理解する  
　├ part3 第3部　OS作り① AMP方式  
　├ part4 第4部　AMP方式アプリケーションの作成  
　├ part5 第5部　OS作り② SMP方式  
　└ part6 第6章　Pico SDKとTry Kernelのハイブリッドシステム  

## 関連リンク

Try Kernelについては以下をご覧ください。  

<https://ytoyoyama.github.io/>

## ライセンスについて

本プログラムはMITライセンスの下でオープンソースとして公開します。著作権および許諾表示を記載すれば、非営利、商用を問わず、使用、改変、複製、再頒布が可能な制限の緩いライセンスですので、本プログラムをOSの自作に活用いただけたらと思います。ライセンスの詳細については、同梱のLICENSEをご参照ください。  
ただし、ブートコードの一部(boot/boot2.c)でPico C/C++ SDKのオブジェクトコードを利用してますので、それについてはPico C/C++ SDKのライセンスが適用されます。ソースファイルの冒頭に記載したライセンスに従ってください。このライセンスも厳しい制約はありません。  

