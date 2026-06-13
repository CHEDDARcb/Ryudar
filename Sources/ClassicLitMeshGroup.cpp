
#include "ClassicLitMeshGroup.h"
#include "GeometryGenerator.h"

namespace Ryudar::ClassicLit
{

// 모델읽어오기
void MeshGroup::Initialize(ComPtr<ID3D11Device> &device, const std::string &basePath,
                           const std::string &filename)
{

	auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);

	Initialize(device, meshes);
}

// Constant Buffer, Vertex, Index Buffer 생성
void MeshGroup::Initialize(ComPtr<ID3D11Device> &device, const std::vector<MeshData> &meshes)
{

	// Sampler 만들기
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());

	// ConstantBuffer 만들기
	m_vertexConstantData.modelWorld = Matrix();
	m_vertexConstantData.view = Matrix();
	m_vertexConstantData.projection = Matrix();
	D3D11Utils::CreateConstantBuffer(device, m_vertexConstantData, m_vertexConstantBuffer);
	D3D11Utils::CreateConstantBuffer(device, m_lightingConstantData, m_lightingConstantBuffer);
	D3D11Utils::CreateConstantBuffer(device, m_shadingConstantData, m_shadingConstantBuffer);

	// Vertex Shader, Pixel Shader, Input Layout 만들기
	vector<D3D11_INPUT_ELEMENT_DESC> classicLitInputElements = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	D3D11Utils::CreateVertexShaderAndInputLayout(device, L"ClassicLitVertexShader.hlsl",
	                                             classicLitInputElements, m_vertexShader,
	                                             m_inputLayout);

	D3D11Utils::CreatePixelShader(device, L"ClassicLitPixelShader.hlsl", m_pixelShader);

	// MeshData 목록으로 Mesh 목록 만들기
	std::size_t totalVertexCount = 0;
	m_meshes.reserve(m_meshes.size() + meshes.size());
	for (const auto &meshData : meshes)
	{
		totalVertexCount += meshData.vertices.size();

		auto newMesh = std::make_shared<Mesh>();
		D3D11Utils::CreateVertexBuffer(device, meshData.vertices, newMesh->vertexBuffer);
		newMesh->m_indexCount = UINT(meshData.indices.size());
		D3D11Utils::CreateIndexBuffer(device, meshData.indices, newMesh->indexBuffer);

		if (!meshData.textureFilename.empty())
		{
			// Texture 만들기
			// https://opengameart.org/content/3-crate-textures-w-bump-normal
			// https://learnopengl.com/Getting-started/Textures
			std::cout << meshData.textureFilename << std::endl;
			D3D11Utils::CreateTexture(device, meshData.textureFilename, newMesh->texture,
			                          newMesh->textureResourceView);
		}

		newMesh->vertexConstantBuffer = m_vertexConstantBuffer;

		this->m_meshes.push_back(newMesh);
	}

	// 노멀 벡터 그리기
	m_normalLines = std::make_shared<Mesh>();

	std::vector<Vertex> normalVertices;
	std::vector<uint32_t> normalIndices;
	// 각 정점마다 노멀 벡터의 시작점과 끝점이 필요하므로 2배
	normalVertices.reserve(totalVertexCount * 2);
	normalIndices.reserve(totalVertexCount * 2);

	// 여러 메쉬의 normal 들을 하나로 합치기
	size_t offset = 0;
	for (const auto &meshData : meshes)
	{
		for (size_t i = 0; i < meshData.vertices.size(); i++)
		{

			auto v = meshData.vertices[i];

			v.texcoord.x = 0.0f; // 시작점 표시
			normalVertices.push_back(v);

			v.texcoord.x = 1.0f; // 끝점 표시
			normalVertices.push_back(v);

			normalIndices.push_back(uint32_t(2 * (i + offset)));
			normalIndices.push_back(uint32_t(2 * (i + offset) + 1));
		}
		offset += meshData.vertices.size();
	}

	D3D11Utils::CreateVertexBuffer(device, normalVertices, m_normalLines->vertexBuffer);
	m_normalLines->m_indexCount = UINT(normalIndices.size());
	D3D11Utils::CreateIndexBuffer(device, normalIndices, m_normalLines->indexBuffer);

	D3D11Utils::CreateVertexShaderAndInputLayout(device, L"NormalVertexShader.hlsl",
	                                             classicLitInputElements, m_normalVertexShader,
	                                             m_inputLayout);
	D3D11Utils::CreatePixelShader(device, L"NormalPixelShader.hlsl", m_normalPixelShader);

	D3D11Utils::CreateConstantBuffer(device, m_normalVertexConstantData,
	                                 m_normalVertexConstantBuffer);
}

