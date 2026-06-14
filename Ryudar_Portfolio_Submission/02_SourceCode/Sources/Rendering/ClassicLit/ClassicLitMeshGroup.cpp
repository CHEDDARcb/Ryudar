
#include "Rendering/ClassicLit/ClassicLitMeshGroup.h"
#include "Geometry/GeometryGenerator.h"

namespace Ryudar::ClassicLit
{

void MeshGroup::Initialize(ID3D11Device *device, const std::string &basePath,
                           const std::string &filename)
{

	auto meshes = GeometryGenerator::ReadFromFile(basePath, filename);

	Initialize(device, meshes);
}

void MeshGroup::Initialize(ID3D11Device *device, const std::vector<MeshData> &meshes)
{

	// 基本Material TextureとIBL Cubemapで共有するSampler。
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ThrowIfFailed(device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf()),
	              "Create ClassicLit sampler state");

	// 全Meshが共有するShader Constant Bufferを生成する。
	m_vertexConstantData.modelWorld = Matrix();
	m_vertexConstantData.view = Matrix();
	m_vertexConstantData.projection = Matrix();
	D3D11Utils::CreateConstantBuffer(device, m_vertexConstantData, m_vertexConstantBuffer);
	D3D11Utils::CreateConstantBuffer(device, m_lightingConstantData, m_lightingConstantBuffer);
	D3D11Utils::CreateConstantBuffer(device, m_shadingConstantData, m_shadingConstantBuffer);

	// 通常Mesh用のRendering Pipelineを構築する。
	vector<D3D11_INPUT_ELEMENT_DESC> classicLitInputElements = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	D3D11Utils::CreateVertexShaderAndInputLayout(device, L"ClassicLitVertexShader.hlsl",
	                                             classicLitInputElements, m_vertexShader,
	                                             m_inputLayout);

	D3D11Utils::CreatePixelShader(device, L"ClassicLitPixelShader.hlsl", m_pixelShader);

	// CPU側Meshデータを個別のGPU Bufferへ変換する。
	std::size_t totalVertexCount = 0;
	m_meshes.reserve(m_meshes.size() + meshes.size());
	for (const auto &meshData : meshes)
	{
		totalVertexCount += meshData.vertices.size();

		Mesh newMesh;
		D3D11Utils::CreateVertexBuffer(device, meshData.vertices, newMesh.vertexBuffer);
		newMesh.m_indexCount = UINT(meshData.indices.size());
		D3D11Utils::CreateIndexBuffer(device, meshData.indices, newMesh.indexBuffer);

		if (!meshData.textureFilename.empty())
		{
			// https://opengameart.org/content/3-crate-textures-w-bump-normal
			// https://learnopengl.com/Getting-started/Textures
			std::cout << meshData.textureFilename << std::endl;
			D3D11Utils::CreateTexture(device, meshData.textureFilename, newMesh.texture,
			                          newMesh.textureResourceView);
		}

		newMesh.vertexConstantBuffer = m_vertexConstantBuffer;

		m_meshes.push_back(std::move(newMesh));
	}

	// 各頂点に開始点と終了点を作り、Normal可視化用のLineを構成する。
	std::vector<Vertex> normalVertices;
	std::vector<uint32_t> normalIndices;
	normalVertices.reserve(totalVertexCount * 2);
	normalIndices.reserve(totalVertexCount * 2);

	// 複数MeshのNormal Lineを一つのDebug Meshへまとめる。
	size_t offset = 0;
	for (const auto &meshData : meshes)
	{
		for (size_t i = 0; i < meshData.vertices.size(); i++)
		{

			auto v = meshData.vertices[i];

			v.texcoord.x = 0.0f; // 開始点
			normalVertices.push_back(v);

			v.texcoord.x = 1.0f; // 終了点
			normalVertices.push_back(v);

			normalIndices.push_back(uint32_t(2 * (i + offset)));
			normalIndices.push_back(uint32_t(2 * (i + offset) + 1));
		}
		offset += meshData.vertices.size();
	}

	D3D11Utils::CreateVertexBuffer(device, normalVertices, m_normalLines.vertexBuffer);
	m_normalLines.m_indexCount = UINT(normalIndices.size());
	D3D11Utils::CreateIndexBuffer(device, normalIndices, m_normalLines.indexBuffer);

	D3D11Utils::CreateVertexShaderAndInputLayout(device, L"NormalVertexShader.hlsl",
	                                             classicLitInputElements, m_normalVertexShader,
	                                             m_inputLayout);
	D3D11Utils::CreatePixelShader(device, L"NormalPixelShader.hlsl", m_normalPixelShader);

	D3D11Utils::CreateConstantBuffer(device, m_normalVertexConstantData,
	                                 m_normalVertexConstantBuffer);
}

void MeshGroup::SetEnvironmentMaps(ID3D11ShaderResourceView *diffuse,
                                   ID3D11ShaderResourceView *specular)
{
	m_diffuseIBLSRV = diffuse;
	m_specularIBLSRV = specular;
}

