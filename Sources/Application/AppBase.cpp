// 헤더 포함 순서 참고:
// https://google.github.io/styleguide/cppguide.html#Names_and_Order_of_Includes

#include "Application/AppBase.h"
#include "Graphics/D3D11Exception.h"

#include <algorithm>
#include <cstdlib>

#include "Graphics/D3D11Utils.h"

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언.
// vcpkg로 ImGui를 사용할 때 IDE 경고가 보일 수 있다.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

namespace Ryudar
{

using namespace std;

// Win32 콜백 함수에서 현재 AppBase 인스턴스로 메시지를 전달하기 위한 포인터.
AppBase *g_appBase = nullptr;

// RegisterClassEx에 등록되는 실제 Win32 메시지 콜백 함수.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (g_appBase)
	{
		return g_appBase->MsgProc(hWnd, msg, wParam, lParam);
	}

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

AppBase::AppBase()
    : m_screenWidth(1280)
    , m_screenHeight(720)
    , m_mainWindow(0)
    , m_screenViewport(D3D11_VIEWPORT())
{
	g_appBase = this;

	m_camera.SetAspectRatio(this->GetAspectRatio());
}

AppBase::~AppBase()
{
	// DestroyWindow는 WndProc를 동기적으로 호출하므로 AppBase와 ImGui가 살아 있을 때 처리한다.
	if (m_mainWindow)
	{
		DestroyWindow(m_mainWindow);
		m_mainWindow = nullptr;
	}

	if (m_imguiWin32Initialized)
	{
		ImGui_ImplWin32_Shutdown();
	}
	if (m_imguiDx11Initialized)
	{
		ImGui_ImplDX11_Shutdown();
	}
	if (m_imguiContextInitialized)
	{
		ImGui::DestroyContext();
	}

	g_appBase = nullptr;

	// UnregisterClass(wc.lpszClassName, wc.hInstance); 생략

	// ComPtr가 참조 카운트를 관리하므로 Direct3D 리소스는 자동으로 해제된다.
	// https:learn.microsoft.com/en-us/cpp/cppcx/wrl/comptr-class?view=msvc-170
}

float AppBase::GetAspectRatio() const noexcept
{
	return float(m_screenWidth - m_guiWidth) / m_screenHeight;
}

int AppBase::Run()
{
	// 메인 메시지 루프.
	MSG msg = {0};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
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

			// ImGui 렌더링 후 백 버퍼와 프런트 버퍼를 교체한다.
			ThrowIfFailed(m_swapChain->Present(1, 0), "Present swap chain");
		}
	}

	return static_cast<int>(msg.wParam);
}

