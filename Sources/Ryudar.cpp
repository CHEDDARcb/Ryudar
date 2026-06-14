#include "Ryudar.h"

#include <DirectXCollision.h>
#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요
#include <tuple>
#include <vector>

#include "GeometryGenerator.h"

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

	/* 큐브매핑 준비 만들기*/
	m_cubeMapping.Initialize(m_device, L"./Assets/Textures/Cubemaps/Stonewall_diffuseIBL.dds",
	                         L"./Assets/Textures/Cubemaps/Stonewall_specularIBL.dds");

	/*물체만들기*/
	// 구
	MeshData sphere = GeometryGenerator::MakeSphere(1.3f, 100, 100);
	sphere.textureFilename = "./Assets/Textures/ojwD8.jpg";
	m_sphere.meshGroup.Initialize(m_device, {sphere});
	m_sphere.meshGroup.m_diffuseIBLSRV = m_cubeMapping.m_diffuseIBLSRV;
	m_sphere.meshGroup.m_specularIBLSRV = m_cubeMapping.m_specularIBLSRV;
	m_sphere.transform.translation = Vector3(0.0f, 0.3f, 3.0f);
	m_sphere.transform.scaling = Vector3(1.0f);

	// 캐릭터
	m_character.meshGroup.Initialize(m_device, "./Assets/Zelda/", "zeldaPosed001.fbx");
	m_character.meshGroup.m_diffuseIBLSRV = m_cubeMapping.m_diffuseIBLSRV;
	m_character.meshGroup.m_specularIBLSRV = m_cubeMapping.m_specularIBLSRV;
	m_character.transform.translation = Vector3(0.0f, 0.3f, 3.0f);
	m_character.transform.scaling = Vector3(1.8f);

	BuildFilters();

	return true;
}

