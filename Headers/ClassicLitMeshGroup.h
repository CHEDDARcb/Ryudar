#pragma once
// 여러 Mesh를 하나의 렌더링 단위로 묶어 셰이더, 상수 버퍼, 텍스처를 관리한다.
// 일반 메쉬 렌더링과 노멀 벡터 시각화 기능을 함께 관리한다.

#include "ClassicLitConstantData.h"
#include "ClassicLitRenderSettings.h"
#include "D3D11Utils.h"
#include "Mesh.h"
#include "MeshData.h"

namespace Ryudar::ClassicLit
{

class MeshGroup
{
public:
	// 모델 파일을 읽어 MeshData로 변환한 뒤 GPU 리소스를 생성한다.
	void Initialize(ComPtr<ID3D11Device> &device, const std::string &basePath,
	                const std::string &filename);

	// 이미 만들어진 MeshData 목록으로 정점/인덱스 버퍼와 셰이더 리소스를 생성한다.
	void Initialize(ComPtr<ID3D11Device> &device, const std::vector<MeshData> &meshes);

	// CPU에서 수정한 셰이더 상수 데이터를 GPU constant buffer에 업로드한다.
	void UpdateConstantBuffers(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context);

	// 기본 메쉬를 그린 뒤, 옵션이 켜져 있으면 노멀 벡터를 선으로 추가 렌더링한다.
	void Render(ComPtr<ID3D11DeviceContext> &context);

private:
	// MeshGroup이 유지하는 Mesh들의 셰이더, 텍스처, constant buffer 설정을 GPU에 적용한다.
	void ApplyRenderSettings();

public:
	// Ryudar::Update()에서 월드/뷰/프로젝션 행렬을 채운 뒤 vertex shader로 전달한다.
	VertexConstantData m_vertexConstantData;

	// Ryudar의 GUI에서 조명, 재질, 텍스처 사용 여부를 수정한 뒤 pixel shader로 전달한다.
	LightingConstantData m_lightingConstantData;
	ShadingConstantData m_shadingConstantData;
	RenderSettings m_renderSettings;

	// CubeMapping에서 공유받는 IBL 큐브맵 리소스.
	ComPtr<ID3D11ShaderResourceView> m_diffuseIBLSRV;
	ComPtr<ID3D11ShaderResourceView> m_specularIBLSRV;

	// 노멀 벡터 시각화 길이를 조절하는 constant buffer 데이터.
	NormalVertexConstantData m_normalVertexConstantData;

	// normal scale이 바뀐 프레임에만 GPU constant buffer를 갱신한다.
	bool m_drawNormalsDirtyFlag = true;

	// true면 Render()에서 노멀 벡터를 LINELIST로 추가 렌더링한다.
	bool m_drawNormals = false;

private:
	// 같은 셰이더와 constant buffer를 공유해 그릴 메쉬 목록.
	std::vector<shared_ptr<Mesh>> m_meshes;

	// 삼각형 메쉬 렌더링에 사용하는 기본 셰이더와 input layout.
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	// 일반 텍스처와 IBL 큐브맵 샘플링에 사용하는 sampler state.
	ComPtr<ID3D11SamplerState> m_samplerState;

	// 모든 메쉬가 공유하는 기본 셰이더 constant buffer.
	ComPtr<ID3D11Buffer> m_vertexConstantBuffer;
	ComPtr<ID3D11Buffer> m_lightingConstantBuffer;
	ComPtr<ID3D11Buffer> m_shadingConstantBuffer;

	// 노멀 벡터를 선으로 그릴 때 사용하는 셰이더.
	ComPtr<ID3D11VertexShader> m_normalVertexShader;
	ComPtr<ID3D11PixelShader> m_normalPixelShader;

	// 각 정점 위치에서 normal 방향으로 뻗는 선분들을 모아둔 디버그 메쉬.
	shared_ptr<Mesh> m_normalLines;

	// 노멀 벡터 시각화 전용 constant buffer.
	ComPtr<ID3D11Buffer> m_normalVertexConstantBuffer;
	ComPtr<ID3D11Buffer> m_normalPixelConstantBuffer;
};
} // namespace Ryudar::ClassicLit