bool AppBase::Initialize()
{
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

void AppBase::OnMouseMove(WPARAM btnState, int mouseX, int mouseY)
{

	// 마우스 좌표를 NDC 좌표계로 변환한다.
	// 화면 좌측 상단은 (0, 0), 우측 하단은 (width - 1, height - 1)이다.
	// NDC 좌표계는 좌측 하단이 (-1, -1), 우측 상단이 (1, 1)이다.
	float x = mouseX * 2.0f / m_screenWidth - 1.0f;
	float y = -mouseY * 2.0f / m_screenHeight + 1.0f;

	// 커서가 화면 밖으로 나갔을 때도 값이 유효 범위에 머물도록 제한한다.
	x = std::clamp(x, -1.0f, 1.0f);
	y = std::clamp(y, -1.0f, 1.0f);

	if (m_useFirstPersonView)
	{
		m_camera.UpdateMouse(x, y);
	}
}

// hwnd: 메시지를 받은 윈도우.
// msg, wParam, lParam: Win32 메시지와 추가 데이터.
LRESULT AppBase::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		try
		{
			if (m_swapChain)
			{
				m_screenWidth = int(LOWORD(lParam));
				m_screenHeight = int(HIWORD(lParam));

				if (m_screenWidth == 0 || m_screenHeight == 0)
				{
					break;
				}

				m_guiWidth = 0;

				// ResizeBuffers 전에 백 버퍼와 관련된 바인딩을 해제한다.
				m_context->OMSetRenderTargets(0, nullptr, nullptr);

				ID3D11ShaderResourceView *nullSRV[16] = {};
				m_context->PSSetShaderResources(0, 16, nullSRV);

				m_renderTargetView.Reset();
				m_postProcessInputSRV.Reset();
				m_depthStencilView.Reset();
				m_postProcessInputTexture.Reset();

				ThrowIfFailed(m_swapChain->ResizeBuffers(0, UINT(m_screenWidth),
				                                         UINT(m_screenHeight), DXGI_FORMAT_UNKNOWN,
				                                         0),
				              "Resize swap-chain buffers");

				CreateRenderTargetView();

				D3D11Utils::CreateDepthBuffer(m_device.Get(), m_screenWidth, m_screenHeight,
				                              numQualityLevels, m_depthStencilView);

				SetViewport();

				m_camera.SetAspectRatio(GetAspectRatio());
				OnResize();
			}
		}
		catch (const std::exception &exception)
		{
			std::cerr << "Resize failed: " << exception.what() << '\n';

			PostQuitMessage(EXIT_FAILURE);
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

		if (wParam == 27)
		{
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

bool AppBase::InitMainWindow()
{
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
	if (!RegisterClassEx(&wc))
	{
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

	if (!m_mainWindow)
	{
		cout << "CreateWindow() failed." << endl;
		return false;
	}

	ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
	UpdateWindow(m_mainWindow);

	return true;
}

bool AppBase::InitDirect3D()
{
	// 하드웨어 드라이버를 사용할 수 없는 환경에서는 WARP로 테스트할 수 있다.
	const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	// const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_WARP;

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 지원할 기능 수준을 높은 버전부터 요청한다.
	const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_9_3};

	D3D_FEATURE_LEVEL featureLevel;

	// D3D11 장치와 컨텍스트 생성
	ThrowIfFailed(D3D11CreateDevice(nullptr,           // 기본 어댑터 사용
	                                driverType,        // 하드웨어 / WARP
	                                0,                 // 소프트웨어 래스터라이저 모듈 핸들
	                                createDeviceFlags, // 디버그 계층 등의 생성 옵션
	                                featureLevels,     // 요청할 기능 수준 목록
	                                ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
	                                m_device.GetAddressOf(), &featureLevel,
	                                m_context.GetAddressOf()),
	              "Create D3D11 device and context");

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		cout << "D3D Feature Level 11 unsupported." << endl;
		return false;
	}

	// 4배 MSAA 지원 여부 확인
	numQualityLevels = 0;
	ThrowIfFailed(
	    m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQualityLevels),
	    "Check multisample quality levels");

	if (numQualityLevels == 0)
	{
		cout << "4x MSAA not supported. Fallback to no MSAA." << endl;
	}

	// D3D 장치에서 DXGI 장치, 어댑터, 팩터리 순서로 상위 객체를 얻는다.
	ComPtr<IDXGIDevice> dxgiDevice;
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ComPtr<IDXGIFactory> dxgiFactory;

	ThrowIfFailed(m_device.As(&dxgiDevice), "Query IDXGIDevice interface");

	ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()),
	              "Get DXGI adapter");

	ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory),
	                                     reinterpret_cast<void **>(dxgiFactory.GetAddressOf())),
	              "Get DXGI factory");

	// 스왑 체인 생성
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferDesc.Width = m_screenWidth;
	sd.BufferDesc.Height = m_screenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// 창 모드에서는 영향이 작지만 기본 새로 고침 빈도를 명시한다.
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	// sd.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// 백 버퍼 두 개로 더블 버퍼링한다.
	sd.BufferCount = 2;

	sd.OutputWindow = m_mainWindow;
	sd.Windowed = TRUE;

	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (numQualityLevels > 0)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = numQualityLevels - 1;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	ThrowIfFailed(dxgiFactory->CreateSwapChain(m_device.Get(), &sd, m_swapChain.GetAddressOf()),
	              "Create swap chain");

	// 필요하면 Alt+Enter 기본 전체 화면 전환을 비활성화할 수 있다.
	// dxgiFactory->MakeWindowAssociation(m_mainWindow, DXGI_MWA_NO_ALT_ENTER);

	// 렌더 타깃 뷰 생성
	CreateRenderTargetView();

	// 뷰포트 설정
	SetViewport();

	// 래스터라이저 상태 생성
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(rastDesc));

	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;

	ThrowIfFailed(m_device->CreateRasterizerState(&rastDesc, m_solidRasterizerState.GetAddressOf()),
	              "Create solid rasterizer state");

	rastDesc.FillMode = D3D11_FILL_WIREFRAME;

	ThrowIfFailed(m_device->CreateRasterizerState(&rastDesc, m_wireRasterizerState.GetAddressOf()),
	              "Create wireframe rasterizer state");

	// 깊이 버퍼와 DSV 생성
	D3D11Utils::CreateDepthBuffer(m_device.Get(), m_screenWidth, m_screenHeight, numQualityLevels,
	                              m_depthStencilView);

	// 깊이/스텐실 상태 생성
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// 현재 스텐실은 사용하지 않는다.
	depthStencilDesc.StencilEnable = false;

	ThrowIfFailed(
	    m_device->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.GetAddressOf()),
	    "Create depth-stencil state");

	return true;
}

