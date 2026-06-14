#pragma once
// 후처리 한 패스를 표현하는 클래스.
// 입력 SRV를 전체 화면 사각형에 샘플링해 자체 RTV/SRV 또는 back buffer로 출력한다.

#include <memory>
#include <string>
#include <vector>

#include "D3D11Utils.h"
#include "Mesh.h"

namespace Ryudar
{

enum class FilterType
{
	Sampling,       // SamplingPixelShader.hlsl
	BlurHorizontal, // BlurXPixelShader.hlsl
	BlurVertical,   // BlurYPixelShader.hlsl
	Combine,        // CombinePixelShader.hlsl
};

// Sampling/blur/combine 계열 pixel shader가 공유하는 constant buffer 데이터.
struct SamplingPixelConstantData
{
	// 현재 필터 출력 텍스처 기준 1 texel의 가로/세로 크기.
	float texelWidth = 0.f;
	float texelHeight = 0.f;

	// 밝은 영역 추출과 bloom 합성에 사용하는 파라미터.
	float threshold = 0.f;
	float strength = 0.f;
};

static_assert(sizeof(SamplingPixelConstantData) == 16,
              "SamplingPixelConstantData must match the HLSL cbuffer layout");

class ImageFilter
{
public:
	// 지정한 픽셀 셰이더와 출력 해상도로 후처리 패스 하나를 생성한다.
	ImageFilter(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context, FilterType type,
	            int width, int height);

	// 전체 화면 사각형, 파이프라인 상태, 출력 텍스처, constant buffer를 만든다.
	void Initialize(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
	                FilterType type, int width, int height);

	// threshold, strength, texel size 같은 픽셀 셰이더 상수를 GPU에 업로드한다.
	void UpdateConstantBuffers(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context);

	// 설정된 SRV를 읽어 전체 화면 사각형에 샘플링하고, 설정된 RTV에 결과를 쓴다.
	void Render(ComPtr<ID3D11DeviceContext> &context);

	// 이 필터의 pixel shader가 읽을 입력 texture view들을 설정한다.
	void SetShaderResources(const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources);

	// 이 필터가 결과를 출력할 render target view들을 설정한다.
	void SetRenderTargets(const std::vector<ComPtr<ID3D11RenderTargetView>> &targets);

private:
	static std::wstring GetPixelShaderFilename(FilterType type);

public:
	// 이 필터의 출력 텍스처를 다음 필터가 읽기 위한 view.
	ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;

	// 이 필터가 자신의 출력 텍스처에 렌더링하기 위한 view.
	// combine pass처럼 back buffer에 직접 출력하는 경우 SetRenderTargets()로 교체된다.
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	// 필터 체인 소유자가 수정하는 픽셀 셰이더 상수.
	SamplingPixelConstantData m_pixelConstData;

protected:
	// 전체 화면 패스용 메쉬와 파이프라인 상태.
	std::shared_ptr<Mesh> m_mesh;

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
