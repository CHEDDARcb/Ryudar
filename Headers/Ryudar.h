#pragma once

#include <algorithm>
#include <iostream>
#include <memory>

#include "AppBase.h"
#include "BasicMeshGroup.h"
#include "CubeMapping.h"
#include "GeometryGenerator.h"
#include "ImageFilter.h"
#include "Light.h"

namespace Ryudar
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

class Ryudar : public AppBase
{
public:
	Ryudar();

	virtual bool Initialize() override;
	virtual void UpdateGUI() override;
	virtual void Update(float dt) override;
	virtual void Render() override;

	virtual void OnResize() override;

	void BuildFilters();

protected:
	void DrawMeshSelectorGUI();
	void DrawShadingModeGUI(BasicMeshGroup &meshGroup);
	void DrawRenderOptionsGUI(BasicMeshGroup &meshGroup);
	void DrawPostProcessingGUI();
	void DrawModelGUI();
	void DrawMaterialGUI(BasicMeshGroup &meshGroup);
	void DrawLightGUI(BasicMeshGroup &meshGroup);
	void DrawRimLightGUI(BasicMeshGroup &meshGroup);

	BasicMeshGroup m_meshGroupSphere;
	BasicMeshGroup m_meshGroupCharacter;
	CubeMapping m_cubeMapping;

	Vector3 m_modelTranslation = Vector3(0.0f, 0.3f, 3.0f);
	Vector3 m_modelRotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_modelScaling = Vector3(1.0f);

	int m_selectedLightType = 0;
	Light m_editableLight;

	int m_selectedMeshIndex = 0; // Sphere, character.
	bool m_meshSelectionChanged = false;

	std::vector<shared_ptr<ImageFilter>> m_filters;

	int m_postProcessConstantsDirty = 1; // 처음에 한 번 실행
	int m_downsampleCount = 16;
	int m_blurRepeatCount = 5;
	float m_bloomThreshold = 1.0f;
	float m_bloomStrength = 1.0f;
};
} // namespace Ryudar
