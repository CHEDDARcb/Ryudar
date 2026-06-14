#pragma once
// 1인칭 카메라의 위치, 방향, projection 설정을 관리한다.
// 입력 변화에 따라 view/projection 행렬을 계산해 렌더링 코드에 제공한다.

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
	// 현재 카메라의 위치와 방향을 고려해서 View행렬 반환
	Matrix GetViewRow();
	// 동일한 projection행렬 반환
	Matrix GetProjRow();
	// 카메라의 위치 반환
	Vector3 GetEyePos();

	// 마우스 입력에 따라 카메라의 회전과 이동 방향을 업데이트한다.
	// mouseNdcX/Y는 화면 중앙이 (0,0), 왼쪽 아래가 (-1,-1), 오른쪽 위가 (1,1)인 정규화된 마우스
	// 좌표이다.
	void UpdateMouse(float mouseNdcX, float mouseNdcY);

	// 키보드 입력에 따라 카메라를 앞/뒤, 좌/우로 이동시킨다.
	// dt는 이전 프레임과의 시간 차이로, 이동 거리를 프레임 속도에 맞춰 조절하는 데 사용한다.
	void MoveForward(float dt);
	void MoveRight(float dt);

	// 카메라가 내부적으로 가지고 있는 AspectRatio를 업데이트
	// projection행렬 계산을 위해 내부적으로 AspectRatio를 가지고 있음
	void SetAspectRatio(float aspect);

	void ResetCameraSet();

private:
	// m_position : 월드 좌표계에서 카메라의 위치
	// m_viewDir : 카메라가 보는 방향, 걸어가는 방향
	// m_upDir : 위쪽 방향, 중력의 반대방향이 기본
	// m_rightDir : 오른쪽 방향, eyeDir과 upDir로부터 계산

	// 카메라 기준 벡터
	Vector3 m_position = Vector3(0.0f, 0.4f, 0.0f); // 0.15f는 눈높이 정도
	Vector3 m_viewDir = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 m_upDir = Vector3(0.0f, 1.0f, 0.0f); // 고정
	// viewDir이 회전하면 rightDir도 같이 회전
	Vector3 m_rightDir = Vector3(1.0f, 0.0f, 0.0f);

	// 회전과 이동 설정
	// roll, pitch, yaw
	// https://en.wikipedia.org/wiki/Aircraft_principal_axes
	float m_pitch = 0.0f; // x축
	float m_yaw = 0.0f;   // y축

	float m_speed = 1.0f; // 움직이는 속도

	// 프로젝션 설정
	float m_projFovAngleY = 70.0f;
	float m_nearZ = 0.01f;
	float m_farZ = 100.0f;
	float m_aspect = 16.0f / 9.0f;
	ProjectionType m_projectionType = ProjectionType::Perspective;
};
} // namespace Ryudar
