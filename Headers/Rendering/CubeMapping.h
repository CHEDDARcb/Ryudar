#pragma once
// 큐브맵 텍스처를 로드하고 스카이박스 및 IBL용 환경 리소스를 관리한다.
// 다른 메시가 사용할 디퓨즈 및 스페큘러 IBL SRV를 외부에 제공한다.

#include <string>
#include <wrl.h>

#include "Graphics/D3D11Utils.h"
#include "Geometry/GeometryGenerator.h"
#include "Scene/Material.h"
#include "Geometry/Vertex.h"

namespace Ryudar
{

using Microsoft::WRL::ComPtr;

class CubeMapping
{
public:
	// 큐브맵과 스카이박스 렌더링에 필요한 GPU 리소스를 생성한다.
	void Initialize(ID3D11Device *device, const wchar_t *diffuseFilename,
	                const wchar_t *specularFilename);
	// 카메라 회전과 투영을 스카이박스 상수 버퍼에 반영한다.
	void UpdateConstantBuffers(ID3D11DeviceContext *context, const Matrix &viewCol,
	                           const Matrix &projCol);
	void Render(ID3D11DeviceContext *context);

public:
	// 장면의 다른 메시가 IBL 입력으로 공유하는 큐브맵 뷰.
	ComPtr<ID3D11ShaderResourceView> m_diffuseIBLSRV;
	ComPtr<ID3D11ShaderResourceView> m_specularIBLSRV;

private:
	struct VertexConstantData
	{
		Matrix viewProj; // 셰이더 연산을 줄이기 위해 CPU에서 미리 곱한다.
	};
	static_assert((sizeof(VertexConstantData) % 16) == 0,
	              "Constant Buffer size must be 16-byte aligned");

	// 스카이박스 메쉬와 렌더링 파이프라인
	Mesh m_cubeMesh;

	ComPtr<ID3D11SamplerState> m_samplerState;

	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	// 매 프레임 카메라 뷰 및 투영 행렬로 갱신된다.
	CubeMapping::VertexConstantData vertexConstantData;
};
} // namespace Ryudar
