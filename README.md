# Ryudar 2.0

Ryudar 2.0 は、既存の Ryudar 1.0 をベースに、よりゲームエンジンらしい構造へリファクタリングするための開発ブランチです。

Ryudar 1.0 では、DirectX 11 を用いたリアルタイム 3D レンダリング、Lighting、Material、Cubemap、IBL、Bloom、ImGui による操作 UI などを 1 つのレンダリングアプリケーションとして実装しました。

Ryudar 2.0 では、その実装を維持しながら、アプリケーション固有の処理とエンジン基盤を分離し、今後の拡張に耐えられる構造へ整理していきます。

## 動画

全体の動作確認用動画は以下から確認できます。

[![Ryudar demo video](https://img.youtube.com/vi/rWNu4yBpQ5Q/maxresdefault.jpg)](https://youtu.be/rWNu4yBpQ5Q)

## 主な機能

### Shading Model

![Shading model demo](https://github.com/user-attachments/assets/223ae696-c540-4e4b-9bd2-235eb8daddcb)

Phong と Blinn-Phong を切り替え、specular highlight の計算モデルを比較できます。

### Texture Toggle

![Texture toggle demo](https://github.com/user-attachments/assets/ebbdcdae-36cb-4e5d-a9fe-ea4def72e0ec)

Texture sampling の有効 / 無効を切り替え、material color と texture 適用結果を確認できます。

### Debug View

![Debug view demo](https://github.com/user-attachments/assets/950a5a5b-f78d-4261-8ad7-84060b6fadbc)

Normal 可視化と wireframe 表示で、mesh の向きや形状を確認できます。

### Object Transform / Selection

![Object transform demo](https://github.com/user-attachments/assets/d5e9f9c8-4d56-4997-b5c7-7304fe75fc56)

選択中の object に対して Translation、Rotation、Scale を個別に適用できます。

### Material Parameters

![Material parameters demo](https://github.com/user-attachments/assets/56540391-4165-434e-9925-185d2e546b2f)

Diffuse、Specular、Shininess を調整し、材質の反射特性を変更できます。

### Light Type

![Light type demo](https://github.com/user-attachments/assets/59c4e298-c683-4bb8-9911-ffc5ecfac839)

Directional、Point、Spot Light を切り替え、light type ごとの照明結果を確認できます。

### Rim Light

![Rim light demo](https://github.com/user-attachments/assets/4167d10f-6d2a-4c01-a10d-843619a4561c)

Rim Light により、object の輪郭付近を強調できます。

### Image Based Lighting

![Image based lighting demo](https://github.com/user-attachments/assets/4e0cba32-80c3-4a88-87a0-32b6177cc9e1)

IBL により、environment map を使った diffuse / specular lighting を適用できます。

### Post Processing

![Post processing demo](https://github.com/user-attachments/assets/5071617f-efec-4452-848c-ccf892af26b7)

Bloom の threshold、strength、blur を調整し、画面全体の発光表現を制御できます。

### First-Person Camera

![First-person camera demo](https://github.com/user-attachments/assets/347152c2-b8c0-46d4-a8db-ac5c6c442e2c)

WASD と mouse 操作で、scene 内を First-Person Camera として移動できます。

## 目的

Ryudar 2.0 の目的は、Ryudar 1.0 の機能を単なるデモアプリケーションとしてではなく、再利用可能なゲームエンジン風アーキテクチャへ移行することです。

主な方針は以下です。

- Application と Engine の責務を分離する
- Scene、Object、Component、Renderer の関係を整理する
- Rendering pipeline をより明確な単位へ分割する
- Material、Mesh、Texture、Shader などの resource 管理を整理する
- Post Process を独立した pipeline として扱いやすくする
- GUI 操作と engine runtime state の境界を明確にする
- 将来的な scene 拡張、複数 object 管理、renderer 差し替えに備える

## ベースライン

Ryudar 1.0 の基準点は Git tag として保存しています。

```text
ryudar-v1.0
```

Ryudar 1.0 の状態を確認したい場合は、以下で参照できます。

```bash
git switch --detach ryudar-v1.0
```

再び Ryudar 2.0 の開発ブランチへ戻る場合は以下を使用します。

```bash
git switch ryudar-2.0-engine-refactor
```

## 開発ブランチ

Ryudar 2.0 のリファクタリング作業は、以下のブランチで進めます。

```text
ryudar-2.0-engine-refactor
```

このブランチは `main` から分岐しています。

## リファクタリング予定

現時点で想定している主な変更は以下です。

- `AppBase` の役割を engine runtime と application layer に分ける
- `Ryudar` に集中している scene 初期化、GUI、render flow を分割する
- `SceneObject` をより拡張しやすい scene entity として整理する
- `ClassicLit::MeshGroup` の責務を renderer、mesh resource、material state に分ける
- `CubeMapping` を environment / skybox resource として扱いやすくする
- `ImageFilter` と Bloom 構築処理を post-process pipeline として整理する
- `D3D11Utils` の低レベル API helper と engine-level resource creation を分ける
- ドキュメントを Ryudar 2.0 の構造に合わせて更新する

## 現在の状態

この README は Ryudar 2.0 リファクタリング開始時点の案内です。

今後、実装の分割が進むにつれて、以下の内容を更新していきます。

- 新しいフォルダー構成
- Engine / Application の境界
- Scene 管理方式
- Rendering pipeline
- Resource 管理方式
- Build / Run 手順

