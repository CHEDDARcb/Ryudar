#include "Application/Ryudar.h"

#include <DirectXCollision.h>
#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요
#include <tuple>
#include <vector>

#include "Graphics/D3D11Exception.h"
#include "Geometry/GeometryGenerator.h"

namespace Ryudar
{

using namespace std;
using namespace DirectX;

std::ostream &operator<<(std::ostream &out, const DirectX::SimpleMath::Matrix &matrix)
{
	out << "{\n";
	out << "{" << matrix._11 << ", " << matrix._12 << ", " << matrix._13 << ", " << matrix._14
	    << "},\n";
	out << "{" << matrix._21 << ", " << matrix._22 << ", " << matrix._23 << ", " << matrix._24
	    << "},\n";
	out << "{" << matrix._31 << ", " << matrix._32 << ", " << matrix._33 << ", " << matrix._34
	    << "},\n";
	out << "{" << matrix._41 << ", " << matrix._42 << ", " << matrix._43 << ", " << matrix._44
	    << "}\n";
	out << "}";

	return out;
}

Ryudar::Ryudar()
    : AppBase()
{
}

bool Ryudar::Initialize()
{
	if (!AppBase::Initialize())
		return false;

	// 스카이박스와 IBL에 사용할 환경 맵을 준비한다.
	m_cubeMapping.Initialize(m_device.Get(),
	                         L"./Assets/Textures/Cubemaps/Stonewall_diffuseIBL.dds",
	                         L"./Assets/Textures/Cubemaps/Stonewall_specularIBL.dds");

	// 구와 캐릭터는 렌더 설정과 변환을 각각 독립적으로 유지한다.
	MeshData sphere = GeometryGenerator::MakeSphere(1.3f, 100, 100);
	sphere.textureFilename = "./Assets/Textures/ojwD8.jpg";
	m_sphere.meshGroup.Initialize(m_device.Get(), {sphere});
	m_sphere.meshGroup.SetEnvironmentMaps(m_cubeMapping.m_diffuseIBLSRV.Get(),
	                                      m_cubeMapping.m_specularIBLSRV.Get());
	m_sphere.transform.translation = Vector3(0.0f, 0.3f, 3.0f);
	m_sphere.transform.scaling = Vector3(1.0f);

	m_character.meshGroup.Initialize(m_device.Get(), "./Assets/Zelda/", "zeldaPosed001.fbx");
	m_character.meshGroup.SetEnvironmentMaps(m_cubeMapping.m_diffuseIBLSRV.Get(),
	                                         m_cubeMapping.m_specularIBLSRV.Get());
	m_character.transform.translation = Vector3(0.0f, 0.3f, 3.0f);
	m_character.transform.scaling = Vector3(1.8f);

	BuildFilters();

	return true;
}

void Ryudar::Update(float dt)
{
	auto &selectedObject = GetSelectedObject();
	auto &visibleMeshGroup = selectedObject.meshGroup;

	// 1인칭 시점이 꺼지면 데모의 고정 카메라 위치를 유지한다.
	if (m_useFirstPersonView)
	{
		if (m_keyPressed[87])
			m_camera.MoveForward(dt);
		if (m_keyPressed[83])
			m_camera.MoveForward(-dt);
		if (m_keyPressed[68])
			m_camera.MoveRight(dt);
		if (m_keyPressed[65])
			m_camera.MoveRight(-dt);
	}
	else
	{
		m_camera.ResetCameraSet();
	}

	Matrix viewRow = m_camera.GetViewRow();
	Matrix projRow = m_camera.GetProjRow();
	Vector3 eyeWorld = m_camera.GetEyePos();

	// DirectX의 행 벡터 기준 모델 행렬을 만든다.
	Matrix modelRow = selectedObject.transform.GetModelMatrix();

	// 비균등 크기 변환에서도 노멀 방향을 보존하도록 역전치 행렬을 사용한다.
	auto invTransposeRow = modelRow;
	invTransposeRow.Translation(Vector3(0.0f)); // 방향 벡터에는 이동을 적용하지 않는다.
	invTransposeRow = invTransposeRow.Invert().Transpose();

	const auto selectedLightIndex = static_cast<std::size_t>(m_selectedLightType);

	// HLSL 상수 버퍼의 행렬 배치에 맞춰 전치해 전달한다.
	visibleMeshGroup.SetTransformConstants(
	    modelRow.Transpose(), invTransposeRow.Transpose(), viewRow.Transpose(),
	    projRow.Transpose());
	visibleMeshGroup.SetActiveLight(selectedLightIndex, m_editableLight);
	visibleMeshGroup.SetEyePosition(eyeWorld);

	visibleMeshGroup.UpdateConstantBuffers(m_context.Get());

	// 스카이박스는 같은 카메라 뷰와 투영을 공유한다.
	m_cubeMapping.UpdateConstantBuffers(m_context.Get(), viewRow.Transpose(), projRow.Transpose());

	if (m_postProcessConstantsDirty)
	{
		m_filters[0]->SetThreshold(m_bloomThreshold);
		m_filters[0]->UpdateConstantBuffers(m_context.Get());
		m_filters.back()->SetStrength(m_bloomStrength);
		m_filters.back()->UpdateConstantBuffers(m_context.Get());

		m_postProcessConstantsDirty = false;
	}
}

void Ryudar::Render()
{
	SetViewport();

	float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.Get(),
	                                 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
	m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	if (m_drawAsWire)
	{
		m_context->RSSetState(m_wireRasterizerState.Get());
	}
	else
	{
		m_context->RSSetState(m_solidRasterizerState.Get());
	}

	m_cubeMapping.Render(m_context.Get());
	GetSelectedObject().meshGroup.Render(m_context.Get());

	// MSAA 백 버퍼를 후처리 셰이더가 읽을 수 있는 단일 샘플 텍스처로 리졸브한다.
	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
	              "Get swap-chain back buffer");
	m_context->ResolveSubresource(m_postProcessInputTexture.Get(), 0, backBuffer.Get(), 0,
	                              DXGI_FORMAT_R8G8B8A8_UNORM);

