#include "Rendering/Camera.h"
#include <cassert>
#include <iostream>
namespace Ryudar
{

using namespace std;
using namespace DirectX;

Matrix Camera::GetViewRow() const noexcept
{
	return Matrix::CreateTranslation(-m_position) * Matrix::CreateRotationY(-m_yaw) *
	       Matrix::CreateRotationX(m_pitch);
}

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY) noexcept
{
	// 正規化画面座標を上下180度、左右360度の回転へ変換する。
	// https://en.wikipedia.org/wiki/Aircraft_principal_axes
	m_pitch = mouseNdcY * DirectX::XM_PIDIV2;
	m_yaw = mouseNdcX * DirectX::XM_2PI;

	// 回転後のForward Vectorを基準に、移動用のRight Vectorも再計算する。
	m_viewDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), Matrix::CreateRotationX(-m_pitch) *
	                                                              Matrix::CreateRotationY(m_yaw));
	m_rightDir = m_upDir.Cross(m_viewDir);
}

void Camera::MoveForward(float dt) noexcept
{
	m_position += m_viewDir * m_speed * dt;
}

void Camera::MoveRight(float dt) noexcept
{
	m_position += m_rightDir * m_speed * dt;
}

void Camera::SetAspectRatio(float aspect) noexcept { m_aspect = aspect; }

void Camera::ResetCameraSet() noexcept
{
	m_position = Vector3(0.0f, 0.4f, 0.0f);
	m_viewDir = Vector3(0.0f, 0.0f, 1.0f);
	m_upDir = Vector3(0.0f, 1.0f, 0.0f);
	m_rightDir = Vector3(1.0f, 0.0f, 0.0f);

	m_pitch = 0.0f;
	m_yaw = 0.0f;
}

Matrix Camera::GetProjRow() const noexcept
{
	switch (m_projectionType)
	{
	case ProjectionType::Perspective:
		return XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY), m_aspect, m_nearZ,
		                                m_farZ);

	case ProjectionType::Orthographic:
		return XMMatrixOrthographicOffCenterLH(-m_aspect, m_aspect, -1.0f, 1.0f, m_nearZ, m_farZ);
	}

	assert(false && "Invalid ProjectionType");
	return Matrix();
}

} // namespace Ryudar
