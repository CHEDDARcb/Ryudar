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

	/*바닥만들기*/
	// MeshData ground = GeometryGenerator::MakeSquare(2.0f);
	// ground.textureFilename = "./Assets/Textures/blender_uv_grid_2k.png";
	// m_meshGroupGround.Initialize(m_device, {ground});
	// m_meshGroupGround.m_diffuseResView = m_cubeMapping.m_diffuseResView;
	// m_meshGroupGround.m_specularResView = m_cubeMapping.m_specularResView;

	/*물체만들기*/
	// 구
	MeshData sphere = GeometryGenerator::MakeSphere(1.3f, 100, 100);
	sphere.textureFilename = "ojwD8.jpg";
	m_meshGroupSphere.Initialize(m_device, {sphere});
	m_meshGroupSphere.m_diffuseResView = m_cubeMapping.m_diffuseResView;
	m_meshGroupSphere.m_specularResView = m_cubeMapping.m_specularResView;

	// 캐릭터
	m_meshGroupCharacter.Initialize(m_device, "./Assets/Zelda/", "zeldaPosed001.fbx");
	m_meshGroupCharacter.m_diffuseResView = m_cubeMapping.m_diffuseResView;
	m_meshGroupCharacter.m_specularResView = m_cubeMapping.m_specularResView;

	BuildFilters();

	return true;
}

static int changedMesh = 0;

void Ryudar::Update(float dt)
{
	// 렌더링할 물체 선택
	auto &visibleMeshGroup = m_visibleMeshIndex == 0 ? m_meshGroupSphere : m_meshGroupCharacter;

	// 모델일 경우, 위치, 크기 조정
	if (changedMesh == 1)
	{
		m_modelTranslation = Vector3(0.0f, 0.3f, 3.0f);
		m_modelScaling = m_visibleMeshIndex == 0 ? Vector3(1.0f) : Vector3(1.8f);
		changedMesh = 0;
	}

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

	// std::cout << m_camera.GetEyePos().x << ", " << m_camera.GetEyePos().y << ", "
	//           << m_camera.GetEyePos().z << std::endl;

	Matrix viewRow = m_camera.GetViewRow();
	Matrix projRow = m_camera.GetProjRow();
	Vector3 eyeWorld = m_camera.GetEyePos();

	// 모델의 변환 DirectX-Row Major
	auto modelRow =
	    Matrix::CreateScale(m_modelScaling) * Matrix::CreateRotationY(m_modelRotation.y) *
	    Matrix::CreateRotationX(m_modelRotation.x) * Matrix::CreateRotationZ(m_modelRotation.z) *
	    Matrix::CreateTranslation(m_modelTranslation);

	// 노멀벡터용 InverseTranspose행렬
	auto invTransposeRow = modelRow;
	invTransposeRow.Translation(Vector3(0.0f)); // Translation을 0으로 만듬.
	invTransposeRow = invTransposeRow.Invert().Transpose();

	/*카메라 클래스로 빼냄*/
	// View행렬
	// auto viewRow = Matrix::CreateRotationY(m_viewRot.y) *
	//                Matrix::CreateRotationX(m_viewRot.x) *
	//                Matrix::CreateTranslation(0.0f, 0.0f, 2.0f);
	//
	// // Projection 행렬
	// const float aspect = AppBase::GetAspectRatio();
	// Matrix projRow =
	//     m_usePerspectiveProjection
	//         ? XMMatrixPerspectiveFovLH(
	//             XMConvertToRadians(m_projFovAngleY), aspect, m_nearZ, m_farZ)
	//         : XMMatrixOrthographicOffCenterLH(
	//             -aspect, aspect, -1.0f, 1.0f, m_nearZ, m_farZ);
	//
	// //(0., 0, 0)의 카메라좌표계의 시점의 위치를 월드좌표계로 변경시켜주시기위해,
	// // 기존의 월드좌표계->카메라좌표계로 변환시켜주는 행렬을,
	// // 카메라 좌표계->월드 좌표계로 고쳐주기위해 행렬을 역행렬(역연산)시킴
	// auto eyeWorld = Vector3::Transform(Vector3(0.0f), viewRow.Invert());

	// MeshGroup의 ConstantBuffers 업데이트
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		// 다른 조명 끄기
		if (i != m_lightType)
		{
			visibleMeshGroup.m_basicPixelConstantData.lights[i].strength *= 0.0f;
		}
		else
		{
			visibleMeshGroup.m_basicPixelConstantData.lights[i] = m_lightFromGUI;
		}
	}

	// Row Major(DirectX)에서 Column major(HLSL)변환
	visibleMeshGroup.m_basicVertexConstantData.modelWorld = modelRow.Transpose();
	visibleMeshGroup.m_basicVertexConstantData.invTranspose = invTransposeRow.Transpose();
	visibleMeshGroup.m_basicVertexConstantData.view = viewRow.Transpose();
	visibleMeshGroup.m_basicVertexConstantData.projection = projRow.Transpose();

	visibleMeshGroup.m_basicPixelConstantData.eyeWorld = eyeWorld;

	visibleMeshGroup.UpdateConstantBuffers(m_device, m_context);

	// 큐브 매핑 Constant Buffer 업데이트
	// m_cubeMapping.UpdateConstantBuffers(
	//     m_device, m_context, (Matrix::CreateRotationY(m_viewRot.y)*
	//                           Matrix::CreateRotationX(m_viewRot.x)).Transpose(),
	//     projRow.Transpose());
	m_cubeMapping.UpdateConstantBuffers(m_device, m_context, viewRow.Transpose(),
	                                    projRow.Transpose());

	if (m_dirtyflag)
	{
		m_filters[1]->m_pixelConstData.threshold = m_threshold;
		m_filters[1]->UpdateConstantBuffers(m_device, m_context);
		m_filters.back()->m_pixelConstData.strength = m_strength;
		m_filters.back()->UpdateConstantBuffers(m_device, m_context);

		m_dirtyflag = 0;
	}
}

