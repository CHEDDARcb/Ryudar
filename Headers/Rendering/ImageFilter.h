#pragma once
// 후처리 한 패스를 표현하는 클래스.
// 입력 SRV를 전체 화면 사각형에 샘플링해 자체 RTV/SRV 또는 백 버퍼로 출력한다.

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "Graphics/D3D11Utils.h"
#include "Geometry/Mesh.h"

namespace Ryudar
{

enum class FilterType
{
	Sampling,       // SamplingPixelShader.hlsl
	BlurHorizontal, // BlurXPixelShader.hlsl
	BlurVertical,   // BlurYPixelShader.hlsl
	Combine,        // CombinePixelShader.hlsl
};

// 샘플링, 블러, 합성 픽셀 셰이더가 공유하는 상수 버퍼 데이터.
struct SamplingPixelConstantData
{
	// 현재 출력 텍스처에서 한 텍셀의 가로 및 세로 크기.
	float texelWidth = 0.f;
	float texelHeight = 0.f;

	// 밝은 영역 추출과 블룸 합성에 사용하는 매개변수.
	float threshold = 0.f;
	float strength = 0.f;
};

static_assert(sizeof(SamplingPixelConstantData) == 16,
              "SamplingPixelConstantData must match the HLSL cbuffer layout");

class ImageFilter
{
public:
	// 지정한 픽셀 셰이더와 출력 해상도로 후처리 패스 하나를 생성한다.
	ImageFilter(ID3D11Device *device, FilterType type, int width, int height);

	// 임계값, 강도, 텍셀 크기 등의 픽셀 셰이더 상수를 GPU에 업로드한다.
	void UpdateConstantBuffers(ID3D11DeviceContext *context);

	// 설정된 SRV를 읽어 전체 화면 사각형에 샘플링하고, 설정된 RTV에 결과를 쓴다.
	void Render(ID3D11DeviceContext *context);

	// 이 필터의 픽셀 셰이더가 읽을 입력 텍스처 뷰를 설정한다.
	void SetShaderResources(
	    std::initializer_list<ID3D11ShaderResourceView *> resources);

	// 이 필터가 결과를 출력할 렌더 타깃 뷰를 설정한다.
	void SetRenderTargets(std::initializer_list<ID3D11RenderTargetView *> targets);

	// 다음 필터가 이 필터의 출력을 읽을 때 필요한 셰이더 리소스 뷰를 반환한다.
	ID3D11ShaderResourceView *GetShaderResourceView() const noexcept;

	void SetThreshold(float threshold) noexcept;
	void SetStrength(float strength) noexcept;

private:
	// 전체 화면 사각형, 파이프라인 상태, 출력 텍스처, 상수 버퍼를 만든다.
	void Initialize(ID3D11Device *device, FilterType type, int width, int height);

	static std::wstring GetPixelShaderFilename(FilterType type);

	// 전체 화면 패스용 메쉬와 파이프라인 상태.
	Mesh m_mesh;

	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;
	ComPtr<ID3D11SamplerState> m_samplerState;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;
	ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	D3D11_VIEWPORT m_viewport;
	SamplingPixelConstantData m_pixelConstData;

	// D3D11 바인딩 호출을 위해 보관하는 비소유 원시 포인터.
	std::vector<ID3D11ShaderResourceView *> m_shaderResources;
	std::vector<ID3D11RenderTargetView *> m_renderTargets;
};

} // namespace Ryudar
