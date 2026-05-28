#include "CubeMapping.h"

namespace Ryudar
{

void CubeMapping::Initialize(ComPtr<ID3D11Device> &device, const wchar_t *diffuseFilename,
                             const wchar_t *specularFilename)
{
	// .dds 파일 읽어들여서 초기화
	D3D11Utils::CreateCubemapTexture(device, diffuseFilename, m_diffuseResView);
	D3D11Utils::CreateCubemapTexture(device, specularFilename, m_specularResView);

	// 메쉬(버퍼) 생성
	m_cubeMesh = std::make_shared<Mesh>();
	// VertexConstantBuffer 생성
	D3D11Utils::CreateConstantBuffer(device, vertexConstantData, m_cubeMesh->vertexConstantBuffer);

	// 커다란 박스 초기화
	// 세상이 커다란 박스 안에 갇혀 있는 구조.
	// ->상자 안에서 상자를 바라보는 구조.
	// -D3D11_CULL_MODE::D3D11_CULL_NONE 또는 삼각형 뒤집기
	// ex) std::reverse(myvector.begin(), myvector.end());
	// MeshData cubeMeshData = GeometryGenerator::MakeBox(20.0f);
	MeshData cubeMeshData = GeometryGenerator::MakeSphere(10.0f, 100, 100);
	std::reverse(cubeMeshData.indices.begin(), cubeMeshData.indices.end());

	// VertexBuffer 생성
	D3D11Utils::CreateVertexBuffer(device, cubeMeshData.vertices, m_cubeMesh->vertexBuffer);
	// IndexBuffer 생성
	m_cubeMesh->m_indexCount = UINT(cubeMeshData.indices.size());
	D3D11Utils::CreateIndexBuffer(device, cubeMeshData.indices, m_cubeMesh->indexBuffer);

	// 쉐이더 생성
	// 실제로는 "POSITION"만 사용.
	vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	// VertexShader 설정.
	D3D11Utils::CreateVertexShaderAndInputLayout(device, L"CubeMappingVertexShader.hlsl",
	                                             basicInputElements, m_vertexShader, m_inputLayout);
	// PixelShader 설정.
	D3D11Utils::CreatePixelShader(device, L"CubeMappingPixelShader.hlsl", m_pixelShader);

	// Texture sampler 만들기
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the Sample State
	device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());
}

void CubeMapping::UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                                        ComPtr<ID3D11DeviceContext> &context, const Matrix &viewCol,
                                        const Matrix &projCol)
{

	vertexConstantData.viewProj = projCol * viewCol;

	D3D11Utils::UpdateBuffer(device, context, vertexConstantData, m_cubeMesh->vertexConstantBuffer);
}

void CubeMapping::Render(ComPtr<ID3D11DeviceContext> &context)
{

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Input Assembler
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetVertexBuffers(0, 1, m_cubeMesh->vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_cubeMesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VertexShader
	context->VSSetShader(m_vertexShader.Get(), 0, 0);
	context->VSSetConstantBuffers(0, 1, m_cubeMesh->vertexConstantBuffer.GetAddressOf());

	// Pixel Shader
	// Diffuse, Specular 선택 가능
	ID3D11ShaderResourceView *views[1] = {m_specularResView.Get()};
	context->PSSetShaderResources(0, 1, views);
	context->PSSetShader(m_pixelShader.Get(), 0, 0);
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	context->DrawIndexed(m_cubeMesh->m_indexCount, 0, 0);
}
} // namespace Ryudar