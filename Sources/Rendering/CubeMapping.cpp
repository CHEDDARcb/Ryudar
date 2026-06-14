#include "Rendering/CubeMapping.h"

namespace Ryudar
{

void CubeMapping::Initialize(ID3D11Device *device, const wchar_t *diffuseFilename,
                             const wchar_t *specularFilename)
{
	// IBL용 디퓨즈 및 스페큘러 큐브맵을 로드한다.
	D3D11Utils::CreateCubemapTexture(device, diffuseFilename, m_diffuseIBLSRV);
	D3D11Utils::CreateCubemapTexture(device, specularFilename, m_specularIBLSRV);

	D3D11Utils::CreateConstantBuffer(device, vertexConstantData, m_cubeMesh.vertexConstantBuffer);

	// 카메라를 감싸는 큰 구를 스카이박스로 사용한다.
	// 안쪽 면이 보이도록 인덱스 순서를 뒤집는다.
	MeshData cubeMeshData = GeometryGenerator::MakeSphere(10.0f, 100, 100);
	std::reverse(cubeMeshData.indices.begin(), cubeMeshData.indices.end());

	D3D11Utils::CreateVertexBuffer(device, cubeMeshData.vertices, m_cubeMesh.vertexBuffer);
	m_cubeMesh.m_indexCount = UINT(cubeMeshData.indices.size());
	D3D11Utils::CreateIndexBuffer(device, cubeMeshData.indices, m_cubeMesh.indexBuffer);

	// 스카이박스는 위치만 사용하지만 공통 Vertex 입력 레이아웃을 재사용한다.
	vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	D3D11Utils::CreateVertexShaderAndInputLayout(device, L"CubeMappingVertexShader.hlsl",
	                                             basicInputElements, m_vertexShader, m_inputLayout);
	D3D11Utils::CreatePixelShader(device, L"CubeMappingPixelShader.hlsl", m_pixelShader);

	// 큐브맵 샘플링에 사용할 샘플러를 생성한다.
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ThrowIfFailed(device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf()),
	              "Create cubemap sampler state");
}

void CubeMapping::UpdateConstantBuffers(ID3D11DeviceContext *context,
                                        const Matrix &viewCol, const Matrix &projCol)
{
	// 스카이박스는 카메라 위치를 따라가므로 회전과 투영만 반영한다.
	vertexConstantData.viewProj = projCol * viewCol;

	D3D11Utils::UpdateBuffer(context, vertexConstantData, m_cubeMesh.vertexConstantBuffer.Get());
}

void CubeMapping::Render(ID3D11DeviceContext *context)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetVertexBuffers(0, 1, m_cubeMesh.vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_cubeMesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(m_vertexShader.Get(), 0, 0);
	context->VSSetConstantBuffers(0, 1, m_cubeMesh.vertexConstantBuffer.GetAddressOf());

	// 배경에는 스페큘러 큐브맵을 사용하고 두 IBL 뷰는 메시에도 공유한다.
	ID3D11ShaderResourceView *views[1] = {m_specularIBLSRV.Get()};
	context->PSSetShaderResources(0, 1, views);
	context->PSSetShader(m_pixelShader.Get(), 0, 0);
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	context->DrawIndexed(m_cubeMesh.m_indexCount, 0, 0);
}
} // namespace Ryudar
