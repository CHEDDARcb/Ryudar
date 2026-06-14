#pragma once
// First-Person Cameraの位置、向き、Projection設定を管理する。
// 入力変化に応じてView MatrixとProjection Matrixを計算し、レンダリング処理へ提供する。

#include <directxtk/SimpleMath.h>

namespace Ryudar
{
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

enum class ProjectionType
{
	Perspective,
	Orthographic,
};

class Camera
{
public:
	// 現在のCamera位置と向きからView Matrixを計算する。
	Matrix GetViewRow() const noexcept;
	// 選択されたProjection方式に合うProjection Matrixを計算する。
	Matrix GetProjRow() const noexcept;
	// World座標系のCamera位置を返す。
	Vector3 GetEyePos() const noexcept { return m_position; }

	// Mouse入力に応じてCameraの回転と移動方向を更新する。
	// mouseNdcX/Yは画面中央が(0,0)、左下が(-1,-1)、右上が(1,1)となる正規化座標。
	void UpdateMouse(float mouseNdcX, float mouseNdcY) noexcept;

	// Keyboard入力に応じてCameraを前後・左右へ移動する。
	// dtは前フレームとの差分時間で、移動量をFrame Rateから独立させるために使用する。
	void MoveForward(float dt) noexcept;
	void MoveRight(float dt) noexcept;

	// Projection Matrix計算に使用する画面のアスペクト比を更新する。
	void SetAspectRatio(float aspect) noexcept;

	// Camera位置と向きを初期視点へ戻す。
	void ResetCameraSet() noexcept;

private:
	// Camera基準ベクトル
	Vector3 m_position = Vector3(0.0f, 0.4f, 0.0f);
	Vector3 m_viewDir = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 m_upDir = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 m_rightDir = Vector3(1.0f, 0.0f, 0.0f);

	// Camera回転・移動設定
	// https://en.wikipedia.org/wiki/Aircraft_principal_axes
	float m_pitch = 0.0f; // x軸
	float m_yaw = 0.0f;   // y軸

	float m_speed = 1.0f;

	// Projection設定
	float m_projFovAngleY = 70.0f;
	float m_nearZ = 0.01f;
	float m_farZ = 100.0f;
	float m_aspect = 16.0f / 9.0f;
	ProjectionType m_projectionType = ProjectionType::Perspective;
};
} // namespace Ryudar
