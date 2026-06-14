#include "ImageFilter.h"

#include <cassert>

#include "GeometryGenerator.h"

namespace Ryudar
{

ImageFilter::ImageFilter(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
                         FilterType type, int width, int height)
{
	Initialize(device, context, type, width, height);
}

void ImageFilter::Initialize(ComPtr<ID3D11Device> &device,
                             ComPtr<ID3D11DeviceContext> &context, FilterType type, int width,
                             int height)
{
	MeshData meshData = GeometryGenerator::MakeSquare();

	m_mesh = std::make_shared<Mesh>();

	D3D11Utils::CreateVertexBuffer(device, meshData.vertices, m_mesh->vertexBuffer);
	m_mesh->m_indexCount = UINT(meshData.indices.size());
	D3D11Utils::CreateIndexBuffer(device, meshData.indices, m_mesh->indexBuffer);

	std::vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA,
	     0},
	};

	D3D11Utils::CreateVertexShaderAndInputLayout(device, L"SamplingVertexShader.hlsl",
	                                             basicInputElements, m_vertexShader,
	                                             m_inputLayout);

	D3D11Utils::CreatePixelShader(device, GetPixelShaderFilename(type), m_pixelShader);

	// 필터 입력 텍스처를 샘플링할 때 사용할 sampler.
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

	// 후처리 사각형은 화면 전체에 그리므로 culling 없이 렌더링한다.
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = false;

	device->CreateRasterizerState(&rastDesc, m_rasterizerState.GetAddressOf());

	// 이 필터가 출력하는 텍스처 크기에 맞춰 viewport를 고정한다.
	ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = float(width);
	m_viewport.Height = float(height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	ComPtr<ID3D11Texture2D> texture;

	D3D11_TEXTURE2D_DESC txtDesc;
	ZeroMemory(&txtDesc, sizeof(txtDesc));
	txtDesc.Width = width;
	txtDesc.Height = height;
	txtDesc.MipLevels = txtDesc.ArraySize = 1;
	txtDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	txtDesc.SampleDesc.Count = 1;
	txtDesc.Usage = D3D11_USAGE_DEFAULT;
	txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	txtDesc.MiscFlags = 0;
	txtDesc.CPUAccessFlags = 0;

	D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
	viewDesc.Format = txtDesc.Format;
	viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipSlice = 0;

	// 같은 texture를 RTV로는 쓰고, SRV로는 다음 필터가 읽는다.
	device->CreateTexture2D(&txtDesc, nullptr, texture.GetAddressOf());
	device->CreateRenderTargetView(texture.Get(), &viewDesc, m_renderTargetView.GetAddressOf());
	device->CreateShaderResourceView(texture.Get(), nullptr, m_shaderResourceView.GetAddressOf());

	// blur shader가 주변 texel을 샘플링할 때 사용하는 1 texel 크기.
	m_pixelConstData.texelWidth = 1.0f / width;
	m_pixelConstData.texelHeight = 1.0f / height;

	D3D11Utils::CreateConstantBuffer(device, m_pixelConstData, m_mesh->pixelConstantBuffer);

	// Combine pass는 Ryudar::BuildFilters()에서 렌더 타겟을 back buffer로 교체한다.
	SetRenderTargets({m_renderTargetView});
}

void ImageFilter::UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                                        ComPtr<ID3D11DeviceContext> &context)
{
	D3D11Utils::UpdateBuffer(device, context, m_pixelConstData, m_mesh->pixelConstantBuffer);
}

void ImageFilter::Render(ComPtr<ID3D11DeviceContext> &context)
{
	assert(!m_shaderResources.empty());
	assert(!m_renderTargets.empty());

	context->OMSetRenderTargets(UINT(m_renderTargets.size()), m_renderTargets.data(), nullptr);
	context->RSSetState(m_rasterizerState.Get());
	context->RSSetViewports(1, &m_viewport);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetVertexBuffers(0, 1, m_mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, UINT(m_shaderResources.size()), m_shaderResources.data());
	context->PSSetConstantBuffers(0, 1, m_mesh->pixelConstantBuffer.GetAddressOf());
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	context->DrawIndexed(m_mesh->m_indexCount, 0, 0);
}

void ImageFilter::SetShaderResources(
    const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources)
{
	m_shaderResources.clear();
	for (const auto &resource : resources)
	{
		m_shaderResources.push_back(resource.Get());
	}
}

void ImageFilter::SetRenderTargets(
    const std::vector<ComPtr<ID3D11RenderTargetView>> &targets)
{
	m_renderTargets.clear();
	for (const auto &target : targets)
	{
		m_renderTargets.push_back(target.Get());
	}
}

std::wstring ImageFilter::GetPixelShaderFilename(FilterType type)
{
	switch (type)
	{
	case FilterType::Sampling:
		return L"SamplingPixelShader.hlsl";

	case FilterType::BlurHorizontal:
		return L"BlurXPixelShader.hlsl";

	case FilterType::BlurVertical:
		return L"BlurYPixelShader.hlsl";

	case FilterType::Combine:
		return L"CombinePixelShader.hlsl";
	}

	assert(false && "Invalid FilterType");
	return L"SamplingPixelShader.hlsl";
}

} // namespace Ryudar
