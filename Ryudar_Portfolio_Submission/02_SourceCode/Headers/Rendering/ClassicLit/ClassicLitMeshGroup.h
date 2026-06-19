#pragma once
// 複数のMeshを一つのレンダリング単位にまとめ、Shader、Constant Buffer、Textureを管理する。
// 通常MeshのレンダリングとNormal Vector可視化を同時に管理する。

#include "Rendering/ClassicLit/ClassicLitConstantData.h"
#include "Rendering/ClassicLit/ClassicLitRenderSettings.h"
#include "Graphics/D3D11Utils.h"
#include "Geometry/Mesh.h"
#include "Geometry/MeshData.h"

namespace Ryudar::ClassicLit
{

class MeshGroup
{
public:
	// ModelファイルをMeshDataへ変換し、GPUリソースを生成する。
	void Initialize(ID3D11Device *device, const std::string &basePath,
	                const std::string &filename);

	// 既存のMeshData一覧からVertex/Index BufferとShaderリソースを生成する。
	void Initialize(ID3D11Device *device, const std::vector<MeshData> &meshes);

	// CPU側で変更したShader定数をGPU Constant BufferへUploadする。
	void UpdateConstantBuffers(ID3D11DeviceContext *context);

	// 通常Meshを描画し、オプション有効時はNormal VectorをLineとして追加描画する。
	void Render(ID3D11DeviceContext *context);

	void SetEnvironmentMaps(ID3D11ShaderResourceView *diffuse,
	                        ID3D11ShaderResourceView *specular);
	void SetTransformConstants(const Matrix &modelWorld, const Matrix &invTranspose,
	                           const Matrix &view, const Matrix &projection) noexcept;
	void SetActiveLight(std::size_t index, const Light &light);
	void SetEyePosition(const Vector3 &eyePosition) noexcept;

	RenderSettings &GetRenderSettings() noexcept;
	const RenderSettings &GetRenderSettings() const noexcept;

	bool GetDrawNormals() const noexcept;
	void SetDrawNormals(bool enabled) noexcept;

	float GetNormalScale() const noexcept;
	void SetNormalScale(float scale) noexcept;

private:
	// CPU側のレンダリング設定をGPU Constant Buffer形式へ変換する。
	void ApplyRenderSettings() noexcept;

	VertexConstantData m_vertexConstantData;
	LightingConstantData m_lightingConstantData;
	ShadingConstantData m_shadingConstantData;
	RenderSettings m_renderSettings;

	ComPtr<ID3D11ShaderResourceView> m_diffuseIBLSRV;
	ComPtr<ID3D11ShaderResourceView> m_specularIBLSRV;

	NormalVertexConstantData m_normalVertexConstantData;
	bool m_drawNormalsDirtyFlag = true;
	bool m_drawNormals = false;

	// 同じShaderとConstant Bufferを共有して描画するMesh一覧。
	std::vector<Mesh> m_meshes;

	// Triangle Meshレンダリング用の基本ShaderとInput Layout。
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	// 通常TextureとIBL CubemapのSamplingに使用するSampler State。
	ComPtr<ID3D11SamplerState> m_samplerState;

	// 全Meshが共有する基本Shader Constant Buffer。
	ComPtr<ID3D11Buffer> m_vertexConstantBuffer;
	ComPtr<ID3D11Buffer> m_lightingConstantBuffer;
	ComPtr<ID3D11Buffer> m_shadingConstantBuffer;

	// Normal VectorをLine描画するためのShader。
	ComPtr<ID3D11VertexShader> m_normalVertexShader;
	ComPtr<ID3D11PixelShader> m_normalPixelShader;

	// 各頂点からNormal方向へ伸びるLineをまとめたDebug Mesh。
	Mesh m_normalLines;

	// Normal Vector可視化専用Constant Buffer。
	ComPtr<ID3D11Buffer> m_normalVertexConstantBuffer;
	ComPtr<ID3D11Buffer> m_normalPixelConstantBuffer;
};
} // namespace Ryudar::ClassicLit
