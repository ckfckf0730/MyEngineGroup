個人作品の紹介：<br>
.net Windows Formの基づくエンジン。<br>
UnityEngineの使い方参考して作っています。<br>
ゲームエンジンの基礎技術の学習、および求職の展示を目的として作られます。<br>

枠組み：<br>
二つのプロジェクトある、エンジンと３Dレンダーです。<br>
他の第三者サードパーティのロードテクスチャや、基本モデル作るプロジェクトも補助的な使用する。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/frame.jpg)

エンジンプロジェクトはエンジン・コアとエンジンエディタと分ける。<br>
エンジン・コアは基本なゲイムサイクル、レンダーのAPIを呼び出す、Entity、Componentなどの機能を実現する部分です。<br>
エンジンエディタはゲームプロジェクトにグラフィカルユーザーインターフェースの編集部分です。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/editor.jpg)




実装された機能と学習体験：<br>

３Dレンダープロジェクト；<br>
レンダーのため作られたC++プロジェクトです。<br>
Directx12を使用して、モジュール化で、DLLファイルを作成して、C＃のメインプロジェクトに各図形のAPI提供しています。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/d3dapi.jpg)<br>
ロード可能のモデルは、本「Directx12の魔導書」に教えられたPMDフォーマット、Boneデータ含めて、付属アニメ機能のVMD動画ファイルを使えるモデルです。<br>
もう一つ使うモデルは、自分が規範を設定した基本モデルです。Boneなど動画データはなくて、Cube、Sphereのような普通のモデルに使われる。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/vd1.jpg) ![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/vd2.jpg)<br>


Scene、Entity、Component：<br>
EntityはUnityのGameOjbectのようなものです。<br>
左のTreeViewはSceneの全てのEntity展示します。<br>
真ん中の画面はエディターカメラの画面で、３Dのモデル展示します。右マウス押したら、エディターカメラに回転、移動させられます。<br>
右のPanelは選択したEntityの全てのComponentを示す。見えるFieldはPublic、又は[MyAttributeShowInspector] 属性加えたのです。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/editor.jpg)<br>


ゲームプロジェクトの構築：<br>
MyEngineGroup中の数個プロジェクトはエンジンを作るためのです。<br>
エンジンを用いてゲームを作るのは、ゲームプロジェクトを作成しないとなりません。<br>
そしてエンジンの作業場所はゲームプロジェクトのディレクトリです。<br>
新しゲームプロジェクトを選択して、ディレクトリを指定すると、Assetsフォルダに基本なリソースファイルをコッピーします。<br>
Libraryフォルダ予約しました。現在どんなDLLファイルが必要がまだわかりません。以後プログラムに生成ビルドの機能実現するとき考えます。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/selectProject.jpg)<br>


Sceneデータの直列化：



SceneのRun、runtimeカメラの転換：



将来加える機能の展望：<br>

ゲームプロジェクト作成する途中、Visual Studio IDE開発環境も構築します。<br>
EntityにScriptを添付して、編集できます。<br>
こんな機能はVisual Studio の拡張機能知識が必要で、动的アセンブリの自動的に生成、使用も勉強するべきです。<br>


レンダーパイプラインはシェーダによって、自動的に構築や、<br>
グローバル光や、影などシェーダに簡単使われるなどシェーダー自動化ツール。<br>


既存のエンジン機能を改善すること。<br>


OpenGLレンダーモジュール追加する？<br>


Windowsプラットフォームに始め、プログラムに生成ビルド。<br>