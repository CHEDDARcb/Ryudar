#pragma once
// 프로젝트의 실제 데모 씬을 구현하는 AppBase 파생 클래스.
// 메쉬 선택, 조명/재질 GUI, 큐브맵, 블룸 후처리 체인을 관리한다.

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

	// 씬에 필요한 메쉬, 큐브맵, 후처리 필터를 초기화한다.
	virtual bool Initialize() override;

	// ImGui 창을 구성하고 현재 선택된 메쉬의 편집 UI를 그린다.
	virtual void UpdateGUI() override;

	// 입력과 GUI 상태를 반영해 카메라, 조명, 모델, 상수 버퍼를 갱신한다.
	virtual void Update(float dt) override;

	// 큐브맵, 선택된 메쉬, 후처리 필터 체인을 순서대로 렌더링한다.
	virtual void Render() override;

	// 창 크기 변경 시 화면 크기에 의존하는 후처리 필터를 다시 만든다.
	virtual void OnResize() override;

protected:
	// 현재 back buffer 크기에 맞춰 블룸 후처리 필터 체인을 구성한다.
	void BuildFilters();

	// 렌더링할 메쉬를 선택하는 GUI.
	void DrawMeshSelectorGUI();

	// Phong/Blinn-Phong 등 셰이딩 방식을 선택하는 GUI.
	void DrawShadingModeGUI(BasicMeshGroup &meshGroup);

	// 텍스처, 와이어프레임, 노멀 시각화 같은 렌더링 옵션 GUI.
	void DrawRenderOptionsGUI(BasicMeshGroup &meshGroup);

	// 블룸 사용 여부와 threshold/strength를 조절하는 후처리 GUI.
	void DrawPostProcessingGUI();

	// 선택된 모델의 위치, 회전, 크기를 조절하는 GUI.
	void DrawModelGUI();

	// 선택된 메쉬의 재질 값을 조절하는 GUI.
	void DrawMaterialGUI(BasicMeshGroup &meshGroup);

	// 기본 조명과 IBL/환경 반사 옵션을 조절하는 GUI.
	void DrawLightGUI(BasicMeshGroup &meshGroup);

	// 림 라이트 색상과 강도를 조절하는 GUI.
	void DrawRimLightGUI(BasicMeshGroup &meshGroup);

	// 씬 리소스
	BasicMeshGroup m_meshGroupSphere;
	BasicMeshGroup m_meshGroupCharacter;
	CubeMapping m_cubeMapping;

	// GUI에서 조절하는 모델 변환값
	Vector3 m_modelTranslation = Vector3(0.0f, 0.3f, 3.0f);
	Vector3 m_modelRotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_modelScaling = Vector3(1.0f);

	// 조명 GUI 상태
	int m_selectedLightType = 0;
	Light m_editableLight;

	// 메쉬 선택 상태
	int m_selectedMeshIndex = 0; // Sphere, character.
	bool m_meshSelectionChanged = false;

	// 후처리 필터 체인
	std::vector<shared_ptr<ImageFilter>> m_filters;

	// 블룸 설정
	int m_postProcessConstantsDirty = 1; // 처음에 한 번 실행
	int m_downsampleCount = 16;
	int m_blurRepeatCount = 5;
	float m_bloomThreshold = 1.0f;
	float m_bloomStrength = 1.0f;
};
} // namespace Ryudar
