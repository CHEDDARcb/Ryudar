#pragma once
// 큐브맵 텍스처를 로드하고 스카이박스 및 IBL용 환경 리소스를 관리한다.
// 다른 메쉬가 사용할 diffuse/specular IBL SRV를 외부에 공유한다.

#include <string>
#include <wrl.h>

#include "D3D11Utils.h"
#include "GeometryGenerator.h"
#include "Material.h"
#include "Vertex.h"

namespace Ryudar
{

using Microsoft::WRL::ComPtr;

class CubeMapping
{
public:
	void Initialize(ComPtr<ID3D11Device> &device, const wchar_t *diffuseFilename,
	                const wchar_t *specularFilename);
	void UpdateConstantBuffers(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
	                           const Matrix &viewCol, const Matrix &projCol);
	void Render(ComPtr<ID3D11DeviceContext> &context);

public:
	// IBL을 위해 다른 메쉬들도 사용
	ComPtr<ID3D11ShaderResourceView> m_diffuseIBLSRV;
	ComPtr<ID3D11ShaderResourceView> m_specularIBLSRV;

private:
	struct VertexConstantData
	{
		Matrix viewProj; // 미리 곱해서 사용
	};
	static_assert((sizeof(VertexConstantData) % 16) == 0,
	              "Constant Buffer size must be 16-byte aligned");

	// 스카이박스 메쉬와 렌더링 파이프라인
	std::shared_ptr<Mesh> m_cubeMesh;

	ComPtr<ID3D11SamplerState> m_samplerState;

	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	// 매 프레임 카메라 view/projection으로 갱신된다.
	CubeMapping::VertexConstantData vertexConstantData;
};
} // namespace Ryudar