void MeshGroup::UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                                      ComPtr<ID3D11DeviceContext> &context)
{
	ApplyRenderSettings();

	D3D11Utils::UpdateBuffer(device, context, m_vertexConstantData, m_vertexConstantBuffer);
	D3D11Utils::UpdateBuffer(device, context, m_lightingConstantData, m_lightingConstantBuffer);
	D3D11Utils::UpdateBuffer(device, context, m_shadingConstantData, m_shadingConstantBuffer);

	// 노멀 벡터 그리기
	if (m_drawNormals && m_drawNormalsDirtyFlag)
	{
		D3D11Utils::UpdateBuffer(device, context, m_normalVertexConstantData,
		                         m_normalVertexConstantBuffer);
		m_drawNormalsDirtyFlag = false;
	}
}

void MeshGroup::Render(ComPtr<ID3D11DeviceContext> &context)
{
	// VS, PS 쉐이더 설정
	context->VSSetShader(m_vertexShader.Get(), 0, 0);
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	context->PSSetShader(m_pixelShader.Get(), 0, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (const auto &mesh : m_meshes)
	{
		// Input Assembler
		context->IASetInputLayout(m_inputLayout.Get());
		context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// VS ConstantBuffer
		context->VSSetConstantBuffers(0, 1, mesh->vertexConstantBuffer.GetAddressOf());

		// PS ConstantBuffer
		// 물체 렌더링할 때 큐브맵도 같이 사용
		ID3D11ShaderResourceView *resViews[3] = {mesh->textureResourceView.Get(),
		                                         m_diffuseIBLSRV.Get(), m_specularIBLSRV.Get()};
		context->PSSetShaderResources(0, 3, resViews);

		ID3D11Buffer *pixelConstantBuffers[2] = {m_lightingConstantBuffer.Get(),
		                                         m_shadingConstantBuffer.Get()};
		context->PSSetConstantBuffers(0, 2, pixelConstantBuffers);

		context->DrawIndexed(mesh->m_indexCount, 0, 0);
	}

	// 노멀 벡터 그리기
	if (m_drawNormals)
	{
		context->IASetInputLayout(m_inputLayout.Get());
		context->IASetVertexBuffers(0, 1, m_normalLines->vertexBuffer.GetAddressOf(), &stride,
		                            &offset);
		context->IASetIndexBuffer(m_normalLines->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->VSSetShader(m_normalVertexShader.Get(), 0, 0);
		ID3D11Buffer *pptr[2] = {m_vertexConstantBuffer.Get(), m_normalVertexConstantBuffer.Get()};
		context->VSSetConstantBuffers(0, 2, pptr);
		context->PSSetShader(m_normalPixelShader.Get(), 0, 0);

		context->DrawIndexed(m_normalLines->m_indexCount, 0, 0);
	}
}

void MeshGroup::ApplyRenderSettings()
{
	m_shadingConstantData.material = m_renderSettings.material;

	m_shadingConstantData.shadingOptions.useTexture = m_renderSettings.shading.useTexture;
	m_shadingConstantData.shadingOptions.useBlinnPhong = m_renderSettings.shading.useBlinnPhong;
	m_shadingConstantData.shadingOptions.usePhong = m_renderSettings.shading.usePhong;

	m_shadingConstantData.environment.useIBL = m_renderSettings.environment.useIBL;
	m_shadingConstantData.environment.useEnvironmentReflection =
	    m_renderSettings.environment.useEnvironmentReflection;

	m_shadingConstantData.rimLight.enabled = m_renderSettings.rimLight.useRimLight;
	m_shadingConstantData.rimLight.useSmoothstep = m_renderSettings.rimLight.useSmoothstep;
	m_shadingConstantData.rimLight.rimColor = m_renderSettings.rimLight.rimColor;
	m_shadingConstantData.rimLight.power = m_renderSettings.rimLight.rimPower;
	m_shadingConstantData.rimLight.strength = m_renderSettings.rimLight.rimStrength;
}
} // namespace Ryudar::ClassicLit
