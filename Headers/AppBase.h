#pragma once

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>

#include "Camera.h"
#include "D3D11Utils.h"

namespace Ryudar
{

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

// Win32 window, D3D11 device/swap chain, ImGui frame loop를 관리하는 앱 기반 클래스.
// 파생 클래스는 GUI, update, render 단계만 구현해 데모 씬을 구성한다.
class AppBase
{
public:
	AppBase();
	virtual ~AppBase();

	float GetAspectRatio() const;

	int Run();

	virtual bool Initialize();
	virtual void UpdateGUI() = 0;
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;

	// AppBase가 resize에 필요한 공통 D3D 리소스를 갱신한 뒤 호출한다.
	// 파생 클래스는 화면 크기에 의존하는 리소스가 있으면 여기서 재생성한다.
	virtual void OnResize() {}

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// 필요한 파생 클래스에서만 오버라이드하는 입력 훅.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {};
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {};
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
	bool InitMainWindow();
	bool InitDirect3D();
	bool InitGUI();

	void SetViewport();
	bool CreateRenderTargetView();

public:
	int m_screenWidth; // 클라이언트 렌더링 영역의 너비
	int m_screenHeight;
	int m_guiWidth = 0;
	HWND m_mainWindow;
	UINT numQualityLevels = 0;

	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;

	// Swap chain back buffer에 렌더링하기 위한 화면 출력 대상.
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	// 후처리 셰이더가 m_postProcessInputTexture를 읽기 위한 view.
	ComPtr<ID3D11ShaderResourceView> m_postProcessInputSRV;
	ComPtr<IDXGISwapChain> m_swapChain;

	// MSAA back buffer를 후처리 입력으로 사용하기 위해 resolve하는 임시 텍스처.
	ComPtr<ID3D11Texture2D> m_postProcessInputTexture;

	ComPtr<ID3D11RasterizerState> m_solidRasterizerState;
	ComPtr<ID3D11RasterizerState> m_wireRasterizerState;
	bool m_drawAsWire = false;
	bool m_usePostProcessing = false;

	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	D3D11_VIEWPORT m_screenViewport;

	Camera m_camera;
	bool m_useFirstPersonView = false;

	// Win32 key code별 눌림 상태.
	bool m_keyPressed[256] = {
	    false,
	};
};
} // namespace Ryudar
