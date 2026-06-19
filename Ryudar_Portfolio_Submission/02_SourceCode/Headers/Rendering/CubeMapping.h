#pragma once
// Cubemap Textureを読み込み、SkyboxおよびIBL用の環境リソースを管理する。
// 他のMeshが使用するDiffuse/Specular IBL SRVを外部へ提供する。

#include <string>
#include <wrl.h>

#include "Graphics/D3D11Utils.h"
#include "Geometry/GeometryGenerator.h"
#include "Scene/Material.h"
#include "Geometry/Vertex.h"

namespace Ryudar
{

using Microsoft::WRL::ComPtr;

class CubeMapping
{
public:
	// CubemapとSkyboxのレンダリングに必要なGPUリソースを生成する。
	void Initialize(ID3D11Device *device, const wchar_t *diffuseFilename,
	                const wchar_t *specularFilename);
	// Camera回転とProjectionをSkyboxのConstant Bufferへ反映する。
	void UpdateConstantBuffers(ID3D11DeviceContext *context, const Matrix &viewCol,
	                           const Matrix &projCol);
	void Render(ID3D11DeviceContext *context);

public:
	// シーン内の他MeshがIBL入力として共有するCubemap View。
	ComPtr<ID3D11ShaderResourceView> m_diffuseIBLSRV;
	ComPtr<ID3D11ShaderResourceView> m_specularIBLSRV;

private:
	struct VertexConstantData
	{
		Matrix viewProj; // Shader演算を減らすためCPU側で事前に乗算する。
	};
	static_assert((sizeof(VertexConstantData) % 16) == 0,
	              "Constant Buffer size must be 16-byte aligned");

	// Skybox MeshとRendering Pipeline
	Mesh m_cubeMesh;

	ComPtr<ID3D11SamplerState> m_samplerState;

	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	// 毎フレームCameraのView/Projection Matrixで更新する。
	CubeMapping::VertexConstantData vertexConstantData;
};
} // namespace Ryudar