void Ryudar::Render()
{
	// IA: Input-Assembler stage
	// VS: Vertex Shader
	// PS: Pixel Shader
	// RS: Rasterizer stage
	// OM: Output-Merger stage

	// m_context->RSSetViewports(1, &m_screenViewport);
	// Sleep(100);

	SetViewport();

	float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
	m_context->ClearDepthStencilView(m_depthStencilView.Get(),
	                                 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// DepthBuffer를 사용하지 않는 경우
	// m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), NULL);
	m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
	m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	// Rasterizer
	if (m_drawAsWire)
	{
		m_context->RSSetState(m_wireRasterizerState.Get());
	}
	else
	{
		m_context->RSSetState(m_solidRasterizerState.Get());
	}

	// m_meshGroupGround.Render(m_context);
	/* 큐브매핑*/
	m_cubeMapping.Render(m_context);
	/*오브젝트*/
	if (m_visibleMeshIndex == 0)
	{
		m_meshGroupSphere.Render(m_context);
	}
	else
	{
		m_meshGroupCharacter.Render(m_context);
	}

	// 후처리 필터 시작하기 전에 Texture2DMS에 렌더링 된 결과를 Texture2D로 복사
	ComPtr<ID3D11Texture2D> backBuffer;
	m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
	m_context->ResolveSubresource(m_tempTexture.Get(), 0, backBuffer.Get(), 0,
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

// 블룸필터 첫단계: 전체필셀 돌면서
// 픽셀의 평균값(RGB값의 평균->밝기)이 일정 값보다 작으면은
// 0으로 처리.
// 두번째 단계: 가우시안 블러처리.
// 세번째 단계: 원본이미지와 필터처리한 이미지를 합침.
void Ryudar::BuildFilters()
{
	m_filters.clear();

	// 가장 기본필터 -> 자기 자신을 복사하는 필터
	// -> GPU에서는 읽기 쓰기 동시에 안됨.
	// 따로 복사용버퍼를 만들어 줘야함.
	auto copyFilter = make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling",
	                                           m_screenWidth, m_screenHeight);
	// 필터의 기본적인 입력지정
	// 렌더링된 결과(rederTargetView = shaderResourceView)를
	// copyFilter에 넣어줌
	copyFilter->SetShaderResources({this->m_shaderResourceView});
	m_filters.push_back(copyFilter);

	// 해상도를 낮추는 다운 샘플링
	// -> 블러할때 부드럽게 하기위해 횟수를 높이면 GPU과부화 걸림
	// 다운필터에서 어두운부분을 도려내는 작업함.(threshold)
	// 2단계씩 다운샘플링. 안티에일리어싱없앨수있음.
	for (int down = 2; down <= m_down; down *= 2)
	{
		auto downFilter = make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling",
		                                           m_screenWidth / down, m_screenHeight / down);

		if (down == 2)
		{
			downFilter->SetShaderResources({this->m_shaderResourceView});
		}
		else
		{
			downFilter->SetShaderResources({m_filters.back()->m_shaderResourceView});
		}
		downFilter->m_pixelConstData.threshold = 0.0f;
		downFilter->UpdateConstantBuffers(m_device, m_context);
		m_filters.push_back(downFilter);
	}

	// 다운 샘플링한 이미지를 블러처리.
	// 다운 샘플링의 가장 낮은단계에서부터 블러처리
	for (int down = m_down; down >= 1; down /= 2)
	{
		for (int i = 0; i < m_repeat; i++)
		{
			// x축 블러처리
			auto &prevResource = m_filters.back()->m_shaderResourceView;
			m_filters.push_back(make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"BlurX",
			                                             m_screenWidth / down,
			                                             m_screenHeight / down));
			m_filters.back()->SetShaderResources({prevResource});

			// y축 블러처리
			auto &prevResource2 = m_filters.back()->m_shaderResourceView;
			m_filters.push_back(make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"BlurY",
			                                             m_screenWidth / down,
			                                             m_screenHeight / down));
			m_filters.back()->SetShaderResources({prevResource2});

			if (down > 1)
			{
				auto upFilter =
				    make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Sampling",
				                             m_screenWidth / down * 2, m_screenHeight / down * 2);
				upFilter->SetShaderResources({m_filters.back()->m_shaderResourceView});
				upFilter->m_pixelConstData.threshold = 0.0f;
				upFilter->UpdateConstantBuffers(m_device, m_context);
				m_filters.push_back(upFilter);
			}
		}
	}

	// 원본이미지와 블러처리한 이미지를 합치기
	auto combineFilter = make_shared<ImageFilter>(m_device, m_context, L"Sampling", L"Combine",
	                                              m_screenWidth, m_screenHeight);
	combineFilter->SetShaderResources(
	    {copyFilter->m_shaderResourceView, m_filters.back()->m_shaderResourceView});
	combineFilter->SetRenderTargets({this->m_renderTargetView});
	combineFilter->m_pixelConstData.strength = m_strength;
	combineFilter->UpdateConstantBuffers(m_device, m_context);
	m_filters.push_back(combineFilter);
}