	if (m_usePostProcessing)
	{
		for (auto &f : m_filters)
		{
			f->Render(m_context.Get());
		}
	}
}

void Ryudar::OnResize()
{
	// 해상도에 의존하는 필터 출력 텍스처를 다시 만든다.
	BuildFilters();
	m_postProcessConstantsDirty = true;
}

SceneObject &Ryudar::GetSelectedObject() noexcept
{
	switch (m_selectedMeshType)
	{
	case MeshType::Sphere:
		return m_sphere;

	case MeshType::Character:
		return m_character;
	}

	assert(false && "Invalid MeshType");
	return m_sphere; // 컴파일러 경고 방지용, 절대 도달하지 않음
}
void Ryudar::BuildFilters()
{
	m_filters.clear();

	// 밝은 영역을 추출하면서 단계적으로 다운샘플링한다.
	// 첫 패스만 GUI 임계값을 사용하고 이후 패스는 모든 픽셀을 통과시킨다.
	for (int down = 2; down <= m_downsampleCount; down *= 2)
	{
		auto downFilter = std::make_unique<ImageFilter>(
		    m_device.Get(), FilterType::Sampling, m_screenWidth / down, m_screenHeight / down);

		if (down == 2)
		{
			downFilter->SetShaderResources({m_postProcessInputSRV.Get()});
		}
		else
		{
			downFilter->SetShaderResources({m_filters.back()->GetShaderResourceView()});
		}
		downFilter->SetThreshold(0.0f);
		downFilter->UpdateConstantBuffers(m_context.Get());
		m_filters.push_back(std::move(downFilter));
	}

	// 가장 낮은 해상도부터 블러와 업샘플링을 반복해 블룸 이미지를 만든다.
	for (int down = m_downsampleCount; down >= 2; down /= 2)
	{
		for (int i = 0; i < m_blurRepeatCount; i++)
		{
			// 가로 블러 결과를 다음 필터의 입력으로 연결한다.
			auto *prevResource = m_filters.back()->GetShaderResourceView();
			m_filters.push_back(
			    std::make_unique<ImageFilter>(m_device.Get(), FilterType::BlurHorizontal,
			                                  m_screenWidth / down, m_screenHeight / down));
			m_filters.back()->SetShaderResources({prevResource});

			// 세로 블러 결과를 다음 필터의 입력으로 연결한다.
			auto *prevResource2 = m_filters.back()->GetShaderResourceView();
			m_filters.push_back(
			    std::make_unique<ImageFilter>(m_device.Get(), FilterType::BlurVertical,
			                                  m_screenWidth / down, m_screenHeight / down));
			m_filters.back()->SetShaderResources({prevResource2});
		}

		auto upFilter = std::make_unique<ImageFilter>(
		    m_device.Get(), FilterType::Sampling, m_screenWidth / down * 2,
		    m_screenHeight / down * 2);
		upFilter->SetShaderResources({m_filters.back()->GetShaderResourceView()});
		upFilter->SetThreshold(0.0f);
		upFilter->UpdateConstantBuffers(m_context.Get());
		m_filters.push_back(std::move(upFilter));
	}

	// 원본 이미지와 블룸 이미지를 합성해 백 버퍼에 출력한다.
	auto combineFilter = std::make_unique<ImageFilter>(
	    m_device.Get(), FilterType::Combine, m_screenWidth, m_screenHeight);
	combineFilter->SetShaderResources(
	    {m_postProcessInputSRV.Get(), m_filters.back()->GetShaderResourceView()});
	combineFilter->SetRenderTargets({m_renderTargetView.Get()});
	combineFilter->SetStrength(m_bloomStrength);
	combineFilter->UpdateConstantBuffers(m_context.Get());
	m_filters.push_back(std::move(combineFilter));
}

