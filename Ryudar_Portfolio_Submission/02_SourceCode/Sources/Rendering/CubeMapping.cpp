#include "Rendering/CubeMapping.h"

namespace Ryudar
{

void CubeMapping::Initialize(ID3D11Device *device, const wchar_t *diffuseFilename,
                             const wchar_t *specularFilename)
{
	// IBL用のDiffuse/Specular Cubemapを読み込む。
	D3D11Utils::CreateCubemapTexture(device, diffuseFilename, m_diffuseIBLSRV);
	D3D11Utils::CreateCubemapTexture(device, specularFilename, m_specularIBLSRV);

	D3D11Utils::CreateConstantBuffer(device, vertexConstantData, m_cubeMesh.vertexConstantBuffer);

	// Cameraを囲む大きなSphereをSkyboxとして使用する。
	// 内側の面が見えるようIndex順序を反転する。
	MeshData cubeMeshData = GeometryGenerator::MakeSphere(10.0f, 100, 100);
	std::reverse(cubeMeshData.indices.begin(), cubeMeshData.indices.end());

	D3D11Utils::CreateVertexBuffer(device, cubeMeshData.vertices, m_cubeMesh.vertexBuffer);
	m_cubeMesh.m_indexCount = UINT(cubeMeshData.indices.size());
	D3D11Utils::CreateIndexBuffer(device, cubeMeshData.indices, m_cubeMesh.indexBuffer);

	// SkyboxはPositionのみ使用するが、共通Vertex Input Layoutを再利用する。
	vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	D3D11Utils::CreateVertexShaderAndInputLayout(device, L"CubeMappingVertexShader.hlsl",
	                                             basicInputElements, m_vertexShader, m_inputLayout);
	D3D11Utils::CreatePixelShader(device, L"CubeMappingPixelShader.hlsl", m_pixelShader);

	// Cubemap Sampling用のSamplerを生成する。
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
	// SkyboxはCamera位置に追従するため、回転とProjectionのみ反映する。
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

	// 背景にはSpecular Cubemapを使用し、二つのIBL ViewはMeshとも共有する。
	ID3D11ShaderResourceView *views[1] = {m_specularIBLSRV.Get()};
	context->PSSetShaderResources(0, 1, views);
	context->PSSetShader(m_pixelShader.Get(), 0, 0);
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	context->DrawIndexed(m_cubeMesh.m_indexCount, 0, 0);
}
} // namespace Ryudar
