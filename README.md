個人作品の紹介：<br>
.net Windows Formの基づくエンジン。<br>
UnityEngineの使い方参考して作っています。<br>
ゲームエンジンの基礎技術の学習、および求職の展示を目的として作られます。<br>

枠組み：<br>
二つのプロジェクトある、エンジンと３Dレンダーです。<br>
他の第三者サードパーティのロードテクスチャや、基本モデル作るプロジェクトも補助的な使用する。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/frame.jpg)

エンジン・プロジェクトはエンジン・コアとエンジンエディタと分ける。<br>
エンジン・コアは基本なゲイムサイクル、レンダーのAPIを呼び出す、Entity、Componentなどの機能を実現する部分です。<br>
エンジン・エディタはゲーム・プロジェクトにグラフィカルユーザーインターフェースの編集部分です。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/editor.jpg)<br>



実装された機能と学習体験：<br>

３Dレンダー・プロジェクト；<br>
レンダーのため作られたC++プロジェクトです。<br>
Directx12を使用して、各機能をモジュール化で、DLLファイルを作成して、C＃のメイン・プロジェクトに各図形のAPI提供しています。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/d3dapi.jpg)<br>
ロード可能のモデルは、本「Directx12の魔導書」に教えられたPMDフォーマット、Boneデータ含めて、付属アニメ機能のVMD動画ファイルを使えるモデルです。<br>
もう一つ使うモデルは、自分が規範を設定した基本モデルです。Boneなど動画データはなくて、Cube、Sphereのような普通のモデルに使われる。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/vd1.jpg) ![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/vd2.jpg)<br>


Scene、Entity、Component：<br>
EntityはUnityのGameOjbectのようなものです。<br>
左のTreeViewはSceneの全てのEntity展示します。<br>
真ん中の画面はエディターカメラの画面で、３Dのモデル展示します。右マウス押したら、エディターカメラに回転、移動させられます。<br>
右のPanelは選択したEntityの全てのComponentを示す。見えるFieldはPublic、又は[MyAttributeShowInspector] 属性加えたのです。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/EntityAndComponent.jpg)<br>


ゲームプロジェクトの構築：<br>
MyEngineGroup中の数個プロジェクトはエンジンを作るためのです。<br>
エンジンを用いてゲームを作るのは、ゲーム・プロジェクトを作成しないとなりません。<br>
そしてエンジンの作業場所はゲーム・プロジェクトのディレクトリです。<br>
新しゲーム・プロジェクトを選択して、ディレクトリを指定すると、Assetsフォルダに基本なリソースファイルをコッピーします。<br>
Libraryフォルダ予約しました。現在どんなDLLファイルが必要がまだわかりません。以後プログラムに生成ビルドの機能実現するとき考えます。<br>
![describe1](https://github.com/ckfckf0730/MyEngineGroup/blob/main/ReadMe/selectProject.jpg)<br>


Sceneデータの直列化：<br>
シーン・データは今主にシーンに置いた全てのEntityの配列とメーン・カメラの情報を持っています。<br>
直列化の道具はJsonです。<br>
Entityのクラスは他のクラスのメンバーが持って、Jsonで直接直列化ができません<br>

そしてEntitySerializeと言う、直列化のためクラスを作りました。<br>
EntitySerializeがEntityの直接直列化できるメンバーをコピーして、直接直列化できないメンバー（componentのDictionary）、<br>
各componentが第一回の直列化してから、派生クラス名前と生成したJsonテキストとを<string,string>の容器にいれます。<br>
シーン・データがセーブする前、全てのEntityをEntitySerialize転換して、<br>
そしてそのEntitySerializeのリストを持つシーンを第二回の直列化ができます。<br>

逆直列化も同じ、第一回逆直列化EntitySerializeがもらえる。<br>
EntitySerializeが第二回逆直列化で、派生クラスに各componentのオブジェクトを構築します。<br>
そして各componentもう一度Entityにバインディングして、Entity復元ができました。<br>


SceneのRun、runtimeカメラの転換：<br>


ゲーム・プロジェクトVisual Studio開発環境の構築とScriptsのコンパイル；
ゲーム・プロジェクト作成する途中、Visual Studio開発環境の.slnと.csprojファイル作ります。<br>
ゲーム・プロジェクト編集の便利さのため、EngineのAPIの提供が必要です。
AssemblyBuilderを使って、Engineのdllファイルを作ります。
最後criptsのコンパイルがEngineから実行するので、Engineのdllは各interfaceだけ提供して、
その関数の内容は実現しなくても良いです。

criptsのコンパイルが直後、コードで、またEditor部分もcomponentのUIを構築して、Entityに添付できる。
これでEngineを使って、最初のゲームが作られる。（でもゲームのBuildは今できません）



将来加える機能の展望：<br>

レンダーパイプラインはシェーダによって、自動的に構築や、<br>
グローバル光や、影などシェーダに簡単使われるなどシェーダー自動化ツール。<br>


既存のエンジン機能を改善すること。<br>


OpenGLレンダーモジュール追加する？<br>


Windowsプラットフォームに始め、プログラムに生成ビルド。<br>