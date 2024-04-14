個人作品の紹介：
.net Windows Formの基づくエンジン。
UnityEngineの使い方参考して作っています。
ゲームエンジンの基礎技術の学習、および求職の展示を目的として作られます。

枠組み：
二つのプロジェクトある、エンジンと３Dレンダーです。
他の第三者サードパーティのロードテクスチャや、基本モデル作るプロジェクトも補助的な使用する。

![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/frame.jpg)

エンジンプロジェクトはエンジン・コアとエンジンエディタと分ける。
エンジン・コアは基本なゲイムサイクル、レンダーのAPIを呼び出す、Entity、Componentなどの機能を実現する部分です。
エンジンエディタはゲームプロジェクト編集のため、

![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/editor.jpg)




実装された機能と学習体験：

３Dレンダープロジェクト；
レンダーのため作られたC++プロジェクトです。
Directx12を使用して、モジュール化で、DLLファイルを作成して、C＃のメインプロジェクトに各図形のAPI提供しています。
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/d3dapi.jpg)
ロード可能のモデルは、本「Directx12の魔導書」に教えられたPMDフォーマット、Boneデータ含めて、付属アニメ機能のVMD動画ファイルを使えるモデルです。
もう一つ使うモデルは、自分が規範を設定した基本モデルです。Boneなど動画データはなくて、Cube、Sphereのような普通のモデルに使われる。
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/vd1.jpg) ![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/vd2.jpg)


Scene、Entity、Component：
EntityはUnityのGameOjbectのようなものです。
左のTreeViewはSceneの全てのEntity展示します。
真ん中の画面はエディターカメラの画面で、３Dのモデル展示します。
右のPanelは選択したEntityの全てのComponentを示す。見えるFieldはPublic、又は[MyAttributeShowInspector] 属性加えたのです。