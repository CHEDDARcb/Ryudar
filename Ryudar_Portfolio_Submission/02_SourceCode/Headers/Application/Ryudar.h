#pragma once
// プロジェクトのシーンを実装するAppBase派生クラス。
// Mesh選択、Light/Material GUI、Cubemap、Bloom Post Processチェーンを管理する。

#include <algorithm>
#include <iostream>
#include <memory>

#include "Application/AppBase.h"
#include "Rendering/ClassicLit/ClassicLitMeshGroup.h"
#include "Rendering/CubeMapping.h"
#include "Geometry/GeometryGenerator.h"
#include "Rendering/ImageFilter.h"
#include "Scene/Light.h"
#include "Scene/SceneObject.h"

namespace Ryudar
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

enum class MeshType
{
	Sphere,
	Character,
};

class Ryudar : public AppBase
{
public:
	Ryudar();

	// シーンに必要なMesh、Cubemap、Post Process Filterを初期化する。
	virtual bool Initialize() override;

	// ImGuiウィンドウを構築し、現在選択中のMesh編集UIを描画する。
	virtual void UpdateGUI() override;

	// 入力とGUI状態を反映し、Camera、Light、Model、Constant Bufferを更新する。
	virtual void Update(float dt) override;

	// Cubemap、選択中のMesh、Post Process Filterチェーンの順にレンダリングする。
	virtual void Render() override;

	// ウィンドウサイズ変更時に画面サイズ依存のPost Process Filterを再生成する。
	virtual void OnResize() override;

protected:
	// 現在選択中のSceneObjectへの参照を返す。
	SceneObject &GetSelectedObject() noexcept;
	// 現在のBack Bufferサイズに合わせてBloom Filterチェーンを構築する。
	void BuildFilters();

	// レンダリング対象のMeshを選択するGUI。
	void DrawMeshSelectorGUI();

	// Phong/Blinn-PhongなどのShading Modelを選択するGUI。
	void DrawShadingModeGUI(ClassicLit::MeshGroup &meshGroup);

	// Texture、Wireframe、Normal可視化などのレンダリングオプションGUI。
	void DrawRenderOptionsGUI(ClassicLit::MeshGroup &meshGroup);

	// Bloomの有効化、Threshold、Strengthを調整するPost Process GUI。
	void DrawPostProcessingGUI();

	// 選択中Modelの位置、回転、Scaleを調整するGUI。
	void DrawModelGUI(Transform &transform);

	// 選択中MeshのMaterial値を調整するGUI。
	void DrawMaterialGUI(ClassicLit::MeshGroup &meshGroup);

	// Direct LightとIBL/Environment Reflectionオプションを調整するGUI。
	void DrawLightGUI(ClassicLit::MeshGroup &meshGroup);

	// Rim Lightの色と強度を調整するGUI。
	void DrawRimLightGUI(ClassicLit::MeshGroup &meshGroup);

protected:
	// シーンリソース
	SceneObject m_sphere;
	SceneObject m_character;
	CubeMapping m_cubeMapping;

	// Light GUI状態
	LightType m_selectedLightType = LightType::Directional;
	Light m_editableLight;

	// Mesh選択状態
	MeshType m_selectedMeshType = MeshType::Sphere;

	// Post Process Filterチェーン
	std::vector<std::unique_ptr<ImageFilter>> m_filters;

	// Bloom設定
	bool m_postProcessConstantsDirty = true; // 起動時にConstant Bufferを一度更新する。
	int m_downsampleCount = 16;
	int m_blurRepeatCount = 5;
	float m_bloomThreshold = 1.0f;
	float m_bloomStrength = 1.0f;
};
} // namespace Ryudar
