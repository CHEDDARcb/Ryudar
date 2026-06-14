// Header include順序の参考:
// https://google.github.io/styleguide/cppguide.html#Names_and_Order_of_Includes

#include "Application/AppBase.h"
#include "Graphics/D3D11Exception.h"

#include <algorithm>
#include <cstdlib>

#include "Graphics/D3D11Utils.h"

// imgui_impl_win32.cppで定義されるメッセージ処理関数のForward Declaration。
// vcpkg版ImGuiではIDE上に警告が表示される場合がある。
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

namespace Ryudar
{

using namespace std;

// Win32 Callbackから現在のAppBase Instanceへメッセージを転送するためのPointer。
AppBase *g_appBase = nullptr;

// RegisterClassExへ登録するWin32メッセージCallback。
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
	// DestroyWindowはWndProcを同期呼び出しするため、AppBaseとImGuiの生存中に実行する。
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

	// UnregisterClass(wc.lpszClassName, wc.hInstance); は省略

	// ComPtrが参照カウントを管理するため、Direct3Dリソースは自動解放される。
	// https:learn.microsoft.com/en-us/cpp/cppcx/wrl/comptr-class?view=msvc-170
}

float AppBase::GetAspectRatio() const noexcept
{
	return float(m_screenWidth - m_guiWidth) / m_screenHeight;
}

int AppBase::Run()
{
	// Main Message Loop。
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
			// GUI幅の分だけレンダリング領域を縮小する場合に使用する。
			// ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
			// m_guiWidth = int(ImGui::GetWindowWidth());

			ImGui::End();
			ImGui::Render();

			Update(ImGui::GetIO().DeltaTime);

			Render();

			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			// ImGui描画後にBack BufferとFront Bufferを交換する。
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

	// Console Windowが描画Windowを隠さないよう前面へ移動する。
	SetForegroundWindow(m_mainWindow);

	return true;
}

void AppBase::OnMouseMove(WPARAM btnState, int mouseX, int mouseY)
{

	// Mouse座標をNDCへ変換する。
	// 画面座標は左上が(0, 0)、右下が(width - 1, height - 1)。
	// NDCは左下が(-1, -1)、右上が(1, 1)。
	float x = mouseX * 2.0f / m_screenWidth - 1.0f;
	float y = -mouseY * 2.0f / m_screenHeight + 1.0f;

	// Cursorが画面外へ出ても値が有効範囲内に収まるようClampする。
	x = std::clamp(x, -1.0f, 1.0f);
	y = std::clamp(y, -1.0f, 1.0f);

	if (m_useFirstPersonView)
	{
		m_camera.UpdateMouse(x, y);
	}
}

// hwnd: メッセージを受信したWindow。
// msg, wParam, lParam: Win32メッセージと追加データ。
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

				// ResizeBuffersの前にBack Buffer関連のBindingを解除する。
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
			// ESCキーでアプリケーションを終了する。
			DestroyWindow(hwnd);
		}
		break;
	case WM_KEYUP:
		// Keyboardの押下状態を解除する。
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

	// RegisterClassExでWindow Classを登録する。
	if (!RegisterClassEx(&wc))
	{
		cout << "RegisterClassEx() failed." << endl;
		return false;
	}

	// 希望するClient領域サイズから実際のWindowサイズを計算する。
	RECT wr = {0, 0, m_screenWidth, m_screenHeight};
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

	m_mainWindow = CreateWindow(wc.lpszClassName, L"Ryudar Engine", WS_OVERLAPPEDWINDOW,
	                            100,                // Window左上のx座標
	                            100,                // Window左上のy座標
	                            wr.right - wr.left, // Window全体の幅
	                            wr.bottom - wr.top, // Window全体の高さ
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
	// Hardware Driverを使用できない環境ではWARPでテストできる。
	const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	// const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_WARP;

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 対応するFeature Levelを高いVersionから順に要求する。
	const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_9_3};

	D3D_FEATURE_LEVEL featureLevel;

	// D3D11 DeviceとContextを生成する。
	ThrowIfFailed(D3D11CreateDevice(nullptr,           // Default Adapterを使用
	                                driverType,        // Hardware / WARP
	                                0,                 // Software Rasterizer Module Handle
	                                createDeviceFlags, // Debug Layerなどの生成Option
	                                featureLevels,     // 要求するFeature Level一覧
	                                ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
	                                m_device.GetAddressOf(), &featureLevel,
	                                m_context.GetAddressOf()),
	              "Create D3D11 device and context");

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		cout << "D3D Feature Level 11 unsupported." << endl;
		return false;
	}

	// 4x MSAAの対応可否を確認する。
	numQualityLevels = 0;
	ThrowIfFailed(
	    m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQualityLevels),
	    "Check multisample quality levels");

	if (numQualityLevels == 0)
	{
		cout << "4x MSAA not supported. Fallback to no MSAA." << endl;
	}

	// D3D DeviceからDXGI Device、Adapter、Factoryの順に上位Objectを取得する。
	ComPtr<IDXGIDevice> dxgiDevice;
	ComPtr<IDXGIAdapter> dxgiAdapter;
	ComPtr<IDXGIFactory> dxgiFactory;

	ThrowIfFailed(m_device.As(&dxgiDevice), "Query IDXGIDevice interface");

	ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()),
	              "Get DXGI adapter");

	ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory),
	                                     reinterpret_cast<void **>(dxgiFactory.GetAddressOf())),
	              "Get DXGI factory");

	// Swap Chainを生成する。
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferDesc.Width = m_screenWidth;
	sd.BufferDesc.Height = m_screenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Window Modeでは影響が小さいが、基本Refresh Rateを明示する。
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	// sd.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// Back Buffer二つでDouble Bufferingを行う。
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

	// 必要に応じてAlt+Enterによる標準Full Screen切り替えを無効化できる。
	// dxgiFactory->MakeWindowAssociation(m_mainWindow, DXGI_MWA_NO_ALT_ENTER);

	// Render Target Viewを生成する。
	CreateRenderTargetView();

	// Viewportを設定する。
	SetViewport();

	// Rasterizer Stateを生成する。
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

	// Depth BufferとDSVを生成する。
	D3D11Utils::CreateDepthBuffer(m_device.Get(), m_screenWidth, m_screenHeight, numQualityLevels,
	                              m_depthStencilView);

	// Depth/Stencil Stateを生成する。
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// 現在Stencilは使用しない。
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

	// Post Process PassがViewportを変更するため、Main Render前に再Bindする。
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