void Ryudar::DrawMeshSelectorGUI()
{
	if (ImGui::RadioButton("Sphere", m_visibleMeshIndex == 0))
	{
		m_visibleMeshIndex = 0;
		changedMesh = 1;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Character", m_visibleMeshIndex == 1))
	{
		m_visibleMeshIndex = 1;
		changedMesh = 1;
	}
}

void Ryudar::DrawShadingModeGUI(BasicMeshGroup &meshGroup)
{
	auto &pixelData = meshGroup.m_basicPixelConstantData;
	const char *items[] = {"Blinn-Phong Shading", "Phong Shading"};
	int currentItem = pixelData.useBlinnPhong ? 0 : 1;

	if (ImGui::Combo("Shading Option", &currentItem, items, IM_ARRAYSIZE(items)))
	{
		pixelData.useBlinnPhong = currentItem == 0;
		pixelData.usePhong = currentItem == 1;
	}
}

void Ryudar::DrawRenderOptionsGUI(BasicMeshGroup &meshGroup)
{
	DrawShadingModeGUI(meshGroup);
	ImGui::Checkbox("Use Texture", &meshGroup.m_basicPixelConstantData.useTexture);
	// ImGui::Checkbox("Use Perspective Projection", &m_usePerspectiveProjection);
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
		m_dirtyflag += ImGui::SliderFloat("Bloom Threshold", &m_threshold, 0.0f, 1.0f);
		m_dirtyflag += ImGui::SliderFloat("Bloom Strength", &m_strength, 0.0f, 3.0f);
	}
	ImGui::NewLine();
}