bool AppBase::InitGUI()
{

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_imguiContextInitialized = true;
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.DisplaySize = ImVec2(float(m_screenWidth), float(m_screenHeight));
	// ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();
	// ImGui::StyleColorsLight();
	ImGui::GetIO().FontGlobalScale = 2.f;

	if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get()))
	{
		return false;
	}
	m_imguiDx11Initialized = true;

	if (!ImGui_ImplWin32_Init(m_mainWindow))
	{
		return false;
	}
	m_imguiWin32Initialized = true;

	return true;
}

void AppBase::SetViewport()
{
	static int previousGuiWidth = -1;
	static int previousScreenWidth = -1;
	static int previousScreenHeight = -1;

	if (previousGuiWidth != m_guiWidth || previousScreenWidth != m_screenWidth ||
	    previousScreenHeight != m_screenHeight)
	{
		previousGuiWidth = m_guiWidth;
		previousScreenWidth = m_screenWidth;
		previousScreenHeight = m_screenHeight;

		ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
		m_screenViewport.TopLeftX = float(m_guiWidth);
		m_screenViewport.TopLeftY = 0;
		m_screenViewport.Width = float(m_screenWidth - m_guiWidth);
		m_screenViewport.Height = float(m_screenHeight);
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;
	}

	// 후처리 패스가 뷰포트를 바꿀 수 있으므로 메인 렌더링 전에 다시 바인딩한다.
	m_context->RSSetViewports(1, &m_screenViewport);
}

void AppBase::CreateRenderTargetView()
{
	m_renderTargetView.Reset();
	m_postProcessInputSRV.Reset();
	m_postProcessInputTexture.Reset();

	ComPtr<ID3D11Texture2D> backBuffer;

	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
	              "Get swap-chain back buffer");

	ThrowIfFailed(m_device->CreateRenderTargetView(backBuffer.Get(), nullptr,
	                                               m_renderTargetView.GetAddressOf()),
	              "Create back-buffer render target view");

	D3D11_TEXTURE2D_DESC desc;
	backBuffer->GetDesc(&desc);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = 0;

	ThrowIfFailed(
	    m_device->CreateTexture2D(&desc, nullptr, m_postProcessInputTexture.GetAddressOf()),
	    "Create post-process input texture");

	ThrowIfFailed(m_device->CreateShaderResourceView(m_postProcessInputTexture.Get(), nullptr,
	                                                 m_postProcessInputSRV.GetAddressOf()),
	              "Create post-process input shader resource view");
}

} // namespace Ryudar
