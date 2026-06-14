#pragma once

// 씬 객체의 이동, 회전, 크기를 저장하고 모델 행렬로 변환한다.
// 회전값은 라디안 단위이며 Y, X, Z 순서로 적용한다.

#include <directxtk/SimpleMath.h>

namespace Ryudar
{

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

struct Transform
{
	Vector3 translation = Vector3(0.0f);
	Vector3 rotation = Vector3(0.0f);
	Vector3 scaling = Vector3(1.0f);

	Matrix GetModelMatrix() const noexcept
	{
		return Matrix::CreateScale(scaling) * Matrix::CreateRotationY(rotation.y) *
		       Matrix::CreateRotationX(rotation.x) * Matrix::CreateRotationZ(rotation.z) *
		       Matrix::CreateTranslation(translation);
	}
};

} // namespace Ryudar