void Ryudar::DrawMeshSelectorGUI()
{
	if (ImGui::RadioButton("Sphere", m_selectedMeshType == MeshType::Sphere))
	{
		m_selectedMeshType = MeshType::Sphere;
	}

	ImGui::SameLine();

	if (ImGui::RadioButton("Character", m_selectedMeshType == MeshType::Character))
	{
		m_selectedMeshType = MeshType::Character;
	}
}

void Ryudar::DrawShadingModeGUI(ClassicLit::MeshGroup &meshGroup)
{
	auto &shading = meshGroup.GetRenderSettings().shading;
	const char *items[] = {"Phong Shading", "Blinn-Phong Shading"};
	int currentItem = static_cast<int>(shading.model);

	if (ImGui::Combo("Shading Option", &currentItem, items, IM_ARRAYSIZE(items)))
	{
		shading.model = static_cast<ClassicLit::ShadingModel>(currentItem);
	}
}

void Ryudar::DrawRenderOptionsGUI(ClassicLit::MeshGroup &meshGroup)
{
	DrawShadingModeGUI(meshGroup);
	ImGui::Checkbox("Use Texture", &meshGroup.GetRenderSettings().shading.useTexture);
	ImGui::Checkbox("Use Wireframe", &m_drawAsWire);

	bool drawNormals = meshGroup.GetDrawNormals();
	if (ImGui::Checkbox("Draw Normals", &drawNormals))
	{
		meshGroup.SetDrawNormals(drawNormals);
	}

	float normalScale = meshGroup.GetNormalScale();
	if (ImGui::SliderFloat("Normal scale", &normalScale, 0.0f, 1.0f))
	{
		meshGroup.SetNormalScale(normalScale);
	}
	ImGui::NewLine();
}

void Ryudar::DrawPostProcessingGUI()
{
	ImGui::Text("Post Processing");
	ImGui::Checkbox("Use PostProc", &m_usePostProcessing);
	if (m_usePostProcessing)
	{
		m_postProcessConstantsDirty |=
		    ImGui::SliderFloat("Bloom Threshold", &m_bloomThreshold, 0.0f, 1.0f);
		m_postProcessConstantsDirty |=
		    ImGui::SliderFloat("Bloom Strength", &m_bloomStrength, 0.0f, 3.0f);
	}
	ImGui::NewLine();
}

void Ryudar::DrawModelGUI(Transform &transform)
{
	ImGui::Text("Model");

	ImGui::SliderFloat3("Translation", &transform.translation.x, -3.0f, 3.0f);

	ImGui::SliderFloat3("Rotation (Rad)", &transform.rotation.x, -3.13f, 3.14f);

	ImGui::SliderFloat3("Scaling", &transform.scaling.x, 0.1f, 2.0f);
}

