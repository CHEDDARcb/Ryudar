#pragma once
// D3D11 버퍼, 셰이더, 텍스처 생성처럼 반복되는 초기화 코드를 모아둔 유틸리티.
// 렌더링 클래스들이 공통 GPU 리소스를 일관된 방식으로 만들도록 돕는다.

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
	// 현재 백 버퍼 크기와 MSAA 설정에 맞는 깊이/스텐실 뷰를 생성한다.
	static void CreateDepthBuffer(ID3D11Device *device, int screenWidth, int screenHeight,
	                              UINT numQualityLevels,
	                              ComPtr<ID3D11DepthStencilView> &depthStencilView);

	// HLSL 정점 셰이더를 컴파일하고 입력 구조에 맞는 입력 레이아웃을 생성한다.
	static void
	CreateVertexShaderAndInputLayout(ID3D11Device *device, const wstring &filename,
	                                 const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
	                                 ComPtr<ID3D11VertexShader> &vertexShader,
	                                 ComPtr<ID3D11InputLayout> &inputLayout);

	// HLSL 픽셀 셰이더를 컴파일해 D3D11 픽셀 셰이더 객체를 생성한다.
	static void CreatePixelShader(ID3D11Device *device, const wstring &filename,
	                              ComPtr<ID3D11PixelShader> &pixelShader);

	// 인덱스 목록을 변경 불가능한 GPU 인덱스 버퍼로 업로드한다.
	static void CreateIndexBuffer(ID3D11Device *device, const vector<uint32_t> &indices,
	                              ComPtr<ID3D11Buffer> &indexBuffer);

	// 정점 목록을 변경 불가능한 GPU 정점 버퍼로 업로드한다.
	template <typename T_VERTEX>
	static void CreateVertexBuffer(ID3D11Device *device, const vector<T_VERTEX> &vertices,
	                               ComPtr<ID3D11Buffer> &vertexBuffer)
	{
		if (vertices.empty())
		{
			throw std::invalid_argument("Cannot create a vertex buffer from empty data");
		}

		// 생성 이후 CPU에서 수정하지 않는 정적 정점 데이터다.
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage =
		    D3D11_USAGE_IMMUTABLE; // 초기화 후 GPU에서 읽기만 한다.
		bufferDesc.ByteWidth =
		    UINT(sizeof(T_VERTEX) * vertices.size()); // 전체 정점 데이터의 바이트 크기
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.StructureByteStride = sizeof(T_VERTEX); // 데이터를 한번 읽어들을 때의 크기

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		ThrowIfFailed(
		    device->CreateBuffer(&bufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf()),
		    "Create vertex buffer");
	}

	// CPU에서 자주 갱신할 상수 버퍼를 생성하고 초기 데이터를 채운다.
	// T_CONSTANT의 크기는 D3D11 규칙상 16바이트 배수여야 한다.
	template <typename T_CONSTANT>
	static void CreateConstantBuffer(ID3D11Device *device,
	                                 const T_CONSTANT &constantBufferData,
	                                 ComPtr<ID3D11Buffer> &constantBuffer)
	{
		static_assert(sizeof(T_CONSTANT) % 16 == 0,
		              "Constant buffer size must be a multiple of 16");

		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(constantBufferData);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC; // 매 프레임 CPU에서 갱신한다.
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

	// 동적 버퍼를 Map/Unmap으로 갱신한다.
	// 현재 프로젝트에서는 주로 상수 버퍼 업데이트에 사용한다.
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

	// 이미지 파일을 읽어 2D 텍스처와 셰이더 리소스 뷰를 생성한다.
	static void CreateTexture(ID3D11Device *device, const std::string &filename,
	                          ComPtr<ID3D11Texture2D> &texture,
	                          ComPtr<ID3D11ShaderResourceView> &textureResourceView);

	// DDS 큐브맵 파일을 읽어 셰이더 리소스 뷰를 생성한다.
	static void CreateCubemapTexture(ID3D11Device *device, const wchar_t *filename,
	                                 ComPtr<ID3D11ShaderResourceView> &textureResourceView);
};
} // namespace Ryudar
