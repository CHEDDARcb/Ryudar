#pragma once
// GPUレンダリングに必要なVertex/Index Buffer、Constant Buffer、Texture Viewをまとめる。
// 実際の所有者はClassicLit::MeshGroupまたはImageFilterとなる。

#include <d3d11.h>
#include <windows.h>
#include <wrl.h> // ComPtr

namespace Ryudar
{
using Microsoft::WRL::ComPtr;

struct Mesh
{
	// Geometry Buffer
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	UINT m_indexCount = 0;

	// 所有元のMeshGroupまたはFilterが共有するShader Constant Buffer。
	ComPtr<ID3D11Buffer> vertexConstantBuffer;
	ComPtr<ID3D11Buffer> pixelConstantBuffer;

	// 必要に応じて使用するMaterial Texture。
	ComPtr<ID3D11Texture2D> texture;
	ComPtr<ID3D11ShaderResourceView> textureResourceView;
};
} // namespace Ryudar
