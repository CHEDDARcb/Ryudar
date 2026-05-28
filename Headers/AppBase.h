#pragma once

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <iostream>
#include <vector>
// #include "./ImGuizmo/ImGuizmo.h"

#include "Camera.h"
#include "D3D11Utils.h"

namespace Ryudar {

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

// 모드 프로그램이 공통적으로 사요할 기능을 가지고있는 부모 클래스
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

    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // 마우스 조작을 위한 편의오버라이드
    virtual void OnMouseDown(WPARAM btnState, int x, int y){};
    virtual void OnMouseUp(WPARAM btnState, int x, int y){};
    virtual void OnMouseMove(WPARAM btnState, int x, int y);

  protected:
    bool InitMainWindow();
    bool InitDirect3D();
    bool InitGUI();

    void SetViewport();
    bool CreateRenderTargetView();

  public:
    int m_screenWidth; // 렌더링할 최종 화면의 해상도
    int m_screenHeight;
    int m_guiWidth = 0;
    HWND m_mainWindow;
    UINT numQualityLevels = 0;

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;

    //  GPU가 그린 이미지를 저장하고 화면에 출력하는 역할을 하며, 주로 화면에 렌더링할 결과를 담는
    //  용도로 사용
    // ->"그리기 대상"
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    // 셰이더에서 리소스를 읽을 수 있도록 하여, 그래픽 효과를 위해 필요한 텍스처 또는 데이터를
    // 제공하는 역할
    // ->"읽기 전용 데이터"
    // 렌더링결과(renderTargetView)를 쉐이더(shaderResourceView)에 넣주는 개념.
    // 결과적으로 renderTargetView와 shaderResourceView는 내부적으로 같은 메모리를 지정
    ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    ComPtr<IDXGISwapChain> m_swapChain;
    // 렌더타겟의 Texture2DMS를 복사할 임시 텍스춰
    ComPtr<ID3D11Texture2D> m_tempTexture;

    ComPtr<ID3D11RasterizerState> m_solidRasterizerState;
    ComPtr<ID3D11RasterizerState> m_wireRasterizerState;
    bool m_drawAsWire = false;
    bool m_usePostProcessing = false;

    // Depth buffer
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    D3D11_VIEWPORT m_screenViewport;

    // 시점을 결정하는 카메라
    Camera m_camera;
    bool m_useFirstPersonView = false;
    // 현재 키보드가 눌렸는지 상태를 저장하는 배열
    bool m_keyPressed[256] = {
        false,
    };
};
} // namespace Ryudar