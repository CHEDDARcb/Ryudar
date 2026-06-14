#pragma once
// 1인칭 카메라의 위치, 방향, 투영 설정을 관리한다.
// 입력 변화에 따라 뷰 행렬과 투영 행렬을 계산해 렌더링 코드에 제공한다.

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
	// 현재 카메라 위치와 방향으로 뷰 행렬을 계산한다.
	Matrix GetViewRow() const noexcept;
	// 선택된 투영 방식에 맞는 투영 행렬을 계산한다.
	Matrix GetProjRow() const noexcept;
	// 월드 좌표계의 카메라 위치를 반환한다.
	Vector3 GetEyePos() const noexcept { return m_position; }

	// 마우스 입력에 따라 카메라의 회전과 이동 방향을 업데이트한다.
	// mouseNdcX/Y는 화면 중앙이 (0,0), 왼쪽 아래가 (-1,-1), 오른쪽 위가 (1,1)인 정규화된 마우스
	// 좌표이다.
	void UpdateMouse(float mouseNdcX, float mouseNdcY) noexcept;

	// 키보드 입력에 따라 카메라를 앞/뒤, 좌/우로 이동시킨다.
	// dt는 이전 프레임과의 시간 차이로, 이동 거리를 프레임 속도에 맞춰 조절하는 데 사용한다.
	void MoveForward(float dt) noexcept;
	void MoveRight(float dt) noexcept;

	// 투영 행렬 계산에 사용하는 화면 종횡비를 갱신한다.
	void SetAspectRatio(float aspect) noexcept;

	// 카메라 위치와 방향을 초기 시점으로 되돌린다.
	void ResetCameraSet() noexcept;

private:
	// 카메라 기준 벡터
	Vector3 m_position = Vector3(0.0f, 0.4f, 0.0f);
	Vector3 m_viewDir = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 m_upDir = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 m_rightDir = Vector3(1.0f, 0.0f, 0.0f);

	// 카메라 회전과 이동 설정
	// https://en.wikipedia.org/wiki/Aircraft_principal_axes
	float m_pitch = 0.0f; // x축
	float m_yaw = 0.0f;   // y축

	float m_speed = 1.0f;

	// 프로젝션 설정
	float m_projFovAngleY = 70.0f;
	float m_nearZ = 0.01f;
	float m_farZ = 100.0f;
	float m_aspect = 16.0f / 9.0f;
	ProjectionType m_projectionType = ProjectionType::Perspective;
};
} // namespace Ryudar
