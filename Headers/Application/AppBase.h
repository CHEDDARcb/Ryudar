#pragma once
// Win32 윈도우, D3D11 기본 리소스, ImGui 프레임 루프를 관리하는 앱 기반 클래스.
// 파생 클래스는 씬별 GUI, 업데이트, 렌더링 로직만 구현한다.

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>

#include "Rendering/Camera.h"
#include "Graphics/D3D11Utils.h"

namespace Ryudar
{

using Microsoft::WRL::ComPtr;
using std::vector;
using std::wstring;

class AppBase
{
public:
	AppBase();
	virtual ~AppBase();

	// 렌더링 영역의 가로/세로 비율을 반환한다.
	// GUI 영역을 제외한 화면을 기준으로 카메라 투영에 사용한다.
	float GetAspectRatio() const noexcept;

	// Win32 메시지 루프를 실행하고 매 프레임 GUI, Update, Render를 호출한다.
	int Run();

	// Win32 창, D3D11 장치와 스왑 체인, ImGui 등 공통 실행 환경을 초기화한다.
	virtual bool Initialize();

	// 파생 클래스가 매 프레임 ImGui 컨트롤을 구성하는 단계.
	virtual void UpdateGUI() = 0;

	// 파생 클래스가 입력, 씬 상태, 상수 버퍼를 갱신하는 단계.
	virtual void Update(float dt) = 0;

	// 파생 클래스가 실제 D3D 렌더링 명령을 기록하는 단계.
	virtual void Render() = 0;

	// AppBase가 창 크기에 의존하는 공통 D3D 리소스를 갱신한 뒤 호출한다.
	// 파생 클래스는 화면 크기에 의존하는 리소스가 있으면 여기서 재생성한다.
	virtual void OnResize() {}

	// Win32 메시지를 처리한다.
	// ImGui 입력 전달, 키와 마우스 상태, 창 크기 변경에 따른 리소스 재생성을 담당한다.
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// 필요한 파생 클래스에서만 오버라이드하는 입력 훅.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {};
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {};
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
	// Win32 윈도우 클래스를 등록하고 메인 창을 생성한다.
	bool InitMainWindow();

	// D3D11 장치, 스왑 체인, 렌더/깊이 타깃, 래스터라이저 상태를 생성한다.
	bool InitDirect3D();

	// Win32/D3D11 백엔드와 ImGui 컨텍스트를 초기화한다.
	bool InitGUI();

	// 현재 클라이언트 크기에 맞춰 뷰포트와 카메라 종횡비를 갱신한다.
	void SetViewport();

	// 스왑 체인 백 버퍼 기반 RTV와 후처리 입력 텍스처/SRV를 생성한다.
	// 창 크기가 바뀌면 기존 뷰를 해제한 뒤 다시 호출한다.
	void CreateRenderTargetView();

protected:
	// 윈도우 상태
	int m_screenWidth; // 클라이언트 렌더링 영역의 너비
	int m_screenHeight;
	int m_guiWidth = 0;
	HWND m_mainWindow;

	// D3D 핵심 객체
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;
	UINT numQualityLevels = 0;

	// 렌더 타겟
	// 스왑 체인 백 버퍼에 렌더링하기 위한 화면 출력 대상.
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	// 후처리 입력 리소스
	// MSAA 백 버퍼를 후처리 입력으로 사용하기 위해 리졸브하는 임시 텍스처.
	ComPtr<ID3D11Texture2D> m_postProcessInputTexture;

	// 후처리 셰이더가 m_postProcessInputTexture를 읽기 위한 뷰.
	ComPtr<ID3D11ShaderResourceView> m_postProcessInputSRV;

	// 래스터라이저 상태
	ComPtr<ID3D11RasterizerState> m_solidRasterizerState;
	ComPtr<ID3D11RasterizerState> m_wireRasterizerState;

	// 뷰포트와 카메라
	D3D11_VIEWPORT m_screenViewport;

	Camera m_camera;

	// 실행 옵션과 입력 상태
	bool m_drawAsWire = false;
	bool m_usePostProcessing = false;
	bool m_useFirstPersonView = false;

	// 부분 초기화 실패 시 생성된 ImGui 리소스만 안전하게 정리하기 위한 상태.
	bool m_imguiContextInitialized = false;
	bool m_imguiDx11Initialized = false;
	bool m_imguiWin32Initialized = false;

	// Win32 키 코드별 눌림 상태.
	bool m_keyPressed[256] = {
	    false,
	};
};
} // namespace Ryudar
