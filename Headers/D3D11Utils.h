#pragma once
// D3D11 버퍼, 셰이더, 텍스처 생성처럼 반복되는 초기화 코드를 모아둔 유틸리티.
// 렌더링 클래스들이 공통 GPU 리소스를 일관된 방식으로 만들도록 돕는다.

#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr
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
using std::shared_ptr;
using std::vector;
using std::wstring;

class D3D11Utils
{
public:
	// 현재 back buffer 크기와 MSAA 설정에 맞는 depth/stencil view를 생성한다.
	static bool CreateDepthBuffer(ComPtr<ID3D11Device> &device, int screenWidth, int screenHeight,
	                              UINT &numQualityLevels,
	                              ComPtr<ID3D11DepthStencilView> &depthStencilView);

	// HLSL vertex shader를 컴파일하고, 해당 입력 구조에 맞는 input layout을 함께 생성한다.
	static void
	CreateVertexShaderAndInputLayout(ComPtr<ID3D11Device> &device, const wstring &filename,
	                                 const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
	                                 ComPtr<ID3D11VertexShader> &m_vertexShader,
	                                 ComPtr<ID3D11InputLayout> &m_inputLayout);

	// HLSL pixel shader를 컴파일해 D3D11 pixel shader 객체를 생성한다.
	static void CreatePixelShader(ComPtr<ID3D11Device> &device, const wstring &filename,
	                              ComPtr<ID3D11PixelShader> &m_pixelShader);

	// 인덱스 목록을 GPU index buffer로 업로드한다.
	static void CreateIndexBuffer(ComPtr<ID3D11Device> &device, const vector<uint32_t> &indices,
	                              ComPtr<ID3D11Buffer> &indexBuffer);

	// 정점 목록을 변경 불가능한 GPU vertex buffer로 업로드한다.
	template <typename T_VERTEX>
	static void CreateVertexBuffer(ComPtr<ID3D11Device> &device, const vector<T_VERTEX> &vertices,
	                               ComPtr<ID3D11Buffer> &vertexBuffer)
	{
		// D3D11_USAGE enumeration (d3d11.h)
		// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage =
		    D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X, GPU에서만 읽을 수있음. CPU엑세스 불가.
		bufferDesc.ByteWidth =
		    UINT(sizeof(T_VERTEX) * vertices.size()); // 확보하려는 vertexMemory의 총 크기
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;                     // 0 if no CPU access is necessary.
		bufferDesc.StructureByteStride = sizeof(T_VERTEX); // 데이터를 한번 읽어들을 때의 크기

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		const HRESULT hr =
		    device->CreateBuffer(&bufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "CrteateBuffer() failed. " << std::hex << hr << std::endl;
		}
	}

	// CPU에서 자주 갱신할 constant buffer를 생성하고 초기 데이터를 채운다.
	// T_CONSTANT의 크기는 D3D11 규칙상 16바이트 배수여야 한다.
	template <typename T_CONSTANT>
	static void CreateConstantBuffer(ComPtr<ID3D11Device> &device,
	                                 const T_CONSTANT &constantBufferData,
	                                 ComPtr<ID3D11Buffer> &constantBuffer)
	{

		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(constantBufferData);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC; // 물체를 매 프레임 회전 -> 회전행렬이 매 프레임 변화
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &constantBufferData;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		auto hr = device->CreateBuffer(&cbDesc, &initData, constantBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "CreateConstantBuffer() CreateBuffer failed." << std::endl;
		}
	}

	// dynamic buffer를 Map/Unmap으로 갱신한다.
	// 현재 프로젝트에서는 주로 constant buffer 업데이트에 사용한다.
	template <typename T_DATA>
	static void UpdateBuffer(ComPtr<ID3D11Device> &device, ComPtr<ID3D11DeviceContext> &context,
	                         const T_DATA &bufferData, ComPtr<ID3D11Buffer> &buffer)
	{
		if (!buffer)
		{
			std::cout << "UpdateBuffer() buffer was not initialized." << std::endl;
		}

		D3D11_MAPPED_SUBRESOURCE ms;
		context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &bufferData, sizeof(bufferData));
		context->Unmap(buffer.Get(), NULL);
	}

	// 이미지 파일을 읽어 2D texture와 shader resource view를 생성한다.
	static void CreateTexture(ComPtr<ID3D11Device> &device, const std::string &filename,
	                          ComPtr<ID3D11Texture2D> &texture,
	                          ComPtr<ID3D11ShaderResourceView> &textureResourceView);

	// DDS cubemap 파일을 읽어 shader resource view를 생성한다.
	static void CreateCubemapTexture(ComPtr<ID3D11Device> &device, const wchar_t *filename,
	                                 ComPtr<ID3D11ShaderResourceView> &textureResourceView);
};
} // namespace Ryudar
