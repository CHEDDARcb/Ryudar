#include "Graphics/D3D11Utils.h"

#include <directxtk/DDSTextureLoader.h> // Cubemap読み込みに使用
#include <dxgi.h>
#include <dxgi1_4.h>
#include <filesystem>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Ryudar
{

using namespace std;
using namespace DirectX;

namespace
{

wstring ResolveShaderPath(const wstring &filename)
{
	namespace fs = std::filesystem;

	fs::path shaderPath(filename);
	if (fs::exists(shaderPath))
	{
		return shaderPath.wstring();
	}

	fs::path shaderFolderPath = fs::path(L"Shaders") / shaderPath.filename();
	return shaderFolderPath.wstring();
}

std::string GetShaderCompileError(ID3DBlob *errorBlob)
{
	if (!errorBlob)
	{
		return {};
	}

	return std::string(static_cast<const char *>(errorBlob->GetBufferPointer()),
	                   errorBlob->GetBufferSize());
}

} // namespace

void D3D11Utils::CreateDepthBuffer(ID3D11Device *device, int screenWidth, int screenHeight,
                                   UINT numQualityLevels,
                                   ComPtr<ID3D11DepthStencilView> &depthStencilView)
{
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	depthStencilBufferDesc.Width = screenWidth;
	depthStencilBufferDesc.Height = screenHeight;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (numQualityLevels > 0)
	{
		depthStencilBufferDesc.SampleDesc.Count = 4;
		depthStencilBufferDesc.SampleDesc.Quality = numQualityLevels - 1;
	}
	else
	{
		depthStencilBufferDesc.SampleDesc.Count = 1;
		depthStencilBufferDesc.SampleDesc.Quality = 0;
	}
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthStencilBuffer;

	ThrowIfFailed(device->CreateTexture2D(&depthStencilBufferDesc, nullptr,
	                                      depthStencilBuffer.GetAddressOf()),
	              "Create depth-stencil texture");

	ThrowIfFailed(device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr,
	                                             depthStencilView.GetAddressOf()),
	              "Create depth-stencil view");
}

void D3D11Utils::CreateVertexShaderAndInputLayout(
    ID3D11Device *device, const wstring &filename,
    const vector<D3D11_INPUT_ELEMENT_DESC> &inputElements, ComPtr<ID3D11VertexShader> &vertexShader,
    ComPtr<ID3D11InputLayout> &inputLayout)
{
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;

	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// mainをEntry PointとしてCompileし、HLSL includeを許可する。
	const wstring shaderPath = ResolveShaderPath(filename);
	HRESULT hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
	                                "main", "vs_5_0", compileFlags, 0, shaderBlob.GetAddressOf(),
	                                errorBlob.GetAddressOf());

	ThrowIfFailed(hr, "Compile vertex shader", GetShaderCompileError(errorBlob.Get()));

	ThrowIfFailed(device->CreateVertexShader(shaderBlob->GetBufferPointer(),
	                                         shaderBlob->GetBufferSize(), nullptr,
	                                         vertexShader.GetAddressOf()),
	              "Create vertex shader");

	ThrowIfFailed(
	    device->CreateInputLayout(inputElements.data(), static_cast<UINT>(inputElements.size()),
	                              shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
	                              inputLayout.GetAddressOf()),
	    "Create input layout");
}

void D3D11Utils::CreatePixelShader(ID3D11Device *device, const wstring &filename,
                                   ComPtr<ID3D11PixelShader> &pixelShader)
{
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;

	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// mainをEntry PointとしてCompileし、HLSL includeを許可する。
	const wstring shaderPath = ResolveShaderPath(filename);
	HRESULT hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
	                                "main", "ps_5_0", compileFlags, 0,
	                                shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());

	ThrowIfFailed(hr, "Compile pixel shader", GetShaderCompileError(errorBlob.Get()));

	ThrowIfFailed(device->CreatePixelShader(shaderBlob->GetBufferPointer(),
	                                        shaderBlob->GetBufferSize(), nullptr,
	                                        pixelShader.GetAddressOf()),
	              "Create pixel shader");
}

void D3D11Utils::CreateIndexBuffer(ID3D11Device *device,
                                   const std::vector<uint32_t> &indices,
                                   ComPtr<ID3D11Buffer> &indexBuffer)
{
	if (indices.empty())
	{
		throw std::invalid_argument("Cannot create an index buffer from empty data");
	}

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 生成後はGPUからの読み取りのみ。
	bufferDesc.ByteWidth = UINT(sizeof(uint32_t) * indices.size());
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.StructureByteStride = sizeof(uint32_t);

	D3D11_SUBRESOURCE_DATA indexBufferData = {0};
	indexBufferData.pSysMem = indices.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	ThrowIfFailed(device->CreateBuffer(&bufferDesc, &indexBufferData, indexBuffer.GetAddressOf()),
	              "Create index buffer");
}

void D3D11Utils::CreateTexture(ID3D11Device *device, const std::string &filename,
                               ComPtr<ID3D11Texture2D> &texture,
                               ComPtr<ID3D11ShaderResourceView> &textureResourceView)
{
	int width = 0;
	int height = 0;
	int channels = 0;

	using ImageData = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
	ImageData image(stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha),
	                stbi_image_free);
	if (!image)
	{
		throw std::runtime_error("Failed to load texture: " + filename);
	}

	// 読み込んだRGBAデータからImmutableな2D Textureを生成する。
	D3D11_TEXTURE2D_DESC txtDesc = {};
	txtDesc.Width = width;
	txtDesc.Height = height;
	txtDesc.MipLevels = txtDesc.ArraySize = 1;
	txtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	txtDesc.SampleDesc.Count = 1;
	txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
	txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = image.get();
	initData.SysMemPitch = txtDesc.Width * sizeof(uint8_t) * 4;
	// initData.SysMemSlicePitch = 0;

	ThrowIfFailed(device->CreateTexture2D(&txtDesc, &initData, texture.GetAddressOf()),
	              "Create texture", filename);
	ThrowIfFailed(
	    device->CreateShaderResourceView(texture.Get(), nullptr, textureResourceView.GetAddressOf()),
	    "Create texture shader resource view", filename);
}

void D3D11Utils::CreateCubemapTexture(ID3D11Device *device, const wchar_t *filename,
                                      ComPtr<ID3D11ShaderResourceView> &textureResourceView)
{
	ComPtr<ID3D11Texture2D> texture;

	// https://github.com/microsoft/DirectXTK/wiki/DDSTextureLoader
	auto hr = CreateDDSTextureFromFileEx(
	    device, filename, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0,
	    D3D11_RESOURCE_MISC_TEXTURECUBE, // Cubemapリソースとして生成する。
	    DDS_LOADER_FLAGS(false), (ID3D11Resource **)texture.GetAddressOf(),
	    textureResourceView.GetAddressOf(), nullptr);

	ThrowIfFailed(hr, "Load cubemap texture");
}
} // namespace Ryudar
