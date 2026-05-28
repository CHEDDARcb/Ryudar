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
	BasicMeshGroup m_meshGroupGround;
	CubeMapping m_cubeMapping;

	bool m_usePerspectiveProjection = true;
	Vector3 m_modelTranslation = Vector3(0.0f, 0.3f, 3.0f);
	Vector3 m_modelRotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_modelScaling = Vector3(1.0f);
	Vector3 m_viewRot = Vector3(0.0f);

	float m_projFovAngleY = 70.0f;
	float m_nearZ = 0.01f;
	float m_farZ = 100.0f;

	int m_lightType = 0;
	Light m_lightFromGUI;

	int m_visibleMeshIndex = 0; // Sphere, character.

	std::vector<shared_ptr<ImageFilter>> m_filters;

	int m_dirtyflag = 1; // 처음에 한 번 실행
	int m_down = 16;
	int m_repeat = 5;
	float m_threshold = 1.0f;
	float m_strength = 1.0f;
};
} // namespace Ryudar