void Ryudar::DrawMaterialGUI(ClassicLit::MeshGroup &meshGroup)
{
	auto &settings = meshGroup.GetRenderSettings();
	auto &material = settings.material;

	ImGui::Text("Material");
	if (settings.environment.useIBL)
	{
		ImGui::SliderFloat3("Material FresnelR0", &material.fresnelR0.x, 0.0f, 1.0f);
	}

	float diffuse = (material.diffuse.x + material.diffuse.y + material.diffuse.z) / 3.0f;
	if (ImGui::SliderFloat("Material Diffuse", &diffuse, 0.0f, 3.0f))
	{
		material.diffuse = Vector3(diffuse);
	}

	float specular = (material.specular.x + material.specular.y + material.specular.z) / 3.0f;
	if (ImGui::SliderFloat("Material Specular", &specular, 0.0f, 3.0f))
	{
		material.specular = Vector3(specular);
	}

	ImGui::SliderFloat("Material Shininess", &material.shininess, 0.01f, 20.0f);
	ImGui::NewLine();
}

void Ryudar::DrawLightGUI(ClassicLit::MeshGroup &meshGroup)
{
	auto &environment = meshGroup.GetRenderSettings().environment;

	ImGui::Text("Light");
	ImGui::Checkbox("Use Image Based Light", &environment.useIBL);
	if (!environment.useIBL)
	{
		if (ImGui::RadioButton("Directional Light", m_selectedLightType == LightType::Directional))
		{
			m_selectedLightType = LightType::Directional;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Point Light", m_selectedLightType == LightType::Point))
		{
			m_selectedLightType = LightType::Point;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Spot Light", m_selectedLightType == LightType::Spot))
		{
			m_selectedLightType = LightType::Spot;
		}

		// 점광원과 스포트라이트가 공유하는 감쇠 범위를 편집한다.
		if (m_selectedLightType == LightType::Point || m_selectedLightType == LightType::Spot)
		{
			ImGui::SliderFloat3("Light Position", &m_editableLight.position.x, -5.0f, 5.0f);
			ImGui::SliderFloat("Light fallOffStart", &m_editableLight.fallOffStart, 0.0f, 5.0f);
			ImGui::SliderFloat("Light fallOffEnd", &m_editableLight.fallOffEnd, 0.0f, 10.0f);

			// 스포트라이트에서만 원뿔 집중도를 편집한다.
			if (m_selectedLightType == LightType::Spot)
			{
				ImGui::SliderFloat("Light spotPower", &m_editableLight.spotPower, 1.0f, 512.0f);
			}
		}

		DrawRimLightGUI(meshGroup);
	}
	else
	{
		ImGui::Checkbox("Use Environment Reflection", &environment.useEnvironmentReflection);
	}
}

void Ryudar::DrawRimLightGUI(ClassicLit::MeshGroup &meshGroup)
{
	auto &rimLight = meshGroup.GetRenderSettings().rimLight;

	ImGui::Checkbox("Use Rim Light", &rimLight.useRimLight);
	if (rimLight.useRimLight)
	{
		ImGui::SliderFloat("Rim Strength", &rimLight.rimStrength, 0.0f, 10.0f);
		ImGui::Checkbox("Use Smoothstep", &rimLight.useSmoothstep);
		ImGui::SliderFloat3("Rim Color", &rimLight.rimColor.x, 0.0f, 1.0f);
		ImGui::SliderFloat("Rim Power", &rimLight.rimPower, 0.01f, 10.0f);
	}
}

void Ryudar::UpdateGUI()
{
	ImGui::Checkbox("Use FPV", &m_useFirstPersonView);

	DrawMeshSelectorGUI();

	auto &selectedObject = GetSelectedObject();
	auto &meshGroup = selectedObject.meshGroup;

	DrawRenderOptionsGUI(meshGroup);
	DrawPostProcessingGUI();
	DrawModelGUI(selectedObject.transform);
	DrawMaterialGUI(meshGroup);
	DrawLightGUI(meshGroup);
}

} // namespace Ryudar
