#pragma once
// 후처리 한 패스를 표현하는 클래스.
// 입력 SRV를 전체 화면 사각형에 샘플링해 자체 RTV/SRV 또는 back buffer로 출력한다.

#include "D3D11Utils.h"
#include "GeometryGenerator.h"
#include "Mesh.h"

namespace Ryudar
{

class ImageFilter
{
public:
	// 지정한 셰이더와 출력 해상도로 후처리 패스 하나를 생성한다.
	ImageFilter(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
	            const wstring vertexPrefix, const wstring pixelPrefix, int width, int height)
	{
		Initialize(device, context, vertexPrefix, pixelPrefix, width, height);
	}

	// 전체 화면 사각형, 셰이더, sampler, 출력 텍스처/RTV/SRV, constant buffer를 만든다.
	void Initialize(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
	                const wstring vertexPrefix, const wstring pixelPrefix, int width, int height)
	{

		MeshData meshData = GeometryGenerator::MakeSquare();

		m_mesh = std::make_shared<Mesh>();

		D3D11Utils::CreateVertexBuffer(device, meshData.vertices, m_mesh->vertexBuffer);
		m_mesh->m_indexCount = UINT(meshData.indices.size());
		D3D11Utils::CreateIndexBuffer(device, meshData.indices, m_mesh->indexBuffer);

		vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
		    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
		    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA,
		     0},
		};

		D3D11Utils::CreateVertexShaderAndInputLayout(device, vertexPrefix + L"VertexShader.hlsl",
		                                             basicInputElements, m_vertexShader,
		                                             m_inputLayout);

		D3D11Utils::CreatePixelShader(device, pixelPrefix + L"PixelShader.hlsl", m_pixelShader);

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
		ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC)); // Need this
		rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
		rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
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
		txtDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 이미지 처리용 중간 텍스처.
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
		device->CreateTexture2D(&txtDesc, NULL, texture.GetAddressOf());
		device->CreateRenderTargetView(texture.Get(), &viewDesc, m_renderTargetView.GetAddressOf());
		device->CreateShaderResourceView(texture.Get(), nullptr,
		                                 m_shaderResourceView.GetAddressOf());

		// blur shader가 주변 texel을 샘플링할 때 사용하는 1 texel 크기.
		m_pixelConstData.texelWidth = 1.0f / width;
		m_pixelConstData.texelHeight = 1.0f / height;

		D3D11Utils::CreateConstantBuffer(device, m_pixelConstData, m_mesh->pixelConstantBuffer);

		// 기본 렌더타겟 설정.
		// CombineFilter의 경우, 렌더타겟이 스왑체인의 back buffer이므로
		// Ryudar::BuildFilters()에서 SetRenderTargets(this->m_renderTargetView)로 바꿔준다.
		this->SetRenderTargets({m_renderTargetView});
	}

	// threshold, strength, texel size 같은 픽셀 셰이더 상수를 GPU에 업로드한다.
	void UpdateConstantBuffers(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context)
	{

		D3D11Utils::UpdateBuffer(device, context, m_pixelConstData, m_mesh->pixelConstantBuffer);
	}

	// 설정된 SRV를 읽어 전체 화면 사각형에 샘플링하고, 설정된 RTV에 결과를 쓴다.
	void Render(ComPtr<ID3D11DeviceContext> &context)
	{
		assert(m_shaderResources.size() > 0);
		assert(m_renderTargets.size() > 0);

		// 이 필터 결과를 어느 render target에 쓸지 지정한다.
		context->OMSetRenderTargets(UINT(m_renderTargets.size()), m_renderTargets.data(), nullptr);
		// float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		// context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
		context->RSSetState(m_rasterizerState.Get());
		context->RSSetViewports(1, &m_viewport);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		context->IASetInputLayout(m_inputLayout.Get());
		context->IASetVertexBuffers(0, 1, m_mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->VSSetShader(m_vertexShader.Get(), 0, 0);
		context->PSSetShader(m_pixelShader.Get(), 0, 0);
		context->PSSetShaderResources(0, UINT(m_shaderResources.size()), m_shaderResources.data());
		context->PSSetConstantBuffers(0, 1, m_mesh->pixelConstantBuffer.GetAddressOf());
		context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
		context->DrawIndexed(m_mesh->m_indexCount, 0, 0);
	}

	// 이 필터의 pixel shader가 읽을 입력 texture view들을 설정한다.
	void SetShaderResources(const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources)
	{

		m_shaderResources.clear();
		for (const auto &res : resources)
		{
			m_shaderResources.push_back(res.Get());
		}
	}

	// 이 필터가 결과를 출력할 render target view들을 설정한다.
	void SetRenderTargets(const std::vector<ComPtr<ID3D11RenderTargetView>> &targets)
	{

		m_renderTargets.clear();
		for (const auto &tar : targets)
		{
			m_renderTargets.push_back(tar.Get());
		}
	}

public:
	// 이 필터의 출력 텍스처를 다음 필터가 읽기 위한 view.
	ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;

	// 이 필터가 자신의 출력 텍스처에 렌더링하기 위한 view.
	// combine pass처럼 back buffer에 직접 출력하는 경우 SetRenderTargets()로 교체된다.
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	// Sampling/blur/combine 계열 pixel shader가 공유하는 constant buffer 데이터.
	struct SamplingPixelConstantData
	{
		// 현재 필터 출력 텍스처 기준 1 texel의 가로/세로 크기.
		float texelWidth = 0.f;
		float texelHeight = 0.f;

		// 밝은 영역 추출과 bloom 합성에 사용하는 파라미터.
		float threshold = 0.f;
		float strength = 0.f;

		// 추후 필터 옵션 확장을 위한 여유 공간.
		Vector4 options;
	};

	static_assert((sizeof(SamplingPixelConstantData) % 16) == 0,
	              "Constant Buffer size must be 16-byte aligned");

	// 필터 체인 소유자가 수정하는 픽셀 셰이더 상수.
	SamplingPixelConstantData m_pixelConstData;

protected:
	// 전체 화면 패스용 메쉬와 파이프라인 상태.
	shared_ptr<Mesh> m_mesh;

	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;
	ComPtr<ID3D11SamplerState> m_samplerState;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;

	D3D11_VIEWPORT m_viewport;

	// D3D11 바인딩 호출을 위해 캐싱하는 비소유 raw view.
	std::vector<ID3D11ShaderResourceView *> m_shaderResources;
	std::vector<ID3D11RenderTargetView *> m_renderTargets;
};
} // namespace Ryudar
