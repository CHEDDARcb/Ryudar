#pragma once
// Win32ウィンドウ、D3D11基本リソース、ImGuiフレームループを管理する基底クラス。
// 派生クラスはシーン固有のGUI、更新、レンダリング処理のみを実装する。

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

	// レンダリング領域のアスペクト比を返す。
	// GUI領域を除いた画面を基準にカメラ投影で使用する。
	float GetAspectRatio() const noexcept;

	// Win32メッセージループを実行し、毎フレームGUI、Update、Renderを呼び出す。
	int Run();

	// Win32ウィンドウ、D3D11 DeviceとSwap Chain、ImGuiなどの実行環境を初期化する。
	virtual bool Initialize();

	// 派生クラスが毎フレームImGuiコントロールを構築する。
	virtual void UpdateGUI() = 0;

	// 派生クラスが入力、シーン状態、Constant Bufferを更新する。
	virtual void Update(float dt) = 0;

	// 派生クラスがD3Dレンダリングコマンドを発行する。
	virtual void Render() = 0;

	// AppBaseがウィンドウサイズ依存の共通D3Dリソースを更新した後に呼び出す。
	// 派生クラスは画面サイズ依存のリソースをここで再生成する。
	virtual void OnResize() {}

	// Win32メッセージを処理する。
	// ImGuiへの入力転送、キーとマウス状態、サイズ変更時のリソース再生成を担当する。
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// 必要な派生クラスのみがオーバーライドする入力フック。
	virtual void OnMouseDown(WPARAM btnState, int x, int y) {};
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {};
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
	// Win32ウィンドウクラスを登録し、メインウィンドウを生成する。
	bool InitMainWindow();

	// D3D11 Device、Swap Chain、Render/Depth Target、Rasterizer Stateを生成する。
	bool InitDirect3D();

	// Win32/D3D11バックエンドとImGui Contextを初期化する。
	bool InitGUI();

	// 現在のクライアントサイズに合わせてViewportとカメラのアスペクト比を更新する。
	void SetViewport();

	// Swap ChainのBack Bufferを基にRTVとPost Process入力Texture/SRVを生成する。
	// ウィンドウサイズ変更時は既存Viewを解放してから再度呼び出す。
	void CreateRenderTargetView();

protected:
	// ウィンドウ状態
	int m_screenWidth; // クライアント描画領域の幅
	int m_screenHeight;
	int m_guiWidth = 0;
	HWND m_mainWindow;

	// D3D主要オブジェクト
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;
	UINT numQualityLevels = 0;

	// Render Target
	// Swap ChainのBack Bufferへ描画するための画面出力先。
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	// Post Process入力リソース
	// MSAA Back BufferをPost Process入力に使うためResolveする一時Texture。
	ComPtr<ID3D11Texture2D> m_postProcessInputTexture;

	// Post Process Shaderがm_postProcessInputTextureを読み込むためのView。
	ComPtr<ID3D11ShaderResourceView> m_postProcessInputSRV;

	// Rasterizer State
	ComPtr<ID3D11RasterizerState> m_solidRasterizerState;
	ComPtr<ID3D11RasterizerState> m_wireRasterizerState;

	// Viewportとカメラ
	D3D11_VIEWPORT m_screenViewport;

	Camera m_camera;

	// 実行オプションと入力状態
	bool m_drawAsWire = false;
	bool m_usePostProcessing = false;
	bool m_useFirstPersonView = false;

	// 初期化途中の失敗時に、生成済みImGuiリソースのみを安全に破棄するための状態。
	bool m_imguiContextInitialized = false;
	bool m_imguiDx11Initialized = false;
	bool m_imguiWin32Initialized = false;

	// Win32キーコードごとの押下状態。
	bool m_keyPressed[256] = {
	    false,
	};
};
} // namespace Ryudar