void Ryudar::Update(float dt)
{
	// 렌더링할 물체 선택
	auto &selectedObject = GetSelectedObject();
	auto &visibleMeshGroup = selectedObject.meshGroup;

	// 카메라의 이동
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

	// 모델의 변환 DirectX-Row Major
	Matrix modelRow = selectedObject.transform.GetModelMatrix();

	// 노멀벡터용 InverseTranspose행렬
	auto invTransposeRow = modelRow;
	invTransposeRow.Translation(Vector3(0.0f)); // Translation을 0으로 만듬.
	invTransposeRow = invTransposeRow.Invert().Transpose();

	const auto selectedLightIndex = static_cast<std::size_t>(m_selectedLightType);
	// MeshGroup의 ConstantBuffers 업데이트
	for (std::size_t i = 0; i < MaxLights; ++i)
	{
		// 다른 조명 끄기
		if (i != selectedLightIndex)
		{
			visibleMeshGroup.m_lightingConstantData.lights[i].strength = Vector3(0.0f);
		}
		else
		{
			visibleMeshGroup.m_lightingConstantData.lights[i] = m_editableLight;
		}
	}

	// Row Major(DirectX)에서 Column major(HLSL)변환
	visibleMeshGroup.m_vertexConstantData.modelWorld = modelRow.Transpose();
	visibleMeshGroup.m_vertexConstantData.invTranspose = invTransposeRow.Transpose();
	visibleMeshGroup.m_vertexConstantData.view = viewRow.Transpose();
	visibleMeshGroup.m_vertexConstantData.projection = projRow.Transpose();

	visibleMeshGroup.m_lightingConstantData.eyeWorld = eyeWorld;

	visibleMeshGroup.UpdateConstantBuffers(m_device, m_context);

	// 큐브 매핑 Constant Buffer 업데이트
	m_cubeMapping.UpdateConstantBuffers(m_device, m_context, viewRow.Transpose(),
	                                    projRow.Transpose());

	if (m_postProcessConstantsDirty)
	{
		m_filters[0]->m_pixelConstData.threshold = m_bloomThreshold;
		m_filters[0]->UpdateConstantBuffers(m_device, m_context);
		m_filters.back()->m_pixelConstData.strength = m_bloomStrength;
		m_filters.back()->UpdateConstantBuffers(m_device, m_context);

		m_postProcessConstantsDirty = 0;
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

	/* 큐브매핑*/
	m_cubeMapping.Render(m_context);
	/*오브젝트*/
	GetSelectedObject().meshGroup.Render(m_context);

	// MSAA가 적용된 back buffer의 씬 렌더링 결과를
	// 후처리 셰이더가 읽을 수 있는 single-sample texture로 resolve한다.
	ComPtr<ID3D11Texture2D> backBuffer;
	m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	m_context->ResolveSubresource(m_postProcessInputTexture.Get(), 0, backBuffer.Get(), 0,
	                              DXGI_FORMAT_R8G8B8A8_UNORM);

	// 후처리 필터
	if (m_usePostProcessing)
	{
		for (auto &f : m_filters)
		{
			f->Render(m_context);
		}
	}
}

void Ryudar::OnResize()
{
	// 후처리 필터 다시 만들기
	BuildFilters();
	m_postProcessConstantsDirty = 1;
}

SceneObject &Ryudar::GetSelectedObject()
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
// 블룸필터
// 첫단계: 전체필셀 돌면서
// 픽셀의 평균값(RGB값의 평균->밝기)이 일정 값보다 작으면은
// 0으로 처리.
// 두번째 단계: 가우시안 블러처리.
// 세번째 단계: 원본이미지와 필터처리한 이미지를 합침.
void Ryudar::BuildFilters()
{
	m_filters.clear();

	// Bloom에 사용할 밝은 영역을 추출하면서 단계적으로 해상도를 낮춘다.
	// 첫 번째 down filter는 GUI threshold를 사용하고(Ryudar::Update()에서 조절),
	// 이후 sampling 필터는 threshold 0으로 통과시킨다.
	for (int down = 2; down <= m_downsampleCount; down *= 2)
	{
		auto downFilter = make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling",
		                                           m_screenWidth / down, m_screenHeight / down);

		if (down == 2)
		{
			downFilter->SetShaderResources({this->m_postProcessInputSRV});
		}
		else
		{
			downFilter->SetShaderResources({m_filters.back()->m_shaderResourceView});
		}
		downFilter->m_pixelConstData.threshold = 0.0f;
		downFilter->UpdateConstantBuffers(m_device, m_context);
		m_filters.push_back(downFilter);
	}

	// 가장 낮은 해상도부터 blur와 upsample을 반복해 bloom 이미지를 만든다.
	for (int down = m_downsampleCount; down >= 2; down /= 2)
	{
		// m_blurRepeatCount = 1;
		for (int i = 0; i < m_blurRepeatCount; i++)
		{
			// X축 blur 결과를 다음 필터의 입력으로 연결한다.
			auto &prevResource = m_filters.back()->m_shaderResourceView;
			m_filters.push_back(make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"BlurX",
			                                             m_screenWidth / down,
			                                             m_screenHeight / down));
			m_filters.back()->SetShaderResources({prevResource});

			// Y축 blur 결과를 다음 필터의 입력으로 연결한다.
			auto &prevResource2 = m_filters.back()->m_shaderResourceView;
			m_filters.push_back(make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"BlurY",
			                                             m_screenWidth / down,
			                                             m_screenHeight / down));
			m_filters.back()->SetShaderResources({prevResource2});
		}

		auto upFilter =
		    make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling",
		                             m_screenWidth / down * 2, m_screenHeight / down * 2);
		upFilter->SetShaderResources({m_filters.back()->m_shaderResourceView});
		upFilter->m_pixelConstData.threshold = 0.0f;
		upFilter->UpdateConstantBuffers(m_device, m_context);
		m_filters.push_back(upFilter);
	}

	// 원본 이미지와 bloom 이미지를 합성해 최종 결과를 back buffer에 출력한다.
	auto combineFilter = make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Combine",
	                                              m_screenWidth, m_screenHeight);
	combineFilter->SetShaderResources(
	    {m_postProcessInputSRV, m_filters.back()->m_shaderResourceView});
	combineFilter->SetRenderTargets({this->m_renderTargetView});
	combineFilter->m_pixelConstData.strength = m_bloomStrength;
	combineFilter->UpdateConstantBuffers(m_device, m_context);
	m_filters.push_back(combineFilter);
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
	auto &shading = meshGroup.m_renderSettings.shading;
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
	ImGui::Checkbox("Use Texture", &meshGroup.m_renderSettings.shading.useTexture);
	ImGui::Checkbox("Use Wireframe", &m_drawAsWire);
	ImGui::Checkbox("Draw Normals", &meshGroup.m_drawNormals);
	if (ImGui::SliderFloat("Normal scale", &meshGroup.m_normalVertexConstantData.scale, 0.0f, 1.0f))
	{
		meshGroup.m_drawNormalsDirtyFlag = true;
	}
	ImGui::NewLine();
}

void Ryudar::DrawPostProcessingGUI()
{
	ImGui::Text("Post Processing");
	ImGui::Checkbox("Use PostProc", &m_usePostProcessing);
	if (m_usePostProcessing)
	{
		m_postProcessConstantsDirty +=
		    ImGui::SliderFloat("Bloom Threshold", &m_bloomThreshold, 0.0f, 1.0f);
		m_postProcessConstantsDirty +=
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
	auto &material = meshGroup.m_renderSettings.material;

	ImGui::Text("Material");
	if (meshGroup.m_renderSettings.environment.useIBL)
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
	auto &environment = meshGroup.m_renderSettings.environment;

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

		// Point Light or Spot Light
		if (m_selectedLightType == LightType::Point || m_selectedLightType == LightType::Spot)
		{
			ImGui::SliderFloat3("Light Position", &m_editableLight.position.x, -5.0f, 5.0f);
			ImGui::SliderFloat("Light fallOffStart", &m_editableLight.fallOffStart, 0.0f, 5.0f);
			ImGui::SliderFloat("Light fallOffEnd", &m_editableLight.fallOffEnd, 0.0f, 10.0f);

			// Spot Light only
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
	auto &rimLight = meshGroup.m_renderSettings.rimLight;

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
