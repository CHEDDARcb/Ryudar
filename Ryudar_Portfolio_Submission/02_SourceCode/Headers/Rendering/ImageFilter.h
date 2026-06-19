#pragma once
// Post Processの1 Passを表すクラス。
// 入力SRVをFull-Screen QuadでSampleし、自身のRTV/SRVまたはBack Bufferへ出力する。

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

// Sampling、Blur、Combine Pixel Shaderが共有するConstant Bufferデータ。
struct SamplingPixelConstantData
{
	// 現在の出力Textureにおける1 Texelの幅と高さ。
	float texelWidth = 0.f;
	float texelHeight = 0.f;

	// Bright Pass抽出とBloom合成に使用するパラメータ。
	float threshold = 0.f;
	float strength = 0.f;
};

static_assert(sizeof(SamplingPixelConstantData) == 16,
              "SamplingPixelConstantData must match the HLSL cbuffer layout");

class ImageFilter
{
public:
	// 指定したPixel Shaderと出力解像度でPost Process Passを生成する。
	ImageFilter(ID3D11Device *device, FilterType type, int width, int height);

	// Threshold、Strength、Texel SizeなどのPixel Shader定数をGPUへUploadする。
	void UpdateConstantBuffers(ID3D11DeviceContext *context);

	// 設定されたSRVをFull-Screen QuadでSampleし、結果をRTVへ書き込む。
	void Render(ID3D11DeviceContext *context);

	// このFilterのPixel Shaderが読み込む入力Texture Viewを設定する。
	void SetShaderResources(
	    std::initializer_list<ID3D11ShaderResourceView *> resources);

	// このFilterの出力先Render Target Viewを設定する。
	void SetRenderTargets(std::initializer_list<ID3D11RenderTargetView *> targets);

	// 次のFilterがこのFilterの出力を読むためのShader Resource Viewを返す。
	ID3D11ShaderResourceView *GetShaderResourceView() const noexcept;

	void SetThreshold(float threshold) noexcept;
	void SetStrength(float strength) noexcept;

private:
	// Full-Screen Quad、Pipeline State、出力Texture、Constant Bufferを生成する。
	void Initialize(ID3D11Device *device, FilterType type, int width, int height);

	static std::wstring GetPixelShaderFilename(FilterType type);

	// Full-Screen Pass用のMeshとPipeline State。
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

	// D3D11 Bind呼び出し用に保持する非所有Raw Pointer。
	std::vector<ID3D11ShaderResourceView *> m_shaderResources;
	std::vector<ID3D11RenderTargetView *> m_renderTargets;
};

} // namespace Ryudar
