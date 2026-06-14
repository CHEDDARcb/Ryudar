#pragma once
// GPU 렌더링에 필요한 정점/인덱스 버퍼, 상수 버퍼, 텍스처 뷰를 묶는 구조체.
// 실제 렌더링 소유자는 ClassicLit::MeshGroup이나 ImageFilter가 담당한다.

#include <d3d11.h>
#include <windows.h>
#include <wrl.h> // ComPtr

namespace Ryudar
{
using Microsoft::WRL::ComPtr;

struct Mesh
{
	// 지오메트리 버퍼
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	UINT m_indexCount = 0;

	// 소유한 메시 그룹이나 필터가 공유하는 셰이더 상수 버퍼.
	ComPtr<ID3D11Buffer> vertexConstantBuffer;
	ComPtr<ID3D11Buffer> pixelConstantBuffer;

	// 선택적으로 사용하는 재질 텍스처.
	ComPtr<ID3D11Texture2D> texture;
	ComPtr<ID3D11ShaderResourceView> textureResourceView;
};
} // namespace Ryudar
