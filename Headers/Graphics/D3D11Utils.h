#pragma once
// D3D11 Buffer、Shader、Texture生成などの共通初期化処理をまとめたUtility。
// 各レンダリングクラスがGPUリソースを一貫した方法で生成できるようにする。

#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr

#include "Graphics/D3D11Exception.h"

struct ID3D11Buffer;
struct ID3D11DepthStencilView;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11VertexShader;
namespace Ryudar
{

using Microsoft::WRL::ComPtr;
using std::vector;
using std::wstring;

class D3D11Utils
{
public:
	// 現在のBack BufferサイズとMSAA設定に合うDepth/Stencil Viewを生成する。
	static void CreateDepthBuffer(ID3D11Device *device, int screenWidth, int screenHeight,
	                              UINT numQualityLevels,
	                              ComPtr<ID3D11DepthStencilView> &depthStencilView);

	// HLSL Vertex ShaderをCompileし、入力構造に合うInput Layoutを生成する。
	static void
	CreateVertexShaderAndInputLayout(ID3D11Device *device, const wstring &filename,
	                                 const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
	                                 ComPtr<ID3D11VertexShader> &vertexShader,
	                                 ComPtr<ID3D11InputLayout> &inputLayout);

	// HLSL Pixel ShaderをCompileしてD3D11 Pixel Shaderオブジェクトを生成する。
	static void CreatePixelShader(ID3D11Device *device, const wstring &filename,
	                              ComPtr<ID3D11PixelShader> &pixelShader);

	// Index一覧をImmutableなGPU Index BufferへUploadする。
	static void CreateIndexBuffer(ID3D11Device *device, const vector<uint32_t> &indices,
	                              ComPtr<ID3D11Buffer> &indexBuffer);

	// 頂点一覧をImmutableなGPU Vertex BufferへUploadする。
	template <typename T_VERTEX>
	static void CreateVertexBuffer(ID3D11Device *device, const vector<T_VERTEX> &vertices,
	                               ComPtr<ID3D11Buffer> &vertexBuffer)
	{
		if (vertices.empty())
		{
			throw std::invalid_argument("Cannot create a vertex buffer from empty data");
		}

		// 生成後にCPUから変更しない静的頂点データ。
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage =
		    D3D11_USAGE_IMMUTABLE; // 初期化後はGPUからの読み取りのみ。
		bufferDesc.ByteWidth =
		    UINT(sizeof(T_VERTEX) * vertices.size()); // 頂点データ全体のByte数
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.StructureByteStride = sizeof(T_VERTEX); // 1要素あたりのByte数

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		ThrowIfFailed(
		    device->CreateBuffer(&bufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf()),
		    "Create vertex buffer");
	}

	// CPUから頻繁に更新するConstant Bufferを生成し、初期データを設定する。
	// T_CONSTANTのサイズはD3D11の規則により16Byteの倍数でなければならない。
	template <typename T_CONSTANT>
	static void CreateConstantBuffer(ID3D11Device *device,
	                                 const T_CONSTANT &constantBufferData,
	                                 ComPtr<ID3D11Buffer> &constantBuffer)
	{
		static_assert(sizeof(T_CONSTANT) % 16 == 0,
		              "Constant buffer size must be a multiple of 16");

		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(constantBufferData);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC; // 毎フレームCPUから更新する。
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &constantBufferData;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		ThrowIfFailed(device->CreateBuffer(&cbDesc, &initData, constantBuffer.GetAddressOf()),
		              "Create constant buffer");
	}

	// Dynamic BufferをMap/Unmapで更新する。
	// このプロジェクトでは主にConstant Bufferの更新に使用する。
	template <typename T_DATA>
	static void UpdateBuffer(ID3D11DeviceContext *context, const T_DATA &bufferData,
	                         ID3D11Buffer *buffer)
	{
		if (!buffer)
		{
			throw std::logic_error("UpdateBuffer() received an uninitialized buffer");
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		ThrowIfFailed(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		              "Map dynamic buffer");

		std::memcpy(mappedResource.pData, &bufferData, sizeof(bufferData));

		context->Unmap(buffer, 0);
	}

	// 画像ファイルを読み込み、2D TextureとShader Resource Viewを生成する。
	static void CreateTexture(ID3D11Device *device, const std::string &filename,
	                          ComPtr<ID3D11Texture2D> &texture,
	                          ComPtr<ID3D11ShaderResourceView> &textureResourceView);

	// DDS Cubemapファイルを読み込み、Shader Resource Viewを生成する。
	static void CreateCubemapTexture(ID3D11Device *device, const wchar_t *filename,
	                                 ComPtr<ID3D11ShaderResourceView> &textureResourceView);
};
} // namespace Ryudar