void MeshGroup::SetTransformConstants(const Matrix &modelWorld, const Matrix &invTranspose,
                                      const Matrix &view, const Matrix &projection) noexcept
{
	m_vertexConstantData.modelWorld = modelWorld;
	m_vertexConstantData.invTranspose = invTranspose;
	m_vertexConstantData.view = view;
	m_vertexConstantData.projection = projection;
}

void MeshGroup::SetActiveLight(std::size_t index, const Light &light)
{
	if (index >= MaxLights)
	{
		throw std::out_of_range("Active light index is out of range");
	}

	for (auto &currentLight : m_lightingConstantData.lights)
	{
		currentLight.strength = Vector3(0.0f);
	}

	m_lightingConstantData.lights[index] = light;
}

void MeshGroup::SetEyePosition(const Vector3 &eyePosition) noexcept
{
	m_lightingConstantData.eyeWorld = eyePosition;
}

RenderSettings &MeshGroup::GetRenderSettings() noexcept
{
	return m_renderSettings;
}

const RenderSettings &MeshGroup::GetRenderSettings() const noexcept
{
	return m_renderSettings;
}

bool MeshGroup::GetDrawNormals() const noexcept
{
	return m_drawNormals;
}

void MeshGroup::SetDrawNormals(bool enabled) noexcept
{
	m_drawNormals = enabled;
}

float MeshGroup::GetNormalScale() const noexcept
{
	return m_normalVertexConstantData.scale;
}

void MeshGroup::SetNormalScale(float scale) noexcept
{
	if (m_normalVertexConstantData.scale == scale)
	{
		return;
	}

	m_normalVertexConstantData.scale = scale;
	m_drawNormalsDirtyFlag = true;
}

void MeshGroup::UpdateConstantBuffers(ID3D11DeviceContext *context)
{
	ApplyRenderSettings();

	D3D11Utils::UpdateBuffer(context, m_vertexConstantData, m_vertexConstantBuffer.Get());
	D3D11Utils::UpdateBuffer(context, m_lightingConstantData, m_lightingConstantBuffer.Get());
	D3D11Utils::UpdateBuffer(context, m_shadingConstantData, m_shadingConstantBuffer.Get());

	// Scale変更時のみNormal描画用Constant Bufferを更新する。
	if (m_drawNormals && m_drawNormalsDirtyFlag)
	{
		D3D11Utils::UpdateBuffer(context, m_normalVertexConstantData,
		                         m_normalVertexConstantBuffer.Get());
		m_drawNormalsDirtyFlag = false;
	}
}

void MeshGroup::Render(ID3D11DeviceContext *context)
{
	// 通常Mesh描画用のShaderとSamplerをBindする。
	context->VSSetShader(m_vertexShader.Get(), 0, 0);
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	context->PSSetShader(m_pixelShader.Get(), 0, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	for (const auto &mesh : m_meshes)
	{
		context->IASetInputLayout(m_inputLayout.Get());
		context->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->VSSetConstantBuffers(0, 1, mesh.vertexConstantBuffer.GetAddressOf());

		// Material TextureとIBL CubemapをまとめてBindする。
		ID3D11ShaderResourceView *resViews[3] = {mesh.textureResourceView.Get(),
		                                         m_diffuseIBLSRV.Get(), m_specularIBLSRV.Get()};
		context->PSSetShaderResources(0, 3, resViews);

		ID3D11Buffer *pixelConstantBuffers[2] = {m_lightingConstantBuffer.Get(),
		                                         m_shadingConstantBuffer.Get()};
		context->PSSetConstantBuffers(0, 2, pixelConstantBuffers);

		context->DrawIndexed(mesh.m_indexCount, 0, 0);
	}

	// Normal表示有効時は同じModel TransformでLine Meshを追加描画する。
	if (m_drawNormals)
	{
		context->IASetInputLayout(m_inputLayout.Get());
		context->IASetVertexBuffers(0, 1, m_normalLines.vertexBuffer.GetAddressOf(), &stride,
		                            &offset);
		context->IASetIndexBuffer(m_normalLines.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->VSSetShader(m_normalVertexShader.Get(), 0, 0);
		ID3D11Buffer *pptr[2] = {m_vertexConstantBuffer.Get(), m_normalVertexConstantBuffer.Get()};
		context->VSSetConstantBuffers(0, 2, pptr);
		context->PSSetShader(m_normalPixelShader.Get(), 0, 0);

		context->DrawIndexed(m_normalLines.m_indexCount, 0, 0);
	}
}

void MeshGroup::ApplyRenderSettings() noexcept
{
	m_shadingConstantData.material = m_renderSettings.material;

	m_shadingConstantData.shadingOptions.useTexture = m_renderSettings.shading.useTexture;
	m_shadingConstantData.shadingOptions.shadingModel =
	    static_cast<uint32_t>(m_renderSettings.shading.model);

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
