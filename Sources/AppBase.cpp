// Header include order:
// https://google.github.io/styleguide/cppguide.html#Names_and_Order_of_Includes

#include "AppBase.h"

#include <algorithm>

#include "D3D11Utils.h"

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언.
// vcpkg로 ImGui를 사용할 때 IDE 경고가 보일 수 있다.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

namespace Ryudar {

using namespace std;

// Win32 콜백 함수에서 현재 AppBase 인스턴스로 메시지를 전달하기 위한 포인터.
AppBase *g_appBase = nullptr;

// RegisterClassEx에 등록되는 실제 Win32 메시지 콜백 함수.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return g_appBase->MsgProc(hWnd, msg, wParam, lParam);
}

AppBase::AppBase()
    : m_screenWidth(1280), m_screenHeight(720), m_mainWindow(0),
      m_screenViewport(D3D11_VIEWPORT()) {
    g_appBase = this;

    m_camera.SetAspectRatio(this->GetAspectRatio());
}

AppBase::~AppBase() {
    g_appBase = nullptr;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow(m_mainWindow);
    // UnregisterClass(wc.lpszClassName, wc.hInstance); 생략

    // COMPtr가 참조 카운트를 관리하므로 Direct3D 리소스는 자동으로 해제된다.
    // ComPtr automatically maintains a reference count for the underlying
    // interface pointer and releases the interface when the reference count
    // goes to zero.
    // https:learn.microsoft.com/en-us/cpp/cppcx/wrl/comptr-class?view=msvc-170
    // Example: m_d3dDevice.Reset(); can be omitted.
}

float AppBase::GetAspectRatio() const { return float(m_screenWidth - m_guiWidth) / m_screenHeight; }

int AppBase::Run() {
    // 메인 메시지 루프.
    MSG msg = {0};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();

            ImGui::NewFrame();
            ImGui::Begin("Scene Control");

            ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);

            UpdateGUI();

            m_guiWidth = 0;
            // GUI 폭만큼 렌더링 영역을 줄이고 싶을 때 사용.
            // ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
            // m_guiWidth = int(ImGui::GetWindowWidth());

            ImGui::End();
            ImGui::Render();

            Update(ImGui::GetIO().DeltaTime);

            Render();

            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // ImGui 렌더링 후 back buffer와 front buffer를 교체한다.
            m_swapChain->Present(1, 0);
        }
    }

    return 0;
}

bool AppBase::Initialize() {
    if (!InitMainWindow())
        return false;

    if (!InitDirect3D())
        return false;

    if (!InitGUI())
        return false;

    // 콘솔 창이 렌더링 창을 가리지 않도록 렌더링 창을 앞으로 가져온다.
    SetForegroundWindow(m_mainWindow);

    return true;
}

void AppBase::OnMouseMove(WPARAM btnState, int mouseX, int mouseY) {

    // 마우스 좌표를 NDC 좌표계로 변환한다.
    // 화면 좌측 상단은 (0, 0), 우측 하단은 (width - 1, height - 1)이다.
    // NDC 좌표계는 좌측 하단이 (-1, -1), 우측 상단이 (1, 1)이다.
    float x = mouseX * 2.0f / m_screenWidth - 1.0f;
    float y = -mouseY * 2.0f / m_screenHeight + 1.0f;

    // 커서가 화면 밖으로 나갔을 때도 값이 유효 범위에 머물도록 제한한다.
    x = std::clamp(x, -1.0f, 1.0f);
    y = std::clamp(y, -1.0f, 1.0f);

    if (m_useFirstPersonView) {
        m_camera.UpdateMouse(x, y);
    }
}

// hwnd: 메시지를 받은 윈도우.
// msg, wParam, lParam: Win32 메시지와 추가 데이터.
LRESULT AppBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (m_swapChain) {
            std::cout << (UINT)LOWORD(lParam) << " " << (UINT)HIWORD(lParam) << std::endl;
            m_screenWidth = int(LOWORD(lParam));
            m_screenHeight = int(HIWORD(lParam));
            m_guiWidth = 0;

            m_renderTargetView.Reset();
            m_swapChain->ResizeBuffers(0,                    // 현재 버퍼 개수 유지
                                       (UINT)LOWORD(lParam), // 변경된 너비
                                       (UINT)HIWORD(lParam), // 변경된 높이
                                       DXGI_FORMAT_UNKNOWN,  // 기존 포맷 유지
                                       0);
            CreateRenderTargetView();
            D3D11Utils::CreateDepthBuffer(m_device, m_screenWidth, m_screenHeight, numQualityLevels,
                                          m_depthStencilView);
            SetViewport();
        }
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_LBUTTONUP:
        break;
    case WM_RBUTTONUP:
        break;
    case WM_KEYDOWN:
        m_keyPressed[wParam] = true;

        if (wParam == 27) {
            // ESC 키를 누르면 프로그램을 종료한다.
            DestroyWindow(hwnd);
        }
        break;
    case WM_KEYUP:
        // 키보드 눌림 상태를 해제한다.
        m_keyPressed[wParam] = false;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

