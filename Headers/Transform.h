#pragma once

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

	Matrix GetModelMatrix() const
	{
		return Matrix::CreateScale(scaling) * Matrix::CreateRotationY(rotation.y) *
		       Matrix::CreateRotationX(rotation.x) * Matrix::CreateRotationZ(rotation.z) *
		       Matrix::CreateTranslation(translation);
	}
};

} // namespace Ryudar