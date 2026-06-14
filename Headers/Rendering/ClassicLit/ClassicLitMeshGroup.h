#pragma once
// 여러 Mesh를 하나의 렌더링 단위로 묶어 셰이더, 상수 버퍼, 텍스처를 관리한다.
// 일반 메쉬 렌더링과 노멀 벡터 시각화 기능을 함께 관리한다.

#include "Rendering/ClassicLit/ClassicLitConstantData.h"
#include "Rendering/ClassicLit/ClassicLitRenderSettings.h"
#include "Graphics/D3D11Utils.h"
#include "Geometry/Mesh.h"
#include "Geometry/MeshData.h"

namespace Ryudar::ClassicLit
{

class MeshGroup
{
public:
	// 모델 파일을 읽어 MeshData로 변환한 뒤 GPU 리소스를 생성한다.
	void Initialize(ID3D11Device *device, const std::string &basePath,
	                const std::string &filename);

	// 이미 만들어진 MeshData 목록으로 정점/인덱스 버퍼와 셰이더 리소스를 생성한다.
	void Initialize(ID3D11Device *device, const std::vector<MeshData> &meshes);

	// CPU에서 수정한 셰이더 상수 데이터를 GPU 상수 버퍼에 업로드한다.
	void UpdateConstantBuffers(ID3D11DeviceContext *context);

	// 기본 메쉬를 그린 뒤, 옵션이 켜져 있으면 노멀 벡터를 선으로 추가 렌더링한다.
	void Render(ID3D11DeviceContext *context);

	void SetEnvironmentMaps(ID3D11ShaderResourceView *diffuse,
	                        ID3D11ShaderResourceView *specular);
	void SetTransformConstants(const Matrix &modelWorld, const Matrix &invTranspose,
	                           const Matrix &view, const Matrix &projection) noexcept;
	void SetActiveLight(std::size_t index, const Light &light);
	void SetEyePosition(const Vector3 &eyePosition) noexcept;

	RenderSettings &GetRenderSettings() noexcept;
	const RenderSettings &GetRenderSettings() const noexcept;

	bool GetDrawNormals() const noexcept;
	void SetDrawNormals(bool enabled) noexcept;

	float GetNormalScale() const noexcept;
	void SetNormalScale(float scale) noexcept;

private:
	// CPU 렌더 설정을 GPU 상수 버퍼 형식으로 변환한다.
	void ApplyRenderSettings() noexcept;

	VertexConstantData m_vertexConstantData;
	LightingConstantData m_lightingConstantData;
	ShadingConstantData m_shadingConstantData;
	RenderSettings m_renderSettings;

	ComPtr<ID3D11ShaderResourceView> m_diffuseIBLSRV;
	ComPtr<ID3D11ShaderResourceView> m_specularIBLSRV;

	NormalVertexConstantData m_normalVertexConstantData;
	bool m_drawNormalsDirtyFlag = true;
	bool m_drawNormals = false;

	// 같은 셰이더와 상수 버퍼를 공유해 그릴 메시 목록.
	std::vector<Mesh> m_meshes;

	// 삼각형 메시 렌더링에 사용하는 기본 셰이더와 입력 레이아웃.
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	// 일반 텍스처와 IBL 큐브맵 샘플링에 사용하는 샘플러 상태.
	ComPtr<ID3D11SamplerState> m_samplerState;

	// 모든 메시가 공유하는 기본 셰이더 상수 버퍼.
	ComPtr<ID3D11Buffer> m_vertexConstantBuffer;
	ComPtr<ID3D11Buffer> m_lightingConstantBuffer;
	ComPtr<ID3D11Buffer> m_shadingConstantBuffer;

	// 노멀 벡터를 선으로 그릴 때 사용하는 셰이더.
	ComPtr<ID3D11VertexShader> m_normalVertexShader;
	ComPtr<ID3D11PixelShader> m_normalPixelShader;

	// 각 정점에서 노멀 방향으로 뻗는 선분을 모아둔 디버그 메시.
	Mesh m_normalLines;

	// 노멀 벡터 시각화 전용 상수 버퍼.
	ComPtr<ID3D11Buffer> m_normalVertexConstantBuffer;
	ComPtr<ID3D11Buffer> m_normalPixelConstantBuffer;
};
} // namespace Ryudar::ClassicLit