bool AppBase::InitMainWindow() {
    WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"Ryudar",
                     NULL};

    // RegisterClassEx로 윈도우 클래스를 등록한다.
    if (!RegisterClassEx(&wc)) {
        cout << "RegisterClassEx() failed." << endl;
        return false;
    }

    // 원하는 클라이언트 영역 크기를 기준으로 실제 윈도우 크기를 계산한다.
    RECT wr = {0, 0, m_screenWidth, m_screenHeight};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

    m_mainWindow = CreateWindow(wc.lpszClassName, L"Ryudar Engine", WS_OVERLAPPEDWINDOW,
                                100,                // 윈도우 좌측 상단 x 좌표
                                100,                // 윈도우 좌측 상단 y 좌표
                                wr.right - wr.left, // 윈도우 전체 너비
                                wr.bottom - wr.top, // 윈도우 전체 높이
                                NULL, NULL, wc.hInstance, NULL);

    if (!m_mainWindow) {
        cout << "CreateWindow() failed." << endl;
        return false;
    }

    ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
    UpdateWindow(m_mainWindow);

    return true;
}

bool AppBase::InitDirect3D() {
    // 하드웨어 드라이버 생성에 실패하는 환경에서는 WARP 드라이버로 바꿔 테스트할 수 있다.
    // const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_WARP;
    const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    const D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_9_3};
    D3D_FEATURE_LEVEL featureLevel;

    if (FAILED(D3D11CreateDevice(nullptr,
                                 driverType,
                                 0,
                                 createDeviceFlags,
                                 featureLevels,
                                 ARRAYSIZE(featureLevels),
                                 D3D11_SDK_VERSION,
                                 device.GetAddressOf(),
                                 &featureLevel,
                                 context.GetAddressOf()))) {
        cout << "D3D11CreateDevice() failed." << endl;
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        cout << "D3D Feature Level 11 unsupported." << endl;
        return false;
    }

    // 4x MSAA 지원 여부를 확인한다.
    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQualityLevels);
    if (numQualityLevels <= 0) {
        cout << "MSAA not supported." << endl;
    }

    // numQualityLevels = 0; // MSAA를 강제로 끄고 싶을 때 사용.

    if (FAILED(device.As(&m_device))) {
        cout << "device.As() failed." << endl;
        return false;
    }
    if (FAILED(context.As(&m_context))) {
        cout << "context.As() failed" << endl;
        return false;
    }

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferDesc.Width = m_screenWidth;
    sd.BufferDesc.Height = m_screenHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferCount = 2;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_mainWindow;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    if (numQualityLevels > 0) {
        sd.SampleDesc.Count = 4;
        sd.SampleDesc.Quality = numQualityLevels - 1;
    } else {
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
    }

    // 여기서는 swap chain을 함께 생성하기 위해 device/context를 다시 받는다.
    if (FAILED(D3D11CreateDeviceAndSwapChain(
            0,
            driverType,
            0,
            createDeviceFlags, featureLevels, 1, D3D11_SDK_VERSION, &sd, m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(), &featureLevel, m_context.GetAddressOf()))) {
        cout << "D3D11CreateDeviceAndSwapChain() failed." << endl;
        return false;
    }

    CreateRenderTargetView();
    SetViewport();

    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;

    m_device->CreateRasterizerState(&rastDesc, m_solidRasterizerState.GetAddressOf());

    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
    m_device->CreateRasterizerState(&rastDesc, m_wireRasterizerState.GetAddressOf());

    D3D11Utils::CreateDepthBuffer(m_device, m_screenWidth, m_screenHeight, numQualityLevels,
                                  m_depthStencilView);

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
    if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc,
                                                 m_depthStencilState.GetAddressOf()))) {
        cout << "CreateDepthStencilState() failed. " << endl;
    }

    return true;
}

bool AppBase::InitGUI() {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.DisplaySize = ImVec2(float(m_screenWidth), float(m_screenHeight));
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();
    // ImGui::StyleColorsLight();

    if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get())) {
        return false;
    }

    if (!ImGui_ImplWin32_Init(m_mainWindow)) {
        return false;
    }

    return true;
}

void AppBase::SetViewport() {
    static int previousGuiWidth = -1;
    // 윈도우 크기 변경을 막고 싶으면 m_guiWidth 기준 캐시를 사용할 수 있다.
    // static int previousGuiWidth = m_guiWidth;

    if (previousGuiWidth != m_guiWidth) {
        previousGuiWidth = m_guiWidth;
        ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
        m_screenViewport.TopLeftX = float(m_guiWidth);
        m_screenViewport.TopLeftY = 0;
        m_screenViewport.Width = float(m_screenWidth - m_guiWidth);
        m_screenViewport.Height = float(m_screenHeight);
        m_screenViewport.MinDepth = 0.0f;
        m_screenViewport.MaxDepth = 1.0f;

        m_context->RSSetViewports(1, &m_screenViewport);
    }
}

bool AppBase::CreateRenderTargetView() {
    ComPtr<ID3D11Texture2D> backBuffer;

    // Swap chain에서 back buffer를 가져와 render target과 shader resource를 만든다.
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (backBuffer) {
        m_device->CreateRenderTargetView(backBuffer.Get(), NULL, m_renderTargetView.GetAddressOf());
        m_device->CreateShaderResourceView(backBuffer.Get(), nullptr,
                                           m_shaderResourceView.GetAddressOf());

        D3D11_TEXTURE2D_DESC desc;
        backBuffer->GetDesc(&desc);
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = 0;

        if (FAILED(m_device->CreateTexture2D(&desc, nullptr, m_tempTexture.GetAddressOf()))) {
            cout << "Failed()" << endl;
        }

        // MSAA back buffer는 직접 shader resource로 읽기 어려우므로 tempTexture를 사용한다.
        m_device->CreateShaderResourceView(m_tempTexture.Get(), nullptr,
                                           m_shaderResourceView.GetAddressOf());
    } else {
        cout << "CreateRenderTargetView() failed. " << endl;
        return false;
    }
    return true;
}

} // namespace Ryudar