void Ryudar::DrawModelGUI()
{
	ImGui::Text("Model");
	ImGui::SliderFloat3("m_modelTranslastion", &m_modelTranslation.x, -2.0f, 2.0f);
	ImGui::SliderFloat3("m_modelRotaion(Rad)", &m_modelRotation.x, -3.13f, 3.14f);
	ImGui::SliderFloat3("m_modelScaling", &m_modelScaling.x, 0.1f, 2.0f);

	// ImGui::Text("View");
	// ImGui::SliderFloat2("m_viewRot", &m_viewRot.x, -3.14f, 3.14f);
	//
	// ImGui::Text("Projection");
	// ImGui::SliderFloat("m_nearZ", &m_nearZ, 0.01f, 10.0f);
	// ImGui::SliderFloat("m_farZ", &m_farZ, 0.01f, 10.0f);
	// ImGui::SliderFloat("m_projFovAngleY(Deg)", &m_projFovAngleY, 10.0f, 180.0f);
	// ImGui::NewLine();
}

void Ryudar::DrawMaterialGUI(BasicMeshGroup &meshGroup)
{
	auto &material = meshGroup.m_basicPixelConstantData.material;

	ImGui::Text("Material");
	// ImGui::SliderFloat3("Material FresnelR0",
	//                     &material.fresnelR0.x, 0.0f, 1.0f);
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

void Ryudar::DrawLightGUI(BasicMeshGroup &meshGroup)
{
	ImGui::Text("Light");
	ImGui::Checkbox("Use Image Based Light", &meshGroup.m_basicPixelConstantData.useIBL);
	if (!meshGroup.m_basicPixelConstantData.useIBL)
	{
		if (ImGui::RadioButton("Directional Light", m_lightType == 0))
		{
			m_lightType = 0;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Point Light", m_lightType == 1))
		{
			m_lightType = 1;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Spot Light", m_lightType == 2))
		{
			m_lightType = 2;
		}

		// Point Light or Spot Light
		if (m_lightType == 1 || m_lightType == 2)
		{
			ImGui::SliderFloat3("Light Position", &m_lightFromGUI.position.x, -5.0f, 5.0f);
			ImGui::SliderFloat("Light fallOffStart", &m_lightFromGUI.fallOffStart, 0.0f, 5.0f);
			ImGui::SliderFloat("Light fallOffEnd", &m_lightFromGUI.fallOffEnd, 0.0f, 10.0f);

			// Spot Light only
			if (m_lightType == 2)
			{
				ImGui::SliderFloat("Light spotPower", &m_lightFromGUI.spotPower, 1.0f, 512.0f);
			}
		}

		DrawRimLightGUI(meshGroup);
	}
	else
	{
		ImGui::Checkbox("Use EnvironmentMapping", &meshGroup.m_basicPixelConstantData.useEvMapping);
	}
}

void Ryudar::DrawRimLightGUI(BasicMeshGroup &meshGroup)
{
	ImGui::Checkbox("Use Rim Light", &meshGroup.m_basicPixelConstantData.useRimLight);
	if (meshGroup.m_basicPixelConstantData.useRimLight)
	{
		ImGui::SliderFloat("Rim Strength", &meshGroup.m_basicPixelConstantData.rimStrength, 0.0f,
		                   10.0f);
		ImGui::Checkbox("Use Smoothstep", &meshGroup.m_basicPixelConstantData.useSmoothstep);
		ImGui::SliderFloat3("Rim Color", &meshGroup.m_basicPixelConstantData.rimColor.x, 0.0f,
		                    1.0f);
		ImGui::SliderFloat("Rim Power", &meshGroup.m_basicPixelConstantData.rimPower, 0.01f, 10.0f);
	}
}

void Ryudar::UpdateGUI()
{
	ImGui::Checkbox("Use FPV", &m_useFirstPersonView);

	DrawMeshSelectorGUI();

	auto &meshGroup = m_visibleMeshIndex == 0 ? m_meshGroupSphere : m_meshGroupCharacter;
	DrawRenderOptionsGUI(meshGroup);
	DrawPostProcessingGUI();
	DrawModelGUI();
	DrawMaterialGUI(meshGroup);
	DrawLightGUI(meshGroup);
}

} // namespace Ryudar
